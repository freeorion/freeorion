from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_BIOTERM",
    description="SHP_BIOTERM_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=600 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_BOMBARD_WEAPON_TECHS"],
    prerequisites=["SHP_DEATH_SPORE", "GRO_BIOTERROR"],
    unlock=Item(type=UnlockShipPart, name="SP_BIOTERM"),
    graphic="icons/ship_parts/bioterm.png",
)
