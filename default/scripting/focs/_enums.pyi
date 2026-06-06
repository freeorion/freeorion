from focs._types import (
    _Affilation,
    _Aggregator,
    _CaptureResult,
    _Empire,
    _EnqueuedObjectType,
    _ItemType,
    _MeterType,
    _PlanetEnvironment,
    _PlanetSize,
    _PlanetType,
    _Resource,
    _StarType,
    _Visibility,
)

UnlockPolicy = _ItemType()
UnlockShipPart = _ItemType()
UnlockBuilding = _ItemType()
UnlockShipHull = _ItemType()

Blue = _StarType()
White = _StarType()
Orange = _StarType()
Yellow = _StarType()
Red = _StarType()
Neutron = _StarType()
NoStar = _StarType()
BlackHole = _StarType()

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

Capacity = _MeterType()
MaxCapacity = _MeterType()
SecondaryStat = _MeterType()
MaxSecondaryStat = _MeterType()

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

EnemyOf = _Affilation()
AllyOf = _Affilation()
CanSee = _Affilation()

AnyEmpire = _Empire()

ResourceInfluence = _Resource()
ResourceIndustry = _Resource()

BuildBuilding = _EnqueuedObjectType()

DefaultCaptureResult = _CaptureResult()
DestroyOnCapture = _CaptureResult()
