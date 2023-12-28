# Only check for own buildings. The client may have seen a building once, which has been
# destroyed while outside vision range. In this case the building remains forever in the
# client's context, but it should not stop a player from rebuilding the same type.
from focs._effects import BuildBuilding, Contains, CurrentContent, Enqueued, IsBuilding, OwnedBy, Source

ENQUEUE_BUILD_ONE_PER_PLANET = (
    ~Contains(IsBuilding(name=[CurrentContent]) & OwnedBy(empire=Source.Owner))
    & ~Enqueued(type=BuildBuilding, name=CurrentContent)
    & OwnedBy(empire=Source.Owner)
)

ENQUEUE_ARTIFICIAL_PLANET_EXCLUSION = (
    ~Contains(IsBuilding(name=["BLD_ART_PLANET"]))
    & ~Contains(IsBuilding(name=["BLD_ART_FACTORY_PLANET"]))
    & ~Contains(IsBuilding(name=["BLD_ART_PARADISE_PLANET"]))
    & ~Enqueued(type=BuildBuilding, name="BLD_ART_PLANET")
    & ~Enqueued(type=BuildBuilding, name="BLD_ART_FACTORY_PLANET")
    & ~Enqueued(type=BuildBuilding, name="BLD_ART_PARADISE_PLANET")
)
