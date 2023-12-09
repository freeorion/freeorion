from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SPY_STEALTH_PART_1",
    description="SPY_STEALTH_PART_1_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SPY_CATEGORY",
    researchcost=80 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_STEALTH_PART_TECHS"],
    prerequisites=["SPY_DETECT_2"],
    unlock=Item(type=UnlockShipPart, name="ST_CLOAK_1"),
    graphic="icons/ship_parts/cloak-1.png",
)
