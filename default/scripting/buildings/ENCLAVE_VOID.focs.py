from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    Contains,
    EffectsGroup,
    Focus,
    Happiness,
    IsBuilding,
    NamedReal,
    OwnedBy,
    Planet,
    SetTargetResearch,
    Source,
    Target,
    Value,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER, RESEARCH_PER_POP
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_ENCLAVE_VOID",
    description="BLD_ENCLAVE_VOID_DESC",
    buildcost=300 * BUILDING_COST_MULTIPLIER,
    buildtime=3,
    location=(Planet() & ~Contains(IsBuilding(name=["BLD_ENCLAVE_VOID"])) & OwnedBy(empire=Source.Owner)),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=(
                Planet()
                & OwnedBy(empire=Source.Owner)
                & Focus(type=["FOCUS_RESEARCH"])
                & Happiness(low=NamedReal(name="BLD_ENCLAVE_VOID_MIN_STABILITY", value=10))
            ),
            stackinggroup="BLD_ENCLAVE_VOID_STACK",
            effects=[
                SetTargetResearch(
                    value=Value
                    + Target.Population
                    * NamedReal(name="BLD_ENCLAVE_VOID_TARGET_RESEARCH_PERPOP", value=0.75 * RESEARCH_PER_POP)
                ),
            ],
        ),
    ],
    icon="icons/building/science-institute.png",
)
