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
from macros.misc import DESCRIPTION_EFFECTSGROUP_MACRO
from macros.multiplier import BAD_MULTIPLIER, GOOD_MULTIPLIER, GREAT_MULTIPLIER, VERY_BAD_MULTIPLIER
from macros.priorities import (
    TARGET_AFTER_SCALING_PRIORITY,
    TARGET_EARLY_BEFORE_SCALING_PRIORITY,
    TARGET_SCALING_PRIORITY,
)

try:
    from focs._types import _EffectGroup
except ImportError:
    _EffectGroup = None  # type: ignore[misc,assignment]

NO_INDUSTRY = DESCRIPTION_EFFECTSGROUP_MACRO("NO_INDUSTRY_DESC")


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


def _industry(tag, default_multiplier) -> list[_EffectGroup]:  # type: ignore[reportGeneralTypeIssues]
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


VERY_BAD_INDUSTRY = _industry("VERY_BAD", VERY_BAD_MULTIPLIER)
BAD_INDUSTRY = _industry("BAD", BAD_MULTIPLIER)
AVERAGE_INDUSTRY = _BASIC_INDUSTRY
GOOD_INDUSTRY = _industry("GOOD", GOOD_MULTIPLIER)
GREAT_INDUSTRY = _industry("GREAT", GREAT_MULTIPLIER)
