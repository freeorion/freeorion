from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="CON_ARCH_MONOFILS",
    description="CON_ARCH_MONOFILS_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="CONSTRUCTION_CATEGORY",
    researchcost=125 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_CONSTRUCTION_CATEGORY"],
    prerequisites=["CON_ASYMP_MATS"],
    unlock=Item(type=UnlockBuilding, name="BLD_SPACE_ELEVATOR"),
    graphic="icons/tech/architectural_monofilaments.png",
)
