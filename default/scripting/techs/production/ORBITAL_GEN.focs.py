from focs._effects import (
    EmpireHasAdoptedPolicy,
    IsSource,
    Source,
    StatisticIf,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="PRO_ORBITAL_GEN",
    description="PRO_ORBITAL_GEN_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="PRODUCTION_CATEGORY",
    researchcost=128
    * TECH_COST_MULTIPLIER
    * (
        1
        - 0.25
        * StatisticIf(float, condition=IsSource & EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_INDUSTRIALISM"))
    ),
    researchturns=4,
    tags=["PEDIA_PRODUCTION_CATEGORY"],
    prerequisites=["PRO_FUSION_GEN", "CON_ORBITAL_CON"],
    unlock=Item(type=UnlockBuilding, name="BLD_GAS_GIANT_GEN"),
    graphic="icons/tech/orbital_generation.png",
)
