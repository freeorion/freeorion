from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER
from techs.ship_parts.fuel.fuel import PART_UPGRADE_MAXFUEL_EFFECTS

Tech(
    name="SHP_ANTIMATTER_TANK",
    description="SHP_ANTIMATTER_TANK_DESC",
    short_description="SHIP_FUEL_IMPROVE_SHORT_DESC",
    category="SHIP_PARTS_CATEGORY",
    researchcost=270 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_FUEL_PART_TECHS"],
    prerequisites=["SHP_DEUTERIUM_TANK", "LRN_FORCE_FIELD"],
    effectsgroups=[PART_UPGRADE_MAXFUEL_EFFECTS("SHP_ANTIMATTER_TANK_EFFECT", 1.5)],
    graphic="icons/ship_parts/antimatter_tank.png",
)
