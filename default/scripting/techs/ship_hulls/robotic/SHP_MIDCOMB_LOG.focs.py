from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_MIDCOMB_LOG",
    description="SHP_MIDCOMB_LOG_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=1000 * TECH_COST_MULTIPLIER,
    researchturns=8,
    tags=["PEDIA_ROBOTIC_HULL_TECHS"],
    prerequisites=["SHP_MASSPROP_SPEC", "SHP_TRANSSPACE_DRIVE", "SHP_INTSTEL_LOG"],
    unlock=[Item(type=UnlockShipHull, name="SH_LOGISTICS_FACILITATOR")],
    graphic="icons/ship_hulls/logistics_facilitator_hull_small.png",
)
