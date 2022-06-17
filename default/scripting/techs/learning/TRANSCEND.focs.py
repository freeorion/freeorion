from common.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="LRN_TRANSCEND",
    description="LRN_TRANSCEND_DESC",
    short_description="VICTORY_SHORT_DESC",
    category="LEARNING_CATEGORY",
    researchcost=500.0 * GalaxySize * GameRule(type=float, name="RULE_SINGULARITY_COST_FACTOR") * TECH_COST_MULTIPLIER,
    researchturns=20,
    tags=["PEDIA_LEARNING_CATEGORY"],
    prerequisites=[
        "CON_TRANS_ARCH",
        "LRN_PSY_DOM",
        "PRO_ZERO_GEN",
        "GRO_ENERGY_META",
    ],
    effectsgroups=EffectsGroup(scope=Source, effects=[Victory(reason="VICTORY_TECH")]),
    graphic="icons/tech/singularity_of_transcendence.png",
)
