from common.misc import (
    DESCRIPTION_EFFECTSGROUP_MACRO,
    SUPPLY_DISCONNECTED_INFLUENCE_MALUS,
)
from common.priorities import (
    TARGET_EARLY_BEFORE_SCALING_PRIORITY,
    TARGET_LAST_BEFORE_OVERRIDE_PRIORITY,
    TARGET_SCALING_PRIORITY,
)

BASE_INFLUENCE_COSTS = [
    EffectsGroup(  # colonies consume influence, proportional to square-root of how many populated planets and non-populated outposts the empire controls
        scope=IsSource,
        activation=Planet() & ~Unowned & ~Capital,
        stackinggroup="IMPERIAL_PALACE_MANY_PLANETS_INFLUENCE_PENALTY",
        accountinglabel="COLONY_ADMIN_COSTS_LABEL",
        priority=TARGET_LAST_BEFORE_OVERRIDE_PRIORITY,
        effects=SetTargetInfluence(
            value=Value
            - NamedReal(name="COLONY_ADMIN_COSTS_PER_PLANET", value=0.4)
            * (
                (
                    StatisticCount(
                        float,
                        condition=Planet()
                        & OwnedBy(empire=Source.Owner)
                        & HasSpecies()
                        & ~HasSpecies(name=["SP_EXOBOT"]),
                    )
                    + StatisticCount(
                        float,
                        condition=Ship & OwnedBy(empire=Source.Owner) & (LocalCandidate.OrderedColonizePlanetID != -1),
                    )
                    + StatisticCount(
                        float,
                        condition=IsBuilding()
                        & OwnedBy(empire=Source.Owner)
                        & IsBuilding(
                            name=[
                                "BLD_COL_SUPER_TEST",
                                "BLD_COL_ABADDONI",
                                "BLD_COL_BANFORO",
                                "BLD_COL_CHATO",
                                "BLD_COL_CRAY",
                                "BLD_COL_DERTHREAN",
                                "BLD_COL_EAXAW",
                                "BLD_COL_EGASSEM",
                                "BLD_COL_ETTY",
                                "BLD_COL_FULVER",
                                "BLD_COL_FURTHEST",
                                "BLD_COL_GEORGE",
                                "BLD_COL_GYSACHE",
                                "BLD_COL_HAPPY",
                                "BLD_COL_HHHOH",
                                "BLD_COL_HUMAN",
                                "BLD_COL_KILANDOW",
                                "BLD_COL_KOBUNTURA",
                                "BLD_COL_LAENFA",
                                "BLD_COL_MISIORLA",
                                "BLD_COL_MUURSH",
                                "BLD_COL_PHINNERT",
                                "BLD_COL_SCYLIOR",
                                "BLD_COL_SETINON",
                                "BLD_COL_SILEXIAN",
                                "BLD_COL_SLY",
                                "BLD_COL_SSLITH",
                                "BLD_COL_TAEGHIRUS",
                                "BLD_COL_TRITH",
                                "BLD_COL_REPLICON",
                                "BLD_COL_UGMORS",
                            ]
                        ),
                    )
                    + (
                        NamedReal(name="OUTPOST_RELATIVE_ADMIN_COUNT", value=0.25)
                        * (
                            StatisticCount(
                                float,
                                condition=Planet()
                                & OwnedBy(empire=Source.Owner)
                                & (~HasSpecies() | HasSpecies(name=["SP_EXOBOT"])),
                            )
                        )
                    )
                )
                ** 0.5
            )
        ),
    ),
    EffectsGroup(  # species homeworlds consume more influence
        scope=IsSource,
        activation=Planet() & ~Unowned & ~Capital & Homeworld(name=[LocalCandidate.Species]),
        accountinglabel="SPECIES_HOMEWORLD_INDEPENDENCE_DESIRE_LABEL",
        priority=TARGET_LAST_BEFORE_OVERRIDE_PRIORITY,
        effects=SetTargetInfluence(value=Value - 1),
    ),
    EffectsGroup(  # colonies consume more influence if not supply connected to empire's capital
        scope=IsSource,
        activation=Planet()
        & ~Unowned
        & ~EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_CONFEDERATION")
        & ~ResourceSupplyConnected(
            empire=LocalCandidate.Owner, condition=Planet() & OwnedBy(empire=Source.Owner) & Capital
        ),
        accountinglabel="CAPITAL_DISCONNECTION_LABEL",
        priority=TARGET_LAST_BEFORE_OVERRIDE_PRIORITY,
        effects=SetTargetInfluence(
            value=Value
            - NamedReal(name="SUPPLY_DISCONNECTED_INFLUENCE_MALUS", value=SUPPLY_DISCONNECTED_INFLUENCE_MALUS)
        ),
    ),
    EffectsGroup(  # gives human bonuses when AI Aggression set to Beginner
        scope=IsSource,
        activation=Planet() & ~Unowned & IsHuman & (GalaxyMaxAIAggression == 0),  # human player, not human species
        accountinglabel="DIFFICULTY",
        priority=TARGET_LAST_BEFORE_OVERRIDE_PRIORITY,
        effects=SetTargetInfluence(value=Value + 1),
    ),
]

