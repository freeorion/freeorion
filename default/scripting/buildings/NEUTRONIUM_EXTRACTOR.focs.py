from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._buildings import BuildingType
from focs._conditions import Contains, IsBuilding, OwnedBy, Planet, Star
from focs._enums import Neutron
from focs._sources import Source
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET

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
