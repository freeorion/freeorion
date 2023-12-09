from focs._effects import (
    EffectsGroup,
    HasSpecies,
    OwnedBy,
    SetTargetPopulation,
    Source,
    Target,
    Value,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER
from macros.priorities import TARGET_POPULATION_AFTER_SCALING_PRIORITY

Tech(
    name="GRO_SUBTER_HAB",
    description="GRO_SUBTER_HAB_DESC",
    short_description="POPULATION_SHORT_DESC",
    category="GROWTH_CATEGORY",
    researchcost=24 * TECH_COST_MULTIPLIER,
    researchturns=2,
    tags=["PEDIA_GROWTH_CATEGORY"],
    prerequisites=["GRO_PLANET_ECOL"],
    effectsgroups=[
        EffectsGroup(
            scope=HasSpecies() & OwnedBy(empire=Source.Owner),
            accountinglabel="GRO_TECH_ACCOUNTING_LABEL",
            priority=TARGET_POPULATION_AFTER_SCALING_PRIORITY,
            effects=SetTargetPopulation(value=Value + 1 * Target.HabitableSize),
        )
    ],
    graphic="icons/tech/subterranean_construction.png",
)
