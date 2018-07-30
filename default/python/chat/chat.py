from logging import info

from common.configure_logging import redirect_logging_to_freeorion_logger

# Logging is redirected before other imports so that import errors appear in log files.
redirect_logging_to_freeorion_logger()


class ChatProvider:
    def __init__(self):
        info("Chat initialized")

    def load_history(self):
        info("Loading history...")
        # c = fo.GGColor(255, 128, 128, 255)
        # e = (123456789012, "P1", "Test1", c)
        # return [e]
        return []

    def put_history_entity(self, timestamp, player_name, text, text_color):
        info("Chat %s: %s %s" % (player_name, text, text_color))
        return True
