from focs._conditions import (
    Contains,
    IsBuilding,
    OwnedBy,
    Planet,
    Star,
)
from focs._enums import BlackHole, Blue, External, White
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
    name="SH_COMPRESSED_ENERGY",
    description="SH_COMPRESSED_ENERGY_DESC",
    speed=120,
    fuel=5,
    NoDefaultFuelEffect=True,
    stealth=45,
    structure=10,
    slots=[Slot(type=External, position=(0.65, 0.40))],
    buildcost=5 * FLEET_UPKEEP_MULTIPLICATOR * SHIP_HULL_COST_MULTIPLIER,
    buildtime=2,
    tags=["ENERGY_HULL", "PEDIA_HULL_LINE_ENERGY", "GOOD_FUEL_EFFICIENCY"],
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
    icon="icons/ship_hulls/compressed_energy_hull_small.png",
    graphic="hulls_design/compressed_energy_hull.png",
)
