from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_ENDOSYMB_HULL",
    description="SHP_ENDOSYMB_HULL_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=120 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_ORGANIC_HULL_TECHS"],
    prerequisites=["SHP_MONOCELL_EXP", "SHP_ENDOCRINE_SYSTEMS"],
    unlock=[Item(type=UnlockShipHull, name="SH_ENDOSYMBIOTIC")],
    graphic="icons/ship_hulls/endosymbiotic_hull_small.png",
)
