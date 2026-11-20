from focs._conditions import All, IsSource
from focs._effects import (
    EffectsGroup,
    SetDamage,
    SetMaxDamage,
    SetMaxSecondaryStat,
    SetSecondaryStat,
)
from focs._enums import External, Internal
from focs._ship_hulls import Hull, Slot
from focs._value_refs import Value
from macros.misc_pre import SHIP_WEAPON_DAMAGE_FACTOR
from ship_hulls.monster.monster import MONSTER_SHIELD_REGENERATION
from ship_hulls.ship_hulls import UNOWNED_OWNED_VISION_BONUS

Hull(
    name="SH_GUARD_MONSTER_BODY",
    description="SH_GUARD_MONSTER_BODY_DESC",
    speed=0,
    fuel=0,
    NoDefaultFuelEffect=True,
    stealth=5,
    structure=35,
    slots=[
        Slot(type=External, position=(0.11, 0.59)),
        Slot(type=External, position=(0.42, 0.79)),
        Slot(type=External, position=(0.51, 0.16)),
        Slot(type=External, position=(0.90, 0.27)),
        Slot(type=Internal, position=(0.45, 0.40)),
    ],
    buildcost=1,
    buildtime=2,
    producible=False,
    tags=["UNOWNED_FRIENDLY", "PEDIA_HULL_MONSTER_GUARD"],
    location=All,
    effectsgroups=[
        MONSTER_SHIELD_REGENERATION,
        *UNOWNED_OWNED_VISION_BONUS("MODERATE", 40, 30),
        EffectsGroup(
            scope=IsSource,
            effects=[
                SetMaxSecondaryStat(partname="SR_WEAPON_0_1", value=Value + 2),
                SetMaxDamage(partname="SR_WEAPON_4_1", value=Value + (5 * SHIP_WEAPON_DAMAGE_FACTOR)),
                SetSecondaryStat(partname="SR_WEAPON_0_1", value=Value + 2),
                SetDamage(partname="SR_WEAPON_4_1", value=Value + (5 * SHIP_WEAPON_DAMAGE_FACTOR)),
            ],
        ),
    ],
    icon="",
    graphic="icons/monsters/sentinel.png",
)
