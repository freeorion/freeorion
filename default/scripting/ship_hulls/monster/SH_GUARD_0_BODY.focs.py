from focs._conditions import All, IsSource
from focs._effects import (
    EffectsGroup,
    SetCapacity,
    SetMaxCapacity,
    SetMaxSecondaryStat,
    SetSecondaryStat,
)
from focs._enums import External, Internal
from focs._ship_hulls import Hull, Slot
from macros.misc_pre import SHIP_WEAPON_DAMAGE_FACTOR
from ship_hulls.monster.monster import MONSTER_SHIELD_REGENERATION
from ship_hulls.ship_hulls import UNOWNED_OWNED_VISION_BONUS

Hull(
    name="SH_GUARD_0_BODY",
    description="SH_GUARD_0_BODY_DESC",
    speed=0,
    fuel=0,
    NoDefaultFuelEffect=True,
    stealth=5,
    structure=25,
    slots=[
        Slot(type=External, position=(0.20, 0.67)),
        Slot(type=External, position=(0.74, 0.17)),
        Slot(type=External, position=(0.74, 0.42)),
        Slot(type=Internal, position=(0.50, 0.35)),
    ],
    buildcost=1,
    buildtime=2,
    producible=False,
    tags=["UNOWNED_FRIENDLY", "PEDIA_HULL_MONSTER_GUARD"],
    location=All,
    effectsgroups=[
        MONSTER_SHIELD_REGENERATION,
        *UNOWNED_OWNED_VISION_BONUS("WEAK", 10, 10),
        # weaken damage and distribute shots so it is possible to survive using a half armoured starting level colony ship
        EffectsGroup(
            scope=IsSource,
            effects=[
                SetMaxCapacity(partname="SR_ARC_DISRUPTOR", value=SHIP_WEAPON_DAMAGE_FACTOR),
                SetCapacity(partname="SR_ARC_DISRUPTOR", value=SHIP_WEAPON_DAMAGE_FACTOR),
                SetMaxSecondaryStat(partname="SR_ARC_DISRUPTOR", value=5),
                SetSecondaryStat(partname="SR_ARC_DISRUPTOR", value=5),
            ],
        ),
    ],
    icon="",
    graphic="icons/monsters/maintenance_ship.png",
)
