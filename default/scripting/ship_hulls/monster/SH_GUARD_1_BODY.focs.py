from focs._conditions import All
from focs._enums import External
from focs._ship_hulls import Hull, Slot
from ship_hulls.monster.monster import MONSTER_SHIELD_REGENERATION
from ship_hulls.ship_hulls import UNOWNED_OWNED_VISION_BONUS

Hull(
    name="SH_GUARD_1_BODY",
    description="SH_GUARD_1_BODY_DESC",
    speed=0,
    fuel=0,
    NoDefaultFuelEffect=True,
    stealth=5,
    structure=35,
    slots=[
        Slot(type=External, position=(0.20, 0.65)),
        Slot(type=External, position=(0.46, 0.25)),
        Slot(type=External, position=(0.83, 0.35)),
    ],
    buildcost=1,
    buildtime=2,
    producible=False,
    tags=["UNOWNED_FRIENDLY", "PEDIA_HULL_MONSTER_GUARD"],
    location=All,
    effectsgroups=[
        MONSTER_SHIELD_REGENERATION,
        *UNOWNED_OWNED_VISION_BONUS("POOR", 20, 20),
    ],
    icon="",
    graphic="icons/monsters/sentry.png",
)
