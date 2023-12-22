from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_SCAT_AST_HULL",
    description="SHP_SCAT_AST_HULL_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=250 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_ASTEROID_HULL_TECHS"],
    prerequisites=[
        "SHP_MINIAST_SWARM",
        "SHP_HEAVY_AST_HULL",
        "SHP_CAMO_AST_HULL",
        "SHP_MONOMOLEC_LATTICE",
    ],
    unlock=[Item(type=UnlockShipHull, name="SH_SCATTERED_ASTEROID")],
    graphic="icons/ship_hulls/scattered_asteroid_hull_small.png",
)
