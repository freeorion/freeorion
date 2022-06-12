from typing import Callable

from common.statistic_interface import LOG_PREFIX, StatKey, StatValue


class Stats:
    def __init__(self, writer: Callable[[str], None]):
        self._writer = writer

    def write(self, key: StatKey, value: StatValue):
        serialized = key.value.serialize(value)
        self._writer(f"{LOG_PREFIX}{key.value.name}:{serialized}")
