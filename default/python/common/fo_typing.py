"""
This module contains FreeOrion types.

Since a lot of object are operated by their ID
it is hard to distinguish one int from the another.
"""

from collections.abc import Iterable, Iterator
from typing import Generic, NewType, TypeVar

ObjectId = NewType("ObjectId", int)
PlanetId = NewType("PlanetId", ObjectId)
FleetId = NewType("FleetId", ObjectId)
SystemId = NewType("SystemId", ObjectId)
ShipId = NewType("ShipId", ObjectId)
EmpireId = NewType("EmpireId", ObjectId)
BuildingId = NewType("BuildingId", ObjectId)
BuildingName = NewType("BuildingName", str)
SpeciesName = NewType("SpeciesName", str)
SpecialName = NewType("SpecialName", str)
PartName = NewType("PartName", str)
Turn = NewType("Turn", int)
PlayerId = NewType("PlayerId", int)

KT = TypeVar("KT")
VT = TypeVar("VT")


class Set(Generic[VT]):
    def __contains__(self, number: VT) -> bool:
        ...

    def __iter__(self) -> Iterator[VT]:
        ...

    def __len__(self) -> int:
        ...


class Vec(Generic[VT]):
    def __contains__(self, val: VT) -> bool:
        ...

    def __delitem__(self, val: VT) -> None:
        ...

    def __getitem__(self, ind: int) -> KT:
        ...

    def __iter__(self) -> Iterator[VT]:
        ...

    def __len__(self) -> int:
        ...

    def __setitem__(self, ind: int, val: VT) -> None:
        ...

    def append(self, val: VT) -> None:
        ...

    def extend(self, iterable: Iterable[VT]) -> None:
        ...


class Item(Generic[KT, VT]):
    def key(self) -> KT:
        ...

    def data(self) -> VT:
        ...


class Map(Generic[KT, VT]):
    def __contains__(self, key: KT) -> bool:
        ...

    def __delitem__(self, key: KT) -> None:
        ...

    def __getitem__(self, key: KT) -> VT:
        ...

    def __iter__(self) -> Iterator[Item[KT, VT]]:
        ...

    def __len__(self) -> int:
        ...

    def __setitem__(self, key: KT, value: VT) -> None:
        ...
