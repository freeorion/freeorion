from collections import Callable
from typing import Iterator

from analytics.jar import CallInfo, Jar
from analytics.report.base_handler import base_reported


class Reporter:
    def __init__(self):
        self.handlers = {}
        self.base_handler: Callable[[CallInfo], str] = base_reported

    def _make_report_for_turn(self, turn: int, jar: Jar) -> Iterator[str]:
        function = self.handlers.get(jar.name, self.base_handler)
        for info in jar.get_turn(turn):
            yield function(info)

    def report(self, turn: int, jar: Jar) -> str:
        return '\n'.join(self._make_report_for_turn(turn, jar))
