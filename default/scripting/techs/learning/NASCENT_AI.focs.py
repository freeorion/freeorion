from focs._effects import (
    EffectsGroup,
    Happiness,
    NamedReal,
    OwnedBy,
    Planet,
    SetTargetResearch,
    Source,
    TargetPopulation,
    Value,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER
from macros.priorities import TARGET_AFTER_2ND_SCALING_PRIORITY

Tech(
    name="LRN_NASCENT_AI",
    description="LRN_NASCENT_AI_DESC",
    short_description="RESEARCH_SHORT_DESC",
    category="LEARNING_CATEGORY",
    researchcost=48 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_LEARNING_CATEGORY"],
    prerequisites=["LRN_ALGO_ELEGANCE"],
    effectsgroups=[
        EffectsGroup(
            scope=Planet()
            & OwnedBy(empire=Source.Owner)
            & TargetPopulation(low=0.0001)
            & Happiness(low=NamedReal(name="LRN_NASCENT_AI_MIN_STABILITY", value=10)),
            priority=TARGET_AFTER_2ND_SCALING_PRIORITY,
            effects=SetTargetResearch(value=Value + NamedReal(name="LRN_NASCENT_AI_TARGET_RESEARCH_FLAT", value=1)),
        )
    ],
    graphic="icons/tech/basic_autolabs.png",
)
