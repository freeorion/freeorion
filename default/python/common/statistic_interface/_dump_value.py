from abc import ABC
from typing import Dict

from common.statistic_interface._serizlizer import (
    DictSerializer,
    Serializable,
    Serializer,
    to_float,
    to_int,
    to_str,
)


class Dump(ABC):
    def __init__(self, name: str, serializer: Serializer, multi=False):
        self.name = name
        self.multi = multi
        self._serializer = serializer

    def deserialize(self, value: str) -> Serializable:
        return self._serializer.deserialize(value)

    def serialize(self, value: Serializable) -> str:
        return self._serializer.serialize(value)


class DumpInt(Dump):
    def __init__(self, name: str):
        super().__init__(name, to_int)


class DumpFloat(Dump):
    def __init__(self, name: str):
        super().__init__(name, to_float)


class DumpStr(Dump):
    def __init__(self, name: str):
        super().__init__(name, to_str)


class DumpDict(Dump):
    def __init__(self, name: str, items: Dict[str, Serializer]):
        super().__init__(name, DictSerializer(items))


class DumpMultiple(Dump):
    def __init__(self, name: str, serializer: Serializer):
        super().__init__(name, serializer, multi=True)
