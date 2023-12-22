from focs._effects import (
    Capital,
    EffectsGroup,
    Focus,
    HasSpecies,
    OwnedBy,
    Planet,
    SetMaxStockpile,
    Source,
    Target,
    Value,
)
from focs._tech import *
from macros.base_prod import STOCKPILE_PER_POP, TECH_COST_MULTIPLIER

Tech(
    name="PRO_GENERIC_SUPPLIES",
    description="PRO_GENERIC_SUPPLIES_DESC",
    short_description="IMPERIAL_STOCKPILE_SHORT_DESC",
    category="PRODUCTION_CATEGORY",
    researchcost=40 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_PRODUCTION_CATEGORY"],
    prerequisites=[
        "PRO_PREDICTIVE_STOCKPILING",
        "CON_ASYMP_MATS",
        "PRO_ROBOTIC_PROD",
    ],
    effectsgroups=[
        EffectsGroup(
            scope=Capital & OwnedBy(empire=Source.Owner),
            effects=SetMaxStockpile(value=Value + 2, accountinglabel="GENERIC_SUPPLIES_FIXED_BONUS_LABEL"),
        ),
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner) & HasSpecies(),
            effects=SetMaxStockpile(
                value=(Value + 0.5 * Target.Population * STOCKPILE_PER_POP),
                accountinglabel="GENERIC_SUPPLIES_POPULATION_BONUS_LABEL",
            ),
        ),
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner) & Focus(type=["FOCUS_STOCKPILE"]),
            effects=SetMaxStockpile(value=Value + 3, accountinglabel="GENERIC_SUPPLIES_FOCUS_BONUS_LABEL"),
        ),
    ],
)
