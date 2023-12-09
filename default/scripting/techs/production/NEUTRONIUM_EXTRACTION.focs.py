from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="PRO_NEUTRONIUM_EXTRACTION",
    description="PRO_NEUTRONIUM_EXTRACTION_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="PRODUCTION_CATEGORY",
    researchcost=240 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_PRODUCTION_CATEGORY"],
    prerequisites=["PRO_ZERO_GEN", "LRN_STELLAR_TOMOGRAPHY"],
    unlock=[
        Item(type=UnlockBuilding, name="BLD_NEUTRONIUM_EXTRACTOR"),
        Item(type=UnlockBuilding, name="BLD_NEUTRONIUM_FORGE"),
        Item(type=UnlockShipPart, name="AR_NEUTRONIUM_PLATE"),
    ],
    graphic="icons/tech/neutronium_extraction.png",
)
