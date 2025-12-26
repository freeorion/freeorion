from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    Contains,
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
    name="BLD_PLANET_CLOAK",
    description="BLD_PLANET_CLOAK_DESC",
    buildcost=150 * BUILDING_COST_MULTIPLIER,
    buildtime=8,
    location=(Planet() & ~Contains(IsBuilding(name=["BLD_PLANET_CLOAK"])) & OwnedBy(empire=Source.Owner)),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
    ],
    icon="",
)
