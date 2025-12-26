from focs._effects import (
    AllyOf,
    Capital,
    Destroy,
    EffectsGroup,
    GenerateSitRepMessage,
    IsBuilding,
    IsSource,
    LocalCandidate,
    MaxOf,
    Object,
    OwnedBy,
    Partial,
    Planet,
    Population,
    SetOwner,
    SetSpecialCapacity,
    SetVisibility,
    Source,
    Target,
    ThisBuilding,
    Value,
    ValueVisibility,
)
from macros.misc import UNOWNED_EMPIRE_ID
from macros.priorities import END_CLEANUP_PRIORITY, METER_OVERRIDE_PRIORITY

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_COLONY_INDEPENDENCE_DECREE",
    description="BLD_COLONY_INDEPENDENCE_DECREE_DESC",
    captureresult=DestroyOnCapture,  # pyrefly: ignore[unbound-name]
    buildcost=20,
    buildtime=1,
    location=(Planet() & OwnedBy(empire=Source.Owner) & Population() & ~Capital),
    effectsgroups=[
        # TODO: implement SetFocus effect
        # switch independent colony focus to defense so the independent colony can have the highest defense values
        # make colony independent (unowned)
        EffectsGroup(
            scope=((LocalCandidate.PlanetID == Source.PlanetID) & IsBuilding() & IsBuilding(name=[ThisBuilding])),
            activation=(Source.Age == 1),
            priority=METER_OVERRIDE_PRIORITY,
            effects=[
                SetOwner(empire=UNOWNED_EMPIRE_ID),
            ],
        ),
        EffectsGroup(
            scope=Object(id=Source.PlanetID) & Planet(),
            activation=Source.Age == 1,
            # calculation of the Max* values must be finished before declaring independence
            priority=METER_OVERRIDE_PRIORITY,
            effects=[
                SetSpecialCapacity(name="INDEPENDENT_COLONY_SHIELD_SPECIAL", capacity=Value(Target.MaxShield)),
                SetSpecialCapacity(name="INDEPENDENT_COLONY_DEFENSE_SPECIAL", capacity=Value(Target.MaxDefense)),
                SetSpecialCapacity(name="INDEPENDENT_COLONY_TROOPS_SPECIAL", capacity=Value(Target.MaxTroops)),
                SetSpecialCapacity(
                    name="INDEPENDENT_COLONY_POPULATION_SPECIAL", capacity=Value(Target.TargetPopulation)
                ),
                SetOwner(empire=UNOWNED_EMPIRE_ID),
                GenerateSitRepMessage(
                    message="SITREP_COLONY_INDEPENDENT",
                    label="SITREP_COLONY_INDEPENDENT_LABEL",
                    icon="icons/sitrep/planet_captured.png",
                    parameters={
                        "planet": Target.ID,
                        "species": Target.Species,
                    },
                    empire=Source.Owner,
                ),
                SetVisibility(empire=Source.Owner, visibility=MaxOf("Visibility", ValueVisibility, Partial)),
            ],
        ),
        EffectsGroup(
            scope=IsSource | Object(id=Source.PlanetID),
            activation=IsSource,
            priority=END_CLEANUP_PRIORITY,
            effects=[
                SetVisibility(
                    empire=Source.ProducedByEmpireID, visibility=MaxOf("Visibility", Partial, ValueVisibility)
                ),
                SetVisibility(
                    affiliation=AllyOf,
                    empire=Source.ProducedByEmpireID,
                    visibility=MaxOf("Visibility", Partial, ValueVisibility),
                ),
            ],
        ),
        EffectsGroup(
            scope=IsSource,
            activation=Source.Age > 1,
            priority=END_CLEANUP_PRIORITY,
            effects=[
                Destroy,
            ],
        ),
    ],
    icon="icons/sitrep/planet_captured.png",
)
