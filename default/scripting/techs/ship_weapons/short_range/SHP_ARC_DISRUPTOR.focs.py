from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER
from techs.ship_weapons.ship_weapons import (
    WEAPON_BASE_EFFECTS,
    WEAPON_UPGRADE_CAPACITY_EFFECTS,
)

AD_2_upgrade = 2
AD_3_upgrade = 3
AD_2_plus_3_upgrade = AD_2_upgrade + AD_3_upgrade

# In absense of shields this weapon can get almost as good as Plasma. Against shields its pretty useless
Tech(
    name="SHP_WEAPON_ARC_DISRUPTOR_1",
    description="SHP_WEAPON_ARC_DISRUPTOR_1_DESC",
    short_description="SHIP_WEAPON_UNLOCK_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=12 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_SR_WEAPON_TECHS"],
    prerequisites=["SHP_ROOT_AGGRESSION"],
    unlock=Item(type=UnlockShipPart, name="SR_ARC_DISRUPTOR"),
    effectsgroups=WEAPON_BASE_EFFECTS("SR_ARC_DISRUPTOR"),
    graphic="icons/ship_parts/pulse-laser-1.png",
)

Tech(
    name="SHP_WEAPON_ARC_DISRUPTOR_2",
    description="SHP_WEAPON_ARC_DISRUPTOR_2_DESC",
    short_description="SHIP_WEAPON_IMPROVE_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=96 * TECH_COST_MULTIPLIER,
    researchturns=8,
    tags=["PEDIA_SR_WEAPON_TECHS"],
    prerequisites=["SHP_WEAPON_ARC_DISRUPTOR_1"],
    effectsgroups=WEAPON_UPGRADE_CAPACITY_EFFECTS("SHP_WEAPON_ARC_DISRUPTOR_2", "SR_ARC_DISRUPTOR", AD_2_upgrade),
    graphic="icons/ship_parts/pulse-laser-2.png",
)


Tech(
    name="SHP_WEAPON_ARC_DISRUPTOR_3",
    description="SHP_WEAPON_ARC_DISRUPTOR_3_DESC",
    short_description="SHIP_WEAPON_IMPROVE_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=600 * TECH_COST_MULTIPLIER,
    researchturns=12,
    tags=["PEDIA_SR_WEAPON_TECHS"],
    prerequisites=["SHP_WEAPON_ARC_DISRUPTOR_2"],
    effectsgroups=WEAPON_UPGRADE_CAPACITY_EFFECTS(
        "SHP_WEAPON_ARC_DISRUPTOR_3", "SR_ARC_DISRUPTOR", AD_3_upgrade, upgraded_damage_override=AD_2_plus_3_upgrade
    ),
    graphic="icons/ship_parts/pulse-laser-3.png",
)
