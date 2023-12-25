from macros.base_prod import RESEARCH_PER_POP
from macros.priorities import (
    TARGET_AFTER_SCALING_PRIORITY,
    TARGET_EARLY_BEFORE_SCALING_PRIORITY,
    TARGET_SCALING_PRIORITY,
)

try:
    from focs._effects import (
        EffectsGroup,
        Focus,
        GalaxyMaxAIAggression,
        Happiness,
        IsHuman,
        IsSource,
        Planet,
        SetTargetResearch,
        Target,
        Value,
    )
except ModuleNotFoundError:
    pass

from macros.multiplier import BAD_MULTIPLIER

# NO_RESEARCH
# '''[[DESCRIPTION_EFFECTSGROUP_MACRO(NO_RESEARCH_DESC)]]'''

BASIC_RESEARCH = [
    EffectsGroup(
        scope=IsSource,
        activation=Planet() & Focus(type=["FOCUS_RESEARCH"]) & Happiness(low=0),
        accountinglabel="FOCUS_RESEARCH_LABEL",
        priority=TARGET_EARLY_BEFORE_SCALING_PRIORITY,
        effects=SetTargetResearch(value=Value + Target.Population * RESEARCH_PER_POP),
    ),
    EffectsGroup(  # gives human bonuses when AI Aggression set to Beginner
        scope=IsSource,
        activation=Planet() & IsHuman & (GalaxyMaxAIAggression == 0),  # human player, not human species
        accountinglabel="DIFFICULTY",
        priority=TARGET_AFTER_SCALING_PRIORITY,
        effects=SetTargetResearch(value=Value * 2),
    ),
]

# VERY_BAD_RESEARCH
# '''[[BASIC_RESEARCH]]

#         EffectsGroup
#             description = "VERY_BAD_RESEARCH_DESC"
#             scope = Source
#             activation = And [
#                 Planet
#                 Focus type = "FOCUS_RESEARCH"
#             ]
#             accountinglabel = "VERY_BAD_RESEARCH_LABEL"
#             priority = [[TARGET_SCALING_PRIORITY]]
#             effects = SetTargetResearch value = Value*[[VERY_BAD_MULTIPLIER]]
# '''

BAD_RESEARCH = [
    *BASIC_RESEARCH,
    EffectsGroup(
        description="BAD_RESEARCH_DESC",
        scope=IsSource,
        activation=Planet() & Focus(type=["FOCUS_RESEARCH"]),
        accountinglabel="BAD_RESEARCH_LABEL",
        priority=TARGET_SCALING_PRIORITY,
        effects=SetTargetResearch(value=Value * BAD_MULTIPLIER),
    ),
]

# AVERAGE_RESEARCH
# '''[[BASIC_RESEARCH]]
# '''

# GOOD_RESEARCH
# '''[[BASIC_RESEARCH]]

#         EffectsGroup
#             description = "GOOD_RESEARCH_DESC"
#             scope = Source
#             activation = And [
#                 Planet
#                 Focus type = "FOCUS_RESEARCH"
#                 Happiness low = 2
#             ]
#             accountinglabel = "GOOD_RESEARCH_LABEL"
#             priority = [[TARGET_SCALING_PRIORITY]]
#             effects = SetTargetResearch value = Value*[[GOOD_MULTIPLIER]]
# '''

# GREAT_RESEARCH
# '''[[BASIC_RESEARCH]]

#         EffectsGroup
#             description = "GREAT_RESEARCH_DESC"
#             scope = Source
#             activation = And [
#                 Planet
#                 Focus type = "FOCUS_RESEARCH"
#                 Happiness low = 0
#             ]
#             accountinglabel = "GREAT_RESEARCH_LABEL"
#             priority = [[TARGET_SCALING_PRIORITY]]
#             effects = SetTargetResearch value = Value*[[GREAT_MULTIPLIER]]
# '''

# ULTIMATE_RESEARCH
# '''[[BASIC_RESEARCH]]

#         EffectsGroup
#             description = "ULTIMATE_RESEARCH_DESC"
#             scope = Source
#             activation = And [
#                 Planet
#                 Focus type = "FOCUS_RESEARCH"
#             ]
#             accountinglabel = "ULTIMATE_RESEARCH_LABEL"
#             priority = [[TARGET_SCALING_PRIORITY]]
#             effects = SetTargetResearch value = Value*[[ULTIMATE_MULTIPLIER]]
# '''
