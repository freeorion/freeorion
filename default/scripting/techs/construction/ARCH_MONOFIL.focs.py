from common.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="CON_ARCH_MONOFILS",
    description="CON_ARCH_MONOFILS_DESC",
    short_description="BUILDING_UNLOCK_SHORT_DESC",
    category="CONSTRUCTION_CATEGORY",
    researchcost=75 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_CONSTRUCTION_CATEGORY"],
    prerequisites="CON_ASYMP_MATS",
    unlock=Item(type=Building, name="BLD_SPACE_ELEVATOR"),
    graphic="icons/tech/architectural_monofilaments.png",
)
