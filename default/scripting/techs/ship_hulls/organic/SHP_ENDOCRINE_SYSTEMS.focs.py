from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_ENDOCRINE_SYSTEMS",
    description="SHP_ENDOCRINE_SYSTEMS_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=80 * TECH_COST_MULTIPLIER,
    researchturns=8,
    tags=["PEDIA_ORGANIC_HULL_TECHS"],
    prerequisites=["SHP_MULTICELL_CAST", "GRO_LIFECYCLE_MAN"],
    unlock=[Item(type=UnlockShipHull, name="SH_ENDOMORPHIC")],
    graphic="icons/ship_hulls/endomorphic_hull_small.png",
)
