from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_HEAVY_AST_HULL",
    description="SHP_HEAVY_AST_HULL_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=128 * TECH_COST_MULTIPLIER,
    researchturns=8,
    tags=["PEDIA_ASTEROID_HULL_TECHS"],
    prerequisites=["SHP_ASTEROID_REFORM"],
    unlock=[Item(type=UnlockShipHull, name="SH_HEAVY_ASTEROID")],
    graphic="icons/ship_hulls/heavy_asteroid_hull_small.png",
)
