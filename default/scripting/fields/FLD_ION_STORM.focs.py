from focs._effects import (
    Destroy,
    EffectsGroup,
    IsField,
    IsSource,
    LocalCandidate,
    MaxOf,
    MinOf,
    MoveTowards,
    NamedReal,
    SetDetection,
    SetSize,
    SetSpeed,
    SetStealth,
    Size,
    Source,
    Value,
    WithinDistance,
)
from focs._effects_new import MoveInOrbit
from focs._fields import FieldType
from focs._value_refs import RandomNumber, UniverseCentreX, UniverseCentreY, UniverseWidth

FieldType(
    name="FLD_ION_STORM",
    description="FLD_ION_STORM_DESC",
    stealth=0.01,
    tags=["EXOTIC"],
    effectsgroups=[
        EffectsGroup(  # move around
            scope=IsSource,
            effects=[
                SetSpeed(value=10),
                MoveTowards(speed=Source.Speed / 2.8, x=UniverseCentreX, y=UniverseCentreY),
                MoveInOrbit(speed=Source.Speed / 1.4, x=UniverseCentreX, y=UniverseCentreY),
            ],
        ),
        EffectsGroup(  # grow size when young
            scope=IsSource,
            activation=(LocalCandidate.Age <= MaxOf(float, (UniverseWidth**1.1) / 50, 20)) & Size(high=50),
            effects=SetSize(value=Value + MinOf(float, MaxOf(float, Value * RandomNumber(float, 0.05, 0.1), 1.0), 3.0)),
        ),
        EffectsGroup(  # shrink size when old
            scope=IsSource,
            activation=(Source.Age >= MaxOf(float, (UniverseWidth**1.1) / 50, 20)),
            effects=SetSize(value=Value - MinOf(float, MaxOf(float, Value * RandomNumber(float, 0.05, 0.1), 1.0), 3.0)),
        ),
        EffectsGroup(  # affect stealth / detection of objects in storm
            scope=~IsField() & WithinDistance(distance=Source.Size * 0.9, condition=IsSource),
            stackinggroup="ION_STORM_STEALTH_DETECTION_REDUCTION",
            effects=[
                SetStealth(value=Value + NamedReal(name="FLD_ION_STORM_STEALTH_BONUS", value=40)),
                SetDetection(value=Value - NamedReal(name="FLD_ION_STORM_DETECTION_MALUS", value=40)),
            ],
        ),
        EffectsGroup(  # after reaching a certain age, dissipate when small
            scope=IsSource, activation=(LocalCandidate.Age >= 10) & Size(high=10), effects=Destroy
        ),
    ],
    graphic="fields/ion_storm.png",
)
