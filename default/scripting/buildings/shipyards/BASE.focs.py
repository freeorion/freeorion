from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    CanProduceShips,
    Contains,
    EffectsGroup,
    IsBuilding,
    Object,
    OwnedBy,
    Planet,
    SetTargetConstruction,
    Source,
    TargetPopulation,
    Value,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_SHIPYARD_BASE",
    description="BLD_SHIPYARD_BASE_DESC",
    buildcost=10 * BUILDING_COST_MULTIPLIER,
    buildtime=4,
    tags=["ORBITAL", "CTRL_SHIPYARD"],
    location=(
        Planet()
        & TargetPopulation(low=1)
        & ~Contains(IsBuilding(name=["BLD_SHIPYARD_BASE"]))
        & OwnedBy(empire=Source.Owner)
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET & CanProduceShips,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=(Object(id=Source.PlanetID) & Planet()),
            effects=[SetTargetConstruction(value=Value - 10)],
        ),
    ],
    icon="icons/building/shipyard.png",
)
