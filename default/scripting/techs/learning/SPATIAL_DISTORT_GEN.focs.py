from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="LRN_SPATIAL_DISTORT_GEN",
    description="LRN_SPATIAL_DISTORT_GEN_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="LEARNING_CATEGORY",
    researchcost=200 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_LEARNING_CATEGORY"],
    prerequisites=["LRN_NDIM_SUBSPACE"],
    unlock=Item(type=UnlockBuilding, name="BLD_SPATIAL_DISTORT_GEN"),
    graphic="icons/tech/controlled_gravity_wells.png",
)
