"""
This module contains FreeOrion types.

Since a lot of object are operated by their ID
it is hard to distinguish one int from the another.
"""

from typing import NewType

_UniverseObjectId = NewType("UniverseObjectId", int)
ObjectId = NewType("ObjectId", _UniverseObjectId)
PlanetId = NewType("PlanetId", _UniverseObjectId)
FleetId = NewType("FleetId", _UniverseObjectId)
SystemId = NewType("SystemId", _UniverseObjectId)
ShipId = NewType("ShipId", _UniverseObjectId)
BuildingId = NewType("BuildingId", _UniverseObjectId)


EmpireId = NewType("EmpireId", int)
PlayerId = NewType("PlayerId", int)

BuildingName = NewType("BuildingName", str)
SpeciesName = NewType("SpeciesName", str)
SpecialName = NewType("SpecialName", str)
PartName = NewType("PartName", str)

Turn = NewType("Turn", int)

AttackDamage = NewType("AttackDamage", float)
AttackCount = NewType("AttackCount", int)
