from focs._effects import (
    Contains,
    DirectDistanceBetween,
    EffectsGroup,
    Focus,
    Happiness,
    IsSource,
    LocalCandidate,
    Max,
    MaxOf,
    MinOf,
    NamedReal,
    OwnedBy,
    Planet,
    SetEmpireMeter,
    SetTargetResearch,
    Source,
    Statistic,
    System,
    Target,
    Value,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER
from macros.priorities import TARGET_AFTER_2ND_SCALING_PRIORITY

Tech(
    name="LRN_DISTRIB_THOUGHT",
    description="LRN_DISTRIB_THOUGHT_DESC",
    short_description="RESEARCH_AND_SLOT_SHORT_DESC",
    category="LEARNING_CATEGORY",
    researchcost=120 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_LEARNING_CATEGORY"],
    prerequisites=["LRN_PSIONICS"],
    effectsgroups=[
        EffectsGroup(
            scope=Planet()
            & OwnedBy(empire=Source.Owner)
            & Focus(type=["FOCUS_RESEARCH"])
            & Happiness(low=NamedReal(name="LRN_DISTRIB_THOUGHT_MIN_STABILITY", value=10)),
            priority=TARGET_AFTER_2ND_SCALING_PRIORITY,
            effects=SetTargetResearch(
                value=Value
                + MaxOf(
                    float,
                    0.0,
                    MinOf(
                        float,
                        Target.Construction * NamedReal(name="DISTRIB_THOUGH_INFRA_LIMIT_SCALING", value=0.5),
                        NamedReal(name="DISTRIB_THOUGH_RESEARCH_SCALING", value=0.1)
                        * Statistic(
                            float,
                            Max,
                            value=DirectDistanceBetween(Target.ID, LocalCandidate.ID),
                            condition=System
                            & Contains(Planet() & OwnedBy(empire=Source.Owner) & Focus(type=["FOCUS_RESEARCH"])),
                        )
                        ** 0.5,
                    ),
                )
            ),
        ),
        EffectsGroup(
            scope=IsSource,
            effects=SetEmpireMeter(empire=Source.Owner, meter="MILITARY_CATEGORY_NUM_POLICY_SLOTS", value=Value + 1),
        ),
    ],
    graphic="icons/tech/distributed_thought.png",
)
