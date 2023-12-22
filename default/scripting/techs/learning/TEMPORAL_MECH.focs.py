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
    name="LRN_TIME_MECH",
    description="LRN_TIME_MECH_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="LEARNING_CATEGORY",
    researchcost=120 * TECH_COST_MULTIPLIER,
    researchturns=10,
    tags=["PEDIA_LEARNING_CATEGORY"],
    prerequisites=["LRN_EVERYTHING"],
    unlock=Item(type=UnlockBuilding, name="BLD_STARLANE_NEXUS"),
    effectsgroups=[
        EffectsGroup(
            scope=IsSource,
            effects=SetEmpireMeter(empire=Source.Owner, meter="MILITARY_CATEGORY_NUM_POLICY_SLOTS", value=Value + 1),
        )
    ],
    graphic="icons/tech/temporal_mechanics.png",
)
