from focs._effects import (
    EffectsGroup,
    IsBuilding,
    IsField,
    MoveTowards,
    Number,
    OwnedBy,
    Source,
    Target,
    ThisBuilding,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_FIELD_REPELLOR",
    description="BLD_FIELD_REPELLOR_DESC",
    buildcost=1000 * BUILDING_COST_MULTIPLIER,
    buildtime=5,
    location=(Number(high=0, condition=IsBuilding(name=[ThisBuilding]) & OwnedBy(empire=Source.Owner))),
    enqueuelocation=(
        ENQUEUE_BUILD_ONE_PER_PLANET
        & Number(high=0, condition=Enqueued(type=BuildBuilding, name=ThisBuilding, empire=Source.Owner))
    ),
    effectsgroups=[
        EffectsGroup(
            scope=IsField(
                name=[
                    "FLD_ION_STORM",
                    "FLD_METEOR_BLIZZARD",
                    "FLD_MOLECULAR_CLOUD",
                    "FLD_NANITE_SWARM",
                    "FLD_SUBSPACE_RIFT",
                    "FLD_VOID_RIFT",
                ]
            ),
            effects=[
                MoveTowards(speed=-Target.Speed, x=Source.X, y=Source.Y),
            ],
        ),
    ],
    icon="icons/building/field_repellor.png",
)
