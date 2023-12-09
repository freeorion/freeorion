from macros.base_prod import INDUSTRY_PER_POP
from macros.priorities import (
    TARGET_AFTER_SCALING_PRIORITY,
    TARGET_EARLY_BEFORE_SCALING_PRIORITY,
)

# NO_INDUSTRY
# '''[[DESCRIPTION_EFFECTSGROUP_MACRO(NO_INDUSTRY_DESC)]]'''
try:
    from focs._effects import (
        EffectsGroup,
        Focus,
        GalaxyMaxAIAggression,
        Happiness,
        IsHuman,
        IsSource,
        NamedReal,
        Planet,
        SetTargetIndustry,
        Target,
        TargetIndustry,
        Value,
    )
except ModuleNotFoundError:
    pass


BASIC_INDUSTRY = [
    EffectsGroup(
        scope=IsSource,
        activation=Planet() & TargetIndustry(low=0) & Happiness(low=0) & Focus(type=["FOCUS_INDUSTRY"]),
        accountinglabel="FOCUS_INDUSTRY_LABEL",
        priority=TARGET_EARLY_BEFORE_SCALING_PRIORITY,
        effects=SetTargetIndustry(
            value=Value
            + Target.Population * NamedReal(name="INDUSTRY_FOCUS_TARGET_INDUSTRY_PERPOP", value=1.0 * INDUSTRY_PER_POP)
        ),
    ),
    EffectsGroup(  # gives human bonuses when AI Aggression set to Beginner
        scope=IsSource,
        activation=Planet() & IsHuman & (GalaxyMaxAIAggression == 0),  # human player, not human species
        accountinglabel="DIFFICULTY",
        priority=TARGET_AFTER_SCALING_PRIORITY,
        effects=SetTargetIndustry(value=Value * 2),
    ),
]

# VERY_BAD_INDUSTRY
# '''[[BASIC_INDUSTRY]]

#         EffectsGroup
#             description = "VERY_BAD_INDUSTRY_DESC"
#             scope = Source
#             activation = And [
#                 Planet
#                 TargetIndustry low = 0
#                 Happiness low = 0
#                 Focus type = "FOCUS_INDUSTRY"
#             ]
#             accountinglabel = "VERY_BAD_INDUSTRY_LABEL"
#             priority = [[TARGET_SCALING_PRIORITY]]
#             effects = SetTargetIndustry value = Value
#                         * (NamedReal name = "VERY_BAD_INDUSTRY_TARGET_INDUSTRY_SCALING"
#                                      value = [[VERY_BAD_MULTIPLIER]])
# '''

# BAD_INDUSTRY
# '''[[BASIC_INDUSTRY]]

#         EffectsGroup
#             description = "BAD_INDUSTRY_DESC"
#             scope = Source
#             activation = And [
#                 Planet
#                 TargetIndustry low = 0
#                 Happiness low = 0
#                 Focus type = "FOCUS_INDUSTRY"
#             ]
#             accountinglabel = "BAD_INDUSTRY_LABEL"
#             priority = [[TARGET_SCALING_PRIORITY]]
#             effects = SetTargetIndustry value = Value
#                         * (NamedReal name = "BAD_INDUSTRY_TARGET_INDUSTRY_SCALING"
#                                      value = [[BAD_MULTIPLIER]])
# '''

AVERAGE_INDUSTRY = BASIC_INDUSTRY

# GOOD_INDUSTRY
# '''[[BASIC_INDUSTRY]]

#         EffectsGroup
#             description = "GOOD_INDUSTRY_DESC"
#             scope = Source
#             activation = And [
#                 Planet
#                 TargetIndustry low = 0
#                 Happiness low = 0
#                 Focus type = "FOCUS_INDUSTRY"
#             ]
#             accountinglabel = "GOOD_INDUSTRY_LABEL"
#             priority = [[TARGET_SCALING_PRIORITY]]
#             effects = SetTargetIndustry value = Value
#                         * (NamedReal name = "GOOD_INDUSTRY_TARGET_INDUSTRY_SCALING"
#                                      value = [[GOOD_MULTIPLIER]])
# '''

# GREAT_INDUSTRY
# '''[[BASIC_INDUSTRY]]

#         EffectsGroup
#             description = "GREAT_INDUSTRY_DESC"
#             scope = Source
#             activation = And [
#                 Planet
#                 TargetIndustry low = 0
#                 Happiness low = 0
#                 Focus type = "FOCUS_INDUSTRY"
#             ]
#             accountinglabel = "GREAT_INDUSTRY_LABEL"
#             priority = [[TARGET_SCALING_PRIORITY]]
#             effects = SetTargetIndustry value = Value
#                         * (NamedReal name = "GREAT_INDUSTRY_TARGET_INDUSTRY_SCALING"
#                                      value = [[GREAT_MULTIPLIER]])
# '''

# ULTIMATE_INDUSTRY
# '''[[BASIC_INDUSTRY]]

#         EffectsGroup
#             description = "ULTIMATE_INDUSTRY_DESC"
#             scope = Source
#             activation = And [
#                 Planet
#                 TargetIndustry low = 0
#                 Happiness low = 0
#                 Focus type = "FOCUS_INDUSTRY"
#             ]
#             accountinglabel = "ULTIMATE_INDUSTRY_LABEL"
#             priority = [[TARGET_SCALING_PRIORITY]]
#             effects = SetTargetIndustry value = Value
#                         * (NamedReal name = "ULTIMATE_INDUSTRY_TARGET_INDUSTRY_SCALING"
#                                      value = [[ULTIMATE_MULTIPLIER]])
# '''
