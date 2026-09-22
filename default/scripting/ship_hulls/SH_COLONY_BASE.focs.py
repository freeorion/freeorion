from focs._conditions import (
    IsSource,
    OwnedBy,
    Planet,
)
from focs._effects import EffectsGroup, SetMaxStructure
from focs._enums import Internal
from focs._ship_hulls import Hull, Slot
from focs._sources import Source
from focs._value_refs import MinOf
from macros.misc_pre import SHIP_STRUCTURE_FACTOR, SHIP_WEAPON_DAMAGE_FACTOR
from macros.upkeep import FLEET_UPKEEP_MULTIPLICATOR, SHIP_HULL_COST_MULTIPLIER
from ship_hulls.ship_hulls import (
    REGULAR_HULL_DETECTION,
)

Hull(
    name="SH_COLONY_BASE",
    description="SH_COLONY_BASE_DESC",
    speed=0,
    fuel=0,
    NoDefaultFuelEffect=True,
    stealth=5,
    structure=2,
    NoDefaultStructureEffect=True,
    slots=[
        Slot(type=Internal, position=(0.38, 0.50)),
        Slot(type=Internal, position=(0.50, 0.50)),
        Slot(type=Internal, position=(0.62, 0.50)),
    ],
    buildcost=6 * FLEET_UPKEEP_MULTIPLICATOR * SHIP_HULL_COST_MULTIPLIER,
    buildtime=3,
    tags=["PEDIA_HULL_LINE_GENERIC"],
    location=Planet() & OwnedBy(empire=Source.Owner),
    effectsgroups=[
        REGULAR_HULL_DETECTION,
        # There are weapons designed to kill a base hull with a single shot (Arc Disruptor)
        # Also mines should be able to down base hull
        # So we scale the base hull with the lower of those scaling factors
        EffectsGroup(
            scope=IsSource,
            activation=IsSource,
            priority=0,  # DefaultStructureEffect priority is zero
            effects=SetMaxStructure(value=2 * MinOf(float, SHIP_WEAPON_DAMAGE_FACTOR, SHIP_STRUCTURE_FACTOR)),
        ),
    ],
    icon="icons/ship_hulls/colony_base_hull_small.png",
    graphic="hulls_design/colony_base_hull.png",
)
