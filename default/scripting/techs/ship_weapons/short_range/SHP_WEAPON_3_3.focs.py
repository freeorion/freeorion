from common.base_prod import TECH_COST_MULTIPLIER
from techs.ship_weapons.ship_weapons import WEAPON_UPGRADE_CAPACITY_EFFECTS

Tech(
    name="SHP_WEAPON_3_3",
    description="SHP_WEAPON_3_3_DESC",
    short_description="SHIP_WEAPON_IMPROVE_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=150 * TECH_COST_MULTIPLIER,
    researchturns=2,
    tags=["PEDIA_SR_WEAPON_TECHS"],
    prerequisites="SHP_WEAPON_3_2",
    effectsgroups=WEAPON_UPGRADE_CAPACITY_EFFECTS("SR_WEAPON_3_1", 3),
    graphic="icons/ship_parts/plasma-3.png",
)
