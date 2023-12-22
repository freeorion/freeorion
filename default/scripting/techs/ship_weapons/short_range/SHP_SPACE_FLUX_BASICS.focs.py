from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_SPACE_FLUX_BASICS",
    description="SHP_SPACE_FLUX_BASICS_DESC",
    short_description="SHIP_WEAPON_UNLOCK_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=10 * TECH_COST_MULTIPLIER,
    researchturns=2,
    tags=["PEDIA_SR_WEAPON_TECHS"],
    prerequisites=["SHP_ROOT_AGGRESSION"],
    unlock=Item(type=UnlockShipPart, name="SR_FLUX_LANCE"),
    graphic="icons/ship_parts/flux-lance.png",
)
