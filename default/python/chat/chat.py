from logging import info

from common.configure_logging import redirect_logging_to_freeorion_logger

# Logging is redirected before other imports so that import errors appear in log files.
redirect_logging_to_freeorion_logger()


class ChatHistoryProvider:
    def __init__(self):
        """
        Initializes ChatProvider. Doesn't accept arguments.
        """
        info("Chat initialized")

    def load_history(self):
        """
        Loads chat history from external sources.

        Returns list of tuples:
            (unixtime timestamp, player name, text, text color of type freeorion.GGColor)
        """
        info("Loading history...")
        # c = fo.GGColor(255, 128, 128, 255)
        # e = (123456789012, "P1", "Test1", c)
        # return [e]
        return []

    def put_history_entity(self, timestamp, player_name, text, text_color):
        """
        Put chat into external storage.

        Return True if successfully stored. False otherwise.

        Arguments:
        timestamp -- unixtime, number of seconds from 1970-01-01 00:00:00 UTC
        player_name -- player name
        text -- chat text
        text_color -- freeorion.GGColor
        """
        info("Chat %s: %s %s" % (player_name, text, text_color))
        return True
