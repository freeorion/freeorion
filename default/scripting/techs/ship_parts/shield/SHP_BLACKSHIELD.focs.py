from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_BLACKSHIELD",
    description="SHP_BLACKSHIELD_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SHIP_PARTS_CATEGORY",
    researchcost=2500 * TECH_COST_MULTIPLIER,
    researchturns=10,
    tags=["PEDIA_SHIELD_PART_TECHS"],
    prerequisites=["SHP_PLASMA_SHIELD"],
    unlock=Item(type=UnlockShipPart, name="SH_BLACK"),
    graphic="icons/ship_parts/blackshield.png",
)
