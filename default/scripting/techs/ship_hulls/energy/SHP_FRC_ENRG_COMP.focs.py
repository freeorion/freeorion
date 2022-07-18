from common.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_FRC_ENRG_COMP",
    description="SHP_FRC_ENRG_COMP_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=80 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_ENERGY_HULL_TECHS"],
    prerequisites=["LRN_FORCE_FIELD"],
    unlock=[
        Item(type=UnlockShipHull, name="SH_COMPRESSED_ENERGY"),
        Item(type=UnlockBuilding, name="BLD_SHIPYARD_ENRG_COMP"),
    ],
    graphic="icons/ship_hulls/compressed_energy_hull_small.png",
)
