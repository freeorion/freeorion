from fields.fields import CREATE_PLANETS
from focs._effects import (
    AddSpecial,
    CanSee,
    CreateField,
    Destroy,
    EffectsGroup,
    GenerateSitRepMessage,
    IsSource,
    MaxOf,
    MinOf,
    NamedRealLookup,
    NoStar,
    Object,
    OneOf,
    Orange,
    Red,
    SetDetection,
    SetSize,
    SetSpeed,
    SetStarType,
    SetStealth,
    Ship,
    Size,
    Source,
    Star,
    System,
    Value,
    White,
    WithinDistance,
    Yellow,
)
from focs._fields import FieldType
from focs._value_refs import RandomNumber

FieldType(
    name="FLD_NEBULA_1",
    description="FLD_NEBULA_1_DESC",
    stealth=0.01,
    effectsgroups=[
        EffectsGroup(  # shrink slowly when in no-star system
            scope=IsSource,
            activation=Size(low=2, high=120) & Star(type=[NoStar]),
            effects=SetSize(value=Value + RandomNumber(float, -0.8, 0.3)),
        ),
        EffectsGroup(  # shrink slowly when in no-star system
            scope=IsSource,
            activation=Size(low=120) & Star(type=[NoStar]),
            effects=SetSize(value=Value + RandomNumber(float, -0.8, 0)),
        ),
        EffectsGroup(  # spawn new star when small enough
            scope=System & Object(id=Source.SystemID),
            activation=Size(high=5) & Star(type=[NoStar]),
            effects=[
                SetStarType(type=OneOf("StarType", White, Yellow, Orange, Red)),
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
    graphic="fields/star_forming_nebula_1.png",
)
