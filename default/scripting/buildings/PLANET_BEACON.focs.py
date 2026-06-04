from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._buildings import BuildingType
from focs._conditions import (
    Contains,
    Focus,
    InSystem,
    IsBuilding,
    IsSource,
    Number,
    OwnedBy,
    Planet,
    WithinStarlaneJumps,
)
from focs._effects_new import Destroy, EffectsGroup
from focs._sources import RootCandidate, Source
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET

try:
    pass
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_PLANET_BEACON",
    description="BLD_PLANET_BEACON_DESC",
    buildcost=1,
    buildtime=1,
    location=(
        Planet()
        & ~Contains(IsBuilding(name=["BLD_PLANET_BEACON"]))
        & OwnedBy(empire=Source.Owner)
        & Number(high=6, condition=Planet() & InSystem(id=RootCandidate.SystemID))
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=IsSource,
            activation=(WithinStarlaneJumps(jumps=0, condition=Planet() & Focus(type=["FOCUS_PLANET_DRIVE"]))),
            effects=[Destroy],
        ),
    ],
    icon="icons/building/beacon.png",
)
