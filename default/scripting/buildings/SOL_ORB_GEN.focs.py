from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    BlackHole,
    Blue,
    Contains,
    EffectsGroup,
    Focus,
    Happiness,
    IsBuilding,
    IsSource,
    NamedReal,
    NamedRealLookup,
    Neutron,
    NoStar,
    Orange,
    OwnedBy,
    Planet,
    Red,
    ResourceSupplyConnected,
    SetTargetIndustry,
    Source,
    Star,
    Target,
    Value,
    White,
    Yellow,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER, INDUSTRY_PER_POP
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET
from macros.priorities import (
    TARGET_AFTER_2ND_SCALING_PRIORITY,
)

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_SOL_ORB_GEN",
    description="BLD_SOL_ORB_GEN_DESC",
    buildcost=75 * BUILDING_COST_MULTIPLIER,
    buildtime=6,
    tags=["ORBITAL"],
    location=(
        Planet()
        & OwnedBy(empire=Source.Owner)
        & ~Contains(IsBuilding(name=["BLD_SOL_ORB_GEN"]))
        & ~Star(type=[Neutron, BlackHole, NoStar])
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=(
                Planet()
                & Focus(type=["FOCUS_INDUSTRY"])
                & OwnedBy(empire=Source.Owner)
                & Happiness(low=NamedReal(name="BLD_SOL_ORB_GEN_MIN_STABILITY", value=10))
                & ResourceSupplyConnected(empire=Source.Owner, condition=IsSource)
            ),
            activation=(Star(type=[Blue, White])),
            stackinggroup="BLD_SOL_ORB_GEN_EFFECT",
            priority=TARGET_AFTER_2ND_SCALING_PRIORITY,
            effects=[
                SetTargetIndustry(
                    value=Value
                    + Target.Population
                    * NamedReal(name="BLD_SOL_ORB_GEN_BRIGHT_TARGET_INDUSTRY_PERPOP", value=0.75 * INDUSTRY_PER_POP)
                )
            ],
        ),
        EffectsGroup(
            scope=(
                Planet()
                & Focus(type=["FOCUS_INDUSTRY"])
                & OwnedBy(empire=Source.Owner)
                & Happiness(low=NamedRealLookup(name="BLD_SOL_ORB_GEN_MIN_STABILITY"))
                & ResourceSupplyConnected(empire=Source.Owner, condition=IsSource)
            ),
            activation=(
                Star(type=[Yellow, Orange])
                & ~ResourceSupplyConnected(
                    empire=Source.Owner,
                    condition=IsBuilding(name=["BLD_SOL_ORB_GEN"])
                    & OwnedBy(empire=Source.Owner)
                    & Star(type=[Blue, White]),
                )
            ),
            stackinggroup="BLD_SOL_ORB_GEN_EFFECT",
            priority=TARGET_AFTER_2ND_SCALING_PRIORITY,
            effects=[
                SetTargetIndustry(
                    value=Value
                    + Target.Population
                    * NamedReal(name="BLD_SOL_ORB_GEN_NORMAL_TARGET_INDUSTRY_PERPOP", value=0.5 * INDUSTRY_PER_POP)
                )
            ],
        ),
        EffectsGroup(
            scope=(
                Planet()
                & Focus(type=["FOCUS_INDUSTRY"])
                & OwnedBy(empire=Source.Owner)
                & Happiness(low=NamedRealLookup(name="BLD_SOL_ORB_GEN_MIN_STABILITY"))
                & ResourceSupplyConnected(empire=Source.Owner, condition=IsSource)
            ),
            activation=(
                Star(type=[Red])
                & ~ResourceSupplyConnected(
                    empire=Source.Owner,
                    condition=IsBuilding(name=["BLD_SOL_ORB_GEN"])
                    & OwnedBy(empire=Source.Owner)
                    & Star(type=[Yellow, Orange, Blue, White]),
                )
            ),
            stackinggroup="BLD_SOL_ORB_GEN_EFFECT",
            priority=TARGET_AFTER_2ND_SCALING_PRIORITY,
            effects=[
                SetTargetIndustry(
                    value=Value
                    + Target.Population
                    * NamedReal(name="BLD_SOL_ORB_GEN_DIM_TARGET_INDUSTRY_PERPOP", value=0.25 * INDUSTRY_PER_POP)
                )
            ],
        ),
    ],
    icon="icons/building/miniature_sun.png",
)
