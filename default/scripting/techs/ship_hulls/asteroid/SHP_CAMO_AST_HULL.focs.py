from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_CAMO_AST_HULL",
    description="SHP_CAMO_AST_HULL_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=52 * TECH_COST_MULTIPLIER,
    researchturns=2,
    tags=["PEDIA_ASTEROID_HULL_TECHS"],
    prerequisites=["SHP_ASTEROID_HULLS"],
    unlock=[
        Item(type=UnlockShipHull, name="SH_SMALL_CAMOUFLAGE_ASTEROID"),
        Item(type=UnlockShipHull, name="SH_CAMOUFLAGE_ASTEROID"),
    ],
    graphic="icons/ship_hulls/camouflage_asteroid_hull_small.png",
)
