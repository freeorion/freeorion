from __future__ import annotations

from typing import TypeVar

# Default value for int keyword arguments, use it when default value is not clear
from focs._types import (
    _N,
    _Aggregator,
    _Condition,
    _Empire,
    _EmpireId,
    _EnqueuedObjectType,
    _Int,
    _MeterType,
    _PlanetEnvironment,
    _PlanetSize,
    _PlanetType,
    _Resource,
    _ShipPartClass,
    _StarType,
    _Visibility,
)

Capacity = _MeterType()
MaxCapacity = _MeterType()
SecondaryStat = _MeterType()
MaxSecondaryStat = _MeterType()

Blue = _StarType()
White = _StarType()
Orange = _StarType()
Yellow = _StarType()
Red = _StarType()
Neutron = _StarType()
NoStar = _StarType()
BlackHole = _StarType()

Invisible = _Visibility()
Basic = _Visibility()
Partial = _Visibility()
Full = _Visibility()

Tiny = _PlanetSize()
Small = _PlanetSize()
Medium = _PlanetSize()
Large = _PlanetSize()
Huge = _PlanetSize()
AsteroidsSize = _PlanetSize()
GasGiantSize = _PlanetSize()

Swamp = _PlanetType()
Toxic = _PlanetType()
Inferno = _PlanetType()
Radiated = _PlanetType()
Barren = _PlanetType()
Tundra = _PlanetType()
Desert = _PlanetType()
Terran = _PlanetType()
Ocean = _PlanetType()
AsteroidsType = _PlanetType()
GasGiantType = _PlanetType()

Good = _PlanetEnvironment()
Adequate = _PlanetEnvironment()
Poor = _PlanetEnvironment()
Hostile = _PlanetEnvironment()
Uninhabitable = _PlanetEnvironment()

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

BuildBuilding = _EnqueuedObjectType()

AnyEmpire = _Empire()

class _Affilation(_Empire): ...

EnemyOf = _Affilation()
AllyOf = _Affilation()
CanSee = _Affilation()

_O = TypeVar("_O", str, int, float)

CountUnique = _Aggregator()
If = _Aggregator()
Count = _Aggregator()
HistogramMax = _Aggregator()
HistogramMin = _Aggregator()
HistogramSpread = _Aggregator()
Sum = _Aggregator()
Mean = _Aggregator()
RMS = _Aggregator()
Mode = _Aggregator()
Max = _Aggregator()
Min = _Aggregator()
Spread = _Aggregator()
StDev = _Aggregator()
Product = _Aggregator()

def StatisticElse(type_: type[_N], *, condition: _Condition) -> _N: ...

ResourceInfluence = _Resource()
ResourceIndustry = _Resource()

def ShipDesignsLost(*, empire: _EmpireId) -> _Int: ...
def ShipDesignsProduced(*, empire: _EmpireId) -> _Int: ...
def ShipDesignsScrapped(*, empire: _EmpireId) -> _Int: ...

class _ItemType: ...

UnlockPolicy = _ItemType()
UnlockShipPart = _ItemType()
UnlockBuilding = _ItemType()
UnlockShipHull = _ItemType()
