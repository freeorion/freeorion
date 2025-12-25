from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    Capital,
    Contains,
    CountUnique,
    EffectsGroup,
    Focus,
    HasSpecies,
    IsBuilding,
    LocalCandidate,
    NamedReal,
    Object,
    OwnedBy,
    Planet,
    SetEmpireMeter,
    SetTargetInfluence,
    Source,
    Statistic,
    TargetPopulation,
    Unowned,
    Value,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET
from macros.priorities import (
    TARGET_AFTER_2ND_SCALING_PRIORITY,
)

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_TRANSLATOR",
    description="BLD_TRANSLATOR_DESC",
    buildcost=320 * BUILDING_COST_MULTIPLIER,
    buildtime=10,
    location=(
        Planet()
        & OwnedBy(empire=Source.Owner)
        & ~Contains(IsBuilding(name=["BLD_TRANSLATOR"]))
        & TargetPopulation(low=1)
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=(Capital & OwnedBy(empire=Source.Owner)),
            activation=~Unowned,
            stackinggroup="TRANSLATOR_POLICY_SLOT_STACK",
            effects=[SetEmpireMeter(empire=Source.Owner, meter="SOCIAL_CATEGORY_NUM_POLICY_SLOTS", value=Value + 1)],
        ),
        EffectsGroup(
            scope=(Object(id=Source.PlanetID) & Planet() & ~Unowned & Focus(type=["FOCUS_INFLUENCE"])),
            activation=~Unowned,
            priority=TARGET_AFTER_2ND_SCALING_PRIORITY,
            effects=[
                SetTargetInfluence(
                    value=Value
                    + NamedReal(name="TRANSLATOR_PER_SPECIES_INFLUENCE", value=0.5)
                    * (
                        Statistic(
                            float,
                            str,
                            CountUnique,
                            value=LocalCandidate.Species,
                            condition=Planet() & HasSpecies() & OwnedBy(empire=Source.Owner),
                        )
                    )
                    ** 0.5
                )
            ],
        ),
    ],
    icon="icons/building/translator.png",
)
