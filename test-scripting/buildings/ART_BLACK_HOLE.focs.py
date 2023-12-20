from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(
    name="BLD_ART_BLACK_HOLE",
    description="BLD_ART_BLACK_HOLE_DESC",
    buildcost=45 * BUILDING_COST_MULTIPLIER,
    buildtime=6,
    location=Planet()
    & ~Contains(IsBuilding(name=["BLD_ART_BLACK_HOLE"]))
    & OwnedBy(empire=Source.Owner)
    & Star(type=[Red]),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        EffectsGroup(
            scope=Object(id=Source.SystemID) & System,
            activation=Star(type=[Red]),
            stackinggroup="ART_BLACK_HOLE",
            effects=[
                SetStarType(type=BlackHole),
                GenerateSitRepMessage(
                    message="EFFECT_BLACKHOLE",
                    label="EFFECT_BLACKHOLE_LABEL",
                    icon="icons/building/blackhole.png",
                    parameters={"system": Source.SystemID},
                    empire=Source.Owner,
                ),
            ],
        ),
        EffectsGroup(scope=IsSource, effects=Destroy),
    ],
    icon="icons/building/blackhole.png",
)
