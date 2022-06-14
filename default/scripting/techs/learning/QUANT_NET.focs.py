from common.base_prod import RESEARCH_PER_POP, TECH_COST_MULTIPLIER
from common.priorities import TARGET_AFTER_SCALING_PRIORITY

Tech(
    name="LRN_QUANT_NET",
    description="LRN_QUANT_NET_DESC",
    short_description="RESEARCH_SHORT_DESC",
    category="LEARNING_CATEGORY",
    researchcost=200 * TECH_COST_MULTIPLIER,
    researchturns=6,
    tags=["PEDIA_LEARNING_CATEGORY"],
    prerequisites="LRN_NDIM_SUBSPACE",
    effectsgroups=[
        EffectsGroup(
            scope=ProductionCenter
            & OwnedBy(empire=Source.Owner)
            & Focus(type="FOCUS_RESEARCH")
            & Happiness(low=NamedReal(name="LRN_QUANT_NET_MIN_STABILITY", value=10)),
            priority=TARGET_AFTER_SCALING_PRIORITY,
            effects=SetTargetResearch(
                value=Value
                + Target.Population
                * NamedReal(name="LRN_QUANT_NET_TARGET_RESEARCH_PERPOP", value=(0.5 * RESEARCH_PER_POP))
            ),
        )
    ],
    graphic="icons/tech/quantum_networking.png",
)
