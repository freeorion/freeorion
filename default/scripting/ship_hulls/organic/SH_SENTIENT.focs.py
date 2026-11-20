from focs._conditions import (
    Capital,
    Contains,
    IsBuilding,
    IsSource,
    OwnedBy,
    Planet,
    Ship,
    WithinDistance,
)
from focs._effects import EffectsGroup, SetDetection, SetEmpireMeter, SetStealth
from focs._enums import AllyOf, Core, External, Internal
from focs._ship_hulls import Hull, Slot
from focs._sources import Source
from focs._value_refs import Value
from macros.upkeep import FLEET_UPKEEP_MULTIPLICATOR, SHIP_HULL_COST_MULTIPLIER
from ship_hulls.hull_structures import HULL_STRUCTURES
from ship_hulls.organic.organic import LIVING_HULL_EFFECTS_GROUPS, LIVING_HULL_GROWTH_EFFECTS_GROUP
from ship_hulls.ship_hulls import (
    ADD_HULL_FUEL_TO_MAX_FUEL_METER,
    AVERAGE_FUEL_EFFICIENCY,
    SCAVANGE_FUEL_UNOWNED,
    UNOWNED_GOOD_VISION,
    UNOWNED_MOVE,
)

Hull(
    name="SH_SENTIENT",
    description="SH_SENTIENT_DESC",
    speed=100,
    fuel=1,
    NoDefaultFuelEffect=True,
    stealth=25,
    structure=HULL_STRUCTURES["SH_SENTIENT"].BASE_STRUCTURE,
    slots=[
        Slot(type=External, position=(0.40, 0.40)),
        Slot(type=External, position=(0.50, 0.40)),
        Slot(type=External, position=(0.70, 0.40)),
        Slot(type=External, position=(0.70, 0.60)),
        Slot(type=External, position=(0.80, 0.40)),
        Slot(type=External, position=(0.80, 0.60)),
        Slot(type=Internal, position=(0.40, 0.60)),
        Slot(type=Internal, position=(0.50, 0.60)),
        Slot(type=Internal, position=(0.35, 0.75)),
        Slot(type=Core, position=(0.60, 0.50)),
    ],
    buildcost=60 * FLEET_UPKEEP_MULTIPLICATOR * SHIP_HULL_COST_MULTIPLIER,
    buildtime=8,
    tags=["ORGANIC_HULL", "PEDIA_HULL_LINE_ORGANIC", "AVERAGE_FUEL_EFFICIENCY"],
    location=(
        Planet()
        & OwnedBy(empire=Source.Owner)
        & Contains(IsBuilding(name="BLD_SHIPYARD_ORG_XENO_FAC") & OwnedBy(empire=Source.Owner))
        & Contains(IsBuilding(name="BLD_SHIPYARD_ORG_CELL_GRO_CHAMB") & OwnedBy(empire=Source.Owner))
        & Contains(IsBuilding(name="BLD_SHIPYARD_ORG_ORB_INC") & OwnedBy(empire=Source.Owner))
        & Contains(IsBuilding(name="BLD_SHIPYARD_BASE") & OwnedBy(empire=Source.Owner))
    ),
    effectsgroups=[
        EffectsGroup(
            scope=WithinDistance(distance=0, condition=IsSource)
            & Ship
            & (OwnedBy(empire=Source.Owner) | OwnedBy(affiliation=AllyOf, empire=Source.Owner)),
            stackinggroup="FLAGSHIP_EFFECT_SENTIENT",
            effects=[SetStealth(value=Value + 20), SetDetection(value=Value + 50)],
        ),
        EffectsGroup(
            scope=Capital & OwnedBy(empire=Source.Owner),
            stackinggroup="FLAGSHIP_EFFECT_SENTIENT",
            effects=SetEmpireMeter(
                empire=Source.Owner,
                meter="MILITARY_CATEGORY_NUM_POLICY_SLOTS",
                value=Value + 1,
            ),
        ),
        LIVING_HULL_GROWTH_EFFECTS_GROUP("SH_SENTIENT"),
        *AVERAGE_FUEL_EFFICIENCY,
        ADD_HULL_FUEL_TO_MAX_FUEL_METER,
        *LIVING_HULL_EFFECTS_GROUPS,
        SCAVANGE_FUEL_UNOWNED,
        UNOWNED_GOOD_VISION,
        UNOWNED_MOVE,
    ],
    icon="icons/ship_hulls/sentient_hull_small.png",
    graphic="hulls_design/sentient_hull.png",
)
