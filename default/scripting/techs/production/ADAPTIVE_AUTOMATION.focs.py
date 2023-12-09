from focs._effects import (
    EffectsGroup,
    Happiness,
    NamedReal,
    OwnedBy,
    Planet,
    SetTargetIndustry,
    Source,
    TargetPopulation,
    Value,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER
from macros.priorities import TARGET_AFTER_2ND_SCALING_PRIORITY

Tech(
    name="PRO_ADAPTIVE_AUTOMATION",
    description="PRO_ADAPTIVE_AUTOMATION_DESC",
    short_description="INDUSTRY_SHORT_DESC",
    category="PRODUCTION_CATEGORY",
    researchcost=180 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_PRODUCTION_CATEGORY"],
    prerequisites=["LRN_ALGO_ELEGANCE", "PRO_NANOTECH_PROD"],
    effectsgroups=[
        EffectsGroup(
            scope=Planet()
            & OwnedBy(empire=Source.Owner)
            & TargetPopulation(low=0.0001)
            & Happiness(low=NamedReal(name="PRO_ADAPTIVE_AUTO_MIN_STABILITY", value=10)),
            priority=TARGET_AFTER_2ND_SCALING_PRIORITY,
            effects=SetTargetIndustry(value=Value + NamedReal(name="PRO_ADAPTIVE_AUTO_TARGET_INDUSTRY_FLAT", value=2)),
        ),
    ],
    graphic="icons/tech/basic_autofactories.png",
)
