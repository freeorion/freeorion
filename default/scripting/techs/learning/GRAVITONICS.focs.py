from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="LRN_GRAVITONICS",
    description="LRN_GRAVITONICS_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="LEARNING_CATEGORY",
    researchcost=128 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_LEARNING_CATEGORY"],
    prerequisites=["LRN_FORCE_FIELD"],
    unlock=[
        Item(type=UnlockBuilding, name="BLD_STARLANE_BORE"),
        Item(type=UnlockBuilding, name="BLD_SCRYING_SPHERE"),
    ],
    graphic="icons/tech/gravitonics.png",
)
