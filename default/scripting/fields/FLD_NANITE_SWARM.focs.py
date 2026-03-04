from focs._effects import (
    Destroy,
    EffectsGroup,
    IsSource,
    MaxOf,
    MinOf,
    Monster,
    MoveTowards,
    NamedReal,
    Number,
    SetSize,
    SetSpeed,
    SetStructure,
    Ship,
    Size,
    Source,
    StatisticCount,
    Value,
    WithinDistance,
)
from focs._effects_new import MoveInOrbit
from focs._fields import FieldType
from focs._value_refs import RandomNumber, UniverseCentreX, UniverseCentreY, UniverseWidth
from macros.misc import SHIP_STRUCTURE_FACTOR

FieldType(
    name="FLD_NANITE_SWARM",
    description="FLD_NANITE_SWARM_DESC",
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
            activation=(Source.Age <= MaxOf(float, (UniverseWidth**1.1) / 50, 20)) & Size(high=50),
            effects=SetSize(value=Value + MinOf(float, MaxOf(float, Value * RandomNumber(float, 0.05, 0.1), 1.0), 3.0)),
        ),
        EffectsGroup(  # shrink size when old
            scope=IsSource,
            activation=(Source.Age >= MaxOf(float, (UniverseWidth**1.1) / 50, 20)),
            effects=SetSize(value=Value - MinOf(float, MaxOf(float, Value * RandomNumber(float, 0.05, 0.1), 1.0), 3.0)),
        ),
        EffectsGroup(  # increase ship structure
            scope=Ship & WithinDistance(distance=Source.Size * 0.9, condition=IsSource) & ~Monster,
            stackinggroup="NANITE_SWARM_SHIP_REPAIR",
            effects=SetStructure(
                value=Value
                + (NamedReal(name="FLD_NANITE_SWARM_STRUCTURE_FLAT", value=5 * SHIP_STRUCTURE_FACTOR))
                / MaxOf(
                    float,
                    1.0,
                    StatisticCount(
                        float,
                        condition=Ship & WithinDistance(distance=Source.Size * 0.9, condition=IsSource) & ~Monster,
                    ),
                )
            ),
        ),
        EffectsGroup(  # shrink when ships are in...
            scope=IsSource,
            activation=Number(
                low=1, condition=Ship & WithinDistance(distance=Source.Size * 0.9, condition=IsSource) & ~Monster
            ),
            effects=SetSize(
                value=Value
                - StatisticCount(
                    float, condition=Ship & WithinDistance(distance=Source.Size * 0.9, condition=IsSource) & ~Monster
                )
            ),
        ),
        EffectsGroup(  # after reaching a certain age, dissipate when small
            scope=IsSource,
            activation=(Source.Age >= 10) & Size(high=10),
            effects=Destroy,
        ),
    ],
    graphic="fields/nanite_swarm.png",
)
