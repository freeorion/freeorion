from common.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_CONT_BIOADAPT",
    description="SHP_CONT_BIOADAPT_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=125 * TECH_COST_MULTIPLIER,
    researchturns=12,
    tags=["PEDIA_ORGANIC_HULL_TECHS"],
    prerequisites=["GRO_XENO_HYBRIDS", "SHP_ENDOCRINE_SYSTEMS"],
    unlock=[Item(type=UnlockShipHull, name="SH_RAVENOUS")],
    graphic="icons/ship_hulls/ravenous_hull_small.png",
)
