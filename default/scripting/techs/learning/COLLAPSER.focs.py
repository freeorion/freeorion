from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="LRN_COLLAPSER",
    description="LRN_COLLAPSER_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="LEARNING_CATEGORY",
    researchcost=1440 * TECH_COST_MULTIPLIER,
    researchturns=8,
    tags=["PEDIA_LEARNING_CATEGORY"],
    prerequisites=["LRN_ART_BLACK_HOLE", "LRN_STELLAR_TOMOGRAPHY", "LRN_SPATIAL_DISTORT_GEN"],
    unlock=Item(type=UnlockBuilding, name="BLD_BLACK_HOLE_COLLAPSER"),
    graphic="icons/tech/black_hole_collapse.png",
)
