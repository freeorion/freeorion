from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_CHAOS_WAVE",
    description="SHP_CHAOS_WAVE_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=1500 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_BOMBARD_WEAPON_TECHS"],
    prerequisites=["SHP_BIOTERM", "SHP_GRV", "SHP_EMO", "SHP_VOID_SHADOW"],
    unlock=Item(type=UnlockShipPart, name="SP_CHAOS_WAVE"),
    graphic="icons/ship_parts/chaos-wave.png",
)
