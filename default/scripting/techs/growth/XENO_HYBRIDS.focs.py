from common.base_prod import TECH_COST_MULTIPLIER
from common.priorities import TARGET_POPULATION_BEFORE_SCALING_PRIORITY

Tech(
    name="GRO_XENO_HYBRIDS",
    description="GRO_XENO_HYBRIDS_DESC",
    short_description="POPULATION_SHORT_DESC",
    category="GROWTH_CATEGORY",
    researchcost=150 * TECH_COST_MULTIPLIER,
    researchturns=10,
    tags=["PEDIA_GROWTH_CATEGORY"],
    prerequisites="GRO_XENO_GENETICS",
    effectsgroups=[
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner) & Planet(environment=[Poor]),
            accountinglabel="GRO_TECH_ACCOUNTING_LABEL",
            priority=TARGET_POPULATION_BEFORE_SCALING_PRIORITY,
            effects=SetTargetPopulation(value=Value + 1 * Target.HabitableSize),
        ),
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner) & Planet(environment=[Hostile]),
            accountinglabel="GRO_TECH_ACCOUNTING_LABEL",
            priority=TARGET_POPULATION_BEFORE_SCALING_PRIORITY,
            effects=SetTargetPopulation(value=Value + 2 * Target.HabitableSize),
        ),
    ],
    graphic="icons/tech/xenological_hybridization.png",
)
