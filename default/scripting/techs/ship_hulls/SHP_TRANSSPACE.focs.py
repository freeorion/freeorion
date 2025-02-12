from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_TRANSSPACE_HULL",
    description="SHP_TRANSSPACE_HULL_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=80 * TECH_COST_MULTIPLIER,
    researchturns=8,
    tags=["PEDIA_ROBOTIC_HULL_TECHS"],
    prerequisites=["PRO_NANOTECH_PROD"],
    unlock=[
        Item(type=UnlockShipHull, name="SH_TRANSSPATIAL"),
    ],
    graphic="icons/ship_hulls/trans_spatial_hull_small.png",
)

Tech(
    name="SHP_TRANSSPACE_DRIVE",
    description="SHP_TRANSSPACE_DRIVE_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_PARTS_CATEGORY",
    researchcost=400 * TECH_COST_MULTIPLIER,
    researchturns=8,
    tags=["PEDIA_ROBOTIC_HULL_TECHS"],
    prerequisites=["SHP_TRANSSPACE_HULL", "SHP_NANOROBO_MAINT"],
    unlock=[
        Item(type=UnlockShipPart, name="FU_TRANSPATIAL_DRIVE"),
        Item(type=UnlockBuilding, name="BLD_SHIPYARD_CON_ADV_ENGINE"),
    ],
    graphic="icons/ship_parts/engine-4.png",
)
