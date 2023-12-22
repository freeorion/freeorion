from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="GRO_GAIA_TRANS",
    description="GRO_GAIA_TRANS_DESC",
    short_description="GRO_GAIA_TRANS_SHORT_DESC",
    category="GROWTH_CATEGORY",
    researchcost=420 * TECH_COST_MULTIPLIER,
    researchturns=7,
    tags=["PEDIA_GROWTH_CATEGORY"],
    prerequisites=["GRO_TRANSORG_SENT"],
    unlock=Item(type=UnlockBuilding, name="BLD_GAIA_TRANS"),
    graphic="icons/tech/the_living_planet.png",
)
