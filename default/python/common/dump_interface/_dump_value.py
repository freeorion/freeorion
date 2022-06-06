from abc import ABC
from typing import Dict, Sequence

from dump_interface._serizlizer import (
    DictSerializer,
    FloatSerializer,
    IntSerializer,
    Serializable,
    Serializer,
    StrSerializer,
    TupleSerializer,
)


class Dump(ABC):
    def __init__(self, name: str, serializer: Serializer):
        self.name = name
        self._serializer = serializer

    def deserialize(self, value: str) -> Serializable:
        return self._serializer.serialize(value)

    def serialize(self, value: Serializable) -> str:
        return self._serializer.deserialize(value)


class DumpInt(Dump):
    def __init__(self, name: str):
        super().__init__(name, IntSerializer())


class DumpFloat(Dump):
    def __init__(self, name: str):
        super().__init__(name, FloatSerializer())


class DumpStr(Dump):
    def __init__(self, name: str):
        super().__init__(name, StrSerializer())


class DumpTuple(Dump):
    def __init__(self, name: str, items: Sequence[Serializer]):
        super().__init__(name, TupleSerializer(items))


class DumpDict(Dump):
    def __init__(self, name: str, items: Dict[str, Serializer]):
        super().__init__(name, DictSerializer(items))
