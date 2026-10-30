from focs._conditions import (
    Contains,
    IsBuilding,
    OwnedBy,
    Planet,
    Star,
)
from focs._enums import BlackHole, Blue, External, Internal
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
    name="SH_QUANTUM_ENERGY",
    description="SH_QUANTUM_ENERGY_DESC",
    speed=120,
    fuel=2.5,
    NoDefaultFuelEffect=True,
    stealth=-15,
    structure=50,
    slots=[
        Slot(type=External, position=(0.10, 0.25)),
        Slot(type=External, position=(0.22, 0.25)),
        Slot(type=External, position=(0.34, 0.25)),
        Slot(type=External, position=(0.46, 0.25)),
        Slot(type=External, position=(0.58, 0.25)),
        Slot(type=External, position=(0.70, 0.25)),
        Slot(type=External, position=(0.82, 0.25)),
        Slot(type=Internal, position=(0.22, 0.50)),
        Slot(type=Internal, position=(0.34, 0.50)),
        Slot(type=Internal, position=(0.46, 0.50)),
    ],
    buildcost=60 * FLEET_UPKEEP_MULTIPLICATOR * SHIP_HULL_COST_MULTIPLIER,
    buildtime=5,
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
    icon="icons/ship_hulls/quantum_energy_hull_small.png",
    graphic="hulls_design/quantum_energy_hull.png",
)
