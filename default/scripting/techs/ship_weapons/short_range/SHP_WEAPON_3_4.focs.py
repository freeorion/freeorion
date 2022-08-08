from common.base_prod import TECH_COST_MULTIPLIER
from techs.ship_weapons.ship_weapons import WEAPON_UPGRADE_CAPACITY_EFFECTS

Tech(
    name="SHP_WEAPON_3_4",
    description="SHP_WEAPON_3_4_DESC",
    short_description="SHIP_WEAPON_IMPROVE_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=250 * TECH_COST_MULTIPLIER,
    researchturns=2,
    tags=["PEDIA_SR_WEAPON_TECHS"],
    prerequisites="SHP_WEAPON_3_3",
    effectsgroups=WEAPON_UPGRADE_CAPACITY_EFFECTS("SR_WEAPON_3_1", 3),
    graphic="icons/ship_parts/plasma-4.png",
)
