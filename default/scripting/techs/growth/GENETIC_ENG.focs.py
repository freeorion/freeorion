from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="GRO_GENETIC_ENG",
    description="GRO_GENETIC_ENG_DESC",
    short_description="THEORY_SHORT_DESC",
    category="GROWTH_CATEGORY",
    researchcost=9 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_GROWTH_CATEGORY", "THEORY"],
    graphic="icons/tech/genetic_engineering.png",
)
