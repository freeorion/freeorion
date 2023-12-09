from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_DEFLECTOR_SHIELD",
    description="SHP_DEFLECTOR_SHIELD_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SHIP_PARTS_CATEGORY",
    researchcost=240 * TECH_COST_MULTIPLIER,
    researchturns=10,
    tags=["PEDIA_SHIELD_PART_TECHS"],
    prerequisites=["LRN_FORCE_FIELD"],
    unlock=Item(type=UnlockShipPart, name="SH_DEFLECTOR"),
    graphic="icons/ship_parts/deflector_shield.png",
)
