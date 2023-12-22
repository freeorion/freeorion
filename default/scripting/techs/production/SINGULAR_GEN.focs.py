from focs._effects import (
    EmpireHasAdoptedPolicy,
    IsSource,
    Source,
    StatisticIf,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="PRO_SINGULAR_GEN",
    description="PRO_SINGULAR_GEN_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="PRODUCTION_CATEGORY",
    researchcost=320
    * TECH_COST_MULTIPLIER
    * (
        1
        - 0.25
        * StatisticIf(float, condition=IsSource & EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_INDUSTRIALISM"))
    ),
    researchturns=4,
    tags=["PEDIA_PRODUCTION_CATEGORY"],
    prerequisites=["LRN_TIME_MECH", "PRO_SOL_ORB_GEN"],
    unlock=Item(type=UnlockBuilding, name="BLD_BLACK_HOLE_POW_GEN"),
    graphic="icons/tech/singularity_generation.png",
)
