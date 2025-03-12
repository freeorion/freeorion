from buildings.buildings import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    Contains,
    IsBuilding,
    Number,
    OwnedBy,
    Planet,
    Source,
    TargetPopulation,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # type: ignore[reportUnboundVariable]
    name="BLD_NEUTRONIUM_FORGE",
    description="BLD_NEUTRONIUM_FORGE_DESC",
    buildcost=100 * BUILDING_COST_MULTIPLIER,
    buildtime=3,
    tags=["ORBITAL"],
    location=(
        Planet()
        & OwnedBy(empire=Source.Owner)
        & ~Contains(IsBuilding(name=["BLD_NEUTRONIUM_FORGE"]))
        & Contains(IsBuilding(name=["BLD_SHIPYARD_BASE"]) & OwnedBy(empire=Source.Owner))
        & Number(
            low=1,
            high=999,
            condition=(
                OwnedBy(empire=Source.Owner)
                & (IsBuilding(name=["BLD_NEUTRONIUM_EXTRACTOR"]) | IsBuilding(name=["BLD_NEUTRONIUM_SYNTH"]))
            ),
        )
        & TargetPopulation(low=1)
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
    ],
    icon="icons/building/neutronium-forge.png",
)
