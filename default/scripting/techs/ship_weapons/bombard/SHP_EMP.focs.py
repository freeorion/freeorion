from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_EMP",
    description="SHP_EMP_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=120 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_BOMBARD_WEAPON_TECHS"],
    prerequisites=["SHP_BOMBARD", "LRN_FORCE_FIELD"],
    unlock=Item(type=UnlockShipPart, name="SP_EMP"),
    graphic="icons/ship_parts/EMP.png",
)
