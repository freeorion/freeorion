from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    BuildBuilding,
    ContainedBy,
    Contains,
    Destroy,
    EffectsGroup,
    Enqueued,
    GenerateSitRepMessage,
    HasSpecial,
    HasSpecies,
    IsBuilding,
    IsSource,
    LocalCandidate,
    MinOf,
    Number,
    NumberOf,
    Object,
    OwnedBy,
    Planet,
    Population,
    ResourceSupplyConnected,
    SetIndustry,
    SetInfluence,
    SetPopulation,
    SetResearch,
    SetTargetIndustry,
    SetTargetInfluence,
    SetTargetResearch,
    Source,
    Target,
    Value,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.priorities import CONCENTRATION_CAMP_PRIORITY, END_CLEANUP_PRIORITY, POPULATION_DEFAULT_PRIORITY

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_EVACUATION",
    description="BLD_EVACUATION_DESC",
    buildcost=10 * BUILDING_COST_MULTIPLIER,
    buildtime=1,
    location=(
        Planet()
        & OwnedBy(empire=Source.Owner)
        & ~Contains(IsBuilding(name=["BLD_EVACUATION", "BLD_CONC_CAMP"]))
        & ~HasSpecial(name="CONC_CAMP_SLAVE_SPECIAL")
        & ~Enqueued(type=BuildBuilding, name="BLD_CONC_CAMP")
        & HasSpecies()
    ),
    enqueuelocation=(
        Planet()
        & OwnedBy(empire=Source.Owner)
        & ~Contains(IsBuilding(name=["BLD_EVACUATION", "BLD_CONC_CAMP"]))
        & ~HasSpecial(name="CONC_CAMP_SLAVE_SPECIAL")
        & ~Enqueued(type=BuildBuilding, name="BLD_EVACUATION")
        & ~Enqueued(type=BuildBuilding, name="BLD_CONC_CAMP")
        & HasSpecies()
    ),
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        EffectsGroup(
            scope=NumberOf(
                number=1,
                condition=(
                    Planet()
                    & HasSpecies(name=[Source.Planet.Species])
                    & ~Object(id=Source.PlanetID)
                    & ResourceSupplyConnected(empire=Source.Owner, condition=IsSource)
                    & Population(low=1, high=LocalCandidate.TargetPopulation - 1)
                    & ~Contains(IsBuilding(name=["BLD_EVACUATION", "BLD_CONC_CAMP"]))
                ),
            ),
            activation=(
                Number(
                    low=1,
                    condition=(
                        Planet()
                        & HasSpecies(name=[Source.Planet.Species])
                        & ~Object(id=Source.PlanetID)
                        & ResourceSupplyConnected(empire=Source.Owner, condition=IsSource)
                        & Population(low=1, high=LocalCandidate.TargetPopulation - 1)
                    ),
                )
            ),
            priority=POPULATION_DEFAULT_PRIORITY,
            effects=[
                SetPopulation(value=Value + MinOf(float, Source.Planet.Population, 2)),
                GenerateSitRepMessage(
                    message="EFFECT_EVACUEES",
                    label="EFFECT_EVACUEES_LABEL",
                    icon="icons/building/evacuation.png",
                    parameters={"planet": Target.ID},
                    empire=Source.Owner,
                ),
            ],
        ),
        EffectsGroup(  # remove population from location & nullify production
            scope=Object(id=Source.PlanetID) & Planet(),
            activation=ContainedBy(
                Object(id=Source.PlanetID)
                & OwnedBy(empire=Source.Owner)
                # evacuation building needs to exist one turn for technical reasons.
                # it should not destroy a new colony
                & (LocalCandidate.TurnsSinceColonization > 0)
            ),
            priority=CONCENTRATION_CAMP_PRIORITY,
            effects=[
                SetPopulation(value=MinOf(float, Value, Target.Population - 2)),
                SetIndustry(value=0),
                SetTargetIndustry(value=0),
                SetResearch(value=0),
                SetTargetResearch(value=0),
                SetInfluence(value=0),
                SetTargetInfluence(value=0),
            ],
        ),
        EffectsGroup(  # remove evacuation when planet is depopulated or no longer owned by empire that produced this building
            scope=IsSource,
            activation=(
                ~OwnedBy(empire=Source.Owner)
                | ContainedBy(
                    Object(id=Source.PlanetID)
                    & (
                        Population(high=0)
                        # evacuation building needs to exist one turn for technical reasons.
                        # it should not destroy a new colony
                        | (LocalCandidate.TurnsSinceColonization < 1)
                    )
                    | Contains(IsBuilding(name=["BLD_CONC_CAMP"]))
                    | HasSpecial(name="CONC_CAMP_SLAVE_SPECIAL")
                )
            ),
            priority=END_CLEANUP_PRIORITY,
            effects=[Destroy],
        ),
    ],
    icon="icons/building/evacuation.png",
)
