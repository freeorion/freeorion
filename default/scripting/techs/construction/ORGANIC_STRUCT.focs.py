from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="CON_ORGANIC_STRC",
    description="CON_ORGANIC_STRC_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="CONSTRUCTION_CATEGORY",
    researchcost=280 * TECH_COST_MULTIPLIER,
    researchturns=8,
    tags=["PEDIA_CONSTRUCTION_CATEGORY"],
    prerequisites=["GRO_SYMBIOTIC_BIO", "CON_FRC_ENRG_STRC"],
    unlock=Item(type=UnlockBuilding, name="BLD_TRANSFORMER"),
    graphic="icons/tech/transforming_structures.png",
)
