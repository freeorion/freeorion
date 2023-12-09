from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SPY_DIST_MOD",
    description="SPY_DIST_MOD_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SPY_CATEGORY",
    researchcost=120 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_DETECTION_PART_TECHS"],
    prerequisites=["LRN_EVERYTHING"],
    unlock=Item(type=UnlockShipPart, name="SP_DISTORTION_MODULATOR"),
    graphic="icons/ship_parts/distortion_modulator.png",
)
