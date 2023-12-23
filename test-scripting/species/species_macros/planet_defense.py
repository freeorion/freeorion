# // param1: value
# NATIVE_PLANETARY_DEFENSE
# '''EffectsGroup
#             scope = Source
#             activation = And [
#                 Planet
#                 Unowned
#             ]
#             accountinglabel = "NATIVE_PLANETARY_DEFENSE_LABEL"
#             effects = SetMaxDefense value = Value + ( @1@ )
# '''

from macros.misc import PLANET_DEFENSE_FACTOR
from macros.priorities import (
    AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
    TARGET_AFTER_2ND_SCALING_PRIORITY,
)

try:
    from focs._effects import (
        CurrentTurn,
        EffectsGroup,
        Focus,
        GameRule,
        IsSource,
        LocalCandidate,
        MinOf,
        NamedReal,
        Planet,
        SetDefense,
        SetMaxDefense,
        SetTargetHappiness,
        Target,
        Unowned,
        Value,
    )
except ModuleNotFoundError:
    pass


STANDARD_DEFENSE_GROWTH = EffectsGroup(  # increase 1 per turn, up to max
    scope=IsSource,
    activation=Planet() & Unowned & (LocalCandidate.LastTurnAttackedByShip < CurrentTurn),
    accountinglabel="DEF_ROOT_DEFENSE",
    priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
    effects=SetDefense(value=MinOf(float, Value(Target.MaxDefense), Value + PLANET_DEFENSE_FACTOR)),
)


PROTECTION_FOCUS_DEFENSE = EffectsGroup(
    scope=IsSource,
    activation=Planet() & Focus(type=["FOCUS_PROTECTION"]),
    stackinggroup="FOCUS_PROTECTION_DEFENSE_STACK",
    accountinglabel="FOCUS_PROTECTION_LABEL",
    priority=TARGET_AFTER_2ND_SCALING_PRIORITY,
    effects=[
        SetMaxDefense(value=Value * 2),
        SetTargetHappiness(
            value=Value
            + NamedReal(
                name="PROTECION_FOCUS_STABILITY_BONUS",
                value=GameRule(type=float, name="RULE_PROTECTION_FOCUS_STABILITY"),
            )
        ),
    ],
)


AVERAGE_PLANETARY_DEFENSE = [
    PROTECTION_FOCUS_DEFENSE,
    STANDARD_DEFENSE_GROWTH,
]


# #include "../macros/priorities.macros"
