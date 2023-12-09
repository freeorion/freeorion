from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_QUANT_ENRG_MAG",
    description="SHP_QUANT_ENRG_MAG_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=600 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_ENERGY_HULL_TECHS"],
    prerequisites=["LRN_GRAVITONICS", "SHP_ENRG_FRIGATE"],
    unlock=[
        Item(type=UnlockShipHull, name="SH_QUANTUM_ENERGY"),
        Item(type=UnlockBuilding, name="BLD_SHIPYARD_ENRG_SOLAR"),
    ],
    graphic="icons/ship_hulls/quantum_energy_hull_small.png",
)
