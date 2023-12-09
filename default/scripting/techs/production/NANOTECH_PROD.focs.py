from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="PRO_NANOTECH_PROD",
    description="PRO_NANOTECH_PROD_DESC",
    short_description="THEORY_SHORT_DESC",
    category="PRODUCTION_CATEGORY",
    researchcost=80 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_PRODUCTION_CATEGORY", "THEORY"],
    prerequisites=["PRO_ROBOTIC_PROD"],
    graphic="icons/tech/nanotech_production.png",
)
