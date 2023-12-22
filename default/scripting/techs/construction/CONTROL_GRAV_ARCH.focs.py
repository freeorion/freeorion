from focs._effects import (
    EffectsGroup,
    Focus,
    OwnedBy,
    Planet,
    SetMaxSupply,
    Source,
    Value,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="CON_CONTGRAV_ARCH",
    description="CON_CONTGRAV_ARCH_DESC",
    short_description="SUPPLY_SHORT_DESC",
    category="CONSTRUCTION_CATEGORY",
    researchcost=160 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_CONSTRUCTION_CATEGORY"],
    prerequisites=["CON_ARCH_MONOFILS"],
    effectsgroups=[
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner) & Focus(type=["FOCUS_LOGISTICS"]),
            effects=SetMaxSupply(value=Value + 1),
        )
    ],
    graphic="icons/tech/controlled_gravity_architecture.png",
)
