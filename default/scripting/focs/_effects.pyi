from __future__ import annotations

from typing import TypeVar

# Default value for int keyword arguments, use it when default value is not clear
from focs._types import (
    _ID,
    _N,
    _Aggregator,
    _BuildingType,
    _Condition,
    _DesignID,
    _Empire,
    _EmpireId,
    _FleetID,
    _Float,
    _FloatParam,
    _Focus,
    _Int,
    _IntParam,
    _MeterType,
    _PlanetEnvironment,
    _PlanetId,
    _PlanetSize,
    _PlanetType,
    _Resource,
    _ShipPartClass,
    _SpeciesValue,
    _StarType,
    _StringParam,
    _SystemID,
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

def Object(id) -> _Condition: ...

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

class _EnqueuedObjectType: ...

BuildBuilding = _EnqueuedObjectType()

AnyEmpire = _Empire()

ContentFocus = _Focus()

class _Affilation(_Empire): ...

EnemyOf = _Affilation()
AllyOf = _Affilation()
CanSee = _Affilation()

class _SystemInfo:
    LastTurnBattleHere: _Int
    ID: _ID
    NumStarlanes: _Int

class _Object:
    ID: _ID
    System: _SystemInfo
    SystemID: _SystemID
    X: _Int
    Y: _Int
    Owner: _Empire
    Age: _Int
    CreationTurn: _Int
    # Unowned
    # OwnedBy
    # Specials??
    # HasSpecial
    # SpecialAddedOnTurn
    # SpecialCapacity

class _Planet(_Object):
    Species: _SpeciesValue
    PlanetID: _PlanetId
    Focus: _Focus
    TurnsSinceFocusChange: int
    HabitableSize: _Float
    PlanetType: _PlanetType
    OriginalType: _PlanetType
    NextBestPlanetType: _PlanetType
    LastTurnAnnexed: _Int
    LastTurnConquered: _Int
    LastInvadedByEmpire: _Empire
    LastTurnColonized: _Int
    LastColonizedByEmpire: _Empire
    TurnsSinceColonization: _Int
    Construction: _Float
    TargetConstruction: _Float
    Happiness: _Float
    TargetHappiness: _Float
    Industry: _Float
    TargetIndustry: _Float
    Influence: _Float
    TargetInfluence: _Float
    Population: _Float
    TargetPopulation: _Float
    Research: _Float
    TargetResearch: _Float
    Stealth: _Float
    Stockpile: _Float
    MaxStockpile: _Float
    MaxSupply: _Float
    MaxDefense: _Float
    MaxShield: _Float
    MaxTroops: _Float

class _Ship(_Object):
    Species: _SpeciesValue
    DesignID: _DesignID
    Fleet: _Fleet
    FleetID: _FleetID
    LastTurnResupplied: _Int
    OrderedColonizePlanetID: _PlanetId
    Speed: _Int
    Research: _Float
    Industry: _Float
    MaxFuel: _Float
    Stealth: _Float
    MaxStructure: _Float
    MaxShield: _Float
    MaxTroops: _Float
    Structure: _Float
    DestroyFightersPerBattleMax: _Float
    DamageStructurePerBattleMax: _Float

class _Fleet(_Object):
    PreviousSystemID: _SystemID
    NextSystemID: _SystemID
    FleetID: _FleetID
    ProducedByEmpireID: _EmpireId

class _Building(_Object):
    ProducedByEmpireID: _EmpireId

class Source(_Planet, _Ship, _Fleet):
    """
    FOCS Source condition is IsSource, this class is for value ref Source.<something>
    """

    System: _SystemInfo
    CreationTurn: _Int
    Planet: _Planet
    Size: _Float

class LocalCandidate(_Planet, _Ship, _Fleet):
    LastTurnActiveInBattle: _Int
    LastTurnAttackedByShip: _Int
    ETA: _Int
    ArrivedOnTurn: _Int

class RootCandidate(_Planet, _Ship, _Fleet):
    ID: _ID
    Owner: _Empire
    PreviousSystemID: _SystemID
    SystemID: _SystemID
    Species: _SpeciesValue
    PlanetID: _PlanetId

class Target(_Planet, _Ship, _Fleet):
    PlanetID: _PlanetId
    Size: _Float

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

# Conditions

def Enqueued(
    *,
    type: _EnqueuedObjectType | None = None,
    subtype: str | None = None,
    name: str = "",
    low: _IntParam = ...,
    high: _IntParam = ...,
    empire: _EmpireId = ...,
) -> _Condition: ...
def TurnTechResearched(*, empire: _EmpireId, name: str) -> _Condition: ...
def IsBuilding(
    *,
    name: str | _BuildingType | list[str | _BuildingType] | None = None,
    subtype: str | None = None,
) -> _Condition: ...
def Number(*, low: _IntParam = ..., high: _IntParam = ..., condition: _Condition = ...) -> _Condition: ...
def Planet(
    *,
    type: list[_PlanetType] = ...,
    environment: list[_PlanetEnvironment] = ...,
    size: list[_PlanetSize] = ...,
    species: _SpeciesValue = ...,
) -> _Condition: ...
def Star(*, type: list[_StarType]) -> _Condition: ...
def Location(*, type: _Focus, name: _SpeciesValue, name2: _Focus | str) -> _Condition: ...
def HasSpecies(*, name: list[_StringParam] | _StringParam | None = None) -> _Condition: ...
def IsField(*, name: list[_StringParam] | None = None) -> _Condition: ...
def Population(*, low: _FloatParam = ..., high: _FloatParam = ...) -> _Condition: ...
def Industry(*, low: _FloatParam = ..., high: _FloatParam = ...) -> _Condition: ...
def Size(*, low: _FloatParam = ..., high: _FloatParam = ...) -> _Condition: ...
def TargetPopulation(*, low: _FloatParam = ..., high: _FloatParam = ...) -> _Condition: ...
def SpeciesLikes(*, name: _Focus | _BuildingType | _SpeciesValue) -> _Condition: ...
def SpeciesDislikes(*, name: _Focus | _BuildingType | _SpeciesValue) -> _Condition: ...
def Homeworld(*, name: list[_SpeciesValue] = ...) -> _Condition: ...
def HasTag(*, name: str) -> _Condition: ...
def VisibleToEmpire(*, empire: _EmpireId) -> _Condition: ...
def ContainedBy(condition: _Condition, /) -> _Condition: ...
def HasSpecial(*, name: str) -> _Condition: ...
def OwnerHasTech(*, name: str) -> _Condition: ...

System = _Condition()
Ship = _Condition()
Monster = _Condition()
Armed = _Condition()
Unowned = _Condition()
Capital = _Condition()
IsHuman = _Condition()
Fleet = _Condition()
Stationary = _Condition()
NoOpCondition = _Condition()
IsSource = _Condition()
IsRootCandidate = _Condition()
IsTarget = _Condition()
CanColonize = _Condition()
IsAnyObject = _Condition()
CanProduceShips = _Condition()

def InGame() -> _Condition: ...
def InSystem(*, id: _SystemID = _SystemID()) -> _Condition: ...
def Described(*, description: str, condition: _Condition) -> _Condition: ...
def MinimumNumberOf(number: _IntParam, sortkey: _FloatParam, condition: _Condition) -> _Condition: ...
def MaximumNumberOf(number: _IntParam, sortkey: _FloatParam, condition: _Condition) -> _Condition: ...
def NumberOf(number: _IntParam, condition: _Condition) -> _Condition: ...
def UniqueNumberOf(*, number: _IntParam, condition: _Condition, sortkey) -> _Condition: ...
def TargetIndustry(*, low: _FloatParam) -> _Condition: ...
def Stealth(*, high: _FloatParam) -> _Condition: ...
def Speed(*, low: _FloatParam) -> _Condition: ...
def Happiness(*, low: _FloatParam) -> _Condition: ...
def Focus(*, type: list[str]) -> _Condition: ...
def Random(*, probability: _FloatParam) -> _Condition: ...
def OwnedBy(*, affiliation: _EmpireId = _Empire(), empire: _EmpireId = _Empire()) -> _Condition: ...
def WithinStarlaneJumps(*, jumps: _IntParam, condition: _Condition) -> _Condition: ...
def WithinDistance(*, distance: _FloatParam, condition: _Condition) -> _Condition: ...
def ResourceSupplyConnected(*, empire: _EmpireId, condition: _Condition) -> _Condition: ...
def Contains(scope: _Condition, /) -> _Condition: ...
def Turn(*, high: _FloatParam = ..., low: _FloatParam = ...) -> _Condition: ...
def Structure(*, low: _FloatParam = ..., high: _FloatParam = ...) -> _Condition: ...
def ResupplyableBy(*, empire: _EmpireId) -> _Condition: ...
def DesignHasPart(*, name: str, low: _IntParam = ..., high: _IntParam = ...) -> _Condition: ...
def EmpireHasAdoptedPolicy(*, name: str, empire: _EmpireId = _Empire()) -> _Condition: ...
def ProducedByEmpire(*, empire: _Empire) -> _Condition: ...
def HasStarlane(from_: _Condition) -> _Condition: ...
def HasDesign(name: _StringParam) -> _Condition: ...
def StarlaneToWouldCrossExistingStarlane(from_: _Condition) -> _Condition: ...
def StarlaneToWouldBeAngularlyCloseToExistingStarlane(from_: _Condition, maxdotprod: _FloatParam) -> _Condition: ...
def StarlaneToWouldBeCloseToObject(*, distance: _FloatParam, from_: _Condition, closeto: _Condition) -> _Condition: ...

ResourceInfluence = _Resource()
ResourceIndustry = _Resource()

def HasEmpireStockpile(
    *, empire: _EmpireId, resource: _Resource, low: _IntParam = ..., high: _IntParam = ...
) -> _Condition: ...
def ShipDesignsLost(*, empire: _EmpireId) -> _Int: ...
def ShipDesignsProduced(*, empire: _EmpireId) -> _Int: ...
def ShipDesignsScrapped(*, empire: _EmpireId) -> _Int: ...
def OnPlanet(*, id: _ID) -> _Condition: ...

class _ItemType: ...

UnlockPolicy = _ItemType()
UnlockShipPart = _ItemType()
UnlockBuilding = _ItemType()
UnlockShipHull = _ItemType()
