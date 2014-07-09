import freeOrionAIInterface as fo # pylint: disable=import-error
from functools import wraps


def chat_on_error(callable):
    @wraps(callable)
    def wrapper(*args, **kw):
        try:
            return callable(*args, **kw)
        except Exception as e:
            recipient_id = [x for x in fo.allPlayerIDs() if fo.playerIsHost(x)][0]
            message = '%s in "%s" : "%s"' % (fo.userString('AI_ERROR_MSG'), callable.__name__, e)
            fo.sendChatMessage(recipient_id, message)
            raise
    return wrapper
