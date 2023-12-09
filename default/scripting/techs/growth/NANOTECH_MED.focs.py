from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="GRO_NANOTECH_MED",
    description="GRO_NANOTECH_MED_DESC",
    short_description="POLICY_UNLOCK_SHORT_DESC",
    category="GROWTH_CATEGORY",
    researchcost=75 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_GROWTH_CATEGORY", "THEORY"],
    prerequisites=[
        "GRO_GENETIC_MED",
        "PRO_NANOTECH_PROD",
    ],
    unlock=Item(type=UnlockPolicy, name="PLC_POPULATION"),
    graphic="icons/tech/nanotech_medicine.png",
)
