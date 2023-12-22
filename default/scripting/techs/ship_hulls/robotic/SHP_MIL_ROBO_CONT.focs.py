from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_MIL_ROBO_CONT",
    description="SHP_MIL_ROBO_CONT_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=24 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_ROBOTIC_HULL_TECHS"],
    prerequisites=["PRO_ROBOTIC_PROD"],
    unlock=[
        Item(type=UnlockShipHull, name="SH_ROBOTIC"),
        Item(type=UnlockShipPart, name="SH_ROBOTIC_INTERFACE_SHIELDS"),
    ],
    graphic="icons/ship_hulls/robotic_hull_small.png",
)
