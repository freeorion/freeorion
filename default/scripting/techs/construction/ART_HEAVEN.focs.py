from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="CON_ART_HEAVENLY",
    description="CON_ART_HEAVENLY_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="CONSTRUCTION_CATEGORY",
    researchcost=150 * TECH_COST_MULTIPLIER,
    researchturns=6,
    tags=["PEDIA_CONSTRUCTION_CATEGORY"],
    prerequisites=["CON_CONTGRAV_ARCH", "SHP_CONTGRAV_MAINT"],
    unlock=Item(type=UnlockBuilding, name="BLD_ART_MOON"),
    graphic="icons/tech/artificial_heavenly_bodies.png",
)
