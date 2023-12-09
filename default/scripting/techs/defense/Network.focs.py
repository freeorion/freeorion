from focs._effects import (
    EffectsGroup,
    MaxOf,
    OwnedBy,
    Planet,
    SetDefense,
    SetMaxDefense,
    Source,
    Target,
    Value,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER
from macros.misc import PLANET_DEFENSE_FACTOR
from macros.priorities import DEFAULT_PRIORITY


def EG_DEFENSE_NET(multiplier: int, stack_postfix: str):
    return EffectsGroup(
        scope=Planet() & OwnedBy(empire=Source.Owner),
        stackinggroup="DEFENSE_NET_STACK_" + stack_postfix,
        accountinglabel="DEF_TECH_ACCOUNTING_LABEL",
        priority=DEFAULT_PRIORITY,
        effects=SetMaxDefense(value=Value + (multiplier * PLANET_DEFENSE_FACTOR)),
    )


Tech(
    name="DEF_DEFENSE_NET_1",
    description="DEF_DEFENSE_NET_1_DESC",
    short_description="DEFENSE_SHORT_DESC",
    category="DEFENSE_CATEGORY",
    researchcost=30 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_DEFENSE_CATEGORY"],
    prerequisites=["DEF_ROOT_DEFENSE"],
    effectsgroups=[EG_DEFENSE_NET(5, "1")],
    graphic="icons/tech/defense.png",
)

Tech(
    name="DEF_DEFENSE_NET_2",
    description="DEF_DEFENSE_NET_2_DESC",
    short_description="DEFENSE_SHORT_DESC",
    category="DEFENSE_CATEGORY",
    researchcost=96 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_DEFENSE_CATEGORY"],
    prerequisites=["DEF_DEFENSE_NET_1"],
    effectsgroups=[EG_DEFENSE_NET(15, "2")],
    graphic="icons/tech/defense.png",
)

Tech(
    name="DEF_DEFENSE_NET_3",
    description="DEF_DEFENSE_NET_3_DESC",
    short_description="DEFENSE_SHORT_DESC",
    category="DEFENSE_CATEGORY",
    researchcost=240 * TECH_COST_MULTIPLIER,
    researchturns=6,
    tags=["PEDIA_DEFENSE_CATEGORY"],
    prerequisites=["DEF_DEFENSE_NET_2"],
    effectsgroups=[EG_DEFENSE_NET(25, "3")],
    graphic="icons/tech/defense.png",
)

Tech(
    name="DEF_DEFENSE_NET_REGEN_1",
    description="DEF_DEFENSE_NET_REGEN_1_DESC",
    short_description="DEFENSE_SHORT_DESC",
    category="DEFENSE_CATEGORY",
    researchcost=120 * TECH_COST_MULTIPLIER,
    researchturns=6,
    tags=["PEDIA_DEFENSE_CATEGORY"],
    prerequisites=["DEF_DEFENSE_NET_2"],
    effectsgroups=[
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner),
            accountinglabel="DEF_TECH_ACCOUNTING_LABEL",
            effects=SetDefense(value=Value + 0.1 * Target.MaxDefense),
        )
    ],
    graphic="icons/tech/defense_regeneration.png",
)

Tech(
    name="DEF_DEFENSE_NET_REGEN_2",
    description="DEF_DEFENSE_NET_REGEN_2_DESC",
    short_description="DEFENSE_SHORT_DESC",
    category="DEFENSE_CATEGORY",
    researchcost=240 * TECH_COST_MULTIPLIER,
    researchturns=8,
    tags=["PEDIA_DEFENSE_CATEGORY"],
    prerequisites=["DEF_DEFENSE_NET_REGEN_1"],
    effectsgroups=[
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner),
            accountinglabel="DEF_TECH_ACCOUNTING_LABEL",
            effects=SetDefense(value=Value + MaxOf(float, 0.1 * Target.MaxDefense, 0.25 * Target.Construction)),
        )
    ],
    graphic="icons/tech/defense_regeneration.png",
)
