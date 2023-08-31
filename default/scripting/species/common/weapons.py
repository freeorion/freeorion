from common.misc import FIGHTER_DAMAGE_FACTOR, SHIP_WEAPON_DAMAGE_FACTOR
from focs._effects import Armed, DesignHasPart, EffectsGroup, IsSource, SetMaxDamage, SetMaxSecondaryStat, Ship, Value

BAD_WEAPONS = [
    EffectsGroup(
        description="BAD_WEAPONS_DESC",
        scope=IsSource,
        activation=Ship
        & Armed
        & (
            DesignHasPart(name="SR_WEAPON_1_1")
            | DesignHasPart(name="SR_WEAPON_2_1")
            | DesignHasPart(name="SR_WEAPON_3_1")
            | DesignHasPart(name="SR_WEAPON_4_1")
        ),
        effects=[
            SetMaxDamage(partname="SR_WEAPON_1_1", value=Value - SHIP_WEAPON_DAMAGE_FACTOR * 1),
            SetMaxDamage(partname="SR_WEAPON_2_1", value=Value - SHIP_WEAPON_DAMAGE_FACTOR * 2),
            SetMaxDamage(partname="SR_WEAPON_3_1", value=Value - SHIP_WEAPON_DAMAGE_FACTOR * 3),
            SetMaxDamage(partname="SR_WEAPON_4_1", value=Value - SHIP_WEAPON_DAMAGE_FACTOR * 5),
        ],
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Ship
        & Armed
        & (DesignHasPart(name="FT_HANGAR_2") | DesignHasPart(name="FT_HANGAR_3") | DesignHasPart(name="FT_HANGAR_4")),
        # TODO leave a comment why value multiplier are the same, but weaporns have different
        effects=[
            SetMaxSecondaryStat(partname="FT_HANGAR_2", value=Value - FIGHTER_DAMAGE_FACTOR * 1),
            SetMaxSecondaryStat(partname="FT_HANGAR_3", value=Value - FIGHTER_DAMAGE_FACTOR * 1),
            SetMaxSecondaryStat(partname="FT_HANGAR_4", value=Value - FIGHTER_DAMAGE_FACTOR * 1),
        ],
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Ship & Armed & DesignHasPart(name="SR_WEAPON_0_1"),
        effects=[
            SetMaxSecondaryStat(partname="SR_WEAPON_0_1", value=Value - 1),
        ],
    ),
]

GREAT_WEAPONS = [
    EffectsGroup(
        description="GREAT_WEAPONS_DESC",
        scope=IsSource,
        activation=Ship
        & Armed
        & (
            DesignHasPart(name="SR_WEAPON_1_1")
            | DesignHasPart(name="SR_WEAPON_2_1")
            | DesignHasPart(name="SR_WEAPON_3_1")
            | DesignHasPart(name="SR_WEAPON_4_1")
        ),
        effects=[
            SetMaxDamage(partname="SR_WEAPON_1_1", value=Value + SHIP_WEAPON_DAMAGE_FACTOR * 2),
            SetMaxDamage(partname="SR_WEAPON_2_1", value=Value + SHIP_WEAPON_DAMAGE_FACTOR * 4),
            SetMaxDamage(partname="SR_WEAPON_3_1", value=Value + SHIP_WEAPON_DAMAGE_FACTOR * 6),
            SetMaxDamage(partname="SR_WEAPON_4_1", value=Value + SHIP_WEAPON_DAMAGE_FACTOR * 10),
        ],
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Ship
        & Armed
        & (DesignHasPart(name="FT_HANGAR_2") | DesignHasPart(name="FT_HANGAR_3") | DesignHasPart(name="FT_HANGAR_4")),
        effects=[
            SetMaxSecondaryStat(partname="FT_HANGAR_2", value=Value + (FIGHTER_DAMAGE_FACTOR * 3)),
            SetMaxSecondaryStat(partname="FT_HANGAR_3", value=Value + (FIGHTER_DAMAGE_FACTOR * 3)),
            SetMaxSecondaryStat(partname="FT_HANGAR_4", value=Value + (FIGHTER_DAMAGE_FACTOR * 3)),
        ],
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Ship & Armed & DesignHasPart(name="SR_WEAPON_0_1"),
        effects=SetMaxSecondaryStat(partname="SR_WEAPON_0_1", value=Value + 2),
    ),
]

