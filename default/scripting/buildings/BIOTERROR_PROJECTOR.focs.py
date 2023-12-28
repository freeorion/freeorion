from buildings.buildings import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    Contains,
    HasSpecial,
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

BuildingType(  # type: ignore[reportUnboundVariable]
    name="BLD_BIOTERROR_PROJECTOR",
    description="BLD_BIOTERROR_PROJECTOR_DESC",
    buildcost=75 * BUILDING_COST_MULTIPLIER,
    buildtime=8,
    location=(
        Planet()
        & ~Contains(IsBuilding(name=["BLD_BIOTERROR_PROJECTOR"]))
        & OwnedBy(empire=Source.Owner)
        & HasSpecial(name="RESONANT_MOON_SPECIAL")
        & ~Contains(IsBuilding(name=["BLD_BIOTERROR_PROJECTOR"]))
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
    ],
    icon="icons/building/bioterror_projector.png",
)
