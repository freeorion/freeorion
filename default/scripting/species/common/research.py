from common.base_prod import RESEARCH_PER_POP
from common.priorities import (
    TARGET_AFTER_SCALING_PRIORITY,
    TARGET_EARLY_BEFORE_SCALING_PRIORITY,
    TARGET_SCALING_PRIORITY,
)
from species.common.multiplier import BAD_MULTIPLIER

BASIC_RESEARCH = [
    EffectsGroup(
        scope=Source,
        activation=Planet() & Focus(type=["FOCUS_RESEARCH"]) & Happiness(low=0),
        accountinglabel="FOCUS_RESEARCH_LABEL",
        priority=TARGET_EARLY_BEFORE_SCALING_PRIORITY,
        effects=SetTargetResearch(value=Value + Target.Population * RESEARCH_PER_POP),
    ),
    EffectsGroup(  # gives human bonuses when AI Aggression set to Beginner
        scope=Source,
        activation=Planet() & IsHuman & (GalaxyMaxAIAggression == 0),  # human player, not human species
        accountinglabel="DIFFICULTY",
        priority=TARGET_AFTER_SCALING_PRIORITY,
        effects=SetTargetResearch(value=Value * 2),
    ),
]

BAD_RESEARCH = [
    *BASIC_RESEARCH,
    EffectsGroup(
        description="BAD_RESEARCH_DESC",
        scope=Source,
        activation=Planet() & Focus(type=["FOCUS_RESEARCH"]),
        accountinglabel="BAD_RESEARCH_LABEL",
        priority=TARGET_SCALING_PRIORITY,
        effects=SetTargetResearch(value=Value * BAD_MULTIPLIER),
    ),
]
