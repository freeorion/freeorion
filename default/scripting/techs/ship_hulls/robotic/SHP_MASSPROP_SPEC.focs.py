from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_MASSPROP_SPEC",
    description="SHP_MASSPROP_SPEC_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=1000 * TECH_COST_MULTIPLIER,
    researchturns=10,
    tags=["PEDIA_ROBOTIC_HULL_TECHS"],
    prerequisites=["SHP_CONTGRAV_MAINT"],
    unlock=[Item(type=UnlockShipHull, name="SH_TITANIC")],
    graphic="icons/ship_hulls/titanic_hull_small.png",
)
