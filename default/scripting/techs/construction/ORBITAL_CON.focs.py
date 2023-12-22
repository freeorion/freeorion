from focs._effects import (
    EffectsGroup,
    OwnedBy,
    Planet,
    SetMaxSupply,
    Source,
    Value,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="CON_ORBITAL_CON",
    description="CON_ORBITAL_CON_DESC",
    short_description="SUPPLY_SHORT_DESC",
    category="CONSTRUCTION_CATEGORY",
    researchcost=72 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_CONSTRUCTION_CATEGORY"],
    effectsgroups=[
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner),
            accountinglabel="CON_TECH_ACCOUNTING_LABEL",
            effects=SetMaxSupply(value=Value + 1),
        )
    ],
    graphic="icons/tech/orbital_construction.png",
)
