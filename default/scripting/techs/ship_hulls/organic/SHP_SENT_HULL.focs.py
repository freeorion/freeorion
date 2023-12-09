from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_SENT_HULL",
    description="SHP_SENT_HULL_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=320 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_ORGANIC_HULL_TECHS"],
    prerequisites=["SHP_BIOADAPTIVE_SPEC", "SHP_CONT_BIOADAPT"],
    unlock=[Item(type=UnlockShipHull, name="SH_SENTIENT")],
    graphic="icons/ship_hulls/sentient_hull_small.png",
)
