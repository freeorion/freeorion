import freeOrionAIInterface as fo  # pylint: disable=import-error
from functools import wraps
from traceback import format_exc


def dict_from_map(thismap):
    return {el.key(): el.data() for el in thismap}


def UserString(label, default=None):  # this name left with C naming style for compatibility with translation assistance procedures  #pylint: disable=invalid-name
    """ A translation assistance tool is intended to search for this method to identify translatable strings."""
    table_string = fo.userString(label)

    if "ERROR: " + label in table_string:  # implement test for string lookup not found error
        return default or table_string
    else:
        return table_string


def tech_is_complete(tech):
    """
    Return if tech is complete.
    """
    return fo.getEmpire().getTechStatus(tech) == fo.techStatus.complete


def chat_on_error(function):
    @wraps(function)
    def wrapper(*args, **kw):
        try:
            return function(*args, **kw)
        except Exception as e:
            print_error(e, location=function.__name__, trace=False)
            raise
    return wrapper


def print_error(msg, location=None, trace=True):
    """
    Sends error to host chat and print its to log.
    :param msg: message text
    :param location: text that describes error location
    :param trace: flag if print traceback
    """
    print "possible recipients host status: %s" % [(x, fo.playerIsHost(x)) for x in fo.allPlayerIDs()]
    recipient_id = [x for x in fo.allPlayerIDs() if fo.playerIsHost(x)][0]
    if location:
        message = '%s in "%s": "%s"' % (UserString('AI_ERROR_MSG', 'AI_Error: AI script error'), location, msg)
    else:
        message = '%s: "%s"' % (fo.userString('AI_ERROR_MSG'), msg)
    fo.sendChatMessage(recipient_id, message)
    print "\n%s\n" % message
    if trace:
        print format_exc()
