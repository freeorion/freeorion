from focs._effects import (
    EffectsGroup,
    Full,
    HasSpecies,
    IsSource,
    OwnedBy,
    Planet,
    SetDetection,
    SetVisibility,
    Ship,
    Source,
    Unowned,
    Value,
)
from macros.priorities import DEFAULT_PRIORITY

BAD_DETECTION = [
    EffectsGroup(
        description="BAD_DETECTION_DESC",
        scope=IsSource,
        activation=~Ship,
        effects=SetDetection(value=Value - 20),
    ),
    EffectsGroup(scope=IsSource, activation=Ship, effects=SetDetection(value=Value - 9)),
]


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


def COMMUNAL_VISION(species: str):
    return EffectsGroup(
        description="COMMUNAL_VISION_DESC",
        scope=Planet()
        & HasSpecies(name=[species])
        & ~OwnedBy(empire=Source.Owner),  # would be redundant to re-assign visbility to own planets
        activation=Planet() & ~Unowned,
        effects=SetVisibility(empire=Source.Owner, visibility=Full),
    )
