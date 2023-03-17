from common.misc import SHIP_SHIELD_FACTOR
from common.priorities import AFTER_ALL_TARGET_MAX_METERS_PRIORITY, DEFAULT_PRIORITY

STANDARD_SHIP_SHIELDS = [
    EffectsGroup(  # gives human bonuses when AI Aggression set to Beginner
        scope=IsSource,
        activation=IsHuman & (GalaxyMaxAIAggression == 0) & Ship,  # human player, not human species
        accountinglabel="DIFFICULTY",
        priority=DEFAULT_PRIORITY,
        effects=SetMaxShield(value=Value + (1 * SHIP_SHIELD_FACTOR)),
    ),
    EffectsGroup(  # increase to max when not in battle
        scope=IsSource,
        activation=Ship & (LocalCandidate.LastTurnActiveInBattle < CurrentTurn),
        stackinggroup="SHIELD_REGENERATION_EFFECT",
        priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
        effects=SetShield(value=Target.MaxShield),
    ),
]

ULTIMATE_SHIP_SHIELDS = [
    *STANDARD_SHIP_SHIELDS,
    EffectsGroup(
        description="ULTIMATE_SHIP_SHIELD_DESC",
        scope=IsSource & Ship,
        effects=SetMaxShield(
            value=Value
            + (
                1.5
                * SHIP_SHIELD_FACTOR
                * StatisticIf(float, condition=IsSource & DesignHasPart(name="SH_DEFENSE_GRID"))
            )
            + (3.0 * SHIP_SHIELD_FACTOR * StatisticIf(float, condition=IsSource & DesignHasPart(name="SH_DEFLECTOR")))
            + (4.5 * SHIP_SHIELD_FACTOR * StatisticIf(float, condition=IsSource & DesignHasPart(name="SH_PLASMA")))
            + (6.0 * SHIP_SHIELD_FACTOR * StatisticIf(float, condition=IsSource & DesignHasPart(name="SH_MULTISPEC")))
            + (7.5 * SHIP_SHIELD_FACTOR * StatisticIf(float, condition=IsSource & DesignHasPart(name="SH_BLACK")))
        ),
    ),
]
