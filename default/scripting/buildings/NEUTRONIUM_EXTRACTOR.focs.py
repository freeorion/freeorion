from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    Contains,
    IsBuilding,
    Neutron,
    OwnedBy,
    Planet,
    Source,
    Star,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_NEUTRONIUM_EXTRACTOR",
    description="BLD_NEUTRONIUM_EXTRACTOR_DESC",
    buildcost=25 * BUILDING_COST_MULTIPLIER,
    buildtime=5,
    tags=["ORBITAL"],
    location=(
        Planet()
        & ~Contains(IsBuilding(name=["BLD_NEUTRONIUM_EXTRACTOR"]))
        & OwnedBy(empire=Source.Owner)
        & Star(type=[Neutron])
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
    ],
    icon="icons/building/neutronium-forge.png",
)
