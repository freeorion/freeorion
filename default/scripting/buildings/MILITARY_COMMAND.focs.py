from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    Capital,
    ContainedBy,
    Contains,
    EffectsGroup,
    IsBuilding,
    Number,
    Object,
    OwnedBy,
    Planet,
    SetEmpireMeter,
    SetMaxDefense,
    SetMaxTroops,
    Source,
    SpeciesShipsDestroyed,
    TargetPopulation,
    Value,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET
from macros.misc import PLANET_DEFENSE_FACTOR
from macros.priorities import TARGET_EARLY_BEFORE_SCALING_PRIORITY

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_MILITARY_COMMAND",
    description="BLD_MILITARY_COMMAND_DESC",
    captureresult=DestroyOnCapture,  # pyrefly: ignore[unbound-name]
    buildcost=60 * BUILDING_COST_MULTIPLIER,
    buildtime=5,
    location=(
        Planet()
        & OwnedBy(empire=Source.Owner)
        & ~Contains(IsBuilding(name=["BLD_MILITARY_COMMAND"]))
        & TargetPopulation(low=1)
        & Number(high=0, condition=IsBuilding(name=["BLD_MILITARY_COMMAND"]) & OwnedBy(empire=Source.Owner))
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=(Planet() & OwnedBy(empire=Source.Owner) & Capital),
            activation=(ContainedBy(Object(id=Source.PlanetID) & OwnedBy(empire=Source.Owner))),
            stackinggroup="MILITARY_COMMAND_SLOT_METER_EFFECT1",
            effects=[SetEmpireMeter(empire=Source.Owner, meter="MILITARY_CATEGORY_NUM_POLICY_SLOTS", value=Value + 1)],
        ),
        EffectsGroup(
            scope=(Planet() & OwnedBy(empire=Source.Owner) & Capital),
            activation=(5 <= SpeciesShipsDestroyed(empire=Source.Owner)),
            stackinggroup="MILITARY_COMMAND_SLOT_METER_EFFECT2",
            effects=[
                SetEmpireMeter(empire=Source.Owner, meter="MILITARY_CATEGORY_NUM_POLICY_SLOTS", value=Value + 1),
            ],
        ),
        EffectsGroup(
            scope=(Object(id=Source.PlanetID) & Planet() & OwnedBy(empire=Source.ProducedByEmpireID)),
            accountinglabel="BLD_MILITARY_COMMAND",
            priority=TARGET_EARLY_BEFORE_SCALING_PRIORITY,
            effects=[SetMaxDefense(value=Value + 5 * PLANET_DEFENSE_FACTOR), SetMaxTroops(value=Value + 6)],
        ),
    ],
    icon="icons/building/palace.png",
)
