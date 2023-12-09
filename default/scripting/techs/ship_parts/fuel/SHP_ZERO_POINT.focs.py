from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_ZERO_POINT",
    description="SHP_ZERO_POINT_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SHIP_PARTS_CATEGORY",
    researchcost=270 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_FUEL_PART_TECHS"],
    prerequisites=["SHP_ANTIMATTER_TANK", "PRO_ZERO_GEN"],
    unlock=Item(type=UnlockShipPart, name="FU_ZERO_FUEL"),
    graphic="icons/ship_parts/zero-point-generator.png",
)
