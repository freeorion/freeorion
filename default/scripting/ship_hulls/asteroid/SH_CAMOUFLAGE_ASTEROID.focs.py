from focs._conditions import (
    ContainedBy,
    Contains,
    IsBuilding,
    IsSource,
    Object,
    OwnedBy,
    Planet,
    System,
)
from focs._effects import EffectsGroup, SetStealth
from focs._enums import AsteroidsType, Internal
from focs._ship_hulls import Hull, Slot
from focs._sources import Source
from focs._value_refs import Value
from macros.upkeep import FLEET_UPKEEP_MULTIPLICATOR, SHIP_HULL_COST_MULTIPLIER
from ship_hulls.ship_hulls import (
    ADD_HULL_FUEL_TO_MAX_FUEL_METER,
    AVERAGE_BASE_FUEL_REGEN,
    AVERAGE_FUEL_EFFICIENCY,
    REGULAR_HULL_DETECTION,
    SCAVANGE_FUEL_UNOWNED,
    UNOWNED_GOOD_VISION,
    UNOWNED_MOVE,
)

Hull(
    name="SH_CAMOUFLAGE_ASTEROID",
    description="SH_CAMOUFLAGE_ASTEROID_DESC",
    speed=60,
    fuel=2,
    NoDefaultFuelEffect=True,
    stealth=25,
    structure=40,
    slots=[
        Slot(type=Internal, position=(0.25, 0.45)),
        Slot(type=Internal, position=(0.40, 0.45)),
        Slot(type=Internal, position=(0.55, 0.45)),
        Slot(type=Internal, position=(0.70, 0.45)),
    ],
    buildcost=16.0 * FLEET_UPKEEP_MULTIPLICATOR * SHIP_HULL_COST_MULTIPLIER,
    buildtime=2,
    tags=["ASTEROID_HULL", "PEDIA_HULL_LINE_ASTEROIDS", "AVERAGE_FUEL_EFFICIENCY"],
    location=Contains(IsBuilding(name="BLD_SHIPYARD_BASE") & OwnedBy(empire=Source.Owner))
    & ContainedBy(System & Contains(IsBuilding(name="BLD_SHIPYARD_AST") & OwnedBy(empire=Source.Owner))),
    effectsgroups=[
        EffectsGroup(
            scope=IsSource,
            activation=ContainedBy(Object(id=Source.SystemID) & Contains(Planet(type=[AsteroidsType]))),
            accountinglabel="ASTEROID_FIELD_STEALTH",
            effects=SetStealth(value=Value + 40),
        ),
        *AVERAGE_FUEL_EFFICIENCY,
        ADD_HULL_FUEL_TO_MAX_FUEL_METER,
        *AVERAGE_BASE_FUEL_REGEN,
        REGULAR_HULL_DETECTION,
        SCAVANGE_FUEL_UNOWNED,
        UNOWNED_GOOD_VISION,
        UNOWNED_MOVE,
    ],
    icon="icons/ship_hulls/camouflage_asteroid_hull_small.png",
    graphic="hulls_design/camouflage_asteroid_hull.png",
)
