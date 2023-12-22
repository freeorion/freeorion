from focs._effects import (
    EmpireHasAdoptedPolicy,
    IsSource,
    Source,
    StatisticIf,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="PRO_SOL_ORB_GEN",
    description="PRO_SOL_ORB_GEN_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="PRODUCTION_CATEGORY",
    researchcost=200
    * TECH_COST_MULTIPLIER
    * (
        1
        - 0.25
        * StatisticIf(float, condition=IsSource & EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_INDUSTRIALISM"))
    ),
    researchturns=8,
    tags=["PEDIA_PRODUCTION_CATEGORY"],
    prerequisites=["PRO_ORBITAL_GEN"],
    unlock=Item(type=UnlockBuilding, name="BLD_SOL_ORB_GEN"),
    graphic="icons/building/miniature_sun.png",
)
