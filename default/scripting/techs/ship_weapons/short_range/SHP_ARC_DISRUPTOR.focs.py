from common.base_prod import TECH_COST_MULTIPLIER
from techs.ship_weapons.ship_weapons import (
    WEAPON_BASE_EFFECTS,
    WEAPON_UPGRADE_CAPACITY_EFFECTS,
)

# In absense of shields this weapon can get almost as good as Plasma. Against shields its pretty useless
Tech(
    name="SHP_WEAPON_ARC_DISRUPTOR_1",
    description="SHP_WEAPON_ARC_DISRUPTOR_1_DESC",
    short_description="SHIP_WEAPON_UNLOCK_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=6 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_SR_WEAPON_TECHS"],
    prerequisites="SHP_ROOT_AGGRESSION",
    unlock=Item(type=UnlockShipPart, name="SR_ARC_DISRUPTOR"),
    effectsgroups=WEAPON_BASE_EFFECTS("SR_ARC_DISRUPTOR"),
    graphic="icons/ship_parts/pulse-laser-1.png",
)

Tech(
    name="SHP_WEAPON_ARC_DISRUPTOR_2",
    description="SHP_WEAPON_ARC_DISRUPTOR_2_DESC",
    short_description="SHIP_WEAPON_IMPROVE_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=60 * TECH_COST_MULTIPLIER,
    researchturns=8,
    tags=["PEDIA_SR_WEAPON_TECHS"],
    prerequisites="SHP_WEAPON_ARC_DISRUPTOR_1",
    effectsgroups=WEAPON_UPGRADE_CAPACITY_EFFECTS("SR_ARC_DISRUPTOR", 2),
    graphic="icons/ship_parts/pulse-laser-2.png",
)


Tech(
    name="SHP_WEAPON_ARC_DISRUPTOR_3",
    description="SHP_WEAPON_ARC_DISRUPTOR_3_DESC",
    short_description="SHIP_WEAPON_IMPROVE_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=360 * TECH_COST_MULTIPLIER,
    researchturns=12,
    tags=["PEDIA_SR_WEAPON_TECHS"],
    prerequisites="SHP_WEAPON_ARC_DISRUPTOR_2",
    effectsgroups=WEAPON_UPGRADE_CAPACITY_EFFECTS("SR_ARC_DISRUPTOR", 3),
    graphic="icons/ship_parts/pulse-laser-3.png",
)
