from common.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_MONOCELL_EXP",
    description="SHP_MONOCELL_EXP_DESC",
    short_description="SHIP_HULL_UNLOCK_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=45 * TECH_COST_MULTIPLIER,
    researchturns=8,
    tags=["PEDIA_ORGANIC_HULL_TECHS"],
    prerequisites=["SHP_CONT_SYMB"],
    unlock=[
        Item(type=UnlockShipHull, name="SH_PROTOPLASMIC"),
    ],
    graphic="icons/ship_hulls/protoplasmic_hull_small.png",
)
