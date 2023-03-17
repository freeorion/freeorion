from common.misc import PLANET_DEFENSE_FACTOR
from common.priorities import (
    AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
    TARGET_AFTER_2ND_SCALING_PRIORITY,
)

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
        SetTargetHappiness(value=Value + NamedRealLookup(name="PROTECION_FOCUS_STABILITY_BONUS")),
    ],
)


AVERAGE_PLANETARY_DEFENSE = [
    PROTECTION_FOCUS_DEFENSE,
    STANDARD_DEFENSE_GROWTH,
]
