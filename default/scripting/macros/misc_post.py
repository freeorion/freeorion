from focs._conditions import Contains, OwnedBy, Planet, System, WithinStarlaneJumps
from focs._enums import AnyEmpire
from focs._value_refs import (
    NamedIntegerLookup,
)

MINIMUM_DISTANCE_EMPIRE_CHECK = ~WithinStarlaneJumps(
    jumps=NamedIntegerLookup(name="MIN_MONSTER_DISTANCE"),
    condition=System & Contains(Planet() & OwnedBy(affiliation=AnyEmpire)),
)
