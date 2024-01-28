from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_DOMESTIC_MONSTER",
    description="SHP_DOMESTIC_MONSTER_DESC",
    short_description="TAME_MONSTER_SHORT_DESC",
    category="SHIP_HULLS_CATEGORY",
    researchcost=90 * TECH_COST_MULTIPLIER,
    researchturns=6,
    tags=["PEDIA_GROWTH_CATEGORY"],
    prerequisites=[
        "GRO_MEGA_ECO",
        "LRN_TRANSLING_THT",
        "GRO_GENETIC_MED",
    ],
    graphic="icons/monsters/kraken-5.png",
)
