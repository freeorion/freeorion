import freeOrionAIInterface as fo  # pylint: disable=import-error
from traceback import format_exc
from tools import UserString
from functools import wraps


def print_error(msg, location=None, trace=True):
    """
    Sends error to host chat and print its to log.
    :param msg: message text
    :param location: text that describes error location
    :param trace: flag if print traceback
    """
    recipient_id = [x for x in fo.allPlayerIDs() if fo.playerIsHost(x)][0]
    if location:
        message = '%s in "%s": "%s"' % (UserString('AI_ERROR_MSG', 'AI_Error: AI script error'), location, msg)
    else:
        message = '%s: "%s"' % (fo.userString('AI_ERROR_MSG'), msg)
    fo.sendChatMessage(recipient_id, message)
    print "\n%s\n" % message
    if trace:
        print format_exc()



def chat_on_error(function):
    @wraps(function)
    def wrapper(*args, **kw):
        try:
            return function(*args, **kw)
        except Exception as e:
            print_error(e, location=function.__name__, trace=False)
            raise
    return wrapper
