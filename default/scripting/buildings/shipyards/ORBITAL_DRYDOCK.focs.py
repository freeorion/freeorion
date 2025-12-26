from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    AllyOf,
    ContainedBy,
    Contains,
    EffectsGroup,
    EmpireHasAdoptedPolicy,
    GenerateSitRepMessage,
    InSystem,
    IsBuilding,
    LocalCandidate,
    MaximumNumberOf,
    MaxOf,
    MinOf,
    Object,
    OwnedBy,
    Planet,
    Population,
    SetStructure,
    Ship,
    Source,
    Structure,
    Target,
    Turn,
    Value,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET, LOCATION_ALLOW_ENQUEUE_IF_PREREQ_ENQUEUED
from macros.misc import SHIP_STRUCTURE_FACTOR

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

# No repairs below this happiness
CONST_ORB_DRYDOCK_MIN_HAPPY = 5

# Max repair at this happiness
CONST_ORB_DRYDOCK_TARGET_HAPPY = 15.0


# Happiness rate formula
ORB_DRYDOCK_HAPPY_RATE = MinOf(float, Source.Planet.Happiness / CONST_ORB_DRYDOCK_TARGET_HAPPY, 1.0)


# Minimum repair per turn, at target happiness
CONST_ORB_DRYDOCK_MIN_REPAIR = 30.0 * SHIP_STRUCTURE_FACTOR

# Max repair rate, based on MaxStructure
CONST_ORB_DRYDOCK_TARGET_REPAIR = 0.25


def ORB_DRYDOCK_REPAIR_RATE(max_structure):
    """
    Repair rate formula

    """
    return MaxOf(float, CONST_ORB_DRYDOCK_MIN_REPAIR, max_structure * CONST_ORB_DRYDOCK_TARGET_REPAIR)


def ORB_DRYDOCK_REPAIR_VAL(max_structure):
    """
    Formula for amount of structure to repair
    arg1 target max structure

    """
    return ORB_DRYDOCK_HAPPY_RATE * ORB_DRYDOCK_REPAIR_RATE(max_structure)


"""
    Planet in system with the highest happiness that is owned and has a drydock
    """
PLANET_OWNED_DRYDOCK_HIGHEST_HAPPINESS = MaximumNumberOf(
    number=1,
    sortkey=LocalCandidate.Happiness,
    condition=(
        InSystem(id=Source.SystemID)
        & Planet()
        & (
            OwnedBy(empire=Source.Owner)
            | (
                # either ally providing repair or owner of ship being repaired must adopt policy to share repairs
                (
                    EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_ALLIED_REPAIR")
                    | EmpireHasAdoptedPolicy(empire=LocalCandidate.Owner, name="PLC_ALLIED_REPAIR")
                )
                & OwnedBy(affiliation=AllyOf, empire=Source.Owner)
            )
        )
        & Contains(IsBuilding(name=["BLD_SHIPYARD_ORBITAL_DRYDOCK"]))
    ),
)


BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_SHIPYARD_ORBITAL_DRYDOCK",
    description="BLD_SHIPYARD_ORBITAL_DRYDOCK_DESC",
    buildcost=20 * BUILDING_COST_MULTIPLIER,
    buildtime=5,
    tags=["ORBITAL"],
    location=(
        Planet()
        & OwnedBy(empire=Source.Owner)
        & ~Contains(IsBuilding(name=["BLD_SHIPYARD_ORBITAL_DRYDOCK"]))
        & LOCATION_ALLOW_ENQUEUE_IF_PREREQ_ENQUEUED("BLD_SHIPYARD_BASE")
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        #         # ZERO-POP : With no pop, no repair or sitrep messages
        EffectsGroup(
            scope=(
                Ship
                & InSystem(id=Source.SystemID)
                & (
                    OwnedBy(empire=Source.Owner)
                    | (  # either ally providing repair or owner of ship beign repaired must adopt policy to share repairs
                        (
                            EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_ALLIED_REPAIR")
                            | EmpireHasAdoptedPolicy(empire=LocalCandidate.Owner, name="PLC_ALLIED_REPAIR")
                        )
                        & OwnedBy(affiliation=AllyOf, empire=Source.Owner)
                    )
                )
                & Structure(high=LocalCandidate.MaxStructure - 0.001)
            ),
            activation=(
                ContainedBy(Object(id=Source.PlanetID) & PLANET_OWNED_DRYDOCK_HIGHEST_HAPPINESS & Population(high=0))
            ),
            stackinggroup="SHIP_REPAIR",
            priority=75,
            effects=[
                GenerateSitRepMessage(
                    message="EFFECT_DRYDOCK_SHIP_REPAIR_NOPOP",
                    label="SITREP_SHIP_REPAIR_DOCK_NOPOP",
                    icon="icons/sitrep/ship-repair.png",
                    parameters={
                        "ship": Target.ID,
                        "building": Source.ID,
                        "planet": Source.PlanetID,
                    },
                    empire=Source.Owner,
                ),
            ],
        ),
        #         # COMPLETE : Normal repair, completely restored
        EffectsGroup(
            scope=(
                Ship
                & InSystem(id=Source.SystemID)
                & (
                    OwnedBy(empire=Source.Owner)
                    | (
                        # either ally providing repair or owner of ship beign repaired must adopt policy to share repairs
                        (
                            EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_ALLIED_REPAIR")
                            | EmpireHasAdoptedPolicy(empire=LocalCandidate.Owner, name="PLC_ALLIED_REPAIR")
                        )
                        & OwnedBy(affiliation=AllyOf, empire=Source.Owner)
                    )
                )
                & Structure(
                    low=LocalCandidate.MaxStructure - ORB_DRYDOCK_REPAIR_VAL(LocalCandidate.MaxStructure) - 0.001,
                    high=LocalCandidate.MaxStructure - 0.001,
                )
                & Turn(low=LocalCandidate.ArrivedOnTurn + 1)
                & (Source.Planet.Happiness >= CONST_ORB_DRYDOCK_MIN_HAPPY)
            ),
            activation=(
                ContainedBy(PLANET_OWNED_DRYDOCK_HIGHEST_HAPPINESS) & Turn(low=Source.System.LastTurnBattleHere + 1)
            ),
            stackinggroup="SHIP_REPAIR",
            priority=80,  # not macroed to prevent unwanted changes,
            effects=[
                SetStructure(value=Target.MaxStructure),
                GenerateSitRepMessage(
                    message="EFFECT_DRYDOCK_SHIP_REPAIR_COMPLETE",
                    label="SITREP_SHIP_REPAIR_DOCK_COMPLETE",
                    icon="icons/sitrep/ship-repair-complete.png",
                    parameters={
                        "ship": Target.ID,
                        "building": Source.ID,
                        "planet": Source.PlanetID,
                    },
                    empire=Source.Owner,
                ),
            ],
        ),
        #
        #                 # PARTIAL : Normal repair
        EffectsGroup(
            scope=(
                Ship
                & InSystem(id=Source.SystemID)
                & (
                    OwnedBy(empire=Source.Owner)
                    | (
                        # either ally providing repair or owner of ship beign repaired must adopt policy to share repairs
                        (
                            EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_ALLIED_REPAIR")
                            | EmpireHasAdoptedPolicy(empire=LocalCandidate.Owner, name="PLC_ALLIED_REPAIR")
                        )
                        & OwnedBy(affiliation=AllyOf, empire=Source.Owner)
                    )
                )
                & Structure(high=LocalCandidate.MaxStructure - ORB_DRYDOCK_REPAIR_VAL(LocalCandidate.MaxStructure))
                & Turn(low=LocalCandidate.ArrivedOnTurn + 1)
                & (Source.Planet.Happiness >= CONST_ORB_DRYDOCK_MIN_HAPPY)
            ),
            activation=(
                ContainedBy(PLANET_OWNED_DRYDOCK_HIGHEST_HAPPINESS) & Turn(low=Source.System.LastTurnBattleHere + 1)
            ),
            stackinggroup="SHIP_REPAIR",
            priority=80,  # not macroed to prevent unwanted changes,
            effects=[
                SetStructure(value=Value + ORB_DRYDOCK_REPAIR_VAL(Target.MaxStructure)),
                GenerateSitRepMessage(
                    message="EFFECT_DRYDOCK_SHIP_REPAIR_PARTIAL",
                    label="SITREP_SHIP_REPAIR_DOCK_PARTIAL",
                    icon="icons/sitrep/ship-repair.png",
                    parameters={
                        "ship": Target.ID,
                        "building": Source.ID,
                        "planet": Source.PlanetID,
                    },
                    empire=Source.Owner,
                ),
            ],
        ),
        #                 # FALLBACK : No repair
        EffectsGroup(
            scope=(
                Ship
                & InSystem(id=Source.SystemID)
                & (
                    OwnedBy(empire=Source.Owner)
                    | (
                        # either ally providing repair or owner of ship beign repaired must adopt policy to share repairs
                        (
                            EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_ALLIED_REPAIR")
                            | EmpireHasAdoptedPolicy(empire=LocalCandidate.Owner, name="PLC_ALLIED_REPAIR")
                        )
                        & OwnedBy(affiliation=AllyOf, empire=Source.Owner)
                    )
                )
                & Structure(high=LocalCandidate.MaxStructure - 0.001)
                & Turn(low=LocalCandidate.ArrivedOnTurn + 1)
                & (Source.Planet.Happiness < CONST_ORB_DRYDOCK_MIN_HAPPY)
            ),
            activation=(
                ContainedBy(PLANET_OWNED_DRYDOCK_HIGHEST_HAPPINESS) & Turn(low=Source.System.LastTurnBattleHere + 1)
            ),
            stackinggroup="SHIP_REPAIR",
            priority=120,  # not macroed to prevent unwanted changes,
            effects=[
                GenerateSitRepMessage(
                    message="EFFECT_DRYDOCK_SHIP_REPAIR_NONE",
                    label="SITREP_SHIP_REPAIR_DOCK_NONE",
                    icon="icons/sitrep/ship-repair.png",
                    parameters={
                        "ship": Target.ID,
                        "building": Source.ID,
                        "planet": Source.PlanetID,
                    },
                    empire=Source.Owner,
                ),
            ],
        ),
    ],
    icon="icons/building/shipyard-1.png",  # No repairs below this happiness
)
