from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="GRO_GENETIC_MED",
    description="GRO_GENETIC_MED_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="GROWTH_CATEGORY",
    researchcost=24 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_GROWTH_CATEGORY"],
    prerequisites=["GRO_GENETIC_ENG"],
    unlock=Item(type=UnlockBuilding, name="BLD_GENOME_BANK"),
    graphic="icons/tech/genetic_medicine.png",
)
