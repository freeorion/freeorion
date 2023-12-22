from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_MULTISPEC_SHIELD",
    description="SHP_MULTISPEC_SHIELD_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SHIP_PARTS_CATEGORY",
    researchcost=1800 * TECH_COST_MULTIPLIER,
    researchturns=10,
    # Unresearchable
    tags=["PEDIA_SHIELD_PART_TECHS"],
    prerequisites=["SHP_PLASMA_SHIELD", "SPY_DIST_MOD"],
    unlock=Item(type=UnlockShipPart, name="SH_MULTISPEC"),
    graphic="icons/ship_parts/multi-spectral.png",
)
