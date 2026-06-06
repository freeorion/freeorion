from focs._effects_new import Item
from focs._enums import UnlockBuilding, UnlockShipPart
from focs._techs import Tech
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SPY_BIOMIMESIS",
    description="SPY_BIOMIMESIS_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SPY_CATEGORY",
    researchcost=10 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_DETECTION_PART_TECHS"],
    prerequisites=["GRO_GENETIC_ENG"],
    unlock=[Item(type=UnlockShipPart, name="ST_COATING_GG"), Item(type=UnlockBuilding, name="BLD_BIO_LAB")],
    graphic="icons/tech/biomimesis.png",
)
