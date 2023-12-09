from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="LRN_XENOARCH",
    description="LRN_XENOARCH_DESC",
    short_description="XENOARCH_SHORT_DESC",
    category="LEARNING_CATEGORY",
    researchcost=70 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_LEARNING_CATEGORY"],
    prerequisites=["LRN_TRANSLING_THT"],
    unlock=[
        Item(type=UnlockBuilding, name="BLD_XENORESURRECTION_LAB"),
        Item(type=UnlockPolicy, name="PLC_DIVINE_AUTHORITY"),
    ],
    graphic="icons/tech/xenoarchaeology.png",
)
