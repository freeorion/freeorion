from common.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="LRN_EVERYTHING",
    description="LRN_EVERYTHING_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="LEARNING_CATEGORY",
    researchcost=400 * TECH_COST_MULTIPLIER,
    researchturns=6,
    tags=["PEDIA_LEARNING_CATEGORY", "THEORY"],
    prerequisites="LRN_GRAVITONICS",
    unlock=Item(type=UnlockBuilding, name="BLD_FIELD_REPELLOR"),
    graphic="icons/tech/the_theory_of_everything.png",
)
