from focs._effects import (
    EffectsGroup,
    EmpireHasAdoptedPolicy,
    Focus,
    Happiness,
    NamedReal,
    OwnedBy,
    Planet,
    SetTargetIndustry,
    Source,
    StatisticIf,
    Target,
    Value,
)
from focs._tech import *
from macros.base_prod import INDUSTRY_PER_POP, TECH_COST_MULTIPLIER

Tech(
    name="PRO_FUSION_GEN",
    description="PRO_FUSION_GEN_DESC",
    short_description="INDUSTRY_SHORT_DESC",
    category="PRODUCTION_CATEGORY",
    researchcost=48
    * TECH_COST_MULTIPLIER
    * (1 - 0.25 * StatisticIf(float, condition=EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_INDUSTRIALISM"))),
    researchturns=3,
    tags=["PEDIA_PRODUCTION_CATEGORY"],
    prerequisites=["PRO_ROBOTIC_PROD"],
    effectsgroups=[
        EffectsGroup(
            scope=Planet()
            & OwnedBy(empire=Source.Owner)
            & Focus(type=["FOCUS_INDUSTRY"])
            & Happiness(low=NamedReal(name="PRO_FUSION_GEN_MIN_STABILITY", value=18)),
            effects=SetTargetIndustry(
                value=Value
                + Target.Population
                * NamedReal(name="PRO_FUSION_GEN_TARGET_INDUSTRY_PERPOP", value=0.25 * INDUSTRY_PER_POP)
            ),
        )
    ],
    graphic="icons/tech/fusion_generation.png",
)
