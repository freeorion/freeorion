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
from macros.base_prod import INDUSTRY_PER_POP
from macros.priorities import (
    TARGET_AFTER_SCALING_PRIORITY,
    TARGET_EARLY_BEFORE_SCALING_PRIORITY,
    TARGET_SCALING_PRIORITY,
)

_BASIC_INDUSTRY = [
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


def _industry(tag, default_multiplier):
    return [
        *_BASIC_INDUSTRY,
        EffectsGroup(
            description=f"{tag}_INDUSTRY_DESC",
            scope=IsSource,
            activation=Planet() & TargetIndustry(low=0) & Happiness(low=0) & Focus(type=["FOCUS_INDUSTRY"]),
            accountinglabel=f"{tag}_INDUSTRY_LABEL",
            priority=TARGET_SCALING_PRIORITY,
            effects=SetTargetIndustry(
                value=Value * NamedReal(name=f"{tag}_INDUSTRY_TARGET_INDUSTRY_SCALING", value=default_multiplier)
            ),
        ),
    ]
