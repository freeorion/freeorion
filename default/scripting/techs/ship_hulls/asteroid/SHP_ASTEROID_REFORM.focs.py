from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_ASTEROID_REFORM",
    description="SHP_ASTEROID_REFORM_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=180 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_ASTEROID_HULL_TECHS"],
    prerequisites=["SHP_ASTEROID_HULLS"],
    unlock=[
        # Item type = ShipHull name = "SH_AGREGATE_ASTEROID"
        Item(type=UnlockBuilding, name="BLD_SHIPYARD_AST_REF"),
        Item(type=UnlockShipPart, name="AR_ROCK_PLATE"),
    ],
    graphic="icons/ship_parts/rock_plating.png",
)
