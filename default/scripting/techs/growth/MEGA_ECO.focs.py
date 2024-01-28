from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="GRO_MEGA_ECO",
    description="GRO_MEGA_ECO_DESC",
    short_description="GRO_MEGA_ECO_SHORT_DESC",
    category="GROWTH_CATEGORY",
    researchcost=18 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_GROWTH_CATEGORY"],
    prerequisites=["GRO_PLANET_ECOL"],
    unlock=Item(type=UnlockBuilding, name="BLD_NEST_ERADICATOR"),
    graphic="icons/tech/megafauna_ecology.png",
)
