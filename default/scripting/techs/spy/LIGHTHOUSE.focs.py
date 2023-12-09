from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SPY_LIGHTHOUSE",
    description="SPY_LIGHTHOUSE_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="SPY_CATEGORY",
    researchcost=120 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_SPY_CATEGORY"],
    prerequisites=["SPY_DETECT_2"],
    unlock=Item(type=UnlockBuilding, name="BLD_LIGHTHOUSE"),
    graphic="icons/tech/interstellar_lighthouse.png",
)
