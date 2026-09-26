from focs._conditions import (
    ContainedBy,
    Contains,
    IsBuilding,
    OwnedBy,
    System,
)
from focs._enums import External, Internal
from focs._ship_hulls import Hull, Slot
from focs._sources import Source
from macros.upkeep import FLEET_UPKEEP_MULTIPLICATOR, SHIP_HULL_COST_MULTIPLIER
from ship_hulls.asteroid.asteroid import ASTEROID_FIELD_STEALTH_BONUS
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
    name="SH_CRYSTALLIZED_ASTEROID",
    description="SH_CRYSTALLIZED_ASTEROID_DESC",
    speed=60,
    fuel=1,
    NoDefaultFuelEffect=True,
    stealth=5,
    structure=55,
    slots=[
        Slot(type=External, position=(0.50, 0.15)),
        Slot(type=External, position=(0.20, 0.50)),
        Slot(type=External, position=(0.80, 0.50)),
        Slot(type=External, position=(0.50, 0.85)),
        Slot(type=Internal, position=(0.40, 0.50)),
        Slot(type=Internal, position=(0.60, 0.50)),
    ],
    buildcost=20.0 * FLEET_UPKEEP_MULTIPLICATOR * SHIP_HULL_COST_MULTIPLIER,
    buildtime=3,
    tags=["ASTEROID_HULL", "PEDIA_HULL_LINE_ASTEROIDS", "AVERAGE_FUEL_EFFICIENCY"],
    location=Contains(IsBuilding(name="BLD_SHIPYARD_BASE") & OwnedBy(empire=Source.Owner))
    & ContainedBy(System & Contains(IsBuilding(name="BLD_SHIPYARD_AST") & OwnedBy(empire=Source.Owner)))
    & ContainedBy(System & Contains(IsBuilding(name="BLD_SHIPYARD_AST_REF") & OwnedBy(empire=Source.Owner))),
    effectsgroups=[
        *AVERAGE_FUEL_EFFICIENCY,
        ADD_HULL_FUEL_TO_MAX_FUEL_METER,
        *AVERAGE_BASE_FUEL_REGEN,
        ASTEROID_FIELD_STEALTH_BONUS,
        REGULAR_HULL_DETECTION,
        SCAVANGE_FUEL_UNOWNED,
        UNOWNED_GOOD_VISION,
        UNOWNED_MOVE,
    ],
    icon="icons/ship_hulls/crystalized_asteroid_hull_small.png",
    graphic="hulls_design/crystalized_asteroid_hull.png",
)
