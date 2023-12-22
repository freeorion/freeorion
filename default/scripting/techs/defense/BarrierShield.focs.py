from focs._effects import (
    CurrentTurn,
    EffectsGroup,
    LocalCandidate,
    MaxOf,
    MinOf,
    NamedReal,
    OwnedBy,
    Planet,
    SetMaxShield,
    SetShield,
    Source,
    Target,
    Value,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER
from macros.misc import PLANET_SHIELD_FACTOR
from macros.priorities import AFTER_ALL_TARGET_MAX_METERS_PRIORITY, DEFAULT_PRIORITY

Tech(
    name="DEF_PLAN_BARRIER_SHLD_1",
    description="DEF_PLAN_BARRIER_SHLD_1_DESC",
    short_description="SHIELD_SHORT_DESC",
    category="DEFENSE_CATEGORY",
    researchcost=125 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_DEFENSE_CATEGORY"],
    prerequisites=["LRN_FORCE_FIELD"],
    effectsgroups=[
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner),
            accountinglabel="DEF_TECH_ACCOUNTING_LABEL",
            priority=DEFAULT_PRIORITY,
            effects=SetMaxShield(
                value=Value + NamedReal(name="DEF_PLAN_BARRIER_SHLD_1_MAX_SHIELD_FLAT", value=30 * PLANET_SHIELD_FACTOR)
            ),
        ),
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner) & (LocalCandidate.LastTurnAttackedByShip < CurrentTurn - 1),
            accountinglabel="DEF_TECH_ACCOUNTING_LABEL",
            priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
            effects=SetShield(
                value=Value
                + MaxOf(
                    float, MinOf(float, PLANET_SHIELD_FACTOR, Value), 0.25 * Target.Construction * PLANET_SHIELD_FACTOR
                )
            ),
        ),
    ],
    graphic="icons/tech/planetary_barrier_shield.png",
)

Tech(
    name="DEF_PLAN_BARRIER_SHLD_2",
    description="DEF_PLAN_BARRIER_SHLD_2_DESC",
    short_description="SHIELD_SHORT_DESC",
    category="DEFENSE_CATEGORY",
    researchcost=192 * TECH_COST_MULTIPLIER,
    researchturns=6,
    tags=["PEDIA_DEFENSE_CATEGORY"],
    prerequisites=["DEF_PLAN_BARRIER_SHLD_1"],
    effectsgroups=[
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner),
            accountinglabel="DEF_TECH_ACCOUNTING_LABEL",
            priority=DEFAULT_PRIORITY,
            effects=SetMaxShield(
                value=Value + NamedReal(name="DEF_PLAN_BARRIER_SHLD_2_MAX_SHIELD_FLAT", value=60 * PLANET_SHIELD_FACTOR)
            ),
        ),
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner) & (LocalCandidate.LastTurnAttackedByShip < CurrentTurn - 1),
            accountinglabel="DEF_TECH_ACCOUNTING_LABEL",
            priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
            effects=SetShield(
                value=Value
                + MaxOf(
                    float,
                    MinOf(float, 3.0 * PLANET_SHIELD_FACTOR, Value),
                    0.75 * Target.Construction * PLANET_SHIELD_FACTOR,
                )
            ),
        ),
    ],
    graphic="icons/tech/planetary_barrier_shield.png",
)

Tech(
    name="DEF_PLAN_BARRIER_SHLD_3",
    description="DEF_PLAN_BARRIER_SHLD_3_DESC",
    short_description="SHIELD_SHORT_DESC",
    category="DEFENSE_CATEGORY",
    researchcost=360 * TECH_COST_MULTIPLIER,
    researchturns=8,
    tags=["PEDIA_DEFENSE_CATEGORY"],
    prerequisites=["DEF_PLAN_BARRIER_SHLD_2"],
    effectsgroups=[
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner),
            accountinglabel="DEF_TECH_ACCOUNTING_LABEL",
            priority=DEFAULT_PRIORITY,
            effects=SetMaxShield(
                value=Value + NamedReal(name="DEF_PLAN_BARRIER_SHLD_3_MAX_SHIELD_FLAT", value=90 * PLANET_SHIELD_FACTOR)
            ),
        ),
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner) & (LocalCandidate.LastTurnAttackedByShip < CurrentTurn - 1),
            accountinglabel="DEF_TECH_ACCOUNTING_LABEL",
            priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
            effects=SetShield(
                value=Value
                + MaxOf(
                    float,
                    MinOf(float, 5.0 * PLANET_SHIELD_FACTOR, Value),
                    1.0 * Target.Construction * PLANET_SHIELD_FACTOR,
                )
            ),
        ),
    ],
    graphic="icons/tech/planetary_barrier_shield.png",
)

Tech(
    name="DEF_PLAN_BARRIER_SHLD_4",
    description="DEF_PLAN_BARRIER_SHLD_4_DESC",
    short_description="SHIELD_SHORT_DESC",
    category="DEFENSE_CATEGORY",
    researchcost=600 * TECH_COST_MULTIPLIER,
    researchturns=10,
    tags=["PEDIA_DEFENSE_CATEGORY"],
    prerequisites=["DEF_PLAN_BARRIER_SHLD_3"],
    effectsgroups=[
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner),
            accountinglabel="DEF_TECH_ACCOUNTING_LABEL",
            priority=DEFAULT_PRIORITY,
            effects=SetMaxShield(
                value=Value
                + NamedReal(name="DEF_PLAN_BARRIER_SHLD_4_MAX_SHIELD_FLAT", value=150 * PLANET_SHIELD_FACTOR)
            ),
        ),
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner) & (LocalCandidate.LastTurnAttackedByShip < CurrentTurn - 1),
            accountinglabel="DEF_TECH_ACCOUNTING_LABEL",
            priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
            effects=SetShield(
                value=Value
                + MaxOf(
                    float,
                    MinOf(float, 9.0 * PLANET_SHIELD_FACTOR, Value),
                    1.5 * Target.Construction * PLANET_SHIELD_FACTOR,
                )
            ),
        ),
    ],
    graphic="icons/tech/planetary_barrier_shield.png",
)

Tech(
    name="DEF_PLAN_BARRIER_SHLD_5",
    description="DEF_PLAN_BARRIER_SHLD_5_DESC",
    short_description="SHIELD_SHORT_DESC",
    category="DEFENSE_CATEGORY",
    researchcost=1200 * TECH_COST_MULTIPLIER,
    researchturns=12,
    tags=["PEDIA_DEFENSE_CATEGORY"],
    prerequisites=["DEF_PLAN_BARRIER_SHLD_4"],
    effectsgroups=[
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner),
            accountinglabel="DEF_TECH_ACCOUNTING_LABEL",
            priority=DEFAULT_PRIORITY,
            effects=SetMaxShield(
                value=Value
                + NamedReal(name="DEF_PLAN_BARRIER_SHLD_5_MAX_SHIELD_FLAT", value=150 * PLANET_SHIELD_FACTOR)
            ),
        ),
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner) & (LocalCandidate.LastTurnAttackedByShip < CurrentTurn - 1),
            accountinglabel="DEF_TECH_ACCOUNTING_LABEL",
            priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
            effects=SetShield(
                value=Value
                + MaxOf(
                    float,
                    MinOf(float, 14.0 * PLANET_SHIELD_FACTOR, Value),
                    2.5 * Target.Construction * PLANET_SHIELD_FACTOR,
                )
            ),
        ),
    ],
    graphic="icons/tech/planetary_barrier_shield.png",
)
