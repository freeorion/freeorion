from focs._effects import (
    CurrentTurn,
    EffectsGroup,
    IsSource,
    LocalCandidate,
    MinOf,
    Planet,
    SetMaxSupply,
    SetSupply,
    Target,
    Value,
)
from macros.priorities import AFTER_ALL_TARGET_MAX_METERS_PRIORITY

STANDARD_SUPPLY_GROWTH = EffectsGroup(  # increase 1 per turn, up to max
    scope=IsSource,
    activation=Planet()
    & (LocalCandidate.LastTurnConquered < CurrentTurn)
    & (LocalCandidate.LastTurnAttackedByShip < CurrentTurn),
    priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
    effects=SetSupply(value=MinOf(float, Value(Target.MaxSupply), Value + 1)),
)


VERY_BAD_SUPPLY = [
    EffectsGroup(
        description="VERY_BAD_SUPPLY_DESC",
        scope=IsSource,
        activation=Planet(),
        accountinglabel="VERY_BAD_SUPPLY_LABEL",
        effects=SetMaxSupply(value=Value - 1),
    ),
    STANDARD_SUPPLY_GROWTH,
]


BAD_SUPPLY = [
    EffectsGroup(
        description="BAD_SUPPLY_DESC",
        scope=IsSource,
        activation=Planet(),
        accountinglabel="BAD_SUPPLY_LABEL",
        effects=SetMaxSupply(value=Value + 0),
    ),
    STANDARD_SUPPLY_GROWTH,
]


AVERAGE_SUPPLY = [
    EffectsGroup(
        scope=IsSource,
        activation=Planet(),
        accountinglabel="AVERAGE_SUPPLY_LABEL",
        effects=SetMaxSupply(value=Value + 1),
    ),
    STANDARD_SUPPLY_GROWTH,
]

GREAT_SUPPLY = [
    EffectsGroup(
        description="GREAT_SUPPLY_DESC",
        scope=IsSource,
        activation=Planet(),
        accountinglabel="GREAT_SUPPLY_LABEL",
        effects=SetMaxSupply(value=Value + 2),
    ),
    STANDARD_SUPPLY_GROWTH,
]

ULTIMATE_SUPPLY = [
    EffectsGroup(
        description="ULTIMATE_SUPPLY_DESC",
        scope=IsSource,
        activation=Planet(),
        accountinglabel="ULTIMATE_SUPPLY_LABEL",
        effects=SetMaxSupply(value=Value + 3),
    ),
    STANDARD_SUPPLY_GROWTH,
]
