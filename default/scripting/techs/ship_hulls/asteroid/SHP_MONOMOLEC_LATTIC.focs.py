from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_MONOMOLEC_LATTICE",
    description="SHP_MONOMOLEC_LATTICE_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=1000 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_ASTEROID_HULL_TECHS"],
    prerequisites=["SHP_ASTEROID_REFORM"],
    unlock=[
        Item(type=UnlockShipHull, name="SH_CRYSTALLIZED_ASTEROID"),
        Item(type=UnlockShipPart, name="AR_CRYSTAL_PLATE"),
    ],
    graphic="icons/ship_parts/crystal_plating.png",
)
