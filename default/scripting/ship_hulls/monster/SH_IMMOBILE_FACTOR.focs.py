from focs._conditions import (
    All,
    DesignHasHull,
    InSystem,
    IsSource,
    Number,
    Random,
    Ship,
    System,
    Turn,
)
from focs._effects import CreateShip, EffectsGroup
from focs._enums import External, Internal
from focs._ship_hulls import Hull, Slot
from focs._sources import Source
from ship_hulls.monster.monster import MONSTER_SHIELD_REGENERATION
from ship_hulls.ship_hulls import UNOWNED_OWNED_VISION_BONUS

Hull(
    name="SH_IMMOBILE_FACTORY",
    description="SH_IMMOBILE_FACTORY_DESC",
    speed=0,
    fuel=0,
    NoDefaultFuelEffect=True,
    stealth=5,
    structure=1000,
    slots=[
        Slot(type=External, position=(0.20, 0.18)),
        Slot(type=Internal, position=(0.50, 0.26)),
        Slot(type=External, position=(0.80, 0.47)),
    ],
    buildcost=1,
    buildtime=1,
    producible=False,
    tags=["PEDIA_HULL_MONSTER_GUARD"],
    location=All,
    effectsgroups=[
        MONSTER_SHIELD_REGENERATION,
        *UNOWNED_OWNED_VISION_BONUS("WEAK", 10, 10),
        EffectsGroup(
            scope=IsSource,
            activation=~Turn(low=1),
            effects=[
                CreateShip(designname="SM_DRONE"),
                CreateShip(designname="SM_DRONE"),
            ],
        ),
        EffectsGroup(
            scope=System
            & InSystem(id=Source.SystemID)
            # Limits the factory to producing only when there are less than 7 drones in system
            & Number(
                high=7,
                condition=Ship & InSystem(id=Source.SystemID) & DesignHasHull(name="SH_DRONE_BODY"),
            ),
            activation=Random(probability=0.1),
            effects=CreateShip(designname="SM_DRONE"),
        ),
    ],
    icon="",
    graphic="icons/monsters/factory.png",
)
