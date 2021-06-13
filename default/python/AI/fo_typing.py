"""
This module contains FreeOrion types.

Since a lot of object are operated by their ID
it is hard to distinguish one int from the another.
"""

from typing import NewType

TargetId = NewType("TargetId", int)
PlanetId = NewType("PlanetId", TargetId)
FleetId = NewType("FleetId", TargetId)
SystemId = NewType("SystemId", TargetId)
ShipId = NewType("ShipId", TargetId)
EmpireId = NewType("EmpireId", TargetId)
BuildingId = NewType("BuildingId", TargetId)
