from focs._conditions import EmpireHasAdoptedPolicy, IsSource
from focs._effects import Source
from focs._techs import Tech
from focs._value_refs import (
    StatisticIf,
)
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="PRO_ZERO_GEN",
    description="PRO_ZERO_GEN_DESC",
    short_description="THEORY_SHORT_DESC",
    category="PRODUCTION_CATEGORY",
    researchcost=840
    * TECH_COST_MULTIPLIER
    * (
        1
        - 0.25
        * StatisticIf(float, condition=IsSource & EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_INDUSTRIALISM"))
    ),
    researchturns=7,
    tags=["PEDIA_PRODUCTION_CATEGORY", "THEORY"],
    prerequisites=["PRO_NDIM_ASSMB", "PRO_SINGULAR_GEN"],
    graphic="icons/tech/zero_point_energy.png",
)
