from common.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_XENTRONIUM_HULL",
    description="SH_XENTRONIUM_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=500 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_SHIP_HULLS_CATEGORY"],
    prerequisites="SHP_XENTRONIUM_PLATE",
    unlock=[Item(type=UnlockShipHull, name="SH_XENTRONIUM")],
    graphic="icons/ship_hulls/xentronium_hull_small.png",
)
