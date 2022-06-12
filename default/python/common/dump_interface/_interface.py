from abc import ABC
from enum import Enum
from typing import Callable, TypeVar

from common.dump_interface._dump_value import DumpDict, DumpInt
from common.dump_interface._serizlizer import to_float, to_int, to_str

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
    EmpireColors = DumpDict(
        "EmpireColors",
        {
            "R": to_int,
            "G": to_int,
            "B": to_int,
            "A": to_int,
        },
    )
    EmpireID = DumpDict("EmpireID", {"empire_id": to_int, "name": to_str, "turn": to_int})
    CapitalID = DumpDict(
        "CapitalID",
        {
            "capital_planet_id": to_str,
            "capital_planet_name": to_int,
            "capital_species": to_str,
        },
    )
    Output = DumpDict("CurrentOutput", {"turn": to_int, "RP": to_float, "PP": to_float})
    SHIP_CONT = DumpInt("ShipCount")

    @classmethod
    def get_by_value_name(cls, name: str):
        for item in cls:
            if item.value.name == name:
                return item
        else:
            raise ValueError(f"Cannot find value with {name}")


LOG_PREFIX = "##"
