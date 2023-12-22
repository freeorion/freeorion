from focs._effects import (
    AsteroidsType,
    ContainedBy,
    Contains,
    EffectsGroup,
    Focus,
    Happiness,
    InSystem,
    NamedReal,
    OwnedBy,
    Planet,
    Population,
    SetTargetIndustry,
    Source,
    Value,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER
from macros.priorities import TARGET_AFTER_2ND_SCALING_PRIORITY

Tech(
    name="PRO_MICROGRAV_MAN",
    description="PRO_MICROGRAV_MAN_DESC",
    short_description="INDUSTRY_SHORT_DESC",
    category="PRODUCTION_CATEGORY",
    researchcost=80 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_PRODUCTION_CATEGORY"],
    prerequisites=["CON_ORBITAL_CON"],
    effectsgroups=[
        EffectsGroup(
            scope=Planet()
            & OwnedBy(empire=Source.Owner)
            & ~Population(high=0)
            & Focus(type=["FOCUS_INDUSTRY"])
            & Happiness(low=NamedReal(name="PRO_MICROGRAV_MAN_MIN_STABILITY", value=10))
            & InSystem()
            & ContainedBy(Contains(Planet() & Planet(type=[AsteroidsType]) & OwnedBy(empire=Source.Owner))),
            priority=TARGET_AFTER_2ND_SCALING_PRIORITY,
            effects=SetTargetIndustry(value=Value + NamedReal(name="PRO_MICROGRAV_MAN_TARGET_INDUSTRY_FLAT", value=2)),
        )
    ],
    graphic="icons/tech/microgravity_manufacturing.png",
)
