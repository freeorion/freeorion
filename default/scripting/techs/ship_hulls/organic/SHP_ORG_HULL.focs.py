from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_ORG_HULL",
    description="SHP_ORG_HULL_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=16 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_ORGANIC_HULL_TECHS"],
    prerequisites=["GRO_MEGA_ECO"],
    unlock=[Item(type=UnlockShipHull, name="SH_ORGANIC"), Item(type=UnlockBuilding, name="BLD_SHIPYARD_ORG_ORB_INC")],
    graphic="hulls_design/organic_hull.png",
)
