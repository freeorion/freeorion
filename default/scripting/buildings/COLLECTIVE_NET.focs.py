from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    Contains,
    EffectsGroup,
    Focus,
    Happiness,
    IsBuilding,
    NamedReal,
    NamedRealLookup,
    OwnedBy,
    Planet,
    SetTargetIndustry,
    SetTargetResearch,
    Ship,
    Source,
    Stationary,
    Target,
    TargetPopulation,
    Value,
    WithinDistance,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER, INDUSTRY_PER_POP, RESEARCH_PER_POP
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET
from macros.priorities import TARGET_AFTER_2ND_SCALING_PRIORITY

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_COLLECTIVE_NET",
    description="BLD_COLLECTIVE_NET_DESC",
    buildcost=250 * BUILDING_COST_MULTIPLIER,
    buildtime=10,
    location=(
        Planet()
        & ~Contains(IsBuilding(name=["BLD_COLLECTIVE_NET"]))
        & OwnedBy(empire=Source.Owner)
        & TargetPopulation(low=1)
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=(
                Planet()
                & OwnedBy(empire=Source.Owner)
                & Focus(type=["FOCUS_INDUSTRY"])
                & Happiness(low=NamedReal(name="BLD_COLLECTIVE_NET_MIN_STABILITY", value=15))
            ),
            activation=~WithinDistance(distance=200, condition=Ship & ~Stationary),
            stackinggroup="BLD_COLLECTIVE_NET_INDUSTRY_EFFECT",
            priority=TARGET_AFTER_2ND_SCALING_PRIORITY,
            effects=[
                SetTargetIndustry(
                    value=Value
                    + Target.Population
                    * NamedReal(name="BLD_COLLECTIVE_NET_TARGET_INDUSTRY_PERPOP", value=0.5 * INDUSTRY_PER_POP)
                )
            ],
        ),
        EffectsGroup(
            scope=(
                Planet()
                & OwnedBy(empire=Source.Owner)
                & Focus(type=["FOCUS_RESEARCH"])
                & Happiness(low=NamedRealLookup(name="BLD_COLLECTIVE_NET_MIN_STABILITY"))
            ),
            activation=~WithinDistance(distance=200, condition=Ship & ~Stationary),
            stackinggroup="BLD_COLLECTIVE_NET_RESEARCH_EFFECT",
            priority=TARGET_AFTER_2ND_SCALING_PRIORITY,
            effects=[
                SetTargetResearch(
                    value=Value
                    + Target.Population
                    * NamedReal(name="BLD_COLLECTIVE_NET_TARGET_RESEARCH_PERPOP", value=0.5 * RESEARCH_PER_POP)
                )
            ],
        ),
    ],
    icon="icons/building/psi-corps.png",
)
