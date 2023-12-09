from focs._effects import (
    BlackHole,
    Blue,
    EffectsGroup,
    Focus,
    Happiness,
    InSystem,
    NamedReal,
    Neutron,
    Orange,
    OwnedBy,
    Planet,
    Red,
    SetTargetResearch,
    Source,
    Star,
    StatisticCount,
    Target,
    Value,
    White,
    Yellow,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER
from macros.priorities import TARGET_AFTER_SCALING_PRIORITY

Tech(
    name="LRN_STELLAR_TOMOGRAPHY",
    description="LRN_STELLAR_TOMOGRAPHY_DESC",
    short_description="RESEARCH_SHORT_DESC",
    category="LEARNING_CATEGORY",
    researchcost=180 * TECH_COST_MULTIPLIER,
    researchturns=6,
    tags=["PEDIA_LEARNING_CATEGORY"],
    prerequisites=["LRN_EVERYTHING"],
    effectsgroups=[
        EffectsGroup(
            scope=Planet()
            & OwnedBy(empire=Source.Owner)
            & Star(type=[BlackHole])
            & Focus(type=["FOCUS_RESEARCH"])
            & Happiness(low=0),
            priority=TARGET_AFTER_SCALING_PRIORITY,
            effects=SetTargetResearch(
                value=Value
                + StatisticCount(
                    float,
                    condition=Planet()
                    & OwnedBy(empire=Source.Owner)
                    & InSystem(id=Target.SystemID)
                    & Focus(type=["FOCUS_RESEARCH"]),
                )
                * NamedReal(name="LRN_STELLAR_TOMO_BLACK_TARGET_RESEARCH_PERPLANET", value=3.0)
            ),
        ),
        EffectsGroup(
            scope=Planet()
            & OwnedBy(empire=Source.Owner)
            & Star(type=[Neutron])
            & Focus(type=["FOCUS_RESEARCH"])
            & Happiness(low=0),
            priority=TARGET_AFTER_SCALING_PRIORITY,
            effects=SetTargetResearch(
                value=Value
                + StatisticCount(
                    float,
                    condition=Planet()
                    & OwnedBy(empire=Source.Owner)
                    & InSystem(id=Target.SystemID)
                    & Focus(type=["FOCUS_RESEARCH"]),
                )
                * NamedReal(name="LRN_STELLAR_TOMO_NEUTRON_TARGET_RESEARCH_PERPLANET", value=1.0)
            ),
        ),
        EffectsGroup(
            scope=Planet()
            & OwnedBy(empire=Source.Owner)
            & Star(type=[Blue, White, Red, Orange, Yellow])
            & Focus(type=["FOCUS_RESEARCH"])
            & Happiness(low=0),
            priority=TARGET_AFTER_SCALING_PRIORITY,
            effects=SetTargetResearch(
                value=Value
                + StatisticCount(
                    float,
                    condition=Planet()
                    & OwnedBy(empire=Source.Owner)
                    & InSystem(id=Target.SystemID)
                    & Focus(type=["FOCUS_RESEARCH"]),
                )
                * NamedReal(name="LRN_STELLAR_TOMO_NORMAL_STAR_TARGET_RESEARCH_PERPLANET", value=0.2)
            ),
        ),
    ],
    graphic="icons/tech/stellar_tomography.png",
)
