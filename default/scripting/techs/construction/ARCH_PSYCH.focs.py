from common.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="CON_ARCH_PSYCH",
    description="CON_ARCH_PSYCH_DESC",
    short_description="THEORY_SHORT_DESC",
    category="CONSTRUCTION_CATEGORY",
    researchcost=50 * TECH_COST_MULTIPLIER,
    researchturns=4,
    tags=["PEDIA_CONSTRUCTION_CATEGORY", "THEORY"],
    prerequisites="CON_ASYMP_MATS",
    unlock=[Item(type=Policy, name="PLC_MODERATION"), Item(type=Policy, name="PLC_RACIAL_PURITY")],
    effectsgroups=EffectsGroup(
        scope=Source,
        effects=SetEmpireMeter(empire=Source.Owner, meter="SOCIAL_CATEGORY_NUM_POLICY_SLOTS", value=Value + 1),
    ),
    graphic="icons/tech/architecture_psychology.png",
)
