import sys
import logging
import freeorion_logger


class dbgLogger(object):
    """A stream-like object to redirect stdout to C++ process"""
    @staticmethod
    def write(msg):
        freeorion_logger.log(msg)


class errLogger(object):
    """A stream-like object to redirect stderr to C++ process"""
    @staticmethod
    def write(msg):
        freeorion_logger.error(msg)


def redirect_logging_to_freeorion_logger():
    """Redirect stdout, stderr and the logging.logger to hosting process' freeorion_logger"""

    sys.stdout = dbgLogger()
    sys.stderr = errLogger()
    print 'Python stdout and stderr redirected'

