from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    AsteroidsType,
    Contains,
    GasGiantType,
    IsBuilding,
    OwnedBy,
    Planet,
    Source,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_PLANET_DRIVE",
    description="BLD_PLANET_DRIVE_DESC",
    buildcost=125 * BUILDING_COST_MULTIPLIER,
    buildtime=5,
    location=(
        Planet()
        & ~Contains(IsBuilding(name=["BLD_PLANET_DRIVE"]))
        & OwnedBy(empire=Source.Owner)
        & ~Planet(type=[AsteroidsType, GasGiantType])
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
    ],
    icon="icons/building/planetary_stardrive.png",
)
