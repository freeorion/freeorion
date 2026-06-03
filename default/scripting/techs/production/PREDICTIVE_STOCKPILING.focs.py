from focs._conditions import Focus, OwnedBy, Planet
from focs._effects import Source
from focs._effects_new import EffectsGroup, SetMaxStockpile
from focs._techs import Tech
from focs._value_refs import (
    Value,
)

Tech(
    name="PRO_PREDICTIVE_STOCKPILING",
    description="PRO_PREDICTIVE_STOCKPILING_DESC",
    short_description="IMPERIAL_STOCKPILE_SHORT_DESC",
    category="PRODUCTION_CATEGORY",
    researchcost=1,
    researchturns=1,
    tags=["PEDIA_PRODUCTION_CATEGORY"],
    effectsgroups=[
        # Set initial meters
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner) & Focus(type=["FOCUS_STOCKPILE"]),
            effects=SetMaxStockpile(value=Value + 1),
        )
    ],
)
