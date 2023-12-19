try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

# Only check for own buildings. The client may have seen a building once, which has been
# destroyed while outside vision range. In this case the building remains forever in the
# client's context, but it should not stop a player from rebuilding the same type.
ENQUEUE_BUILD_ONE_PER_PLANET = (
    ~Contains(IsBuilding(name=[CurrentContent]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name=CurrentContent)
    & OwnedBy(empire=Source.Owner)
)
