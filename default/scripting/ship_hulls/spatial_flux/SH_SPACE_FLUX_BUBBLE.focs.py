from focs._conditions import (
    Contains,
    IsBuilding,
    OwnedBy,
    Planet,
)
from focs._enums import Internal
from focs._ship_hulls import Hull, Slot
from focs._sources import Source
from macros.upkeep import FLEET_UPKEEP_MULTIPLICATOR, SHIP_HULL_COST_MULTIPLIER
from ship_hulls.ship_hulls import (
    ADD_HULL_FUEL_TO_MAX_FUEL_METER,
    AVERAGE_BASE_FUEL_REGEN,
    GREAT_FUEL_EFFICIENCY,
    REGULAR_HULL_DETECTION,
    SCAVANGE_FUEL_UNOWNED,
    UNOWNED_GOOD_VISION,
    UNOWNED_MOVE,
)
from ship_hulls.spatial_flux.spatial_flux import (
    SPATIAL_FLUX_STEALTH_EFFECTS,
)

Hull(
    name="SH_SPACE_FLUX_BUBBLE",
    description="SH_SPACE_FLUX_BUBBLE_DESC",
    speed=80,
    fuel=6,
    NoDefaultFuelEffect=True,
    stealth=15,
    structure=17,
    slots=[Slot(type=Internal, position=(0.50, 0.50))],
    buildcost=14 * FLEET_UPKEEP_MULTIPLICATOR * SHIP_HULL_COST_MULTIPLIER,
    buildtime=2,
    tags=["PEDIA_HULL_LINE_SPATIAL_FLUX", "GREAT_FUEL_EFFICIENCY"],
    location=(
        Planet()
        & OwnedBy(empire=Source.Owner)
        & Contains(IsBuilding(name="BLD_SHIPYARD_BASE") & OwnedBy(empire=Source.Owner))
        & Contains(IsBuilding(name="BLD_SHIPYARD_ORBITAL_DRYDOCK") & OwnedBy(empire=Source.Owner))
    ),
    effectsgroups=[
        *SPATIAL_FLUX_STEALTH_EFFECTS,
        *GREAT_FUEL_EFFICIENCY,
        ADD_HULL_FUEL_TO_MAX_FUEL_METER,
        *AVERAGE_BASE_FUEL_REGEN,
        REGULAR_HULL_DETECTION,
        SCAVANGE_FUEL_UNOWNED,
        UNOWNED_GOOD_VISION,
        UNOWNED_MOVE,
    ],
    icon="icons/ship_hulls/bulk_freighter_hull_small.png",
    graphic="hulls_design/bulk_freighter_hull.png",
)
