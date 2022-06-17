from common.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="LRN_UNIF_CONC",
    description="LRN_UNIF_CONC_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="LEARNING_CATEGORY",
    researchcost=400 * TECH_COST_MULTIPLIER,
    researchturns=7,
    tags=["PEDIA_LEARNING_CATEGORY"],
    prerequisites=["LRN_GATEWAY_VOID", "LRN_QUANT_NET"],
    unlock=[Item(type=Building, name="BLD_COLLECTIVE_NET"), Item(type=Policy, name="PLC_DREAM_RECURSION")],
    graphic="icons/tech/unified_conscious.png",
)
