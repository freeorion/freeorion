from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_IMPROVED_ENGINE_COUPLINGS",
    description="SHP_IMPROVED_ENGINE_COUPLINGS_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SHIP_PARTS_CATEGORY",
    researchcost=48 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_ENGINE_PART_TECHS"],
    prerequisites=["SHP_GAL_EXPLO"],
    unlock=[Item(type=UnlockShipPart, name="FU_IMPROVED_ENGINE_COUPLINGS"), Item(type=UnlockPolicy, name="PLC_CHARGE")],
    graphic="icons/ship_parts/engine-1.png",
)
