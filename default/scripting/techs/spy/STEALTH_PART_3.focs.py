from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SPY_STEALTH_PART_3",
    description="SPY_STEALTH_PART_3_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SPY_CATEGORY",
    researchcost=350 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_STEALTH_PART_TECHS"],
    prerequisites=["SPY_DETECT_4"],
    unlock=Item(type=UnlockShipPart, name="ST_CLOAK_3"),
    graphic="icons/ship_parts/cloak-3.png",
)
