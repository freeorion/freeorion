from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER
from techs.ship_weapons.ship_weapons import WEAPON_UPGRADE_CAPACITY_EFFECTS, WEAPON_UPGRADE_SECONDARY_STAT_EFFECTS

Tech(
    name="SHP_WEAPON_2_3",
    description="SHP_WEAPON_2_3_DESC",
    short_description="SHIP_WEAPON_IMPROVE_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=60 * TECH_COST_MULTIPLIER,
    researchturns=2,
    tags=["PEDIA_SR_WEAPON_TECHS"],
    prerequisites=["SHP_WEAPON_2_2"],
    effectsgroups=[
        *WEAPON_UPGRADE_CAPACITY_EFFECTS("SHP_WEAPON_2_3", "SR_WEAPON_2_1", 2),
        *WEAPON_UPGRADE_SECONDARY_STAT_EFFECTS("SHP_WEAPON_2_3", "SR_WEAPON_0_1", 1),
    ],
    graphic="icons/ship_parts/laser-3.png",
)
