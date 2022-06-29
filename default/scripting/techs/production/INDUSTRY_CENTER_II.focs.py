from common.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="PRO_INDUSTRY_CENTER_II",
    description="PRO_INDUSTRY_CENTER_II_DESC",
    short_description="BUILDING_REFINE_SHORT_DESC",
    category="PRODUCTION_CATEGORY",
    researchcost=150 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_PRODUCTION_CATEGORY"],
    prerequisites="PRO_INDUSTRY_CENTER_I",
    graphic="icons/tech/industrial_centre_ii.png",
)
