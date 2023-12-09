from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="LRN_ART_BLACK_HOLE",
    description="LRN_ART_BLACK_HOLE_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="LEARNING_CATEGORY",
    researchcost=480 * TECH_COST_MULTIPLIER,
    researchturns=8,
    tags=["PEDIA_LEARNING_CATEGORY"],
    prerequisites=["LRN_TIME_MECH"],
    unlock=Item(type=UnlockBuilding, name="BLD_ART_BLACK_HOLE"),
    graphic="icons/tech/artificial_black_hole.png",
)
