from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    ContainedBy,
    Destroy,
    EffectsGroup,
    Happiness,
    IsAnyObject,
    IsSource,
    NamedReal,
    Object,
    Planet,
    Population,
    SetTargetResearch,
    Source,
    Value,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.priorities import TARGET_AFTER_SCALING_PRIORITY

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_CULTURE_LIBRARY",
    description="BLD_CULTURE_LIBRARY_DESC",
    buildcost=200 * BUILDING_COST_MULTIPLIER,
    buildtime=1,
    tags=["ANTIQUATED"],
    location=~IsAnyObject,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=(
                Object(id=Source.PlanetID)
                & Planet()
                & ~Population(high=0)
                & Happiness(low=NamedReal(name="BLD_CULTURE_LIBRARY_MIN_STABILITY", value=15))
            ),
            priority=TARGET_AFTER_SCALING_PRIORITY,
            effects=[
                SetTargetResearch(value=Value + NamedReal(name="BLD_CULTURE_LIBRARY_TARGET_RESEARCH_FLAT", value=5))
            ],
        ),
        EffectsGroup(
            scope=IsSource,
            activation=ContainedBy(Object(id=Source.PlanetID) & Population(high=0)),
            effects=[Destroy],
        ),
    ],
    icon="icons/building/archive.png",
)
