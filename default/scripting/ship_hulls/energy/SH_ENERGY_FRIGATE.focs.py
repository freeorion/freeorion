from focs._conditions import (
    Contains,
    IsBuilding,
    OwnedBy,
    Planet,
    Star,
)
from focs._enums import BlackHole, Blue, External, Internal, White
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
    name="SH_ENERGY_FRIGATE",
    description="SH_ENERGY_FRIGATE_DESC",
    speed=120,
    fuel=4,
    NoDefaultFuelEffect=True,
    stealth=5,
    structure=15,
    slots=[
        Slot(type=External, position=(0.30, 0.20)),
        Slot(type=Internal, position=(0.25, 0.45)),
        Slot(type=External, position=(0.45, 0.40)),
        Slot(type=External, position=(0.55, 0.47)),
        Slot(type=External, position=(0.65, 0.55)),
    ],
    buildcost=40 * FLEET_UPKEEP_MULTIPLICATOR * SHIP_HULL_COST_MULTIPLIER,
    buildtime=3,
    tags=["PEDIA_HULL_LINE_ENERGY", "GOOD_FUEL_EFFICIENCY"],
    location=(
        Planet()
        & Contains(IsBuilding(name="BLD_SHIPYARD_ENRG_COMP") & OwnedBy(empire=Source.Owner))
        & Contains(IsBuilding(name="BLD_SHIPYARD_BASE") & OwnedBy(empire=Source.Owner))
        & OwnedBy(empire=Source.Owner)
        & Star(type=[White, Blue, BlackHole])
    ),
    effectsgroups=[
        *GOOD_FUEL_EFFICIENCY,
        ADD_HULL_FUEL_TO_MAX_FUEL_METER,
        *AVERAGE_BASE_FUEL_REGEN,
        REGULAR_HULL_DETECTION,
        SCAVANGE_FUEL_UNOWNED,
        UNOWNED_GOOD_VISION,
        UNOWNED_MOVE,
    ],
    icon="icons/ship_hulls/energy_frigate_hull_small.png",
    graphic="hulls_design/energy_frigate_hull.png",
)
