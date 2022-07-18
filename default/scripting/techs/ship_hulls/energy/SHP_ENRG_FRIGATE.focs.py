from common.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_ENRG_FRIGATE",
    description="SHP_ENRG_FRIGATE_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=120 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_ENERGY_HULL_TECHS"],
    prerequisites=["SHP_FRC_ENRG_COMP"],
    unlock=[Item(type=UnlockShipHull, name="SH_ENERGY_FRIGATE")],
    graphic="icons/ship_hulls/energy_frigate_hull_small.png",
)
