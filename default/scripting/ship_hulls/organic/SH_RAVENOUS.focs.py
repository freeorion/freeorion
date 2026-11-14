from focs._conditions import Contains, IsBuilding, IsSource, OwnedBy, Planet
from focs._effects import EffectsGroup, SetDetection
from focs._enums import External, Internal
from focs._ship_hulls import Hull, Slot
from focs._sources import Source
from focs._value_refs import Value
from macros.upkeep import FLEET_UPKEEP_MULTIPLICATOR, SHIP_HULL_COST_MULTIPLIER
from ship_hulls.hull_structures import HULL_STRUCTURES
from ship_hulls.organic.organic import LIVING_HULL_GROWTH_EFFECTS_GROUP
from ship_hulls.ship_hulls import (
    ADD_HULL_FUEL_TO_MAX_FUEL_METER,
    AVERAGE_BASE_FUEL_REGEN,
    AVERAGE_FUEL_EFFICIENCY,
    SCAVANGE_FUEL_UNOWNED,
    UNOWNED_GOOD_VISION,
    UNOWNED_MOVE,
)

Hull(
    name="SH_RAVENOUS",
    description="SH_RAVENOUS_DESC",
    speed=100,
    fuel=1,
    NoDefaultFuelEffect=True,
    stealth=5,
    structure=HULL_STRUCTURES["SH_RAVENOUS"].BASE_STRUCTURE,
    slots=[
        Slot(type=External, position=(0.20, 0.15)),
        Slot(type=External, position=(0.40, 0.15)),
        Slot(type=Internal, position=(0.45, 0.50)),
        Slot(type=Internal, position=(0.55, 0.50)),
        Slot(type=External, position=(0.80, 0.55)),
        Slot(type=External, position=(0.20, 0.75)),
        Slot(type=External, position=(0.40, 0.75)),
    ],
    buildcost=25 * FLEET_UPKEEP_MULTIPLICATOR * SHIP_HULL_COST_MULTIPLIER,
    buildtime=6,
    tags=["ORGANIC_HULL", "PEDIA_HULL_LINE_ORGANIC", "AVERAGE_FUEL_EFFICIENCY"],
    location=(
        Planet()
        & OwnedBy(empire=Source.Owner)
        & Contains(IsBuilding(name="BLD_SHIPYARD_ORG_XENO_FAC") & OwnedBy(empire=Source.Owner))
        & Contains(IsBuilding(name="BLD_SHIPYARD_ORG_ORB_INC") & OwnedBy(empire=Source.Owner))
        & Contains(IsBuilding(name="BLD_SHIPYARD_BASE") & OwnedBy(empire=Source.Owner))
    ),
    effectsgroups=[
        EffectsGroup(scope=IsSource, effects=SetDetection(value=Value + 75)),
        LIVING_HULL_GROWTH_EFFECTS_GROUP("SH_RAVENOUS"),
        *AVERAGE_FUEL_EFFICIENCY,
        ADD_HULL_FUEL_TO_MAX_FUEL_METER,
        *AVERAGE_BASE_FUEL_REGEN,
        SCAVANGE_FUEL_UNOWNED,
        UNOWNED_GOOD_VISION,
        UNOWNED_MOVE,
    ],
    icon="icons/ship_hulls/ravenous_hull_small.png",
    graphic="hulls_design/ravenous_hull.png",
)
