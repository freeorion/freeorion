from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="LRN_GATEWAY_VOID",
    description="LRN_GATEWAY_VOID_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="LEARNING_CATEGORY",
    researchcost=240 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_LEARNING_CATEGORY"],
    prerequisites=["CON_STARGATE"],
    unlock=Item(type=UnlockBuilding, name="BLD_GATEWAY_VOID"),
    graphic="icons/tech/monument_to_exodus.png",
)
