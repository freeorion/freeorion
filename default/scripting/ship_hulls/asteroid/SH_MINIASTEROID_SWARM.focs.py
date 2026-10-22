from focs._conditions import (
    ContainedBy,
    Contains,
    IsBuilding,
    IsSource,
    OwnedBy,
    Planet,
    System,
)
from focs._effects import EffectsGroup, SetMaxShield
from focs._enums import External
from focs._ship_hulls import Hull, Slot
from focs._sources import Source
from focs._value_refs import Value
from macros.misc_pre import SHIP_SHIELD_FACTOR
from macros.upkeep import FLEET_UPKEEP_MULTIPLICATOR, SHIP_HULL_COST_MULTIPLIER
from ship_hulls.asteroid.asteroid import ASTEROID_FIELD_STEALTH_BONUS
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
    name="SH_MINIASTEROID_SWARM",
    description="SH_MINIASTEROID_SWARM_DESC",
    speed=60,
    fuel=4,
    NoDefaultFuelEffect=True,
    stealth=35,
    structure=11,
    slots=[
        Slot(type=External, position=(0.40, 0.55)),
        Slot(type=External, position=(0.60, 0.55)),
    ],
    buildcost=10.0 * FLEET_UPKEEP_MULTIPLICATOR * SHIP_HULL_COST_MULTIPLIER,
    buildtime=3,
    tags=["ASTEROID_HULL", "PEDIA_HULL_LINE_ASTEROIDS", "GOOD_FUEL_EFFICIENCY"],
    location=Planet()
    & Contains(IsBuilding(name="BLD_SHIPYARD_BASE") & OwnedBy(empire=Source.Owner))
    & ContainedBy(System & Contains(IsBuilding(name="BLD_SHIPYARD_AST") & OwnedBy(empire=Source.Owner)))
    & ContainedBy(System & Contains(IsBuilding(name="BLD_SHIPYARD_AST_REF") & OwnedBy(empire=Source.Owner))),
    effectsgroups=[
        *GOOD_FUEL_EFFICIENCY,
        ADD_HULL_FUEL_TO_MAX_FUEL_METER,
        ASTEROID_FIELD_STEALTH_BONUS,
        EffectsGroup(scope=IsSource, effects=SetMaxShield(value=Value + (5 * SHIP_SHIELD_FACTOR))),
        *AVERAGE_BASE_FUEL_REGEN,
        REGULAR_HULL_DETECTION,
        SCAVANGE_FUEL_UNOWNED,
        UNOWNED_GOOD_VISION,
        UNOWNED_MOVE,
    ],
    icon="icons/ship_hulls/mini_asteroid_swarm_small.png",
    graphic="hulls_design/mini_asteroid_swarm.png",
)
