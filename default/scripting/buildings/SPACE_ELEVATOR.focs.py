from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._conditions import Contains, IsBuilding, Object, OwnedBy, Planet, TargetPopulation
from focs._effects_new import EffectsGroup, SetMaxSupply
from focs._enums import AsteroidsType, GasGiantType, Huge, Large, Medium, Small, Tiny
from focs._sources import Source
from focs._value_refs import (
    Value,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_SPACE_ELEVATOR",
    description="BLD_SPACE_ELEVATOR_DESC",
    buildcost=150 * BUILDING_COST_MULTIPLIER,
    buildtime=6,
    location=(
        Planet()
        & OwnedBy(empire=Source.Owner)
        & ~Contains(IsBuilding(name=["BLD_SPACE_ELEVATOR"]))
        & ~Planet(type=[AsteroidsType])
        & TargetPopulation(low=1)
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=(Object(id=Source.PlanetID) & Planet(size=[Tiny])),
            effects=[SetMaxSupply(value=Value + 1)],
        ),
        EffectsGroup(
            scope=(Object(id=Source.PlanetID) & Planet(size=[Small])),
            effects=[SetMaxSupply(value=Value + 2)],
        ),
        EffectsGroup(
            scope=(Object(id=Source.PlanetID) & Planet(size=[Medium])),
            effects=[SetMaxSupply(value=Value + 3)],
        ),
        EffectsGroup(
            scope=(Object(id=Source.PlanetID) & Planet(size=[Large])),
            effects=[SetMaxSupply(value=Value + 4)],
        ),
        EffectsGroup(
            scope=(Object(id=Source.PlanetID) & Planet(size=[Huge])),
            effects=[SetMaxSupply(value=Value + 5)],
        ),
        EffectsGroup(
            scope=(Object(id=Source.PlanetID) & Planet(type=[GasGiantType])),
            effects=[SetMaxSupply(value=Value + 3)],
        ),
    ],
    icon="icons/building/space-elevator.png",
)
