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
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_SHIPYARD_AST",
    description="BLD_SHIPYARD_AST_DESC",
    buildcost=75 * BUILDING_COST_MULTIPLIER,
    buildtime=5,
    location=(
        Planet()
        & ~Contains(IsBuilding(name=["BLD_SHIPYARD_AST"]))
        & Planet(type=[AsteroidsType])
        & OwnedBy(empire=Source.Owner)
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
    icon="icons/building/shipyard-5.png",
)
