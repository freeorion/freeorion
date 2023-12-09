from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="GRO_BIOTERROR",
    description="GRO_BIOTERROR_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="GROWTH_CATEGORY",
    researchcost=80 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_GROWTH_CATEGORY"],
    prerequisites=["GRO_NANOTECH_MED"],
    unlock=Item(type=UnlockBuilding, name="BLD_BIOTERROR_PROJECTOR"),
    graphic="icons/tech/bioterror_facilities.png",
)
