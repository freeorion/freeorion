from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_SPINAL_ANTIMATTER",
    description="SHP_SPINAL_ANTIMATTER_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=300 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_SR_WEAPON_TECHS"],
    prerequisites=["PRO_ZERO_GEN"],
    unlock=Item(type=UnlockShipPart, name="SR_SPINAL_ANTIMATTER"),
    graphic="icons/ship_parts/spinal_antimatter.png",
)
