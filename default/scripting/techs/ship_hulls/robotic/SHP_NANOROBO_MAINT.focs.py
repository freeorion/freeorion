from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_NANOROBO_MAINT",
    description="SHP_NANOROBO_MAINT_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=500 * TECH_COST_MULTIPLIER,
    researchturns=10,
    tags=["PEDIA_ROBOTIC_HULL_TECHS"],
    prerequisites=["SHP_MIL_ROBO_CONT", "PRO_NANOTECH_PROD"],
    unlock=[
        Item(type=UnlockShipHull, name="SH_NANOROBOTIC"),
        Item(type=UnlockBuilding, name="BLD_SHIPYARD_CON_NANOROBO"),
    ],
    graphic="icons/ship_hulls/nano_robotic_hull_small.png",
)
