from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_SOLAR_CONNECTION",
    description="SHP_SOLAR_CONNECTION_DESC",
    short_description="SHP_SOLAR_CONNECTION_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=120 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_SR_WEAPON_TECHS"],
    prerequisites=["SHP_ORGANIC_WAR_ADAPTION"],
    graphic="icons/ship_parts/solarcollector.png",
)
