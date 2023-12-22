from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_KRILL_SPAWN",
    description="SHP_KRILL_SPAWN_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SHIP_WEAPONS_CATEGORY",
    researchcost=9999 * TECH_COST_MULTIPLIER,
    researchturns=9999,
    researchable=False,
    tags=["PEDIA_SHIP_WEAPONS_CATEGORY"],
    unlock=Item(type=UnlockShipPart, name="SP_KRILL_SPAWNER"),
)
