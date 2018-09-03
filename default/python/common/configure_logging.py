from __future__ import print_function

"""configure_logging redirects print and the logger to use the freeorion server.

Output redirected to the freeorion server prints in the appropriate log:
freeorion.log freeoriond.log or AI_<N>.log.

logging messages of levels warning and above are decorated with module name, file name, function name and line number.

Usage:

from common.configure_logging import redirect_logging_to_freeorion_logger
redirect_logging_to_freeorion_logger(log_level)

Then use python logging or print and have it re-directed appropriately.

Notes on using the standard python logger: The python standard library is
composed of two basic parts: loggers and handlers.  Loggers generate the log and
associate level, timing, filename, function, line number and other information
with the log.  Handlers convert the logging stream to file, console, email etc.

* Using the root logger directly is the simplest way to log:
logging.debug(msg)
logging.info(msg)
logging.warn(msg)
logging.error(msg)
logging.fatal(msg)

* The log level of the root logger can be changed with:

logging.getLogger().setLevel(logging.WARN)

* The log level of the root logger can be checked with:

logging.getLogger().isEnableFor(logging.WARN)


* One strength of the python standard logging library is the ability to create
  arbitrary hierarchical loggers.  Loggers are created globally when getLogger()
  is called with any string.  The hierarchical levels are dot separated.  The
  root logger is the empty string.  The following creates a logger:

logging.getLogger("toplevel.2ndlevel")

* Loggers can be filtered by log level hierarchically.  The following turns off
  logging below warning level for all loggers and then turns debug logging on
  for "name.subname" and its children.  This means that the log stream only
  contains logging for that one component.

logging.getLogger().setLevel(logging.ERROR)
logging.getLogger(name.subname).setLevel(logging.DEBUG)

* Logger formats string using the old format style.  Importantly, it only
  formats the string if that level of debugging is enabled.  This means that if
  the formatting is left to the logging sub-system, then there is no overhead to
  format the string if that log level is not enabled.

logging("A formatted %s contained the number nine %d", "string", 9)


* The following log levels have sinks in the game engine that write to the
  logfiles and include their level.  They should be used as follows:

FATAL   - used for errors that will crash the python engine.
ERROR   - used of "major" unrecoverable errors which will affect game play.
          For example being unable to place all the homeworlds.
WARNING - used for "minor", recoverable errors.
          For example not getting a ship object from a (supposedly valid) ship id.
INFO    - used to report game state and progress.
          For example showing a table about colonization possibilities or threat assessment.
DEBUG   - used for low-level implementation or calculation details.
          For example the calculations when rating a fleet.

For more information about the python standard library logger see
https://docs.python.org/2/howto/logging.html
"""
import sys
import logging
import os
import traceback

try:
    import freeorion_logger  # FreeOrion logger interface pylint: disable=import-error
except ImportError as e:

    # Create an alternative logger for use in testing when the server is unavailable
    class _FreeOrionLoggerForTest(object):
        """A stub freeorion_logger for testing"""
        @staticmethod
        def debug(msg, *args):
            print(msg)

        @staticmethod
        def info(msg, *args):
            print(msg)

        @staticmethod
        def warn(msg, *args):
            print(msg)

        @staticmethod
        def error(msg, *args):
            print(msg, file=sys.stderr)

        @staticmethod
        def fatal(msg, *args):
            print(msg, file=sys.stderr)

    freeorion_logger = _FreeOrionLoggerForTest()


class _stdXLikeStream(object):
    """A stream-like object to redirect stdout or stderr to the C++ process."""
    def __init__(self, level):
        self.logger = {
            logging.DEBUG: freeorion_logger.debug,
            logging.INFO: freeorion_logger.info,
            logging.WARNING: freeorion_logger.warn,
            logging.ERROR: freeorion_logger.error,
            logging.FATAL: freeorion_logger.fatal,
            logging.NOTSET: freeorion_logger.debug
        }[level]

    def write(self, msg):
        # Grab the caller's call frame info
        if hasattr(sys, '_getframe'):
            frame = sys._getframe(1)
            try:
                line_number = frame.f_lineno
                function_name = frame.f_code.co_name
                filename = frame.f_code.co_filename
            except:
                (filename, line_number, function_name) = ("", "", "")
            finally:
                # Explicitly del references to the caller's frame to avoid persistent reference cycles
                del frame
        else:
            (filename, line_number, function_name) = ("", "", "")

        try:
            self.logger(msg, "python", str(os.path.split(filename)[1]), str(function_name), str(line_number))

        finally:
            # Explicitly del references to the caller's frame to avoid persistent reference cycles
            del filename
            del line_number
            del function_name


class _LoggerHandler(logging.Handler):
    """A handler to send logs to the C++ process."""
    def __init__(self, level):
        super(_LoggerHandler, self).__init__(level)
        self.logger = {
            logging.DEBUG: freeorion_logger.debug,
            logging.INFO: freeorion_logger.info,
            logging.WARNING: freeorion_logger.warn,
            logging.ERROR: freeorion_logger.error,
            logging.FATAL: freeorion_logger.fatal,
            logging.NOTSET: freeorion_logger.debug
        }[level]

    def emit(self, record):
        record.message = record.getMessage()
        msg = str(record.message) + "\n"
        if record.exc_info:
            if not isinstance(record.exc_info, tuple):
                # record.exc_info is not a local variable and will be garbage collected when record
                # is garbage collected by the logging library
                record.exc_info = sys.exc_info()
            traceback_msg = "".join(traceback.format_exception(*record.exc_info))
            msg += traceback_msg
        self.logger(msg, "python", str(record.filename),
                    str(record.funcName), str(record.lineno))


class _SingleLevelFilter(logging.Filter):
    """This filter selects for only one log level."""
    def __init__(self, _level):
        super(_SingleLevelFilter, self).__init__()
        self.level = _level

    def filter(self, record):
        return record.levelno == self.level


def _create_narrow_handler(level):
    """Create a handler for logger that forwards a single level of log
    to the appropriate stream in the C++ app."""
    h = _LoggerHandler(level)
    h.addFilter(_SingleLevelFilter(level))
    h.setLevel(level)
    return h


def _unhandled_exception_hook(*exc_info):
    traceback_msg = "Uncaught exception: {0}".format(
        "".join(traceback.format_exception(*exc_info)))
    logging.getLogger().error(traceback_msg)


def redirect_logging_to_freeorion_logger(initial_log_level=logging.DEBUG):
    """Redirect stdout, stderr and the logging.logger to hosting process' freeorion_logger."""

    if not hasattr(redirect_logging_to_freeorion_logger, "only_redirect_once"):
        sys.stdout = _stdXLikeStream(logging.DEBUG)
        sys.stderr = _stdXLikeStream(logging.ERROR)
        print('Python stdout and stderr are redirected to ai process.')

        logger = logging.getLogger()
        logger.addHandler(_create_narrow_handler(logging.DEBUG))
        logger.addHandler(_create_narrow_handler(logging.INFO))
        logger.addHandler(_create_narrow_handler(logging.WARNING))
        logger.addHandler(_create_narrow_handler(logging.ERROR))
        logger.addHandler(_create_narrow_handler(logging.FATAL))
        logger.addHandler(_create_narrow_handler(logging.NOTSET))

        # Replace the system unhandled exception handler
        sys.excepthook = _unhandled_exception_hook

        logger.setLevel(initial_log_level)
        logger.info("The python logger is initialized with a log level of %s" %
                    logging.getLevelName(logger.getEffectiveLevel()))

        redirect_logging_to_freeorion_logger.only_redirect_once = True
