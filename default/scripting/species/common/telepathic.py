from focs._effects import (
    AnyEmpire,
    Basic,
    EffectsGroup,
    IsSource,
    MaxOf,
    OwnedBy,
    Planet,
    SetVisibility,
    Source,
    VisibleToEmpire,
    WithinStarlaneJumps,
)


def PRECOGNITIVE_DETECTION(jumps: int):
    return [
        EffectsGroup(
            description="PRECOGNITIVE_DETECTION_DESC",
            scope=Planet()
            & ~VisibleToEmpire(empire=Source.Owner)
            & ~IsSource
            & ~OwnedBy(empire=Source.Owner)
            & WithinStarlaneJumps(jumps=jumps, condition=IsSource),
            activation=OwnedBy(affiliation=AnyEmpire),
            effects=SetVisibility(empire=Source.Owner, visibility=MaxOf("Visibility", Basic, ValueVisibility)),  # type: ignore # noqa: F821
        )
    ]
