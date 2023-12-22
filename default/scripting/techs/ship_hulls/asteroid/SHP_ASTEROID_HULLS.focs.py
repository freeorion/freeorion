from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_ASTEROID_HULLS",
    description="SHP_ASTEROID_HULLS_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=45 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_ASTEROID_HULL_TECHS"],
    prerequisites=["PRO_MICROGRAV_MAN"],
    unlock=[
        Item(type=UnlockShipHull, name="SH_SMALL_ASTEROID"),
        Item(type=UnlockShipHull, name="SH_ASTEROID"),
        Item(type=UnlockBuilding, name="BLD_SHIPYARD_AST"),
    ],
    graphic="icons/ship_hulls/asteroid_hull_small.png",
)
