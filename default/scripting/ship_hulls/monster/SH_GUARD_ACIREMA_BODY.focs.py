from focs._conditions import All, Fleet, IsSource, Object
from focs._effects import (
    EffectsGroup,
    SetCapacity,
    SetDamage,
    SetMaxCapacity,
    SetMaxDamage,
    SetMaxSecondaryStat,
    SetObstructive,
    SetSecondaryStat,
)
from focs._enums import External, Internal
from focs._ship_hulls import Hull, Slot
from focs._sources import Source
from focs._value_refs import Value
from macros.misc_pre import SHIP_WEAPON_DAMAGE_FACTOR
from ship_hulls.monster.monster import MONSTER_SHIELD_REGENERATION
from ship_hulls.ship_hulls import UNOWNED_OWNED_VISION_BONUS

Hull(
    name="SH_GUARD_ACIREMA_BODY",
    description="SH_GUARD_ACIREMA_BODY_DESC",
    speed=120,
    fuel=0,
    NoDefaultFuelEffect=True,
    stealth=5,
    structure=15,
    slots=[
        Slot(type=External, position=(0.30, 0.20)),
        Slot(type=Internal, position=(0.25, 0.45)),
        Slot(type=External, position=(0.45, 0.40)),
        Slot(type=External, position=(0.55, 0.47)),
        Slot(type=External, position=(0.65, 0.55)),
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
                SetMaxCapacity(partname="SR_ARC_DISRUPTOR", value=Value + (2 * SHIP_WEAPON_DAMAGE_FACTOR)),
                SetCapacity(partname="SR_ARC_DISRUPTOR", value=Value + (2 * SHIP_WEAPON_DAMAGE_FACTOR)),
                SetMaxSecondaryStat(partname="SR_ARC_DISRUPTOR", value=Value + 1),
                SetSecondaryStat(partname="SR_ARC_DISRUPTOR", value=Value + 1),
                SetMaxDamage(partname="SR_WEAPON_3_1", value=Value + (3 * SHIP_WEAPON_DAMAGE_FACTOR)),
                SetDamage(partname="SR_WEAPON_3_1", value=Value + (3 * SHIP_WEAPON_DAMAGE_FACTOR)),
            ],
        ),
        EffectsGroup(
            scope=Object(id=Source.FleetID) & Fleet,
            effects=SetObstructive,
        ),
    ],
    icon="",
    graphic="hulls_design/energy_frigate_hull.png",
)
