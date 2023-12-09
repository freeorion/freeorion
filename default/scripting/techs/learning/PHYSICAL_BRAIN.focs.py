from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="LRN_PHYS_BRAIN",
    description="LRN_PHYS_BRAIN_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="LEARNING_CATEGORY",
    researchcost=10 * TECH_COST_MULTIPLIER,
    researchturns=2,
    tags=["PEDIA_LEARNING_CATEGORY"],
    unlock=[Item(type=UnlockPolicy, name="PLC_LIBERTY"), Item(type=UnlockBuilding, name="BLD_AUTO_HISTORY_ANALYSER")],
    graphic="icons/tech/the_physical_brain.png",
)
