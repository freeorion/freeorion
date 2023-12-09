from focs._effects import (
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
    name="PRO_INTERSTELLAR_ENTANGLEMENT_FACTORY",
    description="PRO_INTERSTELLAR_ENTANGLEMENT_FACTORY_DESC",
    short_description="IMPERIAL_STOCKPILE_SHORT_DESC",
    category="PRODUCTION_CATEGORY",
    researchcost=160 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_PRODUCTION_CATEGORY"],
    prerequisites=["PRO_GENERIC_SUPPLIES", "PRO_ADAPTIVE_AUTOMATION", "LRN_GRAVITONICS"],
    unlock=Item(type=UnlockBuilding, name="BLD_STOCKPILING_CENTER"),
    effectsgroups=[
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner) & HasSpecies(),
            accountinglabel="INTERSTELLAR_ENTANGLEMENT_FACTORY_POPULATION_BONUS_LABEL",
            effects=SetMaxStockpile(value=Value + 1.0 * Target.Population * STOCKPILE_PER_POP),
        ),
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner) & Focus(type=["FOCUS_STOCKPILE"]),
            accountinglabel="INTERSTELLAR_ENTANGLEMENT_FACTORY_FIXED_BONUS_LABEL",
            effects=SetMaxStockpile(value=Value + 6),
        ),
    ],
)
