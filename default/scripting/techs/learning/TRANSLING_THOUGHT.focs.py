from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="LRN_TRANSLING_THT",
    description="LRN_TRANSLING_THT_DESC",
    short_description="POLICY_AND_BUILDING_UNLOCK_SHORT_DESC",
    category="LEARNING_CATEGORY",
    researchcost=32 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_LEARNING_CATEGORY", "THEORY"],
    prerequisites=["LRN_PHYS_BRAIN", "LRN_ALGO_ELEGANCE"],
    unlock=[
        Item(type=UnlockPolicy, name="PLC_MARINE_RECRUITMENT"),
        Item(type=UnlockPolicy, name="PLC_NATIVE_APPROPRIATION"),
        Item(type=UnlockBuilding, name="BLD_TRANSLATOR"),
    ],
    graphic="icons/tech/translingustic_thought.png",
)
