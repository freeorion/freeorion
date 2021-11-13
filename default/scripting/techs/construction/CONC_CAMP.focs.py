from common.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="CON_CONC_CAMP",
    description="CON_CONC_CAMP_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="CONSTRUCTION_CATEGORY",
    researchcost=36 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_CONSTRUCTION_CATEGORY"],
    prerequisites="CON_ARCH_PSYCH",
    unlock=Item(type=Building, name="BLD_CONC_CAMP"),
    graphic="icons/building/concentration-camp.png",
)
