from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="CON_PLANET_DRIVE",
    description="CON_PLANET_DRIVE_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="CONSTRUCTION_CATEGORY",
    researchcost=360 * TECH_COST_MULTIPLIER,
    researchturns=6,
    tags=["PEDIA_CONSTRUCTION_CATEGORY"],
    prerequisites=["LRN_SPATIAL_DISTORT_GEN"],
    unlock=[
        Item(type=UnlockBuilding, name="BLD_PLANET_DRIVE"),
        Item(type=UnlockBuilding, name="BLD_PLANET_BEACON"),
        Item(type=UnlockShipPart, name="SP_PLANET_BEACON"),
    ],
    graphic="icons/tech/planetary_stardrive.png",
)
