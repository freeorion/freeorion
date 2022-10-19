from common.misc import SHIP_SHIELD_FACTOR
from common.priorities import AFTER_ALL_TARGET_MAX_METERS_PRIORITY, DEFAULT_PRIORITY

STANDARD_SHIP_SHIELDS = [
    EffectsGroup(  # gives human bonuses when AI Aggression set to Beginner
        scope=Source,
        activation=IsHuman & (GalaxyMaxAIAggression == 0) & Ship,  # human player, not human species
        accountinglabel="DIFFICULTY",
        priority=DEFAULT_PRIORITY,
        effects=SetMaxShield(value=Value + (1 * SHIP_SHIELD_FACTOR)),
    ),
    EffectsGroup(  # increase to max when not in battle
        scope=Source,
        activation=Ship & (LocalCandidate.LastTurnActiveInBattle < CurrentTurn),
        stackinggroup="SHIELD_REGENERATION_EFFECT",
        priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
        effects=SetShield(value=Target.MaxShield),
    ),
]
