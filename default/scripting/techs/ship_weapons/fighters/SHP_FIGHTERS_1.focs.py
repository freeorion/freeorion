from focs._effects import UnlockShipPart
from focs._effects_new import Item
from focs._techs import Tech
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_FIGHTERS_1",
    description="SHP_FIGHTERS_1_DESC",
    short_description="SHIP_WEAPON_UNLOCK_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=12 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_FIGHTER_TECHS"],
    prerequisites=["SHP_ROOT_AGGRESSION"],
    unlock=[
        Item(type=UnlockShipPart, name="FT_BAY_1"),
        Item(type=UnlockShipPart, name="FT_HANGAR_1"),
        Item(type=UnlockShipPart, name="FT_HANGAR_2"),
        Item(type=UnlockShipPart, name="FT_HANGAR_3"),
        Item(type=UnlockShipPart, name="FT_HANGAR_4"),
    ],
    graphic="icons/ship_parts/fighter05.png",
)
