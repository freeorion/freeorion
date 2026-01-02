from focs._effects import (
    Destroy,
    EffectsGroup,
    IsSource,
    MaxOf,
    MinOf,
    Monster,
    MoveTowards,
    Number,
    SetSize,
    SetSpeed,
    SetTargetResearch,
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

FieldType(
    name="FLD_METEOR_BLIZZARD",
    description="FLD_METEOR_BLIZZARD_DESC",
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
            activation=(Source.Age <= MaxOf(float, (UniverseWidth**1.1) / 50, 20)) & Size(high=50),
            effects=SetSize(value=Value + MinOf(float, MaxOf(float, Value * RandomNumber(float, 0.05, 0.1), 1.0), 3.0)),
        ),
        EffectsGroup(  # shrink size when old
            scope=IsSource,
            activation=(Source.Age >= MaxOf(float, (UniverseWidth**1.1) / 50, 20)),
            effects=SetSize(value=Value - MinOf(float, MaxOf(float, Value * RandomNumber(float, 0.05, 0.1), 1.0), 3.0)),
        ),
        EffectsGroup(  # grant research
            scope=Ship & WithinDistance(distance=Source.Size * 0.9, condition=IsSource) & ~Monster,
            stackinggroup="METEOR_BLIZZARD_SHIP_EFFECTS",
            effects=SetTargetResearch(
                value=Value
                + 2
                / (
                    1
                    + StatisticCount(
                        float,
                        condition=Ship & WithinDistance(distance=Source.Size * 0.9, condition=IsSource) & ~Monster,
                    )
                )
                ** 0.5
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
            scope=IsSource, activation=(Source.Age >= 10) & Size(high=10), effects=Destroy
        ),
    ],
    graphic="fields/meteor_blizzard.png",
)
