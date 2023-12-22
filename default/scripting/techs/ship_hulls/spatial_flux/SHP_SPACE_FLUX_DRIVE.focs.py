from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_SPACE_FLUX_DRIVE",
    description="SHP_SPACE_FLUX_DRIVE_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=40 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_FLUX_HULL_TECHS"],
    prerequisites=["SHP_SPACE_FLUX_BASICS"],
    unlock=Item(type=UnlockShipHull, name="SH_SPATIAL_FLUX"),
    graphic="icons/ship_hulls/spatial_flux_hull_small.png",
)
