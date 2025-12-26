from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    Contains,
    IsBuilding,
    OwnedBy,
    Planet,
    Source,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import (
    ENQUEUE_BUILD_ONE_PER_PLANET,
    LOCATION_ALLOW_BUILD_IF_PREREQ_ENQUEUED,
    LOCATION_ALLOW_ENQUEUE_IF_PREREQ_ENQUEUED,
)

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_SHIPYARD_ORG_CELL_GRO_CHAMB",
    description="BLD_SHIPYARD_ORG_CELL_GRO_CHAMB_DESC",
    buildcost=64 * BUILDING_COST_MULTIPLIER,
    buildtime=8,
    tags=["ORBITAL"],
    location=(
        Planet()
        & OwnedBy(empire=Source.Owner)
        & ~Contains(IsBuilding(name=["BLD_SHIPYARD_ORG_CELL_GRO_CHAMB"]))
        & LOCATION_ALLOW_ENQUEUE_IF_PREREQ_ENQUEUED("BLD_SHIPYARD_BASE")
        & LOCATION_ALLOW_BUILD_IF_PREREQ_ENQUEUED("BLD_SHIPYARD_ORG_ORB_INC")
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
    icon="icons/building/cellular-growth-chamber.png",
)
