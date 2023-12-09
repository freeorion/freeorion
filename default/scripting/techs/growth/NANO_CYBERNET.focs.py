from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="GRO_NANO_CYBERNET",
    description="GRO_NANO_CYBERNET_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="GROWTH_CATEGORY",
    researchcost=120 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_GROWTH_CATEGORY"],
    prerequisites=["GRO_NANOTECH_MED"],
    unlock=[Item(type=UnlockShipPart, name="GT_TROOP_POD_2"), Item(type=UnlockPolicy, name="PLC_AUGMENTATION")],
    graphic="icons/tech/nanotech_cybernetics.png",
)
