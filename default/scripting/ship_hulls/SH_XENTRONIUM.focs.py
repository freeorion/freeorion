from focs._conditions import (
    Contains,
    IsBuilding,
    OwnedBy,
    Planet,
)
from focs._enums import Core, External
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
    name="SH_XENTRONIUM",
    description="SH_XENTRONIUM_DESC",
    speed=70,
    fuel=3,
    NoDefaultFuelEffect=True,
    stealth=5,
    structure=50,
    slots=[
        Slot(type=External, position=(0.20, 0.15)),
        Slot(type=External, position=(0.40, 0.15)),
        Slot(type=External, position=(0.40, 0.40)),
        Slot(type=External, position=(0.60, 0.40)),
        Slot(type=Core, position=(0.30, 0.65)),
    ],
    buildcost=50 * FLEET_UPKEEP_MULTIPLICATOR * SHIP_HULL_COST_MULTIPLIER,
    buildtime=5,
    tags=["SHINY", "EXOTIC", "GOOD_FUEL_EFFICIENCY"],
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
    icon="icons/ship_hulls/xentronium_hull_small.png",
    graphic="hulls_design/xentronium_hull.png",
)
