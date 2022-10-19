from common.priorities import AFTER_ALL_TARGET_MAX_METERS_PRIORITY

STANDARD_SUPPLY_GROWTH = EffectsGroup(  # increase 1 per turn, up to max
    scope=Source,
    activation=Planet()
    & (LocalCandidate.LastTurnConquered < CurrentTurn)
    & (LocalCandidate.LastTurnAttackedByShip < CurrentTurn),
    priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
    effects=SetSupply(value=MinOf(float, Value(Target.MaxSupply), Value + 1)),
)

AVERAGE_SUPPLY = [
    EffectsGroup(
        scope=Source, activation=Planet(), accountinglabel="AVERAGE_SUPPLY_LABEL", effects=SetMaxSupply(value=Value + 1)
    ),
    STANDARD_SUPPLY_GROWTH,
]
