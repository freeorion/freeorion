from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_PLASMA_SHIELD",
    description="SHP_PLASMA_SHIELD_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SHIP_PARTS_CATEGORY",
    researchcost=900 * TECH_COST_MULTIPLIER,
    researchturns=10,
    tags=["PEDIA_SHIELD_PART_TECHS"],
    prerequisites=["SHP_DEFLECTOR_SHIELD"],
    unlock=Item(type=UnlockShipPart, name="SH_PLASMA"),
    graphic="icons/ship_parts/plasma_shield.png",
)
