from common.base_prod import INDUSTRY_PER_POP, TECH_COST_MULTIPLIER
from common.priorities import TARGET_AFTER_SCALING_PRIORITY

Tech(
    name="PRO_ROBOTIC_PROD",
    description="PRO_ROBOTIC_PROD_DESC",
    short_description="INDUSTRY_SHORT_DESC",
    category="PRODUCTION_CATEGORY",
    researchcost=24 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_PRODUCTION_CATEGORY"],
    effectsgroups=[
        EffectsGroup(
            scope=ProductionCenter
            & OwnedBy(empire=Source.Owner)
            & Focus(type=["FOCUS_INDUSTRY"])
            & Happiness(low=NamedReal(name="PRO_ROBOTIC_PROD_MIN_STABILITY", value=5)),
            priority=TARGET_AFTER_SCALING_PRIORITY,
            effects=SetTargetIndustry(
                value=Value
                + Target.Population
                * NamedReal(name="PRO_ROBOTIC_PROD_TARGET_INDUSTRY_PERPOP", value=0.25 * INDUSTRY_PER_POP)
            ),
        )
    ],
    graphic="icons/tech/robotic_production.png",
)
