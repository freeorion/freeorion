from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    BlackHole,
    Blue,
    BuildBuilding,
    ContainedBy,
    Contains,
    Enqueued,
    InSystem,
    IsBuilding,
    Object,
    OwnedBy,
    Planet,
    Red,
    RootCandidate,
    Source,
    Star,
    White,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET, LOCATION_ALLOW_ENQUEUE_IF_PREREQ_ENQUEUED

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_SHIPYARD_ENRG_COMP",
    description="BLD_SHIPYARD_ENRG_COMP_DESC",
    buildcost=150 * BUILDING_COST_MULTIPLIER,
    buildtime=5,
    tags=["ORBITAL"],
    location=(
        Planet()
        & OwnedBy(empire=Source.Owner)
        & ~Contains(IsBuilding(name=["BLD_SHIPYARD_ENRG_COMP"]))
        & LOCATION_ALLOW_ENQUEUE_IF_PREREQ_ENQUEUED("BLD_SHIPYARD_BASE")
        & (
            Star(type=[White, Blue, BlackHole])
            | Enqueued(type=BuildBuilding, name="BLD_ART_BLACK_HOLE")
            | ContainedBy(
                Object(id=RootCandidate.SystemID)
                & Star(type=[Red])
                & Contains(
                    InSystem(id=RootCandidate.SystemID)
                    & Planet()
                    & OwnedBy(empire=Source.Owner)
                    & Enqueued(type=BuildBuilding, name="BLD_ART_BLACK_HOLE")
                )
            )
        )
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
    icon="icons/building/shipyard-10.png",
)
