from focs._effects import (
    EffectsGroup,
    IsSource,
    SetEmpireMeter,
    Source,
    Value,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="LRN_MIND_VOID",
    description="LRN_MIND_VOID_DESC",
    short_description="POLICY_AND_SLOT_UNLOCK_SHORT_DESC",
    category="LEARNING_CATEGORY",
    researchcost=120 * TECH_COST_MULTIPLIER,
    researchturns=10,
    tags=["PEDIA_LEARNING_CATEGORY", "THEORY"],
    prerequisites=["LRN_XENOARCH"],
    unlock=Item(type=UnlockPolicy, name="PLC_DESIGN_SIMPLICITY"),
    effectsgroups=[
        EffectsGroup(
            scope=IsSource,
            effects=SetEmpireMeter(empire=Source.Owner, meter="SOCIAL_CATEGORY_NUM_POLICY_SLOTS", value=Value + 1),
        )
    ],
    graphic="icons/tech/mind_of_the_void.png",
)
