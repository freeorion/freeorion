from focs._conditions import HasTag, IsSource, OwnedBy, OwnerHasTech, Planet
from focs._effects_new import AddSpecial, EffectsGroup
from focs._sources import Source
from focs._techs import Tech
from focs._value_refs import (
    StatisticIf,
)
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
