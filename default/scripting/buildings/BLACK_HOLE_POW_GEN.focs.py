from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    BlackHole,
    Contains,
    EffectsGroup,
    EmpireHasAdoptedPolicy,
    Focus,
    Happiness,
    IsBuilding,
    IsSource,
    NamedReal,
    NamedRealLookup,
    OwnedBy,
    Planet,
    ResourceSupplyConnected,
    SetTargetIndustry,
    Source,
    Star,
    StatisticIf,
    Target,
    Value,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER, INDUSTRY_PER_POP
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET
from macros.priorities import (
    TARGET_AFTER_SCALING_PRIORITY,
)

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # type: ignore[reportUnboundVariable]
    name="BLD_BLACK_HOLE_POW_GEN",
    description="BLD_BLACK_HOLE_POW_GEN_DESC",
    buildcost=300
    * BUILDING_COST_MULTIPLIER
    * (
        1
        - 0.25
        * StatisticIf(float, condition=IsSource & EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_INDUSTRIALISM"))
    ),
    buildtime=8,
    tags=["ORBITAL"],
    location=(
        Planet()
        & ~Contains(IsBuilding(name=["BLD_BLACK_HOLE_POW_GEN"]))
        & OwnedBy(empire=Source.Owner)
        & Star(type=[BlackHole])
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=(
                Planet()
                & Focus(type=["FOCUS_INDUSTRY"])
                & OwnedBy(empire=Source.Owner)
                & Happiness(low=NamedRealLookup(name="BLD_BLACK_HOLE_POW_GEN_MIN_STABILITY"))
                & ResourceSupplyConnected(empire=Source.Owner, condition=IsSource)
            ),
            activation=Star(type=[BlackHole]),
            stackinggroup="BLD_BLACK_HOLE_POW_GEN_PRIMARY_EFFECT",
            priority=TARGET_AFTER_SCALING_PRIORITY,
            effects=[
                SetTargetIndustry(
                    value=Value
                    + Target.Population
                    * NamedReal(name="BLD_BLACK_HOLE_POW_GEN_TARGET_INDUSTRY_PERPOP", value=1.0 * INDUSTRY_PER_POP)
                )
            ],
        ),
    ],
    icon="icons/building/blackhole.png",
)
