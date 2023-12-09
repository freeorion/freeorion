from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="LRN_UNIF_CONC",
    description="LRN_UNIF_CONC_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="LEARNING_CATEGORY",
    researchcost=630 * TECH_COST_MULTIPLIER,
    researchturns=7,
    tags=["PEDIA_LEARNING_CATEGORY"],
    prerequisites=["LRN_GATEWAY_VOID", "LRN_QUANT_NET"],
    unlock=[Item(type=UnlockBuilding, name="BLD_COLLECTIVE_NET"), Item(type=UnlockPolicy, name="PLC_DREAM_RECURSION")],
    graphic="icons/tech/unified_conscious.png",
)
