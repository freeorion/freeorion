from focs._effects import (
    Adequate,
    EffectsGroup,
    HasTag,
    Hostile,
    OwnedBy,
    Planet,
    Poor,
    SetTargetPopulation,
    Source,
    StatisticIf,
    Target,
    Value,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER
from macros.priorities import TARGET_POPULATION_BEFORE_SCALING_PRIORITY

Tech(
    name="GRO_XENO_GENETICS",
    description="GRO_XENO_GENETICS_DESC",
    short_description="POPULATION_SHORT_DESC",
    category="GROWTH_CATEGORY",
    researchcost=(
        120
        * TECH_COST_MULTIPLIER
        / (1 + StatisticIf(float, condition=OwnedBy(empire=Source.Owner) & HasTag(name="ADAPTIVE")))
    ),
    researchturns=8,
    tags=["PEDIA_GROWTH_CATEGORY"],
    prerequisites=[
        "GRO_SYMBIOTIC_BIO",
        "GRO_GENETIC_MED",
    ],
    effectsgroups=[
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner) & Planet(environment=[Adequate, Poor]),
            accountinglabel="GRO_TECH_ACCOUNTING_LABEL",
            priority=TARGET_POPULATION_BEFORE_SCALING_PRIORITY,
            effects=SetTargetPopulation(value=Value + 2 * Target.HabitableSize),
        ),
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner) & Planet(environment=[Hostile]),
            accountinglabel="GRO_TECH_ACCOUNTING_LABEL",
            priority=TARGET_POPULATION_BEFORE_SCALING_PRIORITY,
            effects=SetTargetPopulation(value=Value + 1 * Target.HabitableSize),
        ),
    ],
    graphic="icons/tech/xenological_genetics.png",
)
