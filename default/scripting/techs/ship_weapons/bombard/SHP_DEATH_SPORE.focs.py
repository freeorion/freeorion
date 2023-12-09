from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_DEATH_SPORE",
    description="SHP_DEATH_SPORE_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=120 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_BOMBARD_WEAPON_TECHS"],
    prerequisites=["SHP_BOMBARD", "GRO_ADV_ECOMAN"],
    unlock=Item(type=UnlockShipPart, name="SP_DEATH_SPORE"),
    graphic="icons/ship_parts/death-spore.png",
)
