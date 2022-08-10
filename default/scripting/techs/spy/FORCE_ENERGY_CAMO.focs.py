from common.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="CON_FRC_ENRG_CAMO",
    description="CON_FRC_ENRG_CAMO_DESC",
    short_description="STEALTH_SHORT_DESC",
    category="SPY_CATEGORY",
    researchcost=125 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_SPY_CATEGORY"],
    prerequisites="CON_FRC_ENRG_STRC",
    effectsgroups=EffectsGroup(scope=Building() & OwnedBy(empire=Source.Owner), effects=SetStealth(value=Value + 20)),
)
