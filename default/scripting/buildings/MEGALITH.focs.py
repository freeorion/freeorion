from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    Contains,
    EffectsGroup,
    IsBuilding,
    IsSource,
    Object,
    OwnedBy,
    Planet,
    SetEmpireCapital,
    SetIndustry,
    SetInfluence,
    SetMaxSupply,
    SetMaxTroops,
    SetResearch,
    SetTargetConstruction,
    Source,
    Target,
    TargetPopulation,
    Value,
    WithinStarlaneJumps,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET
from macros.priorities import (
    LATE_AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
    TARGET_AFTER_SCALING_PRIORITY,
    TARGET_LAST_BEFORE_OVERRIDE_PRIORITY,
)

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_MEGALITH",
    description="BLD_MEGALITH_DESC",
    buildcost=250 * BUILDING_COST_MULTIPLIER,
    buildtime=10,
    location=(
        Planet()
        & Contains(IsBuilding(name=["BLD_IMPERIAL_PALACE"]))
        & ~Contains(IsBuilding(name=["BLD_MEGALITH"]))
        & OwnedBy(empire=Source.Owner)
        & TargetPopulation(low=1)
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=Object(id=Source.PlanetID) & Planet(),
            stackinggroup="BLD_MEGALITH_CAPITAL_INFRA_EFFECTS",
            priority=TARGET_LAST_BEFORE_OVERRIDE_PRIORITY,
            effects=[
                SetTargetConstruction(value=Value + 30),
                SetEmpireCapital(),
            ],
        ),
        EffectsGroup(
            scope=(Object(id=Source.PlanetID) & Planet()),
            stackinggroup="BLD_MEGALITH_CURRENT_METERS_EFFECTS",
            priority=LATE_AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
            effects=[
                SetIndustry(value=Target.TargetIndustry),
                SetInfluence(value=Target.TargetInfluence),
                SetResearch(value=Target.TargetResearch),
            ],
        ),
        EffectsGroup(
            scope=(Planet() & OwnedBy(empire=Source.Owner) & TargetPopulation(low=1)),
            stackinggroup="BLD_MEGALITH_SUPPLY_EFFECT",
            effects=[SetMaxSupply(value=Value + 1)],
        ),
        EffectsGroup(
            scope=(Planet() & WithinStarlaneJumps(jumps=2, condition=IsSource) & OwnedBy(empire=Source.Owner)),
            priority=TARGET_AFTER_SCALING_PRIORITY,
            effects=[
                SetMaxTroops(value=Value + 10),
            ],
        ),
    ],
    icon="icons/building/megalith.png",
)