GOOD_WEAPONS = [
    EffectsGroup(
        description="GOOD_WEAPONS_DESC",
        scope=IsSource,
        activation=Ship
        & Armed
        & (
            DesignHasPart(name="SR_WEAPON_1_1")
            | DesignHasPart(name="SR_WEAPON_2_1")
            | DesignHasPart(name="SR_WEAPON_3_1")
            | DesignHasPart(name="SR_WEAPON_4_1")
        ),
        effects=[
            SetMaxDamage(partname="SR_WEAPON_1_1", value=Value + SHIP_WEAPON_DAMAGE_FACTOR * 1),
            SetMaxDamage(partname="SR_WEAPON_2_1", value=Value + SHIP_WEAPON_DAMAGE_FACTOR * 2),
            SetMaxDamage(partname="SR_WEAPON_3_1", value=Value + SHIP_WEAPON_DAMAGE_FACTOR * 3),
            SetMaxDamage(partname="SR_WEAPON_4_1", value=Value + SHIP_WEAPON_DAMAGE_FACTOR * 5),
        ],
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Ship
        & Armed
        & (DesignHasPart(name="FT_HANGAR_2") | DesignHasPart(name="FT_HANGAR_3") | DesignHasPart(name="FT_HANGAR_4")),
        effects=[
            SetMaxSecondaryStat(partname="FT_HANGAR_2", value=Value + FIGHTER_DAMAGE_FACTOR * 1.5),
            SetMaxSecondaryStat(partname="FT_HANGAR_3", value=Value + FIGHTER_DAMAGE_FACTOR * 1.5),
            SetMaxSecondaryStat(partname="FT_HANGAR_4", value=Value + FIGHTER_DAMAGE_FACTOR * 1.5),
        ],
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Ship & Armed & DesignHasPart(name="SR_WEAPON_0_1"),
        effects=SetMaxSecondaryStat(partname="SR_WEAPON_0_1", value=Value + 1),
    ),
]
ULTIMATE_WEAPONS = [
    EffectsGroup(
        description="ULTIMATE_WEAPONS_DESC",
        scope=IsSource,
        activation=Ship
        & Armed
        & (
            DesignHasPart(name="SR_WEAPON_1_1")
            | DesignHasPart(name="SR_WEAPON_2_1")
            | DesignHasPart(name="SR_WEAPON_3_1")
            | DesignHasPart(name="SR_WEAPON_4_1")
        ),
        effects=[
            SetMaxDamage(partname="SR_WEAPON_1_1", value=Value + SHIP_WEAPON_DAMAGE_FACTOR * 3),
            SetMaxDamage(partname="SR_WEAPON_2_1", value=Value + SHIP_WEAPON_DAMAGE_FACTOR * 6),
            SetMaxDamage(partname="SR_WEAPON_3_1", value=Value + SHIP_WEAPON_DAMAGE_FACTOR * 9),
            SetMaxDamage(partname="SR_WEAPON_4_1", value=Value + SHIP_WEAPON_DAMAGE_FACTOR * 15),
        ],
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Ship
        & Armed
        & (DesignHasPart(name="FT_HANGAR_2") | DesignHasPart(name="FT_HANGAR_3") | DesignHasPart(name="FT_HANGAR_4")),
        effects=[
            SetMaxSecondaryStat(partname="FT_HANGAR_2", value=Value + FIGHTER_DAMAGE_FACTOR * 4.5),
            SetMaxSecondaryStat(partname="FT_HANGAR_3", value=Value + FIGHTER_DAMAGE_FACTOR * 4.5),
            SetMaxSecondaryStat(partname="FT_HANGAR_4", value=Value + FIGHTER_DAMAGE_FACTOR * 4.5),
        ],
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Ship & Armed & DesignHasPart(name="SR_WEAPON_0_1"),
        effects=SetMaxSecondaryStat(partname="SR_WEAPON_0_1", value=Value + 3),
    ),
]
