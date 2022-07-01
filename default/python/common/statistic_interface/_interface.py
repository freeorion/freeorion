from abc import ABC
from enum import Enum
from typing import Callable, Dict, TypeVar, Union

from common.statistic_interface._dump_value import DumpDict, DumpInt, DumpMultiple
from common.statistic_interface._serizlizer import to_float, to_int, to_str

_ScalarValue = Union[str, int, float]

StatValue = TypeVar("StatValue", Dict[str, _ScalarValue], _ScalarValue)


class Serializer(ABC):
    def __init__(
        self,
        *,
        serializer: Callable[[StatValue], str],
        deserializer: Callable[[str], StatValue],
    ):
        self._deserializer = deserializer
        self._serializer = serializer

    def deserialize(self, value: str) -> StatValue:
        return self._deserializer(value)

    def serialize(self, value: StatValue) -> str:
        return self._serializer(value)


class StatKey(Enum):
    PolicyAdoption = DumpMultiple("PolicyAdoption", to_str)
    PolicyDeAdoption = DumpMultiple("PolicyDeAdoption", to_str)
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
            "capital_planet_id": to_int,
            "capital_planet_name": to_str,
            "capital_species": to_str,
        },
    )
    Output = DumpDict("CurrentOutput", {"turn": to_int, "RP": to_float, "PP": to_float})
    SHIP_CONT = DumpInt("ShipCount")

    def is_multi(self):
        return self.value.multi

    @classmethod
    def get_by_value_name(cls, name: str):
        for item in cls:
            if item.value.name == name:
                return item
        else:
            raise ValueError(f"Cannot find value with {name}")


LOG_PREFIX = "##"
