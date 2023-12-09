from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="CON_ART_PLANET",
    description="CON_ART_PLANET_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="CONSTRUCTION_CATEGORY",
    researchcost=440 * TECH_COST_MULTIPLIER,
    researchturns=8,
    tags=["PEDIA_CONSTRUCTION_CATEGORY"],
    prerequisites=["CON_ART_HEAVENLY", "LRN_SPATIAL_DISTORT_GEN"],
    unlock=[
        Item(type=UnlockBuilding, name="BLD_ART_PLANET"),
        Item(type=UnlockBuilding, name="BLD_ART_FACTORY_PLANET"),
        Item(type=UnlockBuilding, name="BLD_ART_PARADISE_PLANET"),
    ],
    graphic="icons/tech/artificial_planet.png",
)
