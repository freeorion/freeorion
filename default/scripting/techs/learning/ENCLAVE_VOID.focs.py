from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="LRN_ENCLAVE_VOID",
    description="LRN_ENCLAVE_VOID_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="LEARNING_CATEGORY",
    researchcost=240 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_LEARNING_CATEGORY"],
    prerequisites=[
        "LRN_GATEWAY_VOID",
        "LRN_EVERYTHING",
    ],
    unlock=Item(type=UnlockBuilding, name="BLD_ENCLAVE_VOID"),
    graphic="icons/tech/enclave_of_the_void.png",
)
