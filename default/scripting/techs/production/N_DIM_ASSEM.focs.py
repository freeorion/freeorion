from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="PRO_NDIM_ASSMB",
    description="PRO_NDIM_ASSMB_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="PRODUCTION_CATEGORY",
    researchcost=320 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_PRODUCTION_CATEGORY"],
    prerequisites=["LRN_NDIM_SUBSPACE"],
    unlock=Item(type=UnlockBuilding, name="BLD_HYPER_DAM"),
    graphic="icons/tech/n-dimensional_assembly.png",
)
