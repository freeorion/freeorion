from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_SMALL_ROBO",
    description="SHP_SMALL_ROBO_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=16 * TECH_COST_MULTIPLIER,
    researchturns=2,
    tags=["PEDIA_ROBOTIC_HULL_TECHS"],
    prerequisites=["SHP_MIL_ROBO_CONT", "PRO_NANOTECH_PROD"],
    unlock=Item(type=UnlockShipHull, name="SH_SMALL_ROBOTIC"),
    graphic="icons/ship_hulls/generic-medium-hull_small.png",
)
