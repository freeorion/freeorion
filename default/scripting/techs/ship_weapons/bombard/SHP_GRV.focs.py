from focs._effects import UnlockShipPart
from focs._effects_new import Item
from focs._techs import Tech
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_GRV",
    description="SHP_GRV_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=600 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_BOMBARD_WEAPON_TECHS"],
    prerequisites=["SHP_SONIC", "LRN_GRAVITONICS"],
    unlock=Item(type=UnlockShipPart, name="SP_GRV"),
    graphic="icons/ship_parts/gravitic_pulse.png",
)
