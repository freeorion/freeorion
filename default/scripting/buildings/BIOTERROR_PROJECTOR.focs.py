from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._buildings import BuildingType
from focs._conditions import Contains, HasSpecial, IsBuilding, OwnedBy, Planet
from focs._sources import Source
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET

try:
    pass
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
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
