from focs._effects import EffectsGroup, IsBuilding, OwnedBy, SetStealth, Source, Value
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="CON_FRC_ENRG_CAMO",
    description="CON_FRC_ENRG_CAMO_DESC",
    short_description="STEALTH_SHORT_DESC",
    category="SPY_CATEGORY",
    researchcost=180 * TECH_COST_MULTIPLIER,
    researchturns=3,
    tags=["PEDIA_SPY_CATEGORY"],
    prerequisites=["CON_FRC_ENRG_STRC"],
    effectsgroups=[
        EffectsGroup(scope=IsBuilding() & OwnedBy(empire=Source.Owner), effects=SetStealth(value=Value + 20))
    ],
)
