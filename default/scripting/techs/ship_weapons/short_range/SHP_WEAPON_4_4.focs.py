from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER
from techs.ship_weapons.ship_weapons import WEAPON_UPGRADE_CAPACITY_EFFECTS

Tech(
    name="SHP_WEAPON_4_4",
    description="SHP_WEAPON_4_4_DESC",
    short_description="SHIP_WEAPON_IMPROVE_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=1200 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_SR_WEAPON_TECHS"],
    prerequisites=["SHP_WEAPON_4_3"],
    effectsgroups=WEAPON_UPGRADE_CAPACITY_EFFECTS("SHP_WEAPON_4_4", "SR_WEAPON_4_1", 5),
    graphic="icons/ship_parts/death-ray-4.png",
)
