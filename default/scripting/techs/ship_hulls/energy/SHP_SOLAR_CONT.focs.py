from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_SOLAR_CONT",
    description="SHP_SOLAR_CONT_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=1120 * TECH_COST_MULTIPLIER,
    researchturns=7,
    tags=["PEDIA_ENERGY_HULL_TECHS"],
    prerequisites=["SHP_QUANT_ENRG_MAG", "SHP_ENRG_BOUND_MAN", "PRO_ZERO_GEN"],
    unlock=[
        Item(type=UnlockShipHull, name="SH_SOLAR")
        # Item type = Building name = "BLD_SHIPYARD_ENRG_SOLAR"
    ],
    graphic="icons/ship_hulls/solar_hull_small.png",
)
