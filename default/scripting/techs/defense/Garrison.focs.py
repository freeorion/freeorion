from focs._effects import (
    CurrentTurn,
    EffectsGroup,
    LocalCandidate,
    MinOf,
    NamedReal,
    OwnedBy,
    Planet,
    Population,
    SetMaxTroops,
    SetTroops,
    Source,
    Target,
    Value,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER, TROOPS_PER_POP
from macros.priorities import (
    AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
    TARGET_AFTER_SCALING_PRIORITY,
    TARGET_EARLY_BEFORE_SCALING_PRIORITY,
)

Tech(
    name="DEF_GARRISON_1",
    description="DEF_GARRISON_1_DESC",
    short_description="TROOPS_SHORT_DESC",
    category="DEFENSE_CATEGORY",
    researchcost=9 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_DEFENSE_CATEGORY"],
    prerequisites=["DEF_ROOT_DEFENSE"],
    effectsgroups=[
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner),
            stackinggroup="GARRISON_1_TROOPS_STACK",
            accountinglabel="DEF_TECH_ACCOUNTING_LABEL",
            priority=TARGET_AFTER_SCALING_PRIORITY,
            effects=SetMaxTroops(value=Value + NamedReal(name="GARRISON_1_MAX_TROOPS_FLAT", value=6)),
        ),
    ],
    graphic="icons/tech/troops.png",
)

Tech(
    name="DEF_GARRISON_2",
    description="DEF_GARRISON_2_DESC",
    short_description="TROOPS_SHORT_DESC",
    category="DEFENSE_CATEGORY",
    researchcost=25 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_DEFENSE_CATEGORY"],
    prerequisites=["DEF_GARRISON_1"],
    unlock=[
        Item(type=UnlockPolicy, name="PLC_CHECKPOINTS"),
        Item(type=UnlockPolicy, name="PLC_INSURGENCY"),
    ],
    effectsgroups=[
        EffectsGroup(
            scope=Planet()
            & OwnedBy(empire=Source.Owner)
            & (LocalCandidate.LastTurnConquered < CurrentTurn)
            & (LocalCandidate.LastTurnAttackedByShip < CurrentTurn),
            accountinglabel="DEF_TECH_ACCOUNTING_LABEL",
            priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
            effects=SetTroops(
                value=MinOf(
                    float, Value(Target.MaxTroops), Value + NamedReal(name="GARRISON_2_TROOPREGEN_FLAT", value=1)
                )
            ),
        ),
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner) & Population(low=0.01),
            stackinggroup="GARRISON_2_TROOPS_STACK",
            accountinglabel="DEF_TECH_ACCOUNTING_LABEL",
            priority=TARGET_EARLY_BEFORE_SCALING_PRIORITY,
            effects=SetMaxTroops(
                value=Value
                + Target.Population * NamedReal(name="GARRISON_2_MAXTROOPS_PERPOP", value=2 * TROOPS_PER_POP)
            ),
        ),
    ],
    graphic="icons/tech/troops.png",
)

Tech(
    name="DEF_GARRISON_3",
    description="DEF_GARRISON_3_DESC",
    short_description="TROOPS_SHORT_DESC",
    category="DEFENSE_CATEGORY",
    researchcost=84 * TECH_COST_MULTIPLIER,
    researchturns=7,
    tags=["PEDIA_DEFENSE_CATEGORY"],
    prerequisites=["DEF_GARRISON_2"],
    effectsgroups=[
        EffectsGroup(
            scope=Planet()
            & OwnedBy(empire=Source.Owner)
            & (LocalCandidate.LastTurnConquered < CurrentTurn)
            & (LocalCandidate.LastTurnAttackedByShip < CurrentTurn),
            accountinglabel="DEF_TECH_ACCOUNTING_LABEL",
            priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
            effects=SetTroops(
                value=MinOf(
                    float, Value(Target.MaxTroops), Value + NamedReal(name="GARRISON_3_TROOPREGEN_FLAT", value=2)
                )
            ),
        ),
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner),
            stackinggroup="GARRISON_3_TROOPS_STACK",
            accountinglabel="DEF_TECH_ACCOUNTING_LABEL",
            priority=TARGET_AFTER_SCALING_PRIORITY,
            effects=SetMaxTroops(value=Value + NamedReal(name="GARRISON_3_MAX_TROOPS_FLAT", value=18)),
        ),
    ],
    graphic="icons/tech/troops.png",
)

Tech(
    name="DEF_GARRISON_4",
    description="DEF_GARRISON_4_DESC",
    short_description="TROOPS_SHORT_DESC",
    category="DEFENSE_CATEGORY",
    researchcost=216 * TECH_COST_MULTIPLIER,
    researchturns=9,
    tags=["PEDIA_DEFENSE_CATEGORY"],
    prerequisites=["DEF_GARRISON_3"],
    unlock=Item(type=UnlockPolicy, name="PLC_MARTIAL_LAW"),
    effectsgroups=[
        EffectsGroup(
            scope=Planet()
            & OwnedBy(empire=Source.Owner)
            & (LocalCandidate.LastTurnConquered < CurrentTurn)
            & (LocalCandidate.LastTurnAttackedByShip < CurrentTurn),
            accountinglabel="DEF_TECH_ACCOUNTING_LABEL",
            priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
            effects=SetTroops(
                value=MinOf(
                    float, Value(Target.MaxTroops), Value + NamedReal(name="GARRISON_4_TROOPREGEN_FLAT", value=4)
                )
            ),
        ),
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner) & Population(low=0.01),
            stackinggroup="GARRISON_4_TROOPS_STACK",
            accountinglabel="DEF_TECH_ACCOUNTING_LABEL",
            priority=TARGET_EARLY_BEFORE_SCALING_PRIORITY,
            effects=SetMaxTroops(
                value=Value
                + Target.Population * NamedReal(name="GARRISON_4_MAX_TROOPS_PERPOP", value=3 * TROOPS_PER_POP)
            ),
        ),
    ],
    graphic="icons/tech/troops.png",
)
