from focs._buildings import *
from focs._conditions import Contains, IsBuilding, OwnedBy, Planet
from focs._sources import Source
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_BIO_LAB",
    description="BLD_BIO_LAB_DESC",
    buildcost=10 * BUILDING_COST_MULTIPLIER,
    buildtime=4,
    location=(Planet() & OwnedBy(empire=Source.Owner) & ~Contains(IsBuilding(name=["BLD_BIO_LAB"]))),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[],
    icon="icons/tech/biomimesis.png",
)
