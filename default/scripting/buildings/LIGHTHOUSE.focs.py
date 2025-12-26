from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    Contains,
    EffectsGroup,
    EnemyOf,
    InSystem,
    IsBuilding,
    IsSource,
    LocalCandidate,
    Max,
    MinOf,
    NamedReal,
    OwnedBy,
    Planet,
    SetSpeed,
    SetStealth,
    Ship,
    Source,
    Speed,
    Statistic,
    Unowned,
    Value,
    WithinDistance,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_LIGHTHOUSE",
    description="BLD_LIGHTHOUSE_DESC",
    buildcost=25 * BUILDING_COST_MULTIPLIER,
    buildtime=10,
    location=(Planet() & ~Contains(IsBuilding(name=["BLD_LIGHTHOUSE"])) & OwnedBy(empire=Source.Owner)),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=(Ship & WithinDistance(distance=0.00001, condition=IsSource)),
            stackinggroup="LIGHTHOUSE_IMMEDIATE_STEALTH_STACK",
            effects=[SetStealth(value=Value - NamedReal(name="BLD_LIGHTHOUSE_IMMEDIATE_STEALTH_MALUS", value=20))],
        ),
        # Highly Self Illuminated, automatically "registered in catalog"
        EffectsGroup(
            scope=IsSource,
            stackinggroup="LIGHTHOUSE_CATALOG_STEALTH_STACK",
            accountinglabel="BLD_LIGHTHOUSE_CATALOG_STEALTH_LABEL",
            effects=[SetStealth(value=Value - NamedReal(name="BLD_LIGHTHOUSE_SELF_STEALTH_MALUS", value=20))],
        ),
        # takes the maximum age of all lighthouse buildings in the system regardless of owner (by just triggering for the oldest ones)
        EffectsGroup(
            scope=WithinDistance(distance=0.00001, condition=IsSource),
            activation=(
                Source.Age
                >= (
                    Statistic(
                        int,
                        Max,
                        value=LocalCandidate.Age,
                        condition=InSystem(id=Source.SystemID) & IsBuilding(name=["BLD_LIGHTHOUSE"]),
                    )
                )
            ),
            stackinggroup="LIGHTHOUSE_CATALOG_STEALTH_STACK",
            accountinglabel="BLD_LIGHTHOUSE_CATALOG_STEALTH_LABEL",
            effects=[
                SetStealth(
                    value=Value
                    - MinOf(float, Source.Age, NamedReal(name="BLD_LIGHTHOUSE_CATALOG_STEALTH_MALUS", value=10))
                )
            ],
        ),
        EffectsGroup(
            scope=(
                Ship
                & ~(OwnedBy(affiliation=EnemyOf, empire=Source.Owner) | Unowned)
                & WithinDistance(
                    distance=NamedReal(name="BLD_LIGHTHOUSE_SPEEDUP_DISTANCE", value=50), condition=IsSource
                )
                & Speed(low=1)  # Immobile objects do not get the starlane speed boost.
            ),
            stackinggroup="LIGHTHOUSE_SPEED_STACK",
            effects=[SetSpeed(value=Value + NamedReal(name="BLD_LIGHTHOUSE_SPEEDUP", value=20))],
        ),
    ],
    icon="icons/building/lighthouse.png",
)
