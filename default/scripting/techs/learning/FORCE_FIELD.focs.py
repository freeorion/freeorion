from common.base_prod import TECH_COST_MULTIPLIER
from common.misc import PLANET_SHIELD_FACTOR
from common.priorities import DEFAULT_PRIORITY

Tech(
    name="LRN_FORCE_FIELD",
    description="LRN_FORCE_FIELD_DESC",
    short_description="SHIELD_SHORT_DESC",
    category="LEARNING_CATEGORY",
    researchcost=50 * TECH_COST_MULTIPLIER,
    researchturns=5,
    tags=["PEDIA_LEARNING_CATEGORY"],
    prerequisites="LRN_NASCENT_AI",
    unlock=Item(type=ShipPart, name="SH_DEFENSE_GRID"),
    effectsgroups=[
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner),
            stackinggroup="PLANET_SHIELDS_STACK_FF",
            priority=DEFAULT_PRIORITY,
            effects=SetMaxShield(value=Value + (10 * PLANET_SHIELD_FACTOR), accountinglabel="LRN_FORCE_FIELD"),
        )
    ],
    graphic="icons/tech/forcefield_harmonics.png",
)
