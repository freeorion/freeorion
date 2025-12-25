from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    Contains,
    IsBuilding,
    OwnedBy,
    Planet,
    Source,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_NEUTRONIUM_SYNTH",
    description="BLD_NEUTRONIUM_SYNTH_DESC",
    buildcost=1200 * BUILDING_COST_MULTIPLIER,
    buildtime=10,
    location=(Planet() & ~Contains(IsBuilding(name=["BLD_NEUTRONIUM_SYNTH"])) & OwnedBy(empire=Source.Owner)),
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
    ],
    icon="icons/building/neutronium-forge.png",
)
