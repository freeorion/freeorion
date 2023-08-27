from focs._effects import EffectsGroup, IsBuilding, NamedReal, Planet, SetStealth, Value
from focs._tech import *

Tech(
    name="SPY_PLANET_STEALTH_MOD",
    description="SPY_PLANET_STEALTH_MOD_DESC",
    short_description="GAME_MAKE_WORK_SHORT_DESC",
    category="SPY_CATEGORY",
    researchcost=1,
    researchturns=1,
    researchable=False,
    tags=["PEDIA_SPY_CATEGORY"],
    effectsgroups=[
        EffectsGroup(
            scope=IsBuilding() | Planet(),
            stackinggroup="PLANET_STEALTH_MOD_STACK",
            accountinglabel="BASIC_PLANET_STEALTH",
            effects=SetStealth(value=Value + NamedReal(name="PLANET_BASIC_STEALTH", value=5.0)),
        )
    ],
    graphic="",
)
