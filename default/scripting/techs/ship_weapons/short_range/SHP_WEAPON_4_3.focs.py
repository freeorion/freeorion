from common.base_prod import TECH_COST_MULTIPLIER
from techs.ship_weapons.ship_weapons import WEAPON_UPGRADE_CAPACITY_EFFECTS

Tech(
    name="SHP_WEAPON_4_3",
    description="SHP_WEAPON_4_3_DESC",
    short_description="SHIP_WEAPON_IMPROVE_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=750 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_SR_WEAPON_TECHS"],
    prerequisites="SHP_WEAPON_4_2",
    effectsgroups=WEAPON_UPGRADE_CAPACITY_EFFECTS("SR_WEAPON_4_1", 5),
    graphic="icons/ship_parts/death-ray-3.png",
)
