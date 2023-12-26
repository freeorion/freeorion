from buildings.buildings import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    ContainedBy,
    Contains,
    EffectsGroup,
    Happiness,
    IsBuilding,
    NamedReal,
    Object,
    OwnedBy,
    Planet,
    Population,
    SetTargetResearch,
    Source,
    TargetPopulation,
    Value,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET
from macros.priorities import TARGET_AFTER_2ND_SCALING_PRIORITY

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # type: ignore[reportUnboundVariable]
    name="BLD_AUTO_HISTORY_ANALYSER",
    description="BLD_AUTO_HISTORY_ANALYSER_DESC",
    buildcost=100 * BUILDING_COST_MULTIPLIER,
    buildtime=5,
    location=(
        Planet()
        & Contains(IsBuilding(name=["BLD_CULTURE_ARCHIVES"]))
        & ~Contains(IsBuilding(name=["BLD_AUTO_HISTORY_ANALYSER"]))
        & OwnedBy(empire=Source.Owner)
        & TargetPopulation(low=1)
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=(
                Object(id=Source.PlanetID)
                & Planet()
                & ~Population(high=0)
                & OwnedBy(empire=Source.Owner)
                & Happiness(low=0)
            ),
            activation=(ContainedBy(Object(id=Source.PlanetID) & Contains(IsBuilding(name=["BLD_CULTURE_ARCHIVES"])))),
            priority=TARGET_AFTER_2ND_SCALING_PRIORITY,
            effects=[
                SetTargetResearch(
                    value=Value + NamedReal(name="BLD_AUTO_HISTORY_ANALYSER_TARGET_RESEARCH_FLAT", value=5)
                )
            ],
        ),
    ],
    icon="icons/building/science1.png",
)
