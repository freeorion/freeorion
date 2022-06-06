from abc import ABC
from enum import Enum
from typing import Callable, TypeVar

from dump_interface._dump_value import DumpDict, DumpInt, DumpTuple
from dump_interface._serizlizer import FloatSerializer, IntSerializer, StrSerializer

D = TypeVar("D")


class Serializer(ABC):
    def __init__(
        self,
        *,
        serializer: Callable[[D], str],
        deserializer: Callable[[str], D],
    ):
        self._deserializer = deserializer
        self._serializer = serializer

    def deserialize(self, value: str) -> D:
        return self._deserializer(value)

    def serialize(self, value: D) -> str:
        return self._serializer(value)


class DumpKey(Enum):
    EmpireColors = DumpTuple("EmpireColors", [IntSerializer(), IntSerializer(), IntSerializer(), IntSerializer()])
    EmpireID = DumpDict("EmpireID", {"empire_id": IntSerializer(), "name": StrSerializer(), "turn": IntSerializer()})
    CapitalID = DumpDict(
        "CapitalID",
        {
            "capital_planet_id": StrSerializer(),
            "capital_planet_name": IntSerializer(),
            "capital_species": StrSerializer(),
        },
    )
    Output = DumpDict("CurrentOutput", {"turn": IntSerializer(), "RP": FloatSerializer(), "PP": FloatSerializer()})
    SHIP_CONT = DumpInt("ShipCount")

    @classmethod
    def get_by_value_name(cls, name: str):
        for item in cls:
            if item.value.name == name:
                return item
        else:
            raise ValueError(f"Cannot find value with {name}")


LOG_PREFIX = "##"
