from focs._effects import (
    Contains,
    IsBuilding,
    OwnedBy,
    Planet,
    Source,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_TRANSFORMER",
    description="BLD_TRANSFORMER_DESC",
    buildcost=600 * BUILDING_COST_MULTIPLIER,
    buildtime=8,
    effectsgroups=[],
    location=(Planet() & ~Contains(IsBuilding(name=["BLD_TRANSFORMER"])) & OwnedBy(empire=Source.Owner)),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    icon="icons/building/transformer.png",
)
