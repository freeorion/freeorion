from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_SINGULARITY_ENGINE_CORE",
    description="SHP_SINGULARITY_ENGINE_CORE_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SHIP_PARTS_CATEGORY",
    researchcost=800 * TECH_COST_MULTIPLIER,
    researchturns=10,
    tags=["PEDIA_ENGINE_PART_TECHS"],
    prerequisites=["SHP_N_DIMENSIONAL_ENGINE_MATRIX", "PRO_SINGULAR_GEN"],
    unlock=Item(type=UnlockShipPart, name="FU_SINGULARITY_ENGINE_CORE"),
    graphic="icons/ship_parts/engine-3.png",
)
