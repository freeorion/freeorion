from focs._effects import (
    EffectsGroup,
    NamedReal,
    OwnedBy,
    Planet,
    SetMaxShield,
    Source,
    Value,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER
from macros.misc import PLANET_SHIELD_FACTOR
from macros.priorities import DEFAULT_PRIORITY

Tech(
    name="LRN_FORCE_FIELD",
    description="LRN_FORCE_FIELD_DESC",
    short_description="SHIELD_SHORT_DESC",
    category="LEARNING_CATEGORY",
    researchcost=80 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_LEARNING_CATEGORY"],
    prerequisites=["LRN_NASCENT_AI"],
    unlock=Item(type=UnlockShipPart, name="SH_DEFENSE_GRID"),
    effectsgroups=[
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner),
            stackinggroup="PLANET_SHIELDS_STACK_FF",
            priority=DEFAULT_PRIORITY,
            effects=SetMaxShield(
                value=Value + NamedReal(name="LRN_FORCE_FIELD_MAX_SHIELD_FLAT", value=10 * PLANET_SHIELD_FACTOR),
                accountinglabel="LRN_FORCE_FIELD",
            ),
        )
    ],
    graphic="icons/tech/forcefield_harmonics.png",
)
