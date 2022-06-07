from common.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="GRO_TERRAFORM",
    description="GRO_TERRAFORM_DESC",
    short_description="GRO_TERRAFORM_SHORT_DESC",
    category="GROWTH_CATEGORY",
    researchcost=100 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_GROWTH_CATEGORY"],
    prerequisites="GRO_ADV_ECOMAN",
    unlock=[
        Item(type=Building, name="BLD_TERRAFORM_BEST"),
        Item(type=Building, name="BLD_TERRAFORM_TERRAN"),
        Item(type=Building, name="BLD_TERRAFORM_OCEAN"),
        Item(type=Building, name="BLD_TERRAFORM_SWAMP"),
        Item(type=Building, name="BLD_TERRAFORM_TOXIC"),
        Item(type=Building, name="BLD_TERRAFORM_INFERNO"),
        Item(type=Building, name="BLD_TERRAFORM_RADIATED"),
        Item(type=Building, name="BLD_TERRAFORM_BARREN"),
        Item(type=Building, name="BLD_TERRAFORM_TUNDRA"),
        Item(type=Building, name="BLD_TERRAFORM_DESERT"),
        Item(type=Policy, name="PLC_ENVIRONMENTALISM"),
    ],
    graphic="icons/tech/terraform.png",
)
