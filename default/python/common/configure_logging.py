import sys
import logging
import freeorion_logger  # pylint: disable=import-error


class _stdoutLikeStream(object):
    """A stream-like object to redirect stdout to C++ process"""
    @staticmethod
    def write(msg):
        freeorion_logger.debug(msg)


class _stderrLikeStream(object):
    """A stream-like object to redirect stdout to C++ process"""
    @staticmethod
    def write(msg):
        freeorion_logger.error(msg)


class _streamlikeLogger(object):
    """A stream-like object to redirect stdout to C++ process for logger"""
    def __init__(self, level):
        self.logger = {
            logging.DEBUG: freeorion_logger.debug,
            logging.INFO: freeorion_logger.info,
            logging.WARNING: freeorion_logger.warn,
            logging.ERROR: freeorion_logger.error,
            logging.CRITICAL: freeorion_logger.fatal,
            logging.NOTSET: freeorion_logger.debug
        }[level]

    def write(self, msg):
        self.logger(msg)


class _SingleLevelFilter(logging.Filter):
    """This filter selects for only one log level"""
    def __init__(self, _level):
        super(_SingleLevelFilter, self).__init__()
        self.level = _level

    def filter(self, record):
        return record.levelno == self.level


_error_formatter = logging.Formatter('Module %(module)s, File %(filename)s,  Function %(funcName)s():%(lineno)d  - %(message)s')


def _create_narrow_handler(level):
    """Create a handler for logger that forwards a single level of log
    to the appropriate stream in the C++ app"""
    h = logging.StreamHandler(_streamlikeLogger(level))
    h.addFilter(_SingleLevelFilter(level))
    h.setLevel(level)
    if level > logging.INFO:
        h.setFormatter(_error_formatter)
    return h


def _redirect_logging_to_freeorion_logger():
    """Redirect stdout, stderr and the logging.logger to hosting process' freeorion_logger"""

    if not hasattr(_redirect_logging_to_freeorion_logger, "only_redirect_once"):
        sys.stdout = _stdoutLikeStream()
        sys.stderr = _stderrLikeStream()
        print 'Python stdout and stderr are redirected to ai process.'

        logger = logging.getLogger()
        logger.addHandler(_create_narrow_handler(logging.DEBUG))
        logger.addHandler(_create_narrow_handler(logging.INFO))
        logger.addHandler(_create_narrow_handler(logging.WARNING))
        logger.addHandler(_create_narrow_handler(logging.ERROR))
        logger.addHandler(_create_narrow_handler(logging.CRITICAL))
        logger.addHandler(_create_narrow_handler(logging.NOTSET))

        logger.setLevel(logging.DEBUG)
        logger.info("Root logger is redirected to ai process.")

        _redirect_logging_to_freeorion_logger.only_redirect_once = True

_redirect_logging_to_freeorion_logger()
