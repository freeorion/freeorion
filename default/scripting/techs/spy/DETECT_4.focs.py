from focs._effects import (
    EffectsGroup,
    IsSource,
    NamedReal,
    OwnedBy,
    OwnerHasTech,
    Planet,
    SetDetection,
    SetEmpireMeter,
    Source,
    Value,
)
from focs._tech import *
from macros.base_prod import TECH_COST_MULTIPLIER

Tech(
    name="SPY_DETECT_4",
    description="SPY_DETECT_4_DESC",
    short_description="DETECTION_SHORT_DESC",
    category="SPY_CATEGORY",
    researchcost=840 * TECH_COST_MULTIPLIER,
    researchturns=7,
    tags=["PEDIA_SPY_CATEGORY"],
    prerequisites=["SPY_DETECT_3"],
    unlock=Item(type=UnlockShipPart, name="DT_DETECTOR_4"),
    effectsgroups=[
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner),
            activation=~OwnerHasTech(name="SPY_DETECT_5"),
            effects=SetDetection(value=Value + NamedReal(name="SPY_DETECT_4_RANGE", value=125.0)),
        ),
        EffectsGroup(
            scope=IsSource,
            activation=~OwnerHasTech(name="SPY_DETECT_5"),
            effects=SetEmpireMeter(empire=Source.Owner, meter="METER_DETECTION_STRENGTH", value=Value + 70),
        ),
    ],
)
