import sys
import logging
import freeorion_logger  # pylint: disable=import-error


class DbgLogger(object):
    """A stream-like object to redirect stdout to C++ process."""
    @staticmethod
    def write(msg):
        freeorion_logger.debug(msg)


class ErrLogger(object):
    """A stream-like object to redirect stderr to C++ process."""
    @staticmethod
    def write(msg):
        freeorion_logger.error(msg)


class StreamlikeLogger(object):
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


class SingleLevelFilter(logging.Filter):
    """This filter selects for only one log level"""
    def __init__(self, _level):
        super(SingleLevelFilter, self).__init__()
        self.level = _level

    def filter(self, record):
        return record.levelno == self.level


def create_narrow_handler(level):
    """Create a handler for logger that forwards a single level of log
    to the appropriate stream in the C++ app"""
    h = logging.StreamHandler(StreamlikeLogger(level))
    h.addFilter(SingleLevelFilter(level))
    h.setLevel(level)
    return h


def redirect_logging_to_freeorion_logger(name):
    """Redirect stdout, stderr and the logging.logger to hosting process' freeorion_logger"""

    sys.stdout = DbgLogger()
    sys.stderr = ErrLogger()
    print 'Python stdout and stderr redirected'

    logger = logging.getLogger(name)
    logger.addHandler(create_narrow_handler(logging.DEBUG))
    logger.addHandler(create_narrow_handler(logging.INFO))
    logger.addHandler(create_narrow_handler(logging.WARNING))
    logger.addHandler(create_narrow_handler(logging.ERROR))
    logger.addHandler(create_narrow_handler(logging.CRITICAL))
    logger.addHandler(create_narrow_handler(logging.NOTSET))

    logger.setLevel(logging.DEBUG)
    logger.info("logger('{}') is redirected to ai process.".format(name))
