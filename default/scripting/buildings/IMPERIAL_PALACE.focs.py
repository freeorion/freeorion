from buildings.buildings_macros import SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS
from focs._effects import (
    BuildBuilding,
    Capital,
    ContainedBy,
    Contains,
    EffectsGroup,
    Enqueued,
    GameRule,
    HasSpecies,
    IsBuilding,
    IsSource,
    IsTarget,
    LocalCandidate,
    NamedReal,
    Number,
    Object,
    OwnedBy,
    Planet,
    ProducedByEmpire,
    SetEmpireCapital,
    SetEmpireMeter,
    SetMaxDefense,
    SetMaxSupply,
    SetMaxTroops,
    SetSpeciesTargetOpinion,
    SetTargetConstruction,
    SetTargetHappiness,
    SetTargetInfluence,
    Source,
    SpeciesDislikes,
    SpeciesLikes,
    StatisticCount,
    StatisticIf,
    Target,
    TargetPopulation,
    UniqueNumberOf,
    Unowned,
    Value,
)
from macros.base_prod import BUILDING_COST_MULTIPLIER
from macros.misc import PLANET_DEFENSE_FACTOR
from macros.priorities import TARGET_AFTER_SCALING_PRIORITY

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_IMPERIAL_PALACE",
    description="BLD_IMPERIAL_PALACE_DESC",
    captureresult=DestroyOnCapture,  # pyrefly: ignore[unbound-name]
    buildcost=5 * BUILDING_COST_MULTIPLIER * (StatisticCount(float, condition=Planet() & OwnedBy(empire=Source.Owner))),
    buildtime=8,
    location=(
        Planet()
        & OwnedBy(empire=Source.Owner)
        & ~Contains(IsBuilding(name=["BLD_IMPERIAL_PALACE"]))
        & TargetPopulation(low=1)
        & Number(
            high=0,
            condition=IsBuilding(name=["BLD_IMPERIAL_PALACE"])
            & OwnedBy(empire=Source.Owner)
            & ProducedByEmpire(empire=Source.Owner),
        )
    ),
    enqueuelocation=(  # must own production location planet
        Planet()
        & OwnedBy(empire=Source.Owner)  # can't build where another palace exists (even if not owned by this empire)
        & ~Contains(IsBuilding(name=["BLD_IMPERIAL_PALACE"]))  # must have a non-trivial population
        & TargetPopulation(low=1)
        # can't enqueue if already have an enqueued palace anywhere
        & Number(
            high=0,
            condition=(
                IsBuilding(name=["BLD_IMPERIAL_PALACE"])
                & OwnedBy(empire=Source.Owner)
                & ProducedByEmpire(empire=Source.Owner)
            ),
        )
        # can't enqueue if already own a self-built palace
        & Number(
            high=0,
            condition=Planet() & Enqueued(type=BuildBuilding, name="BLD_IMPERIAL_PALACE", empire=Source.Owner),
        )
    ),
    effectsgroups=[
        *SPECIES_LIKES_OR_DISLIKES_BUILDING_STABILITY_EFFECTS,
        # sets empire's capital
        EffectsGroup(
            scope=(Object(id=Source.PlanetID) & Planet() & OwnedBy(empire=Source.ProducedByEmpireID)),
            effects=[SetEmpireCapital()],
        ),
        # one economic policy slot
        EffectsGroup(
            scope=(Object(id=Source.PlanetID) & Planet() & Capital),
            stackinggroup="IMPERIAL_PALACE_ECO_SLOT",
            effects=[SetEmpireMeter(empire=Source.Owner, meter="ECONOMIC_CATEGORY_NUM_POLICY_SLOTS", value=Value + 1)],
        ),
        # flat bonus to supply range of capital
        EffectsGroup(
            scope=(Object(id=Source.PlanetID) & Planet() & OwnedBy(empire=Source.ProducedByEmpireID) & Capital),
            stackinggroup="IMPERIAL_PALACE_SUPPLY_EFFECT",
            effects=[SetMaxSupply(value=Value + NamedReal(name="BLD_IMPERIAL_PALACE_MAX_SUPPLY_FLAT", value=2))],
        ),
        # flat bonus to capital construction
        EffectsGroup(
            scope=(Object(id=Source.PlanetID) & Planet() & OwnedBy(empire=Source.ProducedByEmpireID) & Capital),
            activation=(~Number(low=1, condition=IsBuilding(name=["BLD_MEGALITH"]) & OwnedBy(empire=Source.Owner))),
            stackinggroup="IMPERIAL_PALACE_EFFECT",
            effects=[
                SetTargetConstruction(
                    value=Value + NamedReal(name="BLD_IMPERIAL_PALACE_TARGET_CONSTRUCTION_FLAT", value=20)
                )
            ],
        ),
        # flat bonus to stability of capital species
        EffectsGroup(
            scope=(Planet() & OwnedBy(empire=Source.Owner) & HasSpecies(name=[Source.Planet.Species])),
            activation=(
                OwnedBy(empire=Source.ProducedByEmpireID)
                & ContainedBy(Object(id=Source.PlanetID) & Planet() & HasSpecies() & Capital)
            ),
            stackinggroup="IMPERIAL_PALACE_HAPPINESS",
            accountinglabel="IMPERIAL_PALACE_SPECIES_HAPPINESS",
            effects=[
                SetTargetHappiness(value=Value + NamedReal(name="BLD_IMPERIAL_PALACE_TARGET_HAPPINESS_FLAT", value=5))
            ],
        ),
        # flat bonus to opinion of capital species
        EffectsGroup(
            scope=IsSource,
            activation=~Unowned,
            effects=[
                SetSpeciesTargetOpinion(
                    species=Source.Planet.Species,
                    empire=Source.Owner,
                    opinion=Value + NamedReal(name="BLD_IMPERIAL_PALACE_SPECIES_EMPIRE_OPINION_BOOST_FLAT", value=5),
                )
            ],
        ),
        # bonus or penalty to opinion of species that like or dislike the capital species
        # """
        # Unique
        #             sortkey = LocalCandidate.Species
        #             condition = Or [
        #                 Planet
        #                 SpeciesLikes name = ThisSpecies
        #                 SpeciesDislikes name = ThisSpecies
        #             ]
        # """
        EffectsGroup(
            activation=(~Unowned & ContainedBy(Object(id=Source.PlanetID) & Planet() & HasSpecies() & Capital)),
            scope=UniqueNumberOf(
                number=2147483647,
                sortkey=LocalCandidate.Species,
                condition=Planet()
                | SpeciesLikes(name="BLD_IMPERIAL_PALACE")
                | SpeciesDislikes(name="BLD_IMPERIAL_PALACE"),
            ),
            effects=SetSpeciesTargetOpinion(
                species=Target.Species,
                empire=Source.Owner,
                opinion=Value
                + NamedReal(name="BLD_IMPERIAL_PALACE_SPECIES_EMPIRE_OPINION_FLAT", value=2)
                * (
                    StatisticIf(float, condition=IsTarget & SpeciesLikes(name=Source.Planet.Species))
                    - StatisticIf(float, condition=IsTarget & SpeciesDislikes(name=Source.Planet.Species))
                ),
            ),
        ),
        # flat bonus to defense, troops and influence
        EffectsGroup(
            scope=(Object(id=Source.PlanetID) & Planet() & OwnedBy(empire=Source.ProducedByEmpireID)),
            stackinggroup="PALACE_DEFENSE_STACK",
            accountinglabel="BLD_IMPERIAL_PALACE",
            priority=TARGET_AFTER_SCALING_PRIORITY,
            effects=[
                SetMaxDefense(
                    value=Value
                    + NamedReal(name="BLD_IMPERIAL_PALACE_MAX_DEFENSE_FLAT", value=(5 * PLANET_DEFENSE_FACTOR))
                ),
                SetMaxTroops(value=Value + NamedReal(name="BLD_IMPERIAL_PALACE_MAX_TROOPS_FLAT", value=6)),
                SetTargetInfluence(
                    value=Value
                    + NamedReal(
                        name="BLD_IMPERIAL_PALACE_TARGET_INFLUENCE_FLAT",
                        value=GameRule(type=float, name="RULE_IMPERIAL_PALACE_INFLUENCE"),
                    )
                ),
            ],
        ),
    ],
    icon="icons/building/palace.png",
)
