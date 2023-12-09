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
    name="CON_TRANS_ARCH",
    description="CON_TRANS_ARCH_DESC",
    short_description="VARIOUS_UNLOCK_SHORT_DESC",
    category="CONSTRUCTION_CATEGORY",
    researchcost=1000 * TECH_COST_MULTIPLIER,
    researchturns=10,
    tags=["PEDIA_CONSTRUCTION_CATEGORY"],
    prerequisites=[
        "CON_ORGANIC_STRC",
        "CON_NDIM_STRC",
    ],
    unlock=[
        Item(type=UnlockBuilding, name="BLD_MEGALITH"),
        Item(type=UnlockPolicy, name="PLC_DIVINE_AUTHORITY"),
    ],
    effectsgroups=[
        EffectsGroup(
            scope=IsSource,
            effects=SetEmpireMeter(empire=Source.Owner, meter="SOCIAL_CATEGORY_NUM_POLICY_SLOTS", value=Value + 1),
        )
    ],
    graphic="icons/tech/transcendent_architecture.png",
)
