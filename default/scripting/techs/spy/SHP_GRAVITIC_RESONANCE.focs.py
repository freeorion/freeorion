from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_GRAVITIC_RESONANCE",
    description="SHP_GRAVITIC_RESONANCE_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SPY_CATEGORY",
    researchcost=10 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_DETECTION_PART_TECHS"],
    prerequisites=["CON_ASYMP_MATS"],
    unlock=Item(type=UnlockShipPart, name="SP_GRAVITIC_RESONATOR"),
    graphic="icons/tech/biomimesis.png",
)
