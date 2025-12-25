from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    Contains,
    IsBuilding,
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

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_GENOME_BANK",
    description="BLD_GENOME_BANK_DESC",
    buildcost=15 * BUILDING_COST_MULTIPLIER,
    buildtime=16,
    location=(
        Planet()
        & ~Contains(IsBuilding(name=["BLD_GENOME_BANK"]))
        & OwnedBy(empire=Source.Owner)
        & TargetPopulation(low=1)
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
    ],
    icon="icons/building/genome_bank.png",
)
