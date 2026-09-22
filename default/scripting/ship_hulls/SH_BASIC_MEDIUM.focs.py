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
    GOOD_FUEL_EFFICIENCY,
    REGULAR_HULL_DETECTION,
    SCAVANGE_FUEL_UNOWNED,
    UNOWNED_GOOD_VISION,
    UNOWNED_MOVE,
)

Hull(
    name="SH_BASIC_MEDIUM",
    description="SH_BASIC_MEDIUM_DESC",
    speed=60,
    fuel=3,
    NoDefaultFuelEffect=True,
    stealth=5,
    structure=10,
    slots=[
        Slot(type=External, position=(0.65, 0.25)),
        Slot(type=External, position=(0.65, 0.55)),
        Slot(type=Internal, position=(0.35, 0.35)),
    ],
    buildcost=20 * FLEET_UPKEEP_MULTIPLICATOR * SHIP_HULL_COST_MULTIPLIER,
    buildtime=2,
    tags=["PEDIA_HULL_LINE_GENERIC", "GOOD_FUEL_EFFICIENCY"],
    location=Planet()
    & OwnedBy(empire=Source.Owner)
    & Contains(IsBuilding(name="BLD_SHIPYARD_BASE") & OwnedBy(empire=Source.Owner)),
    effectsgroups=[
        *GOOD_FUEL_EFFICIENCY,
        ADD_HULL_FUEL_TO_MAX_FUEL_METER,
        *AVERAGE_BASE_FUEL_REGEN,
        REGULAR_HULL_DETECTION,
        SCAVANGE_FUEL_UNOWNED,
        UNOWNED_GOOD_VISION,
        UNOWNED_MOVE,
    ],
    icon="icons/ship_hulls/basic-medium-hull_small.png",
    graphic="hulls_design/basic-medium-hull.png",
)
