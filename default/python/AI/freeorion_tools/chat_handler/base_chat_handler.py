from abc import ABCMeta, abstractmethod


class ChatHandlerBase(metaclass=ABCMeta):
    """
    Base class for chat handling.
    """

    @abstractmethod
    def process_message(self, sender_id: int, message: str) -> bool:
        """
        Process chat message and return if message was processed, and should not be send to another handlers.
        """
