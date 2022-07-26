"""
This module contains FreeOrion types.

Since a lot of object are operated by their ID
it is hard to distinguish one int from the another.
"""

from typing import NewType

ObjectId = NewType("TargetId", int)
PlanetId = NewType("PlanetId", ObjectId)
FleetId = NewType("FleetId", ObjectId)
SystemId = NewType("SystemId", ObjectId)
ShipId = NewType("ShipId", ObjectId)
EmpireId = NewType("EmpireId", ObjectId)
BuildingId = NewType("BuildingId", ObjectId)
BuildingName = NewType("BuildingName", str)
SpeciesName = NewType("SpeciesName", str)
SpecialName = NewType("SpecialName", str)
Turn = NewType("Turn", int)
