from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="PRO_EXOBOTS",
    description="PRO_EXOBOTS_DESC",
    short_description="EXOBOT_SHORT_DESC",
    category="PRODUCTION_CATEGORY",
    researchcost=96 * TECH_COST_MULTIPLIER,
    researchturns=6,
    tags=["PEDIA_PRODUCTION_CATEGORY"],
    prerequisites=["LRN_NASCENT_AI", "PRO_NANOTECH_PROD"],
    unlock=[Item(type=UnlockBuilding, name="BLD_COL_EXOBOT"), Item(type=UnlockPolicy, name="PLC_EXOBOT_PRODUCTIVITY")],
    graphic="icons/species/robotic-01.png",
)
