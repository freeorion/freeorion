from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    AsteroidsType,
    Contains,
    Destroy,
    EffectsGroup,
    IsBuilding,
    IsSource,
    OwnedBy,
    Planet,
    Source,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET, LOCATION_ALLOW_BUILD_IF_PREREQ_ENQUEUED

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_SHIPYARD_AST_REF",
    description="BLD_SHIPYARD_AST_REF_DESC",
    buildcost=500 * BUILDING_COST_MULTIPLIER,
    buildtime=5,
    location=(
        Planet()
        & ~Contains(IsBuilding(name=["BLD_SHIPYARD_AST_REF"]))
        & Planet(type=[AsteroidsType])
        & OwnedBy(empire=Source.Owner)
        & LOCATION_ALLOW_BUILD_IF_PREREQ_ENQUEUED("BLD_SHIPYARD_AST")
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=IsSource,
            activation=(~Planet(type=[AsteroidsType])),
            effects=[Destroy],
        ),
    ],
    icon="icons/building/shipyard-6.png",
)
