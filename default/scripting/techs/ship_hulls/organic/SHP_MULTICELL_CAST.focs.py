from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_MULTICELL_CAST",
    description="SHP_MULTICELL_CAST_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=20 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_ORGANIC_HULL_TECHS"],
    prerequisites=["SHP_ORG_HULL", "GRO_GENETIC_ENG"],
    unlock=[
        Item(type=UnlockShipHull, name="SH_STATIC_MULTICELLULAR"),
        Item(type=UnlockBuilding, name="BLD_SHIPYARD_ORG_XENO_FAC"),
    ],
    graphic="icons/ship_hulls/static_multicellular_small.png",
)
