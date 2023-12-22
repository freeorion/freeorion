from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="PRO_TRANS_DESIGN",
    description="PRO_TRANS_DESIGN_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="PRODUCTION_CATEGORY",
    researchcost=120 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_PRODUCTION_CATEGORY"],
    prerequisites=["LRN_PSIONICS", "CON_ARCH_PSYCH"],
    unlock=Item(type=UnlockBuilding, name="BLD_INTERSPECIES_ACADEMY"),
    graphic="icons/tech/transcendent_architecture.png",
)
