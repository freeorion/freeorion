from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_CONTGRAV_MAINT",
    description="SHP_CONTGRAV_MAINT_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=490 * TECH_COST_MULTIPLIER,
    researchturns=7,
    tags=["PEDIA_ROBOTIC_HULL_TECHS"],
    prerequisites=["SHP_MIL_ROBO_CONT", "CON_ARCH_MONOFILS"],
    unlock=[
        Item(type=UnlockShipHull, name="SH_SELF_GRAVITATING"),
        Item(type=UnlockBuilding, name="BLD_SHIPYARD_CON_GEOINT"),
    ],
    graphic="icons/ship_hulls/self_gravitating_hull_small.png",
)
