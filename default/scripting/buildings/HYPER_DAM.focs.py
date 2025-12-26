from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    BlackHole,
    Contains,
    EffectsGroup,
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
    SetTargetPopulation,
    Source,
    Star,
    Target,
    TargetPopulation,
    Value,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER, INDUSTRY_PER_POP
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET
from macros.priorities import (
    TARGET_AFTER_2ND_SCALING_PRIORITY,
    TARGET_POPULATION_AFTER_SCALING_PRIORITY,
)

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_HYPER_DAM",
    description="BLD_HYPER_DAM_DESC",
    buildcost=250 * BUILDING_COST_MULTIPLIER,
    buildtime=8,
    tags=["ORBITAL"],
    location=(
        Planet()
        & ~Contains(IsBuilding(name=["BLD_HYPER_DAM"]))
        & OwnedBy(empire=Source.Owner)
        & TargetPopulation(low=1)
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=(
                Planet()
                & Focus(type=["FOCUS_INDUSTRY"])
                & OwnedBy(empire=Source.Owner)
                & Happiness(low=(NamedReal(name="BLD_HYPER_DAM_MIN_STABILITY", value=12)))
                & ResourceSupplyConnected(empire=Source.Owner, condition=IsSource)
                & (
                    ~ResourceSupplyConnected(
                        empire=Source.Owner,
                        condition=IsBuilding(name=["BLD_BLACK_HOLE_POW_GEN"]) & OwnedBy(empire=Source.Owner),
                    )
                    | ~Happiness(low=NamedRealLookup(name="BLD_BLACK_HOLE_POW_GEN_MIN_STABILITY"))
                )
            ),
            stackinggroup="BLD_HYPER_DAM_BONUS",
            priority=TARGET_AFTER_2ND_SCALING_PRIORITY,
            effects=[
                SetTargetIndustry(
                    value=Value
                    + Target.Population
                    * (NamedReal(name="BLD_HYPER_DAM_TARGET_INDUSTRY_PERPOP", value=1.0 * INDUSTRY_PER_POP))
                )
            ],
        ),
        EffectsGroup(
            scope=(
                Planet()
                & OwnedBy(empire=Source.Owner)
                & ResourceSupplyConnected(empire=Source.Owner, condition=IsSource)
                & ~Star(type=[BlackHole])
                & Focus(type=["FOCUS_INDUSTRY"])
                & (
                    ~ResourceSupplyConnected(
                        empire=Source.Owner,
                        condition=IsBuilding(name=["BLD_BLACK_HOLE_POW_GEN"]) & OwnedBy(empire=Source.Owner),
                    )
                    | ~Happiness(low=NamedRealLookup(name="BLD_BLACK_HOLE_POW_GEN_MIN_STABILITY"))
                )
            ),
            stackinggroup="BLD_HYPER_DAM_MALUS",
            priority=TARGET_POPULATION_AFTER_SCALING_PRIORITY,
            effects=[
                SetTargetPopulation(
                    value=(
                        Value
                        + Target.HabitableSize * NamedReal(name="BLD_HYPER_DAM_TARGET_POPULATION_PERSIZE", value=-1)
                    ),
                ),
            ],
        ),
    ],
    icon="icons/building/blackhole.png",
)
