from focs._effects import (
    EffectsGroup,
    Focus,
    OwnedBy,
    Planet,
    Population,
    SetMaxStockpile,
    Source,
    Target,
    Value,
)
from focs._tech import *
from macros.base_prod import STOCKPILE_PER_POP, TECH_COST_MULTIPLIER

Tech(
    name="PRO_VOID_PREDICTION",
    description="PRO_VOID_PREDICTION_DESC",
    short_description="IMPERIAL_STOCKPILE_SHORT_DESC",
    category="PRODUCTION_CATEGORY",
    researchcost=700 * TECH_COST_MULTIPLIER,
    researchturns=7,
    tags=["PEDIA_PRODUCTION_CATEGORY"],
    prerequisites=["LRN_MIND_VOID", "PRO_GENERIC_SUPPLIES"],
    unlock=Item(type=UnlockPolicy, name="PLC_STOCKPILE_LIQUIDATION"),
    effectsgroups=[
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner) & ~Population(high=0) & Focus(type=["FOCUS_STOCKPILE"]),
            effects=SetMaxStockpile(value=Value + 10 * Target.Population * STOCKPILE_PER_POP),
        )
    ],
)
