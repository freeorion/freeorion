from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    BuildBuilding,
    Contains,
    Described,
    Destroy,
    EffectsGroup,
    Enqueued,
    Focus,
    GenerateSitRepMessage,
    Happiness,
    HasSpecies,
    IsBuilding,
    NamedReal,
    Number,
    NumberOf,
    Object,
    OwnedBy,
    Planet,
    Random,
    RootCandidate,
    SetMaxStockpile,
    SetTargetInfluence,
    SetTargetResearch,
    Source,
    System,
    Target,
    Value,
    WithinStarlaneJumps,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.enqueue import ENQUEUE_BUILD_ONE_PER_PLANET
from macros.priorities import (
    TARGET_AFTER_SCALING_PRIORITY,
)

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_INTERSPECIES_ACADEMY",
    description="BLD_INTERSPECIES_ACADEMY_DESC",
    buildcost=50 * BUILDING_COST_MULTIPLIER,
    buildtime=5,
    location=(
        # Homeworld
        Planet()
        & HasSpecies()
        & ~Contains(IsBuilding(name=["BLD_INTERSPECIES_ACADEMY"]))
        & OwnedBy(empire=Source.Owner)
        & Happiness(low=NamedReal(name="BLD_INTERSPECIES_ACADEMY_MIN_STABILITY_BUILD", value=15))
        & ~WithinStarlaneJumps(
            jumps=3,
            condition=System & Contains(IsBuilding(name=["BLD_INTERSPECIES_ACADEMY"]) & OwnedBy(empire=Source.Owner)),
        )
        & Number(
            high=0,
            condition=IsBuilding(name=["BLD_INTERSPECIES_ACADEMY"])
            & HasSpecies(name=[RootCandidate.Species])
            & OwnedBy(empire=RootCandidate.Owner),
        )
        & Number(
            high=0,
            condition=Described(
                description="CONDITION_INTERSPECIES_ACADEMY_SPECIES_ALREADY_EXISTS",
                condition=Planet()
                & Enqueued(type=BuildBuilding, name="BLD_INTERSPECIES_ACADEMY")
                & HasSpecies(name=[RootCandidate.Species])
                & OwnedBy(empire=RootCandidate.Owner)
                & ~Object(id=RootCandidate.PlanetID),
            ),
        )
        & Number(high=6, condition=IsBuilding(name=["BLD_INTERSPECIES_ACADEMY"]) & OwnedBy(empire=Source.Owner))
    ),
    enqueuelocation=ENQUEUE_BUILD_ONE_PER_PLANET,
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        # Destroy superfluous academies
        EffectsGroup(
            scope=(
                NumberOf(
                    number=1,
                    condition=IsBuilding(name=["BLD_INTERSPECIES_ACADEMY"])
                    & OwnedBy(empire=Source.Owner)
                    & ~Object(id=Source.ID)
                    & HasSpecies(name=[Source.Planet.Species]),
                )
            ),
            activation=(Random(probability=0.3)),
            effects=[
                GenerateSitRepMessage(
                    message="EFFECT_INTERSPECIES_ACADEMY_DESTROY",
                    label="EFFECT_INTERSPECIES_ACADEMY_DESTROY_LABEL",
                    icon="icons/building/blackhole.png",
                    parameters={"planet": Target.PlanetID},
                    empire=Source.Owner,
                ),
                Destroy,
            ],
        ),
        # Apply stockpile effects
        EffectsGroup(
            scope=(
                Object(id=Source.PlanetID)
                & Planet()
                & Focus(type=["FOCUS_STOCKPILE"])
                & OwnedBy(empire=Source.Owner)
                & HasSpecies()
            ),
            accountinglabel="INTERSPECIES_ACADEMY_LABEL",
            priority=TARGET_AFTER_SCALING_PRIORITY,
            effects=[
                SetMaxStockpile(value=Value + NamedReal(name="BLD_INTERSPECIES_ACADEMY_MAX_STOCKPILE_FLAT", value=10))
            ],
        ),
        # Research bonus
        EffectsGroup(
            scope=(
                Object(id=Source.PlanetID)
                & Planet()
                & Focus(type=["FOCUS_RESEARCH"])
                & OwnedBy(empire=Source.Owner)
                & HasSpecies()
                & Happiness(low=NamedReal(name="BLD_INTERSPECIES_ACADEMY_MIN_STABILITY_RESEARCH", value=12))
            ),
            accountinglabel="INTERSPECIES_ACADEMY_LABEL",
            priority=TARGET_AFTER_SCALING_PRIORITY,
            effects=[
                SetTargetResearch(
                    value=Value + NamedReal(name="BLD_INTERSPECIES_ACADEMY_TARGET_RESEARCH_FLAT", value=5)
                )
            ],
        ),
        # Influence bonus
        EffectsGroup(
            scope=(
                Object(id=Source.PlanetID)
                & Planet()
                & Focus(type=["FOCUS_INFLUENCE"])
                & OwnedBy(empire=Source.Owner)
                & HasSpecies()
            ),
            accountinglabel="INTERSPECIES_ACADEMY_LABEL",
            priority=TARGET_AFTER_SCALING_PRIORITY,
            effects=[
                SetTargetInfluence(
                    value=Value + NamedReal(name="BLD_INTERSPECIES_ACADEMY_TARGET_INFLUENCE_FLAT", value=2)
                )
            ],
        ),
    ],
    icon="icons/building/science-institute.png",
)
