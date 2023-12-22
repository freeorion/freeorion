from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_SPACE_FLUX_BUBBLE",
    description="SHP_SPACE_FLUX_BUBBLE_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=24 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_FLUX_HULL_TECHS"],
    prerequisites=["SHP_SPACE_FLUX_BASICS"],
    unlock=Item(type=UnlockShipHull, name="SH_SPACE_FLUX_BUBBLE"),
    graphic="icons/ship_hulls/bulk_freighter_hull_small.png",
)
