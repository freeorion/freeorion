from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_ORGANIC_WAR_ADAPTION",
    description="SHP_ORGANIC_WAR_ADAPTION_DESC",
    short_description="SHP_ORGANIC_WAR_ADAPTION_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=120 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_SR_WEAPON_TECHS"],
    prerequisites=["SHP_WEAPON_2_2", "SHP_ORG_HULL"],
    unlock=Item(type=UnlockShipPart, name="SP_SOLAR_CONCENTRATOR"),
    graphic="icons/ship_parts/solarcollector.png",
)
