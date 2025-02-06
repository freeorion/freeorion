from focs._effects import (
    DesignHasPart,
    EffectsGroup,
    OwnedBy,
    PartsInShipDesign,
    SetMaxCapacity,
    Ship,
    Source,
    Target,
    Value,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER
from techs.ship_weapons.ship_weapons import HANGAR_UPGRADE_SECONDARY_STAT_EFFECT

Tech(
    name="SHP_FIGHTERS_3",
    description="SHP_FIGHTERS_3_DESC",
    short_description="SHIP_WEAPON_IMPROVE_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=360 * TECH_COST_MULTIPLIER,
    researchturns=9,
    tags=["PEDIA_FIGHTER_TECHS"],
    prerequisites=["SHP_FIGHTERS_2"],
    effectsgroups=[
        EffectsGroup(
            scope=Ship
            & OwnedBy(empire=Source.Owner)
            & (
                DesignHasPart(name="FT_HANGAR_1")
                | DesignHasPart(name="FT_HANGAR_2")
                | DesignHasPart(name="FT_HANGAR_3")
                | DesignHasPart(name="FT_HANGAR_4")
            ),
            accountinglabel="SHP_FIGHTERS_3",
            effects=[
                SetMaxCapacity(
                    partname="FT_HANGAR_1", value=Value + PartsInShipDesign(name="FT_HANGAR_1", design=Target.DesignID)
                ),
                HANGAR_UPGRADE_SECONDARY_STAT_EFFECT("SHP_FIGHTERS_3", "FT_HANGAR_2", 2),
                HANGAR_UPGRADE_SECONDARY_STAT_EFFECT("SHP_FIGHTERS_3", "FT_HANGAR_3", 3),
                HANGAR_UPGRADE_SECONDARY_STAT_EFFECT("SHP_FIGHTERS_3", "FT_HANGAR_4", 6),
            ],
        )
    ],
    graphic="icons/ship_parts/fighter05.png",
)
