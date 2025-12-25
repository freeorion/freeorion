from focs._effects import (
    Contains,
    CreateShip,
    Destroy,
    EffectsGroup,
    IsBuilding,
    IsSource,
    OwnedBy,
    Planet,
    Population,
    Source,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.misc import MIN_RECOLONIZING_SIZE

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_COLONY_BASE",
    description="BLD_COLONY_BASE_DESC",
    buildcost=45 * BUILDING_COST_MULTIPLIER,
    buildtime=1,
    location=(
        Planet()
        & ~Contains(IsBuilding(name=["BLD_COLONY_BASE"]))
        & Population(low=MIN_RECOLONIZING_SIZE)
        & OwnedBy(empire=Source.Owner)
    ),
    # TODO: Disable colony base if there is no possibility in the current system
    effectsgroups=[
        EffectsGroup(
            scope=IsSource,
            effects=[CreateShip(designname="SD_COLONY_BASE", empire=Source.Owner, species=Source.Planet.Species)],
        ),
        EffectsGroup(
            scope=IsSource,
            effects=[Destroy],
        ),
    ],
    icon="icons/ship_hulls/colony_base_hull_small.png",
)
