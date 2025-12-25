from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    Contains,
    Destroy,
    EffectsGroup,
    IsBuilding,
    IsSource,
    OwnedBy,
    Planet,
    ProducedByEmpire,
    Source,
    StatisticCount,
    Unowned,
    WithinStarlaneJumps,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_REGIONAL_ADMIN",
    description="BLD_REGIONAL_ADMIN_DESC",
    captureresult=DestroyOnCapture,  # pyrefly: ignore[unbound-name]
    buildcost=12 * BUILDING_COST_MULTIPLIER * StatisticCount(float, condition=Planet() & OwnedBy(empire=Source.Owner)),
    buildtime=6,
    location=(
        Planet()
        & OwnedBy(empire=Source.Owner)
        & ~Contains(IsBuilding(name=["BLD_IMPERIAL_PALACE", "BLD_REGIONAL_ADMIN"]))
        & ~WithinStarlaneJumps(
            jumps=5,
            condition=(IsBuilding(name=["BLD_IMPERIAL_PALACE", "BLD_REGIONAL_ADMIN"]) & OwnedBy(empire=Source.Owner)),
        )
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        # destroy self if somehow acquired by different empire than built this...
        EffectsGroup(
            scope=IsSource,
            activation=(~Unowned & ~ProducedByEmpire(empire=Source.Owner)),
            effects=[Destroy],
        ),
    ],
    icon="icons/building/regional_administration.png",
)
