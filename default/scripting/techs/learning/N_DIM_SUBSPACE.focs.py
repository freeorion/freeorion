from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="LRN_NDIM_SUBSPACE",
    description="LRN_NDIM_SUBSPACE_DESC",
    short_description="THEORY_SHORT_DESC",
    category="LEARNING_CATEGORY",
    researchcost=128 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_LEARNING_CATEGORY", "THEORY"],
    prerequisites=["LRN_FORCE_FIELD"],
    graphic="icons/tech/n-dimensional_subspace.png",
)
