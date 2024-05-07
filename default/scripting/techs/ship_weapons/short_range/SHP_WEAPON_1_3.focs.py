from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER
from techs.ship_weapons.ship_weapons import WEAPON_UPGRADE_CAPACITY_EFFECTS, WEAPON_UPGRADE_SECONDARY_STAT_EFFECTS

Tech(
    name="SHP_WEAPON_1_3",
    description="SHP_WEAPON_1_3_DESC",
    short_description="SHIP_WEAPON_IMPROVE_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=12 * TECH_COST_MULTIPLIER,
    researchturns=2,
    tags=["PEDIA_SR_WEAPON_TECHS"],
    prerequisites=["SHP_WEAPON_1_2"],
    effectsgroups=[
        *WEAPON_UPGRADE_CAPACITY_EFFECTS("SHP_WEAPON_1_3", "SR_WEAPON_1_1", 1),
        *WEAPON_UPGRADE_SECONDARY_STAT_EFFECTS("SHP_WEAPON_1_3", "SR_WEAPON_0_1", 1),
    ],
    graphic="icons/ship_parts/mass-driver-3.png",
)
