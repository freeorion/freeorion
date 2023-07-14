from common.priorities import DEFAULT_PRIORITY
from focs._effects import (
    EffectsGroup,
    IsSource,
    Planet,
    SetDetection,
    Ship,
    Unowned,
    Value,
)

GOOD_DETECTION = [
    EffectsGroup(
        description="GOOD_DETECTION_DESC",
        scope=IsSource,
        effects=SetDetection(value=Value + 25),
    ),
]

GREAT_DETECTION = [
    EffectsGroup(
        description="GREAT_DETECTION_DESC",
        scope=IsSource,
        effects=SetDetection(value=Value + 50),
    ),
]


ULTIMATE_DETECTION = [
    EffectsGroup(
        description="ULTIMATE_DETECTION_DESC",
        scope=IsSource,
        effects=SetDetection(value=Value + 100),
    )
]


def NATIVE_PLANETARY_DETECTION(detection):
    return EffectsGroup(
        scope=IsSource,
        activation=Planet() & Unowned,
        accountinglabel="NATIVE_PLANETARY_DETECTION_LABEL",
        priority=DEFAULT_PRIORITY,
        effects=SetDetection(value=Value + detection),
    )


BAD_DETECTION = [
    EffectsGroup(
        description="BAD_DETECTION_DESC",
        scope=IsSource,
        activation=~Ship,
        effects=SetDetection(value=Value - 20),
    ),
    EffectsGroup(scope=IsSource, activation=Ship, effects=SetDetection(value=Value - 9)),
]
