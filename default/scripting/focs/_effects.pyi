from __future__ import annotations

from typing import TypeVar

# Default value for int keyword arguments, use it when default value is not clear
from focs._types import (
    _N,
    _Condition,
    _EmpireId,
    _Int,
    _ShipPartClass,
)

ShortRangeClass = _ShipPartClass()
FighterBayClass = _ShipPartClass()
FighterHangarClass = _ShipPartClass()
ShieldClass = _ShipPartClass()
ArmourClass = _ShipPartClass()
TroopsClass = _ShipPartClass()
DetectionClass = _ShipPartClass()
StealthClass = _ShipPartClass()
FuelClass = _ShipPartClass()
ColonyClass = _ShipPartClass()
SpeedClass = _ShipPartClass()
GeneralClass = _ShipPartClass()
BombardClass = _ShipPartClass()
IndustryClass = _ShipPartClass()
ResearchClass = _ShipPartClass()
InfluenceClass = _ShipPartClass()
ProductionLocationClass = _ShipPartClass()

_O = TypeVar("_O", str, int, float)

def StatisticElse(type_: type[_N], *, condition: _Condition) -> _N: ...
def ShipDesignsLost(*, empire: _EmpireId) -> _Int: ...
def ShipDesignsProduced(*, empire: _EmpireId) -> _Int: ...
def ShipDesignsScrapped(*, empire: _EmpireId) -> _Int: ...
