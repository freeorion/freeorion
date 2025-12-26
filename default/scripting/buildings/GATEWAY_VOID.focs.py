from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    Contains,
    Destroy,
    EffectsGroup,
    Fleet,
    GenerateSitRepMessage,
    InSystem,
    IsBuilding,
    MinOf,
    OwnedBy,
    Planet,
    SetMaxSupply,
    SetPopulation,
    SetStealth,
    SetSupply,
    SetTargetPopulation,
    Source,
    Target,
    Turn,
    Value,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET
from macros.priorities import (
    POPULATION_OVERRIDE_PRIORITY,
)

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_GATEWAY_VOID",
    description="BLD_GATEWAY_VOID_DESC",
    buildcost=200 * BUILDING_COST_MULTIPLIER,
    buildtime=10,
    location=(Planet() & ~Contains(IsBuilding(name=["BLD_GATEWAY_VOID"])) & OwnedBy(empire=Source.Owner)),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=(Fleet & InSystem(id=Source.SystemID)),
            effects=[
                Destroy,
                GenerateSitRepMessage(
                    message="EFFECT_GATEWAY_VOID_DESTROY",
                    label="EFFECT_GATEWAY_VOID_DESTROY_LABEL",
                    parameters={
                        "buildingtype": "BLD_GATEWAY_VOID",
                        "planet": Source.PlanetID,
                        "fleet": Target.ID,
                    },
                    empire=Source.Owner,
                ),
            ],
        ),
        EffectsGroup(
            scope=(Planet() & InSystem(id=Source.SystemID)),
            activation=Turn(low=Source.CreationTurn + 1),
            priority=POPULATION_OVERRIDE_PRIORITY,  # Overrides both target and current population effects,
            effects=[
                SetTargetPopulation(value=MinOf(float, Value, 0)),
                SetPopulation(value=MinOf(float, Value, 0)),
                SetMaxSupply(value=0),
                SetSupply(value=0),
            ],
        ),
        EffectsGroup(
            scope=InSystem(id=Source.SystemID),
            activation=(Turn(low=Source.CreationTurn + 1)),
            effects=[SetStealth(value=Value + 1000)],
        ),
    ],
    icon="icons/building/monument_to_exodus.png",
)
