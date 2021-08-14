from common.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="LRN_ALGO_ELEGANCE",
    description="LRN_ALGO_ELEGANCE_DESC",
    short_description="RESEARCH_SHORT_DESC",
    category="LEARNING_CATEGORY",
    researchcost=10 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_LEARNING_CATEGORY"],
    unlock=Item(type=Policy, name="PLC_ALGORITHMIC_RESEARCH"),
    graphic="icons/tech/algorithmic_elegance.png",
)
