from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="GRO_TERRAFORM",
    description="GRO_TERRAFORM_DESC",
    short_description="GRO_TERRAFORM_SHORT_DESC",
    category="GROWTH_CATEGORY",
    researchcost=160 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_GROWTH_CATEGORY"],
    prerequisites=["GRO_ADV_ECOMAN"],
    unlock=[
        Item(type=UnlockBuilding, name="BLD_TERRAFORM_BEST"),
        Item(type=UnlockBuilding, name="BLD_TERRAFORM_TERRAN"),
        Item(type=UnlockBuilding, name="BLD_TERRAFORM_OCEAN"),
        Item(type=UnlockBuilding, name="BLD_TERRAFORM_SWAMP"),
        Item(type=UnlockBuilding, name="BLD_TERRAFORM_TOXIC"),
        Item(type=UnlockBuilding, name="BLD_TERRAFORM_INFERNO"),
        Item(type=UnlockBuilding, name="BLD_TERRAFORM_RADIATED"),
        Item(type=UnlockBuilding, name="BLD_TERRAFORM_BARREN"),
        Item(type=UnlockBuilding, name="BLD_TERRAFORM_TUNDRA"),
        Item(type=UnlockBuilding, name="BLD_TERRAFORM_DESERT"),
        Item(type=UnlockPolicy, name="PLC_ENVIRONMENTALISM"),
    ],
    graphic="icons/tech/terraform.png",
)
