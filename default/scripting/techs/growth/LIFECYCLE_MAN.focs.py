from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="GRO_LIFECYCLE_MAN",
    description="GRO_LIFECYCLE_MAN_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="GROWTH_CATEGORY",
    researchcost=240 * TECH_COST_MULTIPLIER,
    researchturns=6,
    tags=["PEDIA_GROWTH_CATEGORY"],
    prerequisites=["GRO_GENETIC_ENG"],
    unlock=[
        Item(type=UnlockShipPart, name="CO_SUSPEND_ANIM_POD"),
        Item(type=UnlockPolicy, name="PLC_NO_GROWTH"),
    ],
    graphic="icons/tech/lifecycle_manipulation.png",
)
