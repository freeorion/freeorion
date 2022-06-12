from typing import Callable, Union

from common.dump_interface import LOG_PREFIX, DumpKey

DumpValue = Union[str, int]


class DumpException(Exception):
    pass


class Dumper:
    def __init__(self, writer: Callable[[str], None]):
        self._writer = writer

    def dump(self, key: DumpKey, value: DumpValue):
        serialized = key.value.serialize(value)
        self._writer(f"{LOG_PREFIX}{key.value.name}:{serialized}")
