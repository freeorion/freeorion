from focs._types import (
    _ID,
    _DesignID,
    _Empire,
    _EmpireId,
    _FleetID,
    _Float,
    _Focus,
    _Int,
    _PlanetId,
    _PlanetType,
    _SpeciesValue,
    _SystemID,
)

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
