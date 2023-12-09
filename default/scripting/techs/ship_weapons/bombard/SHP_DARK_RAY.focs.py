from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_DARK_RAY",
    description="SHP_DARK_RAY_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=120 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_BOMBARD_WEAPON_TECHS"],
    prerequisites=["SHP_BOMBARD", "SPY_STEALTH_2"],
    unlock=Item(type=UnlockShipPart, name="SP_DARK_RAY"),
    graphic="icons/ship_parts/dark-ray.png",
)
