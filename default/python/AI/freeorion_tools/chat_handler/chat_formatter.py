from abc import abstractmethod
from typing import Any


class ChatFormatter:
    """
    Helper class for a message formatting.
    """

    @abstractmethod
    def _wrap_to_tag(self, tag: (str, str), message: Any) -> str:
        open_, close = tag
        return f"<{open_}>{message}</{close}>"

    @abstractmethod
    def _get_tag_rgb(self, color: (int, int, int, int)) -> (str, str):
        return "rgba %s %s %s %s" % color, "rgba"

    @abstractmethod
    def _get_underline_tag(self) -> (str, str):
        return "u", "u"

    def colored(self, color: (int, int, int, int), message: Any):
        return self._wrap_to_tag(self._get_tag_rgb(color), message)

    def underline(self, message: Any) -> str:
        return self._wrap_to_tag(self._get_underline_tag(), message)

    def white(self, message: Any) -> str:
        return self.colored((255, 255, 255, 255), message)

    def red(self, message: Any) -> str:
        return self.colored((255, 0, 0, 255), message)

    def blue(self, message: Any) -> str:
        return self.colored((0, 255, 255, 255), message)

    def yellow(self, message: Any) -> str:
        return self.colored((255, 255, 0, 255), message)
