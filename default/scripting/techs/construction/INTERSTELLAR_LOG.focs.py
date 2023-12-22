from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SHP_INTSTEL_LOG",
    description="SHP_INTSTEL_LOG_DESC",
    short_description="STARLANE_SPEED_SHORT_DESC",
    category="CONSTRUCTION_CATEGORY",
    researchcost=120 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_CONSTRUCTION_CATEGORY"],
    prerequisites=["CON_ORBITAL_CON"],
    unlock=[
        Item(type=UnlockPolicy, name="PLC_NO_SUPPLY"),
        Item(type=UnlockPolicy, name="PLC_CONFEDERATION"),
        Item(type=UnlockPolicy, name="PLC_TRAFFIC_CONTROL"),
    ],
    graphic="icons/tech/stellar_navigation.png",
)
