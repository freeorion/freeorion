from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_NOVA_BOMB",
    description="SHP_NOVA_BOMB_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=960 * TECH_COST_MULTIPLIER,
    researchturns=12,
    tags=["PEDIA_SHIP_WEAPONS_CATEGORY"],
    prerequisites=["LRN_STELLAR_TOMOGRAPHY", "PRO_ZERO_GEN"],
    unlock=[Item(type=UnlockShipPart, name="SP_NOVA_BOMB"), Item(type=UnlockBuilding, name="BLD_NOVA_BOMB_ACTIVATOR")],
    graphic="icons/ship_parts/nova-bomb.png",
)
