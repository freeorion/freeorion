from focs._conditions import HasTag, IsSource, OwnedBy, Planet, Population, VisibleToEmpire, WithinStarlaneJumps
from focs._effects import AnyEmpire, Basic, Source
from focs._effects_new import EffectsGroup, SetVisibility
from focs._value_refs import (
    MaxOf,
    ValueVisibility,
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
            effects=SetVisibility(empire=Source.Owner, visibility=MaxOf("Visibility", Basic, ValueVisibility)),
        )
    ]


def TELEPATHIC_DETECTION(jumps: int) -> list:
    return [
        EffectsGroup(
            description="TELEPATHIC_DETECTION_DESC",
            scope=Planet()
            & ~Population(high=0)
            & ~OwnedBy(empire=Source.Owner)
            & ~VisibleToEmpire(empire=Source.Owner)
            & WithinStarlaneJumps(jumps=jumps, condition=IsSource)
            & ~IsSource,
            activation=OwnedBy(affiliation=AnyEmpire),
            effects=SetVisibility(empire=Source.Owner, visibility=MaxOf("Visibility", Basic, ValueVisibility)),
        )
    ]


def HAEMAESTHETIC_DETECTION(jumps: int) -> list:
    return [
        EffectsGroup(
            description="HAEMAESTHETIC_DETECTION_DESC",
            scope=Planet()
            & ~Population(high=0)
            & HasTag(name="ORGANIC")
            & ~OwnedBy(empire=Source.Owner)
            & ~VisibleToEmpire(empire=Source.Owner)
            & WithinStarlaneJumps(jumps=jumps, condition=IsSource)
            & ~IsSource,
            activation=OwnedBy(affiliation=AnyEmpire),
            effects=SetVisibility(empire=Source.Owner, visibility=MaxOf("Visibility", Basic, ValueVisibility)),
        )
    ]
