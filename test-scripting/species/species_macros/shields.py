from macros.misc import SHIP_SHIELD_FACTOR
from macros.priorities import AFTER_ALL_TARGET_MAX_METERS_PRIORITY, DEFAULT_PRIORITY

try:
    from focs._effects import (
        CurrentTurn,
        EffectsGroup,
        GalaxyMaxAIAggression,
        IsHuman,
        IsSource,
        LocalCandidate,
        SetMaxShield,
        SetShield,
        Ship,
        Target,
        Value,
    )
except ModuleNotFoundError:
    pass


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

# BAD_SHIP_SHIELDS
# '''[[STANDARD_SHIP_SHIELDS]]

# EffectsGroup
#             description = "BAD_SHIP_SHIELD_DESC"
#             scope = And [
#                 Source
#                 Ship
#             ]
#             effects = SetMaxShield value = Value
#                 - (0.5 * [[SHIP_SHIELD_FACTOR]] * (Statistic If condition = And [ Source DesignHasPart name = "SH_DEFENSE_GRID"]))
#                 - (1.0 * [[SHIP_SHIELD_FACTOR]] * (Statistic If condition = And [ Source DesignHasPart name = "SH_DEFLECTOR"]))
#                 - (1.5 * [[SHIP_SHIELD_FACTOR]] * (Statistic If condition = And [ Source DesignHasPart name = "SH_PLASMA"]))
#                 - (2.0 * [[SHIP_SHIELD_FACTOR]] * (Statistic If condition = And [ Source DesignHasPart name = "SH_MULTISPEC"]))
#                 - (2.5 * [[SHIP_SHIELD_FACTOR]] * (Statistic If condition = And [ Source DesignHasPart name = "SH_BLACK"]))
# '''

# GOOD_SHIP_SHIELDS
# '''[[STANDARD_SHIP_SHIELDS]]

# EffectsGroup
#             description = "GOOD_SHIP_SHIELD_DESC"
#             scope = And [
#                 Source
#                 Ship
#             ]
#             effects = SetMaxShield value = Value
#                 + (0.5 * [[SHIP_SHIELD_FACTOR]] * (Statistic If condition = And [ Source DesignHasPart name = "SH_DEFENSE_GRID"]))
#                 + (1.0 * [[SHIP_SHIELD_FACTOR]] * (Statistic If condition = And [ Source DesignHasPart name = "SH_DEFLECTOR"]))
#                 + (1.5 * [[SHIP_SHIELD_FACTOR]] * (Statistic If condition = And [ Source DesignHasPart name = "SH_PLASMA"]))
#                 + (2.0 * [[SHIP_SHIELD_FACTOR]] * (Statistic If condition = And [ Source DesignHasPart name = "SH_MULTISPEC"]))
#                 + (2.5 * [[SHIP_SHIELD_FACTOR]] * (Statistic If condition = And [ Source DesignHasPart name = "SH_BLACK"]))
# '''

# GREAT_SHIP_SHIELDS
# '''[[STANDARD_SHIP_SHIELDS]]

# EffectsGroup
#             description = "GREAT_SHIP_SHIELD_DESC"
#             scope = And [
#                 Source
#                 Ship
#             ]
#             effects = SetMaxShield value = Value
#                 + (1.0 * [[SHIP_SHIELD_FACTOR]] * (Statistic If condition = And [ Source DesignHasPart name = "SH_DEFENSE_GRID"]))
#                 + (2.0 * [[SHIP_SHIELD_FACTOR]] * (Statistic If condition = And [ Source DesignHasPart name = "SH_DEFLECTOR"]))
#                 + (3.0 * [[SHIP_SHIELD_FACTOR]] * (Statistic If condition = And [ Source DesignHasPart name = "SH_PLASMA"]))
#                 + (4.0 * [[SHIP_SHIELD_FACTOR]] * (Statistic If condition = And [ Source DesignHasPart name = "SH_MULTISPEC"]))
#                 + (5.0 * [[SHIP_SHIELD_FACTOR]] * (Statistic If condition = And [ Source DesignHasPart name = "SH_BLACK"]))
# '''

# ULTIMATE_SHIP_SHIELDS
# '''[[STANDARD_SHIP_SHIELDS]]

# EffectsGroup
#             description = "ULTIMATE_SHIP_SHIELD_DESC"
#             scope = And [
#                 Source
#                 Ship
#             ]
#             effects = SetMaxShield value = Value
#                 + (1.5 * [[SHIP_SHIELD_FACTOR]] * (Statistic If condition = And [ Source DesignHasPart name = "SH_DEFENSE_GRID"]))
#                 + (3.0 * [[SHIP_SHIELD_FACTOR]] * (Statistic If condition = And [ Source DesignHasPart name = "SH_DEFLECTOR"]))
#                 + (4.5 * [[SHIP_SHIELD_FACTOR]] * (Statistic If condition = And [ Source DesignHasPart name = "SH_PLASMA"]))
#                 + (6.0 * [[SHIP_SHIELD_FACTOR]] * (Statistic If condition = And [ Source DesignHasPart name = "SH_MULTISPEC"]))
#                 + (7.5 * [[SHIP_SHIELD_FACTOR]] * (Statistic If condition = And [ Source DesignHasPart name = "SH_BLACK"]))
# '''

# #include "../macros/misc.macros"
