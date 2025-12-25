from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    Contains,
    EffectsGroup,
    IsBuilding,
    NamedReal,
    Object,
    OwnedBy,
    Planet,
    SetDetection,
    Source,
    Value,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_SCANNING_FACILITY",
    description="BLD_SCANNING_FACILITY_DESC",
    buildcost=25 * BUILDING_COST_MULTIPLIER,
    buildtime=5,
    location=(Planet() & ~Contains(IsBuilding(name=["BLD_SCANNING_FACILITY"])) & OwnedBy(empire=Source.Owner)),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=(Object(id=Source.PlanetID) & Planet()),
            effects=[SetDetection(value=Value + NamedReal(name="SCANNING_FACILITY_RANGE", value=50.0))],
        ),
    ],
    icon="icons/building/scanning-facility.png",
)
