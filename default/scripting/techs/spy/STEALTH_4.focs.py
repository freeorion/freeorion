from focs._effects import AddSpecial, EffectsGroup, HasTag, OwnedBy, Planet, RemoveSpecial, Source, StatisticIf
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SPY_STEALTH_4",
    description="SPY_STEALTH_4_DESC",
    short_description="SHIP_PART_UNLOCK_SHORT_DESC",
    category="SPY_CATEGORY",
    researchcost=1200
    * TECH_COST_MULTIPLIER
    / (1 + StatisticIf(float, condition=OwnedBy(empire=Source.Owner) & HasTag(name="SNEAKY"))),
    researchturns=8,
    tags=["PEDIA_SPY_CATEGORY"],
    prerequisites=["SPY_STEALTH_3", "SPY_STEALTH_PART_3"],
    unlock=Item(type=UnlockShipPart, name="ST_CLOAK_4"),
    effectsgroups=[
        EffectsGroup(
            scope=OwnedBy(empire=Source.Owner) & Planet(),
            effects=[
                AddSpecial(name="VOID_SLAVE_SPECIAL"),
                RemoveSpecial(name="DIM_RIFT_SLAVE_SPECIAL"),
                RemoveSpecial(name="VOLCANIC_ASH_SLAVE_SPECIAL"),
                RemoveSpecial(name="CLOUD_COVER_SLAVE_SPECIAL"),
            ],
        )
    ],
    graphic="icons/specials_huge/void.png",
)
