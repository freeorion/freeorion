from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_SPACE_FLUX_COMPOSITION",
    description="SHP_SPACE_FLUX_COMPOSITION_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=120 * TECH_COST_MULTIPLIER,
    researchturns=6,
    tags=["PEDIA_FLUX_HULL_TECHS"],
    prerequisites=["SHP_SPACE_FLUX_BUBBLE", "SHP_SPACE_FLUX_DRIVE"],
    unlock=Item(type=UnlockShipHull, name="SH_SPACE_FLUX_COMPOSITE"),
    # FIXME graphic
    graphic="icons/ship_hulls/agregate_asteroid_hull_small.png",
)
