from focs._effects import (
    BlackHole,
    EffectsGroup,
    InSystem,
    NamedReal,
    Neutron,
    NoStar,
    OwnedBy,
    Red,
    SetStealth,
    Ship,
    Source,
    Star,
    StatisticCount,
    Target,
    Value,
)
from focs._tech import *

Tech(
    name="SPY_ROOT_DECEPTION",
    description="SPY_ROOT_DECEPTION_DESC",
    short_description="THEORY_SHORT_DESC",
    category="SPY_CATEGORY",
    researchcost=1,
    researchturns=1,
    tags=["PEDIA_SPY_CATEGORY", "THEORY"],
    effectsgroups=[
        EffectsGroup(
            scope=Ship & OwnedBy(empire=Source.Owner) & Star(type=[NoStar]),
            accountinglabel="SPY_DECEPTION_EMPTY_SPACE_PENALTY",
            effects=SetStealth(value=Value + NamedReal(name="SPY_DECEPTION_EMPTY_SPACE_PENALTY", value=-10.0)),
        ),
        EffectsGroup(
            scope=Ship & OwnedBy(empire=Source.Owner) & Star(type=[Red]),
            accountinglabel="SPY_DECEPTION_DIM_STAR_PENALTY",
            effects=SetStealth(value=Value + NamedReal(name="SPY_DECEPTION_DIM_STAR_PENALTY", value=-5.0)),
        ),
        EffectsGroup(
            scope=Ship & OwnedBy(empire=Source.Owner) & Star(type=[Neutron]),
            accountinglabel="SPY_DECEPTION_SUBSTELLAR_INTERFERENCE",
            effects=SetStealth(value=Value + NamedReal(name="SPY_DECEPTION_NEUTRON_INTERFERENCE", value=5.0)),
        ),
        EffectsGroup(
            scope=Ship & OwnedBy(empire=Source.Owner) & Star(type=[BlackHole]),
            accountinglabel="SPY_DECEPTION_SUBSTELLAR_INTERFERENCE",
            effects=SetStealth(value=Value + NamedReal(name="SPY_DECEPTION_BLACK_INTERFERENCE", value=10.0)),
        ),
        EffectsGroup(
            scope=Ship & InSystem() & OwnedBy(empire=Source.Owner),
            accountinglabel="FLEET_UNSTEALTHINESS",
            effects=SetStealth(
                value=Value
                - StatisticCount(float, condition=Ship & InSystem(id=Target.SystemID) & OwnedBy(empire=Source.Owner))
            ),
        ),
        EffectsGroup(
            scope=Ship & ~InSystem() & OwnedBy(empire=Source.Owner),
            accountinglabel="FLEET_UNSTEALTHINESS",
            effects=SetStealth(
                value=Value
                - StatisticCount(
                    float,
                    condition=Ship
                    & ~InSystem()
                    & (
                        (LocalCandidate.NextSystemID == Target.SystemID)
                        | (LocalCandidate.NextSystemID == Target.SystemID)
                    )
                    & OwnedBy(empire=Source.Owner),
                )
            ),
        ),
    ],
)
