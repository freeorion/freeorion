from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SPY_STEALTH_PART_2",
    description="SPY_STEALTH_PART_2_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SPY_CATEGORY",
    researchcost=150 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_STEALTH_PART_TECHS"],
    prerequisites=["SPY_DETECT_3"],
    unlock=Item(type=UnlockShipPart, name="ST_CLOAK_2"),
    graphic="icons/ship_parts/cloak-2.png",
)
