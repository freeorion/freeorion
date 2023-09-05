from focs._tech import *
from techs.ship_weapons.ship_weapons import WEAPON_BASE_EFFECTS

Tech(
    name="SHP_ROOT_AGGRESSION",
    description="SHP_ROOT_AGGRESSION_DESC",
    short_description="SHIP_WEAPON_UNLOCK_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=1,
    researchturns=1,
    tags=["PEDIA_SHIP_WEAPONS_CATEGORY"],
    unlock=[
        Item(type=UnlockShipPart, name="SR_WEAPON_1_1"),
        Item(type=UnlockShipPart, name="GT_TROOP_POD"),
        Item(type=UnlockShipPart, name="SR_WEAPON_0_1"),
    ],
    effectsgroups=[
        # TODO move at least the flak effect to the part definition
        *WEAPON_BASE_EFFECTS("SR_WEAPON_0_1"),
        *WEAPON_BASE_EFFECTS("SR_WEAPON_1_1"),
    ],
    graphic="icons/tech/planetary_colonialism.png",
)
