from common.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_BIOADAPTIVE_SPEC",
    description="SHP_BIOADAPTIVE_SPEC_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=216 * TECH_COST_MULTIPLIER,
    researchturns=12,
    tags=["PEDIA_ORGANIC_HULL_TECHS"],
    prerequisites=["SHP_MONOCELL_EXP", "GRO_TRANSORG_SENT"],
    unlock=[
        Item(type=UnlockShipHull, name="SH_BIOADAPTIVE"),
    ],
    graphic="icons/ship_hulls/bio_adaptive_hull_small.png",
)
