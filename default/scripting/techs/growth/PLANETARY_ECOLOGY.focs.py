from focs._effects import (
    Adequate,
    EffectsGroup,
    Good,
    OwnedBy,
    OwnerHasTech,
    Planet,
    SetTargetPopulation,
    Source,
    Value,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER
from macros.priorities import TARGET_POPULATION_AFTER_SCALING_PRIORITY

Tech(
    name="GRO_PLANET_ECOL",
    description="GRO_PLANET_ECOL_DESC",
    short_description="POPULATION_SHORT_DESC",
    category="GROWTH_CATEGORY",
    researchcost=8 * TECH_COST_MULTIPLIER,
    researchturns=2,
    tags=["PEDIA_GROWTH_CATEGORY"],
    effectsgroups=[
        EffectsGroup(
            scope=Planet()
            & ~OwnerHasTech(name="GRO_SYMBIOTIC_BIO")
            & OwnedBy(empire=Source.Owner)
            & Planet(environment=[Good, Adequate]),
            accountinglabel="GRO_TECH_ACCOUNTING_LABEL",
            priority=TARGET_POPULATION_AFTER_SCALING_PRIORITY,
            effects=SetTargetPopulation(value=Value + 1),
        )
    ],
    graphic="icons/tech/planetary_ecology.png",
)
