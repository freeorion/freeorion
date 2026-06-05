from focs._conditions import (
    Contains,
    Enqueued,
    IsBuilding,
    OwnedBy,
)
from focs._effects import *
from focs._sources import Source
from focs._value_refs import CurrentContent

# Only check for own buildings. The client may have seen a building once, which has been
# destroyed while outside vision range. In this case the building remains forever in the
# client's context, but it should not stop a player from rebuilding the same type.
ENQUEUE_BUILD_ONE_PER_PLANET = (
    ~Contains(IsBuilding(name=[CurrentContent]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name=CurrentContent)
    & OwnedBy(empire=Source.Owner)
)
