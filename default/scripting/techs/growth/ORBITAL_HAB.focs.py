from common.base_prod import TECH_COST_MULTIPLIER
from common.priorities import TARGET_POPULATION_AFTER_SCALING_PRIORITY

Tech(
    name="CON_ORBITAL_HAB",
    description="CON_ORBITAL_HAB_DESC",
    short_description="POPULATION_SHORT_DESC",
    category="GROWTH_CATEGORY",
    researchcost=350 * TECH_COST_MULTIPLIER,
    researchturns=7,
    tags=["PEDIA_GROWTH_CATEGORY"],
    prerequisites=["PRO_MICROGRAV_MAN"],
    effectsgroups=[
        EffectsGroup(
            scope=HasSpecies() & OwnedBy(empire=Source.Owner),
            priority=TARGET_POPULATION_AFTER_SCALING_PRIORITY,
            effects=SetTargetPopulation(value=Value + 1 * Target.HabitableSize, accountinglabel="ORBITAL_HAB_LABEL"),
        )
    ],
    graphic="icons/tech/orbital_gardens.png",
)
