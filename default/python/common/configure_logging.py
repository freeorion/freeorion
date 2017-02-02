import sys
import freeorion_logger


class DbgLogger(object):
    """A stream-like object to redirect stdout to C++ process"""
    @staticmethod
    def write(msg):
        freeorion_logger.log(msg)


class ErrLogger(object):
    """A stream-like object to redirect stderr to C++ process"""
    @staticmethod
    def write(msg):
        freeorion_logger.error(msg)


def redirect_logging_to_freeorion_logger():
    """Redirect stdout, stderr and the logging.logger to hosting process' freeorion_logger"""

    sys.stdout = DbgLogger()
    sys.stderr = ErrLogger()
    print 'Python stdout and stderr redirected'


