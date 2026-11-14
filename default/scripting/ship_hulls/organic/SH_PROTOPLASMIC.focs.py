from focs._conditions import Contains, IsBuilding, OwnedBy, Planet
from focs._enums import External, Internal
from focs._ship_hulls import Hull, Slot
from focs._sources import Source
from macros.upkeep import FLEET_UPKEEP_MULTIPLICATOR, SHIP_HULL_COST_MULTIPLIER
from ship_hulls.hull_structures import HULL_STRUCTURES
from ship_hulls.organic.organic import LIVING_HULL_EFFECTS_GROUPS, LIVING_HULL_GROWTH_EFFECTS_GROUP
from ship_hulls.ship_hulls import (
    ADD_HULL_FUEL_TO_MAX_FUEL_METER,
    GOOD_FUEL_EFFICIENCY,
    SCAVANGE_FUEL_UNOWNED,
    UNOWNED_GOOD_VISION,
    UNOWNED_MOVE,
)

Hull(
    name="SH_PROTOPLASMIC",
    description="SH_PROTOPLASMIC_DESC",
    speed=100,
    fuel=2,
    NoDefaultFuelEffect=True,
    stealth=25,
    structure=HULL_STRUCTURES["SH_PROTOPLASMIC"].BASE_STRUCTURE,
    slots=[
        Slot(type=External, position=(0.75, 0.40)),
        Slot(type=External, position=(0.65, 0.65)),
        Slot(type=Internal, position=(0.55, 0.30)),
        Slot(type=Internal, position=(0.45, 0.525)),
        Slot(type=Internal, position=(0.35, 0.75)),
    ],
    buildcost=15 * FLEET_UPKEEP_MULTIPLICATOR * SHIP_HULL_COST_MULTIPLIER,
    buildtime=5,
    tags=["ORGANIC_HULL", "PEDIA_HULL_LINE_ORGANIC", "GOOD_FUEL_EFFICIENCY"],
    location=(
        Planet()
        & OwnedBy(empire=Source.Owner)
        & Contains(IsBuilding(name="BLD_SHIPYARD_ORG_CELL_GRO_CHAMB") & OwnedBy(empire=Source.Owner))
        & Contains(IsBuilding(name="BLD_SHIPYARD_ORG_ORB_INC") & OwnedBy(empire=Source.Owner))
        & Contains(IsBuilding(name="BLD_SHIPYARD_BASE") & OwnedBy(empire=Source.Owner))
    ),
    effectsgroups=[
        LIVING_HULL_GROWTH_EFFECTS_GROUP("SH_PROTOPLASMIC"),
        *GOOD_FUEL_EFFICIENCY,
        ADD_HULL_FUEL_TO_MAX_FUEL_METER,
        *LIVING_HULL_EFFECTS_GROUPS,
        SCAVANGE_FUEL_UNOWNED,
        UNOWNED_GOOD_VISION,
        UNOWNED_MOVE,
    ],
    icon="icons/ship_hulls/protoplasmic_hull_small.png",
    graphic="hulls_design/protoplasmic_hull.png",
)
