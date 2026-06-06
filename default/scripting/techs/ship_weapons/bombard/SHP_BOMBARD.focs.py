from focs._effects_new import Item
from focs._enums import UnlockPolicy
from focs._techs import Tech
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_BOMBARD",
    description="SHP_BOMBARD_DESC",
    short_description="SHIP_WEAPON_UNLOCK_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=160 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_BOMBARD_WEAPON_TECHS"],
    prerequisites=["SHP_ROOT_AGGRESSION"],
    unlock=Item(type=UnlockPolicy, name="PLC_TERROR_SUPPRESSION"),
    graphic="",
)
