from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="PRO_INDUSTRY_CENTER_I",
    description="PRO_INDUSTRY_CENTER_I_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="PRODUCTION_CATEGORY",
    researchcost=60 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_PRODUCTION_CATEGORY"],
    prerequisites=["PRO_ROBOTIC_PROD"],
    unlock=[
        Item(type=UnlockBuilding, name="BLD_INDUSTRY_CENTER"),
        Item(type=UnlockPolicy, name="PLC_INDUSTRIALISM"),
    ],
    graphic="icons/tech/industrial_centre_i.png",
)
