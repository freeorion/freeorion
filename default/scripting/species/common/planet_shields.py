from common.misc import PLANET_SHIELD_FACTOR
from common.priorities import (
    AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
    TARGET_AFTER_2ND_SCALING_PRIORITY,
)

STANDARD_SHIELD_GROWTH = EffectsGroup(  # increase 1 per turn, up to max
    scope=Source,
    activation=Planet()
    & Unowned
    & (LocalCandidate.LastTurnConquered < CurrentTurn)
    & (LocalCandidate.LastTurnAttackedByShip < CurrentTurn),
    accountinglabel="DEF_ROOT_DEFENSE",
    priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
    effects=SetShield(value=MinOf(float, Value(Target.MaxShield), Value + PLANET_SHIELD_FACTOR)),
)


PROTECTION_FOCUS_SHIELDS = EffectsGroup(
    scope=Source,
    activation=Planet() & Focus(type=["FOCUS_PROTECTION"]),
    stackinggroup="FOCUS_PROTECTION_SHIELDS_STACK",
    accountinglabel="FOCUS_PROTECTION_LABEL",
    priority=TARGET_AFTER_2ND_SCALING_PRIORITY,
    effects=SetMaxShield(value=Value * 2),
)


AVERAGE_PLANETARY_SHIELDS = [PROTECTION_FOCUS_SHIELDS, STANDARD_SHIELD_GROWTH]
