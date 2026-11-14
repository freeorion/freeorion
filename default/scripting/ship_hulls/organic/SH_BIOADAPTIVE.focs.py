from focs._conditions import Contains, IsBuilding, IsSource, OwnedBy, Planet, Turn
from focs._effects import EffectsGroup, SetDetection, SetStructure
from focs._enums import External, Internal
from focs._ship_hulls import Hull, Slot
from focs._sources import Source
from focs._value_refs import Value
from macros.upkeep import FLEET_UPKEEP_MULTIPLICATOR, SHIP_HULL_COST_MULTIPLIER
from ship_hulls.hull_structures import HULL_STRUCTURES
from ship_hulls.organic.organic import LIVING_HULL_BASE_FUEL_REGEN, LIVING_HULL_GROWTH_EFFECTS_GROUP
from ship_hulls.ship_hulls import (
    ADD_HULL_FUEL_TO_MAX_FUEL_METER,
    AVERAGE_FUEL_EFFICIENCY,
    SCAVANGE_FUEL_UNOWNED,
    UNOWNED_GOOD_VISION,
    UNOWNED_MOVE,
)

Hull(
    name="SH_BIOADAPTIVE",
    description="SH_BIOADAPTIVE_DESC",
    speed=100,
    fuel=1,
    NoDefaultFuelEffect=True,
    stealth=35,
    structure=HULL_STRUCTURES["SH_BIOADAPTIVE"].BASE_STRUCTURE,
    slots=[
        Slot(type=External, position=(0.30, 0.35)),
        Slot(type=External, position=(0.50, 0.35)),
        Slot(type=External, position=(0.70, 0.35)),
        Slot(type=Internal, position=(0.20, 0.65)),
        Slot(type=Internal, position=(0.40, 0.65)),
        Slot(type=Internal, position=(0.60, 0.65)),
    ],
    buildcost=21 * FLEET_UPKEEP_MULTIPLICATOR * SHIP_HULL_COST_MULTIPLIER,
    buildtime=6,
    tags=["ORGANIC_HULL", "PEDIA_HULL_LINE_ORGANIC", "AVERAGE_FUEL_EFFICIENCY"],
    location=(
        Planet()
        & OwnedBy(empire=Source.Owner)
        & Contains(IsBuilding(name="BLD_SHIPYARD_ORG_CELL_GRO_CHAMB") & OwnedBy(empire=Source.Owner))
        & Contains(IsBuilding(name="BLD_SHIPYARD_ORG_ORB_INC") & OwnedBy(empire=Source.Owner))
        & Contains(IsBuilding(name="BLD_SHIPYARD_BASE") & OwnedBy(empire=Source.Owner))
    ),
    effectsgroups=[
        EffectsGroup(
            scope=IsSource,
            activation=Turn(low=Source.CreationTurn + 1),
            effects=SetStructure(value=Value + Value),
        ),
        EffectsGroup(scope=IsSource, effects=SetDetection(value=Value + 75)),
        LIVING_HULL_GROWTH_EFFECTS_GROUP("SH_BIOADAPTIVE"),
        *AVERAGE_FUEL_EFFICIENCY,
        ADD_HULL_FUEL_TO_MAX_FUEL_METER,
        *LIVING_HULL_BASE_FUEL_REGEN,
        SCAVANGE_FUEL_UNOWNED,
        UNOWNED_GOOD_VISION,
        UNOWNED_MOVE,
    ],
    icon="icons/ship_hulls/bio_adaptive_hull_small.png",
    graphic="hulls_design/bio_adaptive_hull.png",
)
