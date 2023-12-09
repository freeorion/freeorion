from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="PRO_INDUSTRY_CENTER_III",
    description="PRO_INDUSTRY_CENTER_III_DESC",
    short_description="BUILDING_REFINE_SHORT_DESC",
    category="PRODUCTION_CATEGORY",
    researchcost=700 * TECH_COST_MULTIPLIER,
    researchturns=7,
    tags=["PEDIA_PRODUCTION_CATEGORY"],
    prerequisites=["PRO_INDUSTRY_CENTER_II"],
    graphic="icons/tech/industrial_centre_iii.png",
)
