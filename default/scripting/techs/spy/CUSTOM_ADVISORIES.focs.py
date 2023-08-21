import custom_sitreps
from focs._tech import *

Tech(
    name="SPY_CUSTOM_ADVISORIES",
    description="SPY_CUSTOM_ADVISORIES_DESC",
    short_description="SPY_CUSTOM_ADVISORIES",
    category="SPY_CATEGORY",
    researchcost=1,
    researchturns=1,
    tags=["PEDIA_SPY_CATEGORY"],
    effectsgroups=custom_sitreps.effectsgroups,
)
