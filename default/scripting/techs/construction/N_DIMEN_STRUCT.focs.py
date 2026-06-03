from focs._conditions import HasSpecies, OwnedBy
from focs._effects import Source, Target
from focs._effects_new import EffectsGroup, SetTargetConstruction, SetTargetPopulation
from focs._techs import Tech
from focs._value_refs import (
    Value,
)
from macros.base_prod import TECH_COST_MULTIPLIER
from macros.priorities import TARGET_POPULATION_AFTER_SCALING_PRIORITY

Tech(
    name="CON_NDIM_STRC",
    description="CON_NDIM_STRC_DESC",
    short_description="POPULATION_SHORT_DESC",
    category="CONSTRUCTION_CATEGORY",
    researchcost=630 * TECH_COST_MULTIPLIER,
    researchturns=9,
    tags=["PEDIA_CONSTRUCTION_CATEGORY"],
    prerequisites=["CON_FRC_ENRG_STRC", "LRN_NDIM_SUBSPACE"],
    effectsgroups=[
        EffectsGroup(
            scope=HasSpecies() & OwnedBy(empire=Source.Owner),
            accountinglabel="CON_TECH_ACCOUNTING_LABEL",
            priority=TARGET_POPULATION_AFTER_SCALING_PRIORITY,
            effects=[
                SetTargetConstruction(value=Value + 10),
                SetTargetPopulation(value=Value + 2 * Target.HabitableSize),
            ],
        )
    ],
    graphic="icons/tech/n-dimensional_structures.png",
)
