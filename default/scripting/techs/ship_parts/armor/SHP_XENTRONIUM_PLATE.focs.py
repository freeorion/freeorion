from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_XENTRONIUM_PLATE",
    description="SHP_XENTRONIUM_PLATE_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SHIP_PARTS_CATEGORY",
    researchcost=1200 * TECH_COST_MULTIPLIER,
    researchturns=6,
    tags=["PEDIA_ARMOR_PART_TECHS"],
    prerequisites=["SHP_DIAMOND_PLATE"],
    unlock=Item(type=UnlockShipPart, name="AR_XENTRONIUM_PLATE"),
    graphic="icons/tech/armor_plating.png",
)
