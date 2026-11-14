from focs._conditions import IsSource, Stationary, Turn
from focs._effects import EffectsGroup, SetDetection, SetFuel, SetMaxStructure, SetStructure
from focs._sources import LocalCandidate, Source, Target
from focs._value_refs import MinOf, NamedIntegerLookup, NamedRealLookup, Value
from macros.priorities import EARLY_AFTER_ALL_TARGET_MAX_METERS_PRIORITY
from ship_hulls.ship_hulls import REFUEL_MESSAGE

# macros used for organic ships

LIVING_HULL_BASE_FUEL_REGEN = [
    EffectsGroup(
        description="LIVING_HULL_BASE_FUEL_REGEN_DESC",
        scope=IsSource,
        activation=Turn(low=LocalCandidate.ArrivedOnTurn + 1) & Stationary & (Source.Fuel < Source.MaxFuel),
        stackinggroup="BASE_FUEL_REGEN",
        accountinglabel="BASE_FUEL_REGEN_LABEL",
        priority=EARLY_AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
        effects=SetFuel(value=MinOf(float, Target.MaxFuel, Value + NamedRealLookup(name="SHP_LIVING_HULL_FUEL_REGEN"))),
    ),
    REFUEL_MESSAGE(0.3),
]

LIVING_HULL_EFFECTS_GROUPS = [
    EffectsGroup(
        scope=IsSource,
        effects=[
            SetStructure(value=Value + NamedRealLookup(name="SHP_LIVING_HULL_REPAIR")),
            SetDetection(value=Value + 40),
        ],
    ),
    *LIVING_HULL_BASE_FUEL_REGEN,
]


def LIVING_HULL_GROWTH_EFFECTS_GROUP(hull_name: str):
    return EffectsGroup(
        scope=IsSource,
        accountinglabel="ORGANIC_GROWTH",
        effects=SetMaxStructure(
            value=Value
            + MinOf(
                float,
                Source.Age * NamedRealLookup(name=f"{hull_name}_GROWTH_PER_TURN"),
                NamedIntegerLookup(name=f"{hull_name}_GROWTH_STRUCTURE"),
            )
        ),
    )
