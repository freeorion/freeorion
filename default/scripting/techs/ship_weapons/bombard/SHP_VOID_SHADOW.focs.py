from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_VOID_SHADOW",
    description="SHP_VOID_SHADOW_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=600 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_BOMBARD_WEAPON_TECHS"],
    prerequisites=["SHP_DARK_RAY", "LRN_GATEWAY_VOID"],
    unlock=Item(type=UnlockShipPart, name="SP_VOID_SHADOW"),
    graphic="icons/ship_parts/void-shadow.png",
)
