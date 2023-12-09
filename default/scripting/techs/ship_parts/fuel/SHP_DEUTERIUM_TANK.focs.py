from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER
from techs.ship_parts.fuel.fuel import PART_UPGRADE_MAXFUEL_EFFECTS

Tech(
    name="SHP_DEUTERIUM_TANK",
    description="SHP_DEUTERIUM_TANK_DESC",
    short_description="SHIP_FUEL_IMPROVE_SHORT_DESC",
    category="SHIP_PARTS_CATEGORY",
    researchcost=48 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_FUEL_PART_TECHS"],
    prerequisites=["SHP_GAL_EXPLO", "PRO_FUSION_GEN"],
    unlock=Item(type=UnlockShipPart, name="FU_RAMSCOOP"),
    effectsgroups=[PART_UPGRADE_MAXFUEL_EFFECTS("SHP_DEUTERIUM_TANK_EFFECT", 0.5)],
    graphic="icons/ship_parts/deuterium_tank.png",
)
