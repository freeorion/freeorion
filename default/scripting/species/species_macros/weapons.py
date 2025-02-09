from focs._effects import (
    Armed,
    DesignHasPart,
    EffectsGroup,
    IsSource,
    NamedReal,
    SetMaxDamage,
    SetMaxSecondaryStat,
    Ship,
    Value,
)
from macros.misc import FIGHTER_DAMAGE_FACTOR, SHIP_WEAPON_DAMAGE_FACTOR


def _weapon(*, tag: str, tier_0: int, tier_1: int, tier_2: int, tier_3: int, tier_4: int, hangar: float):
    return [
        EffectsGroup(
            description=f"{tag}_WEAPONS_DESC",
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
                SetMaxDamage(partname="SR_WEAPON_1_1", value=Value + SHIP_WEAPON_DAMAGE_FACTOR * tier_1),
                SetMaxDamage(partname="SR_WEAPON_2_1", value=Value + SHIP_WEAPON_DAMAGE_FACTOR * tier_2),
                SetMaxDamage(partname="SR_WEAPON_3_1", value=Value + SHIP_WEAPON_DAMAGE_FACTOR * tier_3),
                SetMaxDamage(partname="SR_WEAPON_4_1", value=Value + SHIP_WEAPON_DAMAGE_FACTOR * tier_4),
            ],
        ),
        EffectsGroup(
            scope=IsSource,
            activation=Ship
            & Armed
            & (
                DesignHasPart(name="FT_HANGAR_2")
                | DesignHasPart(name="FT_HANGAR_3")
                | DesignHasPart(name="FT_HANGAR_4")
            ),
            # TODO leave a comment why value multiplier are the same, but weaporns have different
            effects=[
                SetMaxSecondaryStat(
                    partname="FT_HANGAR_2",
                    value=Value
                    + NamedReal(name=tag + "_PILOT_FIGHTER_DAMAGE_BONUS", value=FIGHTER_DAMAGE_FACTOR * hangar),
                ),
                SetMaxSecondaryStat(partname="FT_HANGAR_3", value=Value + FIGHTER_DAMAGE_FACTOR * hangar),
                SetMaxSecondaryStat(partname="FT_HANGAR_4", value=Value + FIGHTER_DAMAGE_FACTOR * hangar),
            ],
        ),
        EffectsGroup(
            scope=IsSource,
            activation=Ship & Armed & DesignHasPart(name="SR_WEAPON_0_1"),
            effects=[
                SetMaxSecondaryStat(partname="SR_WEAPON_0_1", value=Value + tier_0),
            ],
        ),
    ]


BAD_WEAPONS = _weapon(tag="BAD", tier_0=-1, tier_1=-1, tier_2=-2, tier_3=-3, tier_4=-5, hangar=-1.0)
GOOD_WEAPONS = _weapon(tag="GOOD", tier_0=1, tier_1=1, tier_2=2, tier_3=3, tier_4=5, hangar=1.5)
GREAT_WEAPONS = _weapon(tag="GREAT", tier_0=2, tier_1=2, tier_2=4, tier_3=6, tier_4=10, hangar=3.0)
ULTIMATE_WEAPONS = _weapon(tag="ULTIMATE", tier_0=3, tier_1=3, tier_2=6, tier_3=9, tier_4=15, hangar=4.5)
