from focs._conditions import (
    Contains,
    IsBuilding,
    OwnedBy,
    Planet,
    Star,
)
from focs._enums import BlackHole, Blue, External
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
    name="SH_FRACTAL_ENERGY",
    description="SH_FRACTAL_ENERGY_DESC",
    speed=120,
    fuel=2.5,
    NoDefaultFuelEffect=True,
    stealth=-5,
    structure=40,
    slots=[
        Slot(type=External, position=(0.10, 0.15)),
        Slot(type=External, position=(0.30, 0.15)),
        Slot(type=External, position=(0.50, 0.15)),
        Slot(type=External, position=(0.70, 0.15)),
        Slot(type=External, position=(0.90, 0.15)),
        Slot(type=External, position=(0.20, 0.50)),
        Slot(type=External, position=(0.40, 0.50)),
        Slot(type=External, position=(0.60, 0.50)),
        Slot(type=External, position=(0.80, 0.50)),
        Slot(type=External, position=(0.10, 0.85)),
        Slot(type=External, position=(0.30, 0.85)),
        Slot(type=External, position=(0.50, 0.85)),
        Slot(type=External, position=(0.70, 0.85)),
        Slot(type=External, position=(0.90, 0.85)),
    ],
    buildcost=80.0 * FLEET_UPKEEP_MULTIPLICATOR * SHIP_HULL_COST_MULTIPLIER,
    buildtime=7,
    tags=["ENERGY_HULL", "PEDIA_HULL_LINE_ENERGY", "AVERAGE_FUEL_EFFICIENCY"],
    location=(
        Planet()
        & Contains(IsBuilding(name="BLD_SHIPYARD_ENRG_COMP") & OwnedBy(empire=Source.Owner))
        & Contains(IsBuilding(name="BLD_SHIPYARD_BASE") & OwnedBy(empire=Source.Owner))
        & OwnedBy(empire=Source.Owner)
        & Star(type=[Blue, BlackHole])
    ),
    effectsgroups=[
        *AVERAGE_FUEL_EFFICIENCY,
        ADD_HULL_FUEL_TO_MAX_FUEL_METER,
        *AVERAGE_BASE_FUEL_REGEN,
        REGULAR_HULL_DETECTION,
        SCAVANGE_FUEL_UNOWNED,
        UNOWNED_GOOD_VISION,
        UNOWNED_MOVE,
    ],
    icon="icons/ship_hulls/fractal_energy_hull_small.png",
    graphic="hulls_design/fractal_energy_hull.png",
)
