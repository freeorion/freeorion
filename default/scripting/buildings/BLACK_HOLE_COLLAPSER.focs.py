from focs._buildings import BuildingType
from focs._conditions import ContainedBy, Contains, IsSource, OwnedBy, Planet, Star
from focs._effects_new import CreateField, Destroy, EffectsGroup
from focs._enums import BlackHole
from focs._sources import Source
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_BLACK_HOLE_COLLAPSER",
    description="BLD_BLACK_HOLE_COLLAPSER_DESC",
    buildcost=5000 * BUILDING_COST_MULTIPLIER,
    buildtime=10,
    location=(Planet() & OwnedBy(empire=Source.Owner) & ContainedBy(Contains(Planet() & Star(type=[BlackHole])))),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        EffectsGroup(
            scope=IsSource,
            effects=[
                Destroy,
                CreateField(type="FLD_SUBSPACE_RIFT", size=100),
            ],
        ),
    ],
    icon="",
)
