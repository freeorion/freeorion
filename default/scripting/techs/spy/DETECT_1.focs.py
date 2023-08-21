from focs._effects import (
    BlackHole,
    Blue,
    EffectsGroup,
    IsSource,
    NamedReal,
    Neutron,
    NoStar,
    OwnedBy,
    OwnerHasTech,
    Planet,
    SetDetection,
    SetEmpireMeter,
    Ship,
    Source,
    Star,
    Value,
    White,
    Yellow,
)
from focs._tech import *

Tech(
    name="SPY_DETECT_1",
    description="SPY_DETECT_1_DESC",
    short_description="DETECTION_SHORT_DESC",
    category="SPY_CATEGORY",
    researchcost=1,
    researchturns=1,
    researchable=False,
    tags=["PEDIA_SPY_CATEGORY"],
    effectsgroups=[
        EffectsGroup(
            scope=Planet() & OwnedBy(empire=Source.Owner),
            activation=~OwnerHasTech(name="SPY_DETECT_2")
            & ~OwnerHasTech(name="SPY_DETECT_3")
            & ~OwnerHasTech(name="SPY_DETECT_4")
            & ~OwnerHasTech(name="SPY_DETECT_5"),
            effects=SetDetection(value=Value + NamedReal(name="SPY_DETECT_1_RANGE", value=50.0)),
        ),
        EffectsGroup(
            scope=IsSource,
            activation=~OwnerHasTech(name="SPY_DETECT_2")
            & ~OwnerHasTech(name="SPY_DETECT_3")
            & ~OwnerHasTech(name="SPY_DETECT_4")
            & ~OwnerHasTech(name="SPY_DETECT_5"),
            effects=SetEmpireMeter(empire=Source.Owner, meter="METER_DETECTION_STRENGTH", value=Value + 10),
        ),
        EffectsGroup(
            scope=(Planet() | Ship) & OwnedBy(empire=Source.Owner) & Star(type=[Blue, Neutron]),
            accountinglabel="SPY_DETECT_STELLAR_INTERFERENCE",
            effects=SetDetection(value=Value - 15),
        ),
        EffectsGroup(
            scope=(Planet() | Ship) & OwnedBy(empire=Source.Owner) & Star(type=[White]),
            accountinglabel="SPY_DETECT_STELLAR_INTERFERENCE",
            effects=SetDetection(value=Value - 10),
        ),
        EffectsGroup(
            scope=(Planet() | Ship) & OwnedBy(empire=Source.Owner) & Star(type=[Yellow]),
            accountinglabel="SPY_DETECT_STELLAR_INTERFERENCE",
            effects=SetDetection(value=Value - 5),
        ),
        EffectsGroup(
            scope=(Planet() | Ship) & OwnedBy(empire=Source.Owner) & Star(type=[BlackHole]),
            accountinglabel="SPY_DETECT_LENSING",
            effects=SetDetection(value=Value + 5),
        ),
        EffectsGroup(
            scope=(Planet() | Ship) & OwnedBy(empire=Source.Owner) & Star(type=[NoStar]),
            accountinglabel="SPY_DETECT_CLEAN_STELLAR_BACKGROUND",
            effects=SetDetection(value=Value + 10),
        ),
    ],
)
