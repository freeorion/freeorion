from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    Capital,
    ContainedBy,
    Contains,
    EffectsGroup,
    IsBuilding,
    MaxOf,
    OwnedBy,
    Partial,
    Planet,
    SetEmpireMeter,
    SetVisibility,
    Source,
    System,
    Unowned,
    Value,
    ValueVisibility,
    WithinStarlaneJumps,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_SCRYING_SPHERE",
    description="BLD_SCRYING_SPHERE_DESC",
    buildcost=150 * BUILDING_COST_MULTIPLIER,
    buildtime=5,
    location=(
        Planet()
        & OwnedBy(empire=Source.Owner)
        & ~WithinStarlaneJumps(jumps=5, condition=IsBuilding(name=["BLD_SCRYING_SPHERE"]))
    ),
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=Capital & OwnedBy(empire=Source.Owner),
            activation=~Unowned,
            stackinggroup="SCRYING_SPHERE_POLICY_SLOT_STACK",
            effects=[SetEmpireMeter(empire=Source.Owner, meter="SOCIAL_CATEGORY_NUM_POLICY_SLOTS", value=Value + 1)],
        ),
        EffectsGroup(
            scope=(
                (
                    (Planet() | IsBuilding())
                    & ContainedBy(Contains(IsBuilding(name=["BLD_SCRYING_SPHERE"])))
                    & ~OwnedBy(empire=Source.Planet.Owner)
                    # would be redundant to re-assign ownership to own planets)
                )
                | (System & Contains(IsBuilding(name=["BLD_SCRYING_SPHERE"])))
            ),
            activation=ContainedBy(Planet() & ~Unowned),
            effects=[
                SetVisibility(empire=Source.Planet.Owner, visibility=MaxOf("Visibility", Partial, ValueVisibility)),
            ],
        ),
    ],
    icon="icons/specials_huge/ancient_ruins_excavated.png",
)
