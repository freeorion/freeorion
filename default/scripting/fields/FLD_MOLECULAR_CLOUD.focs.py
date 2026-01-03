from focs._effects import (
    Destroy,
    EffectsGroup,
    IsSource,
    LocalCandidate,
    MaxOf,
    MinOf,
    MoveTowards,
    NamedReal,
    SetMaxShield,
    SetSize,
    SetSpeed,
    Ship,
    Size,
    Source,
    Value,
    WithinDistance,
)
from focs._effects_new import MoveInOrbit
from focs._fields import FieldType
from focs._value_refs import RandomNumber, UniverseCentreX, UniverseCentreY, UniverseWidth
from macros.misc import SHIP_SHIELD_FACTOR

FieldType(
    name="FLD_MOLECULAR_CLOUD",
    description="FLD_MOLECULAR_CLOUD_DESC",
    stealth=0.01,
    tags=["EXOTIC"],
    effectsgroups=[
        EffectsGroup(  # move around
            scope=IsSource,
            effects=[
                SetSpeed(value=5),
                MoveTowards(speed=Source.Speed / 2.8, x=UniverseCentreX, y=UniverseCentreY),
                MoveInOrbit(speed=Source.Speed / 1.4, x=UniverseCentreX, y=UniverseCentreY),
            ],
        ),
        EffectsGroup(  # grow size when young
            scope=IsSource,
            activation=(Source.Age <= MaxOf(float, (UniverseWidth**1.1) / 50, 30)) & Size(high=100),
            effects=SetSize(value=Value + MinOf(float, MaxOf(float, Value * RandomNumber(float, 0.05, 0.1), 1.0), 5.0)),
        ),
        EffectsGroup(  # shrink size when old
            scope=IsSource,
            activation=(LocalCandidate.Age >= MaxOf(float, (UniverseWidth**1.1) / 50, 30)),
            effects=SetSize(value=Value - MinOf(float, MaxOf(float, Value * RandomNumber(float, 0.05, 0.1), 1.0), 5.0)),
        ),
        EffectsGroup(  # reduce shields
            scope=Ship & WithinDistance(distance=Source.Size * 0.9, condition=IsSource),
            stackinggroup="MOLECULAR_CLOUD_SHIELD_REDUCTION",
            effects=SetMaxShield(
                value=Value - NamedReal(name="FLD_MOLECULAR_CLOUD_SHIELD_MALUS", value=(15 * SHIP_SHIELD_FACTOR))
            ),
        ),
        EffectsGroup(  # after reaching a certain age, dissipate when small
            scope=IsSource, activation=(LocalCandidate.Age >= 10) & Size(high=10), effects=Destroy
        ),
    ],
    graphic="fields/molecular_cloud.png",
)
