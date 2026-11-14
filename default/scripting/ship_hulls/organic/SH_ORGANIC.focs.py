from focs._conditions import Contains, IsBuilding, IsSource, OwnedBy, Planet
from focs._effects import EffectsGroup, SetStructure
from focs._enums import External, Internal
from focs._ship_hulls import Hull, Slot
from focs._sources import Source
from focs._value_refs import Value
from macros.misc_pre import SHIP_STRUCTURE_FACTOR
from macros.upkeep import FLEET_UPKEEP_MULTIPLICATOR, SHIP_HULL_COST_MULTIPLIER
from ship_hulls.hull_structures import HULL_STRUCTURES
from ship_hulls.organic.organic import LIVING_HULL_BASE_FUEL_REGEN, LIVING_HULL_GROWTH_EFFECTS_GROUP
from ship_hulls.ship_hulls import (
    ADD_HULL_FUEL_TO_MAX_FUEL_METER,
    GREAT_FUEL_EFFICIENCY,
    REGULAR_HULL_DETECTION,
    SCAVANGE_FUEL_UNOWNED,
    UNOWNED_GOOD_VISION,
    UNOWNED_MOVE,
)

Hull(
    name="SH_ORGANIC",
    description="SH_ORGANIC_DESC",
    speed=90,
    fuel=6,
    NoDefaultFuelEffect=True,
    stealth=5,
    structure=HULL_STRUCTURES["SH_ORGANIC"].BASE_STRUCTURE,
    slots=[
        Slot(type=External, position=(0.80, 0.50)),
        Slot(type=Internal, position=(0.60, 0.50)),
    ],
    buildcost=8 * FLEET_UPKEEP_MULTIPLICATOR * SHIP_HULL_COST_MULTIPLIER,
    buildtime=3,
    tags=["ORGANIC_HULL", "PEDIA_HULL_LINE_ORGANIC", "GREAT_FUEL_EFFICIENCY"],
    location=(
        Planet()
        & OwnedBy(empire=Source.Owner)
        & Contains(IsBuilding(name="BLD_SHIPYARD_ORG_ORB_INC") & OwnedBy(empire=Source.Owner))
        & Contains(IsBuilding(name="BLD_SHIPYARD_BASE") & OwnedBy(empire=Source.Owner))
    ),
    effectsgroups=[
        EffectsGroup(scope=IsSource, effects=SetStructure(value=Value + (2 * SHIP_STRUCTURE_FACTOR))),
        LIVING_HULL_GROWTH_EFFECTS_GROUP("SH_ORGANIC"),
        *GREAT_FUEL_EFFICIENCY,
        ADD_HULL_FUEL_TO_MAX_FUEL_METER,
        *LIVING_HULL_BASE_FUEL_REGEN,
        REGULAR_HULL_DETECTION,
        SCAVANGE_FUEL_UNOWNED,
        UNOWNED_GOOD_VISION,
        UNOWNED_MOVE,
    ],
    icon="icons/ship_hulls/organic_hull_small.png",
    graphic="hulls_design/organic_hull.png",
)
