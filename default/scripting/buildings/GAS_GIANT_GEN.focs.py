from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    ContainedBy,
    Contains,
    Destroy,
    EffectsGroup,
    Focus,
    GasGiantType,
    Happiness,
    InSystem,
    IsBuilding,
    IsSource,
    NamedReal,
    Object,
    OwnedBy,
    Planet,
    Population,
    SetTargetIndustry,
    Source,
    Value,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER, INDUSTRY_PER_POP
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET
from macros.priorities import TARGET_AFTER_2ND_SCALING_PRIORITY

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_GAS_GIANT_GEN",
    description="BLD_GAS_GIANT_GEN_DESC",
    buildcost=25 * BUILDING_COST_MULTIPLIER,
    buildtime=3,
    tags=["ORBITAL"],
    location=(
        Planet()
        & ~Contains(IsBuilding(name=["BLD_GAS_GIANT_GEN"]))
        & OwnedBy(empire=Source.Owner)
        & Planet(type=[GasGiantType])
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=(
                InSystem(id=Source.SystemID)
                & Planet()
                & Focus(type=["FOCUS_INDUSTRY"])
                & ~Population(high=0)
                & OwnedBy(empire=Source.Owner)
                & Happiness(low=(NamedReal(name="BLD_GAS_GIANT_GEN_MIN_STABILITY", value=10)))
            ),
            activation=(ContainedBy(Object(id=Source.PlanetID) & Population(high=0))),
            stackinggroup="GAS_GIANT_GEN_STACK",
            priority=TARGET_AFTER_2ND_SCALING_PRIORITY,
            effects=[
                SetTargetIndustry(
                    value=Value
                    + (NamedReal(name="BLD_GAS_GIANT_GEN_OUTPOST_TARGET_INDUSTRY_FLAT", value=25 * INDUSTRY_PER_POP))
                )
            ],
        ),
        EffectsGroup(
            scope=(
                Planet()
                & InSystem(id=Source.SystemID)
                & Focus(type=["FOCUS_INDUSTRY"])
                & ~Population(high=0)
                & OwnedBy(empire=Source.Owner)
                & Happiness(low=10)
            ),
            activation=(ContainedBy(~Population(high=0))),
            stackinggroup="GAS_GIANT_GEN_STACK",
            priority=TARGET_AFTER_2ND_SCALING_PRIORITY,
            effects=[
                SetTargetIndustry(
                    value=Value
                    + (NamedReal(name="BLD_GAS_GIANT_GEN_COLONY_TARGET_INDUSTRY_FLAT", value=15 * INDUSTRY_PER_POP))
                )
            ],
        ),
        EffectsGroup(
            scope=IsSource,
            activation=(~Planet(type=[GasGiantType])),
            effects=[Destroy],
        ),
    ],
    icon="icons/building/gas-giant-generator.png",
)
