from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="CON_STARGATE",
    description="CON_STARGATE_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="CONSTRUCTION_CATEGORY",
    researchcost=400 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_CONSTRUCTION_CATEGORY"],
    prerequisites=[
        "LRN_MIND_VOID",
        "LRN_SPATIAL_DISTORT_GEN",
    ],
    unlock=[
        Item(type=UnlockBuilding, name="BLD_STARGATE"),
    ],
    graphic="icons/building/stargate.png",
)
