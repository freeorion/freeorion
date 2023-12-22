from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_N_DIMENSIONAL_ENGINE_MATRIX",
    description="SHP_N_DIMENSIONAL_ENGINE_MATRIX_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SHIP_PARTS_CATEGORY",
    researchcost=400 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_ENGINE_PART_TECHS"],
    prerequisites=["SHP_IMPROVED_ENGINE_COUPLINGS", "LRN_NDIM_SUBSPACE"],
    unlock=Item(type=UnlockShipPart, name="FU_N_DIMENSIONAL_ENGINE_MATRIX"),
    graphic="icons/ship_parts/engine-2.png",
)
