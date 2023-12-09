from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="GRO_ADV_ECOMAN",
    description="GRO_ADV_ECOMAN_DESC",
    short_description="THEORY_SHORT_DESC",
    category="GROWTH_CATEGORY",
    researchcost=78 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_GROWTH_CATEGORY", "THEORY"],
    prerequisites=["GRO_SUBTER_HAB"],
    graphic="icons/tech/advanced_eco_man.png",
)
