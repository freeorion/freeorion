from focs._effects import (
    EffectsGroup,
    IsSource,
    NamedReal,
    OwnedBy,
    Planet,
    SetDetection,
    SetEmpireMeter,
    Source,
    Value,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SPY_DETECT_5",
    description="SPY_DETECT_5_DESC",
    short_description="DETECTION_SHORT_DESC",
    category="SPY_CATEGORY",
    researchcost=1600 * TECH_COST_MULTIPLIER,
    researchturns=8,
    tags=["PEDIA_SPY_CATEGORY"],
    prerequisites=["SPY_DETECT_4"],
    effectsgroups=[
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner),
            effects=SetDetection(value=Value + NamedReal(name="SPY_DETECT_5_RANGE", value=150.0)),
        ),
        EffectsGroup(
            scope=IsSource,
            effects=SetEmpireMeter(empire=Source.Owner, meter="METER_DETECTION_STRENGTH", value=Value + 200),
        ),
    ],
)