NO_INFLUENCE = [DESCRIPTION_EFFECTSGROUP_MACRO("NO_INFLUENCE_DESC"), *BASE_INFLUENCE_COSTS]

ARTISANS_INFLUENCE_STABILITY = [
    EffectsGroup(  # artistic species generate influence when artisans workshops policy adopted
        scope=IsSource,
        activation=HasTag(name="ARTISTIC")
        & Happiness(low=NamedReal(name="ARTISANS_MIN_STABILITY_FOCUS", value=1))
        & EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_ARTISAN_WORKSHOPS")
        & Focus(type=["FOCUS_INFLUENCE"]),
        accountinglabel="PLC_ARTISAN_WORKSHOPS",
        effects=SetTargetInfluence(value=Value + NamedReal(name="ARTISANS_INFLUENCE_FLAT_FOCUS", value=2.0)),
    ),
    EffectsGroup(
        scope=IsSource,
        activation=HasTag(name="ARTISTIC")
        & Happiness(low=NamedReal(name="ARTISANS_MIN_STABILITY_NO_FOCUS", value=10))
        & EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_ARTISAN_WORKSHOPS")
        & ~Focus(type=["FOCUS_INFLUENCE"]),
        accountinglabel="PLC_ARTISAN_WORKSHOPS",
        effects=SetTargetInfluence(value=Value + NamedReal(name="ARTISANS_INFLUENCE_FLAT_NO_FOCUS", value=0.5)),
    ),
    # TODO: should this move to happiness.macros ?
    EffectsGroup(  # artistic species make other planets with different artistic species on them and in the same system system more stable
        scope=Planet()
        & InSystem(id=Source.SystemID)
        & ~IsSource
        & HasSpecies()
        & ~HasSpecies(name=[Source.Species])
        & HasTag(name="ARTISTIC")
        # (SpeciesSpeciesOpinion species = LocalCandidate.Species species = Source.Species > 0) # TODO: implement within an Or condition, once species-species opinions are working...
        ,
        activation=Planet() & HasSpecies() & HasTag(name="ARTISTIC"),
        accountinglabel="ARTISAN_APPRECIATION",
        effects=SetTargetHappiness(value=Value + 1.0),
    ),
]

BASIC_INFLUENCE = [
    EffectsGroup(  # influence focus generates influence from planets, proportional to sqare-root of population
        scope=IsSource,
        activation=Planet() & Focus(type=["FOCUS_INFLUENCE"]),
        accountinglabel="FOCUS_INFLUENCE_LABEL",
        priority=TARGET_EARLY_BEFORE_SCALING_PRIORITY,
        effects=SetTargetInfluence(
            value=Value
            + (Target.Population**0.5) * NamedReal(name="FOCUS_INFLUENCE_INFLUENCE_PER_SQRT_POP", value=1.0)
        ),
    ),
    *BASE_INFLUENCE_COSTS,
    *ARTISANS_INFLUENCE_STABILITY,
]

AVERAGE_INFLUENCE = BASIC_INFLUENCE

GOOD_INFLUENCE = [
    *BASIC_INFLUENCE,
    EffectsGroup(
        description="GOOD_INFLUENCE_DESC",
        scope=IsSource,
        activation=Planet() & Focus(type=["FOCUS_INFLUENCE"]),
        accountinglabel="GOOD_INFLUENCE_LABEL",
        priority=TARGET_SCALING_PRIORITY,
        effects=SetTargetInfluence(value=Value + (1.25 - 1.0) * Abs(float, Value)),
    ),
]

GREAT_INFLUENCE = [
    *BASIC_INFLUENCE,
    EffectsGroup(
        description="GREAT_INFLUENCE_DESC",
        scope=IsSource,
        activation=Planet() & Focus(type=["FOCUS_INFLUENCE"]),
        accountinglabel="GREAT_INFLUENCE_LABEL",
        priority=TARGET_SCALING_PRIORITY,
        effects=SetTargetInfluence(value=Value + (1.5 - 1.0) * Abs(float, Value)),
    ),
]
