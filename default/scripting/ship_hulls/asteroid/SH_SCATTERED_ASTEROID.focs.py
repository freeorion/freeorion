from focs._conditions import (
    ContainedBy,
    Contains,
    IsBuilding,
    IsSource,
    Object,
    OwnedBy,
    Planet,
    Ship,
    System,
    WithinDistance,
)
from focs._effects import EffectsGroup, SetMaxShield, SetStealth
from focs._enums import AllyOf, AsteroidsType, Core, External, Internal
from focs._ship_hulls import Hull, Slot
from focs._sources import Source
from focs._value_refs import Value
from macros.misc_pre import SHIP_SHIELD_FACTOR
from macros.upkeep import FLEET_UPKEEP_MULTIPLICATOR, SHIP_HULL_COST_MULTIPLIER
from ship_hulls.ship_hulls import (
    ADD_HULL_FUEL_TO_MAX_FUEL_METER,
    AVERAGE_BASE_FUEL_REGEN,
    BAD_FUEL_EFFICIENCY,
    REGULAR_HULL_DETECTION,
    SCAVANGE_FUEL_UNOWNED,
    UNOWNED_GOOD_VISION,
    UNOWNED_MOVE,
)

Hull(
    name="SH_SCATTERED_ASTEROID",
    description="SH_SCATTERED_ASTEROID_DESC",
    speed=60,
    fuel=1.2,
    NoDefaultFuelEffect=True,
    stealth=5,
    structure=140,
    slots=[
        Slot(type=External, position=(0.20, 0.15)),
        Slot(type=External, position=(0.30, 0.15)),
        Slot(type=External, position=(0.40, 0.15)),
        Slot(type=External, position=(0.50, 0.15)),
        Slot(type=External, position=(0.60, 0.15)),
        Slot(type=External, position=(0.70, 0.15)),
        # Slot type = External position = (0.80, 0.15)
        Slot(type=External, position=(0.85, 0.50)),
        Slot(type=External, position=(0.20, 0.85)),
        Slot(type=External, position=(0.30, 0.85)),
        Slot(type=External, position=(0.40, 0.85)),
        Slot(type=External, position=(0.50, 0.85)),
        Slot(type=External, position=(0.60, 0.85)),
        Slot(type=External, position=(0.70, 0.85)),
        # Slot type = External position = (0.80, 0.85)
        Slot(type=Internal, position=(0.20, 0.50)),
        Slot(type=Internal, position=(0.30, 0.50)),
        Slot(type=Internal, position=(0.60, 0.50)),
        Slot(type=Internal, position=(0.70, 0.50)),
        Slot(type=Core, position=(0.45, 0.50)),
    ],
    buildcost=160.0 * FLEET_UPKEEP_MULTIPLICATOR * SHIP_HULL_COST_MULTIPLIER,
    buildtime=8,
    tags=["ASTEROID_HULL", "PEDIA_HULL_LINE_ASTEROIDS", "BAD_FUEL_EFFICIENCY"],
    location=Planet()
    & Contains(IsBuilding(name="BLD_SHIPYARD_BASE") & OwnedBy(empire=Source.Owner))
    & ContainedBy(System & Contains(IsBuilding(name="BLD_SHIPYARD_AST") & OwnedBy(empire=Source.Owner)))
    & ContainedBy(System & Contains(IsBuilding(name="BLD_SHIPYARD_AST_REF") & OwnedBy(empire=Source.Owner))),
    effectsgroups=[
        EffectsGroup(
            scope=IsSource,
            activation=ContainedBy(Object(id=Source.SystemID) & Contains(Planet(type=[AsteroidsType]))),
            accountinglabel="ASTEROID_FIELD_STEALTH",
            effects=SetStealth(value=Value + 40),
        ),
        EffectsGroup(
            scope=Ship
            & (OwnedBy(empire=Source.Owner) | OwnedBy(affiliation=AllyOf, empire=Source.Owner))
            & WithinDistance(distance=0, condition=IsSource),
            stackinggroup="FLAGSHIP_EFFECT_SCATTERED_ASTEROID",
            effects=SetMaxShield(value=Value + (3 * SHIP_SHIELD_FACTOR)),
        ),
        *BAD_FUEL_EFFICIENCY,
        ADD_HULL_FUEL_TO_MAX_FUEL_METER,
        *AVERAGE_BASE_FUEL_REGEN,
        REGULAR_HULL_DETECTION,
        SCAVANGE_FUEL_UNOWNED,
        UNOWNED_GOOD_VISION,
        UNOWNED_MOVE,
    ],
    icon="icons/ship_hulls/scattered_asteroid_hull_small.png",
    graphic="hulls_design/scattered_asteroid_hull.png",
)
