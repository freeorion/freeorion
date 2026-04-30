from focs._effects import (
    AnyEmpire,
    Contains,
    NamedIntegerLookup,
    OwnedBy,
    Planet,
    System,
    WithinStarlaneJumps,
)

MINIMUM_DISTANCE_EMPIRE_CHECK = ~WithinStarlaneJumps(
    jumps=NamedIntegerLookup(name="MIN_MONSTER_DISTANCE"),
    condition=System & Contains(Planet() & OwnedBy(affiliation=AnyEmpire)),
)
