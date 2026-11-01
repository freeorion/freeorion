from focs._conditions import (
    Contains,
    InSystem,
    IsBuilding,
    IsSource,
    OwnedBy,
    Planet,
    Ship,
    Star,
    WithinDistance,
)
from focs._effects import EffectsGroup, SetFuel, SetStealth
from focs._enums import (
    AllyOf,
    BlackHole,
    Blue,
    Core,
    External,
    Internal,
    Orange,
    Red,
    White,
    Yellow,
)
from focs._ship_hulls import Hull, Slot
from focs._sources import Source, Target
from focs._value_refs import Value
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
    name="SH_SOLAR",
    description="SH_SOLAR_DESC",
    speed=120,
    fuel=1.5,
    NoDefaultFuelEffect=True,
    stealth=-55,
    structure=100,
    slots=[
        Slot(type=External, position=(0.10, 0.15)),
        Slot(type=External, position=(0.20, 0.15)),
        Slot(type=External, position=(0.30, 0.15)),
        Slot(type=External, position=(0.40, 0.15)),
        Slot(type=External, position=(0.50, 0.15)),
        Slot(type=External, position=(0.60, 0.15)),
        Slot(type=External, position=(0.70, 0.15)),
        Slot(type=External, position=(0.80, 0.15)),
        # Slot type = External position = (0.90, 0.15)
        Slot(type=External, position=(0.10, 0.85)),
        Slot(type=External, position=(0.20, 0.85)),
        Slot(type=External, position=(0.30, 0.85)),
        Slot(type=External, position=(0.40, 0.85)),
        Slot(type=External, position=(0.50, 0.85)),
        Slot(type=External, position=(0.60, 0.85)),
        Slot(type=External, position=(0.70, 0.85)),
        Slot(type=External, position=(0.80, 0.85)),
        # Slot type = External position = (0.90, 0.85)
        Slot(type=Internal, position=(0.20, 0.40)),
        Slot(type=Internal, position=(0.20, 0.60)),
        Slot(type=Internal, position=(0.35, 0.40)),
        Slot(type=Internal, position=(0.35, 0.60)),
        # Slot type = Internal position = (0.65, 0.40)
        # Slot type = Internal position = (0.65, 0.60)
        # Slot type = Internal position = (0.80, 0.40)
        # Slot type = Internal position = (0.80, 0.60)
        Slot(type=Core, position=(0.50, 0.50)),
    ],
    buildcost=250.0 * FLEET_UPKEEP_MULTIPLICATOR * SHIP_HULL_COST_MULTIPLIER,
    buildtime=8,
    tags=["ENERGY_HULL", "PEDIA_HULL_LINE_ENERGY", "BAD_FUEL_EFFICIENCY"],
    location=(
        Planet()
        & Contains(IsBuilding(name="BLD_SHIPYARD_ENRG_COMP") & OwnedBy(empire=Source.Owner))
        & Contains(IsBuilding(name="BLD_SHIPYARD_ENRG_SOLAR") & OwnedBy(empire=Source.Owner))
        & Contains(IsBuilding(name="BLD_SHIPYARD_BASE") & OwnedBy(empire=Source.Owner))
        & OwnedBy(empire=Source.Owner)
        & Star(type=[BlackHole])
    ),
    effectsgroups=[
        EffectsGroup(
            scope=IsSource,
            activation=Star(type=[Blue, White, Yellow, Orange, Red]),
            stackinggroup="STEALTH_SOLAR_STACK",
            effects=SetStealth(value=Value + 120),
        ),
        EffectsGroup(
            scope=(
                Ship
                & InSystem(id=Source.SystemID)
                & (OwnedBy(empire=Source.Owner) | OwnedBy(affiliation=AllyOf, empire=Source.Owner))
            ),
            activation=InSystem(),
            effects=SetFuel(value=Target.MaxFuel),
        ),
        EffectsGroup(
            scope=(
                WithinDistance(distance=0, condition=IsSource)
                & Ship
                & ~(OwnedBy(empire=Source.Owner) | OwnedBy(affiliation=AllyOf, empire=Source.Owner))
            ),
            stackinggroup="FLAGSHIP_EFFECT_SOLAR",
            effects=SetStealth(value=Value - 100),
        ),
        *BAD_FUEL_EFFICIENCY,
        ADD_HULL_FUEL_TO_MAX_FUEL_METER,
        *AVERAGE_BASE_FUEL_REGEN,
        REGULAR_HULL_DETECTION,
        SCAVANGE_FUEL_UNOWNED,
        UNOWNED_GOOD_VISION,
        UNOWNED_MOVE,
    ],
    icon="icons/ship_hulls/solar_hull_small.png",
    graphic="hulls_design/solar_hull.png",
)
