from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_MINIAST_SWARM",
    description="SHP_MINIAST_SWARM_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=128 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_ASTEROID_HULL_TECHS"],
    prerequisites=["SHP_ASTEROID_REFORM"],
    unlock=[
        Item(type=UnlockShipHull, name="SH_MINIASTEROID_SWARM"),
    ],
    graphic="icons/ship_hulls/mini_asteroid_swarm_small.png",
)
