from common.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_DOMESTIC_MONSTER",
    description="SHP_DOMESTIC_MONSTER_DESC",
    short_description="TAME_MONSTER_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=9 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_SHIP_HULLS_CATEGORY"],
    prerequisites="GRO_PLANET_ECOL",
    graphic="icons/monsters/kraken-5.png",
)
