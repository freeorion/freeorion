from freeorion_tools.chat_handler.base_chat_handler import ChatHandlerBase


class NormalChatHandler(ChatHandlerBase):
    """
    Class for normal message handling.

    It just does nothing and allows next handler to handle that message.
    """

    def process_message(self, sender_id: int, message: str) -> bool:
        return False
