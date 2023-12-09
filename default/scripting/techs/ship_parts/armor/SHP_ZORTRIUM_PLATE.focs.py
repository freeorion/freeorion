from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_ZORTRIUM_PLATE",
    description="SHP_ZORTRIUM_PLATE_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SHIP_PARTS_CATEGORY",
    researchcost=60 * TECH_COST_MULTIPLIER,
    researchturns=2,
    tags=["PEDIA_ARMOR_PART_TECHS"],
    prerequisites=["SHP_ROOT_ARMOR"],
    unlock=Item(type=UnlockShipPart, name="AR_ZORTRIUM_PLATE"),
    graphic="icons/tech/armor_plating.png",
)
