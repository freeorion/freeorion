from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_CONT_SYMB",
    description="SHP_CONT_SYMB_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=32 * TECH_COST_MULTIPLIER,
    researchturns=8,
    tags=["PEDIA_ORGANIC_HULL_TECHS"],
    prerequisites=["SHP_ORG_HULL"],
    unlock=[
        Item(type=UnlockShipHull, name="SH_SYMBIOTIC"),
        Item(type=UnlockBuilding, name="BLD_SHIPYARD_ORG_CELL_GRO_CHAMB"),
    ],
    graphic="icons/ship_hulls/symbiotic_hull_small.png",
)
