from focs._effects import AddSpecial, EffectsGroup, HasTag, IsSource, OwnedBy, OwnerHasTech, Planet, Source, StatisticIf
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SPY_STEALTH_1",
    description="SPY_STEALTH_1_DESC",
    short_description="PLANET_PROTECT_SHORT_DESC",
    category="SPY_CATEGORY",
    researchcost=(
        160
        * TECH_COST_MULTIPLIER
        / (1 + StatisticIf(float, condition=IsSource & OwnerHasTech(name="SPY_STEALTH_PART_1")))
        / (1 + StatisticIf(float, condition=OwnedBy(empire=Source.Owner) & HasTag(name="SNEAKY")))
    ),
    researchturns=5,
    tags=["PEDIA_SPY_CATEGORY"],
    prerequisites=["SPY_ROOT_DECEPTION"],
    effectsgroups=[
        EffectsGroup(
            scope=OwnedBy(empire=Source.Owner) & Planet(),
            activation=~OwnerHasTech(name="SPY_STEALTH_2")
            & ~OwnerHasTech(name="SPY_STEALTH_3")
            & ~OwnerHasTech(name="SPY_STEALTH_4"),
            effects=AddSpecial(name="CLOUD_COVER_SLAVE_SPECIAL"),
        )
    ],
    graphic="icons/specials_huge/cloud_cover.png",
)
