from focs._effects import (
    EffectsGroup,
    Focus,
    Happiness,
    NamedReal,
    NamedRealLookup,
    OwnedBy,
    Planet,
    SetMaxDefense,
    SetMaxFuel,
    SetMaxShield,
    SetTargetIndustry,
    SetTargetResearch,
    Ship,
    Source,
    Target,
    Value,
)
from focs._tech import *
from macros.base_prod import INDUSTRY_PER_POP, RESEARCH_PER_POP, TECH_COST_MULTIPLIER
from macros.misc import PLANET_DEFENSE_FACTOR, PLANET_SHIELD_FACTOR
from macros.priorities import (
    TARGET_AFTER_SCALING_PRIORITY,
    TARGET_EARLY_BEFORE_SCALING_PRIORITY,
)

Tech(
    name="GRO_ENERGY_META",
    description="GRO_ENERGY_META_DESC",
    short_description="VARIOUS_SHORT_DESC",
    category="GROWTH_CATEGORY",
    researchcost=330 * TECH_COST_MULTIPLIER,
    researchturns=15,
    tags=["PEDIA_GROWTH_CATEGORY"],
    prerequisites=[
        "GRO_NANO_CYBERNET",
        "LRN_EVERYTHING",
        "GRO_TRANSORG_SENT",
    ],
    effectsgroups=[
        EffectsGroup(
            scope=Ship & OwnedBy(empire=Source.Owner),
            effects=SetMaxFuel(value=Value + 1),
        ),
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner) & Happiness(low=0),
            stackinggroup="DEFENSE_NET_STACK_GM",
            accountinglabel="GRO_ENERGY_META",
            priority=TARGET_AFTER_SCALING_PRIORITY,
            effects=SetMaxDefense(
                value=Value + NamedReal(name="GRO_ENERGY_META_MAX_DEFENSE_FLAT", value=10 * PLANET_DEFENSE_FACTOR)
            ),
        ),
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner) & Happiness(low=0),
            stackinggroup="PLANET_SHIELDS_STACK_GEM",
            accountinglabel="GRO_ENERGY_META",
            priority=TARGET_AFTER_SCALING_PRIORITY,
            effects=SetMaxShield(
                value=Value + NamedReal(name="GRO_ENERGY_META_MAX_SHIELD_FLAT", value=50 * PLANET_SHIELD_FACTOR)
            ),
        ),
        EffectsGroup(
            scope=Planet()
            & OwnedBy(empire=Source.Owner)
            & Focus(type=["FOCUS_INDUSTRY"])
            & Happiness(low=NamedReal(name="GRO_ENERGY_META_MIN_STABILITY", value=20)),
            priority=TARGET_EARLY_BEFORE_SCALING_PRIORITY,
            effects=SetTargetIndustry(
                value=Value
                + Target.Population
                * NamedReal(name="GRO_ENERGY_META_TARGET_INDUSTRY_PERPOP", value=0.5 * INDUSTRY_PER_POP)
            ),
        ),
        EffectsGroup(
            scope=Planet()
            & OwnedBy(empire=Source.Owner)
            & Focus(type=["FOCUS_RESEARCH"])
            & Happiness(low=NamedRealLookup(name="GRO_ENERGY_META_MIN_STABILITY")),
            priority=TARGET_EARLY_BEFORE_SCALING_PRIORITY,
            effects=SetTargetResearch(
                value=Value
                + Target.Population
                * NamedReal(name="GRO_ENERGY_META_TARGET_RESEARCH_PERPOP", value=1.0 * RESEARCH_PER_POP)
            ),
        ),
    ],
    graphic="icons/tech/pure-energy_metabolism.png",
)
