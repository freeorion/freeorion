from fields.fields import CREATE_PLANETS
from focs._conditions import IsSource, Object, Ship, Size, Star, System, WithinDistance
from focs._effects import (
    AddSpecial,
    CreateField,
    Destroy,
    EffectsGroup,
    GenerateSitRepMessage,
    SetDetection,
    SetSize,
    SetSpeed,
    SetStarType,
    SetStealth,
)
from focs._enums import Blue, CanSee, NoStar, White, Yellow
from focs._fields import FieldType
from focs._sources import Source
from focs._value_refs import (
    MaxOf,
    MinOf,
    NamedRealLookup,
    OneOf,
    RandomNumber,
    Value,
)

FieldType(
    name="FLD_NEBULA_3",
    description="FLD_NEBULA_3_DESC",
    stealth=0.01,
    effectsgroups=[
        EffectsGroup(  # shrink slowly when in no-star system
            scope=IsSource,
            activation=Size(low=2, high=120) & Star(type=[NoStar]),
            effects=SetSize(value=Value + RandomNumber(float, -0.5, 0.3)),
        ),
        EffectsGroup(  # shrink slowly when in no-star system
            scope=IsSource,
            activation=Size(low=120) & Star(type=[NoStar]),
            effects=SetSize(value=Value + RandomNumber(float, -0.5, 0)),
        ),
        EffectsGroup(  # spawn new star when small enough
            scope=System & Object(id=Source.SystemID),
            activation=Size(high=5) & Star(type=[NoStar]),
            effects=[
                SetStarType(type=OneOf("StarType", Blue, Blue, White, Yellow)),
                GenerateSitRepMessage(
                    message="EFFECT_NEBULA",
                    label="EFFECT_NEBULA_LABEL",
                    icon="icons/buttons/addstar.png",
                    parameters={"system": Source.SystemID},
                    affiliation=CanSee,
                    condition=IsSource,
                ),
            ],
        ),
        *CREATE_PLANETS,
        EffectsGroup(  # make ships slower, reduce detection, increase stealth
            scope=Ship & WithinDistance(distance=Source.Size * 0.9, condition=IsSource),
            stackinggroup="NEBULA_SHIP_EFFECTS",
            effects=[
                SetSpeed(
                    value=MaxOf(float, MinOf(float, Value, 5), Value - NamedRealLookup(name="FLD_NEBULA_SPEED_MALUS"))
                ),
                SetStealth(value=Value + NamedRealLookup(name="FLD_NEBULA_STEALTH_BONUS")),
                SetDetection(value=Value - NamedRealLookup(name="FLD_NEBULA_DETECTION_MALUS")),
            ],
        ),
        EffectsGroup(  # dissipate when small
            scope=IsSource,
            activation=Size(high=5),
            effects=[
                Destroy,
                CreateField(type="FLD_ACCRETION_DISC", size=20),
                AddSpecial(name="ACCRETION_DISC_SPECIAL"),
            ],
        ),
    ],
    graphic="fields/star_forming_nebula_3.png",
)
