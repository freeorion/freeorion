from focs._conditions import (
    Contains,
    IsBuilding,
    OwnedBy,
    Planet,
)
from focs._enums import External, Internal
from focs._ship_hulls import Hull, Slot
from focs._sources import Source
from macros.upkeep import FLEET_UPKEEP_MULTIPLICATOR, SHIP_HULL_COST_MULTIPLIER
from ship_hulls.ship_hulls import (
    ADD_HULL_FUEL_TO_MAX_FUEL_METER,
    AVERAGE_BASE_FUEL_REGEN,
    AVERAGE_FUEL_EFFICIENCY,
    REGULAR_HULL_DETECTION,
    SCAVANGE_FUEL_UNOWNED,
    UNOWNED_GOOD_VISION,
    UNOWNED_MOVE,
)

Hull(
    name="SH_BASIC_LARGE",
    description="SH_BASIC_LARGE_DESC",
    speed=60,
    fuel=1.5,
    NoDefaultFuelEffect=True,
    stealth=5,
    structure=15,
    slots=[
        Slot(type=External, position=(0.50, 0.35)),
        Slot(type=External, position=(0.50, 0.60)),
        Slot(type=External, position=(0.80, 0.45)),
        Slot(type=Internal, position=(0.30, 0.40)),
    ],
    buildcost=30 * FLEET_UPKEEP_MULTIPLICATOR * SHIP_HULL_COST_MULTIPLIER,
    buildtime=2,
    tags=["PEDIA_HULL_LINE_GENERIC", "AVERAGE_FUEL_EFFICIENCY"],
    location=Planet()
    & OwnedBy(empire=Source.Owner)
    & Contains(IsBuilding(name="BLD_SHIPYARD_BASE") & OwnedBy(empire=Source.Owner)),
    effectsgroups=[
        *AVERAGE_FUEL_EFFICIENCY,
        ADD_HULL_FUEL_TO_MAX_FUEL_METER,
        *AVERAGE_BASE_FUEL_REGEN,
        REGULAR_HULL_DETECTION,
        SCAVANGE_FUEL_UNOWNED,
        UNOWNED_GOOD_VISION,
        UNOWNED_MOVE,
    ],
    icon="icons/ship_hulls/basic-large-hull_small.png",
    graphic="hulls_design/basic-large-hull.png",
)
