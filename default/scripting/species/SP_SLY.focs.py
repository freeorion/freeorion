from focs._effects import (
    AllyOf,
    Capital,
    ContainedBy,
    Contains,
    EffectsGroup,
    EmpireHasAdoptedPolicy,
    EnemyOf,
    GasGiantType,
    GiveEmpirePolicy,
    HasSpecies,
    InSystem,
    IsSource,
    LocalCandidate,
    Object,
    OwnedBy,
    Planet,
    RootCandidate,
    SetFuel,
    SetStructure,
    Ship,
    Source,
    Stationary,
    Structure,
    Target,
    ThisSpecies,
    Turn,
    Value,
)
from focs._species import *
from species.species_macros.detection import GOOD_DETECTION
from species.species_macros.empire_opinions import COMMON_OPINION_EFFECTS
from species.species_macros.env import GASEOUS_STANDARD_EP, NARROW_EP
from species.species_macros.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.species_macros.happiness import INDEPENDENT_HAPPINESS
from species.species_macros.industry import AVERAGE_INDUSTRY
from species.species_macros.influence import GOOD_INFLUENCE
from species.species_macros.planet_defense import AVERAGE_PLANETARY_DEFENSE
from species.species_macros.planet_shields import AVERAGE_PLANETARY_SHIELDS
from species.species_macros.population import AVERAGE_POPULATION
from species.species_macros.research import BAD_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stealth import GREAT_STEALTH
from species.species_macros.stockpile import GREAT_STOCKPILE
from species.species_macros.supply import VERY_BAD_SUPPLY
from species.species_macros.troops import BAD_OFFENSE_TROOPS, GREAT_DEFENSE_TROOPS
from species.species_macros.weapons import BAD_WEAPONS

Species(
    name="SP_SLY",
    description="SP_SLY_DESC",
    gameplay_description="SP_SLY_GAMEPLAY_DESC",
    playable=True,
    can_produce_ships=True,
    can_colonize=True,
    tags=[
        "VERY_BAD_SUPPLY",
        "PEDIA_PC_FUEL",
        "GASEOUS",
        "GOOD_INFLUENCE",
        "GREAT_STOCKPILE",
        "NO_TERRAFORM",
        "BAD_RESEARCH",
        "BAD_WEAPONS",
        "GOOD_DETECTION",
        "GREAT_STEALTH",
        "BAD_OFFENSE_TROOPS",
        "PEDIA_GASEOUS_SPECIES_CLASS",
        "INDEPENDENT_HAPPINESS",
        "PEDIA_INDEPENDENT",
    ],
    foci=[
        HAS_INDUSTRY_FOCUS,
        HAS_RESEARCH_FOCUS,
        HAS_INFLUENCE_FOCUS,
        HAS_GROWTH_FOCUS,
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_INDUSTRY",
    likes=[
        "FOCUS_INDUSTRY",
        "FOCUS_INFLUENCE",
        "FRACTAL_GEODES_SPECIAL",
        "SUCCULENT_BARNACLES_SPECIAL",
        "SHIMMER_SILK_SPECIAL",
        "SPARK_FOSSILS_SPECIAL",
        "MIMETIC_ALLOY_SPECIAL",
        "CRYSTALS_SPECIAL",
        "KRAKEN_NEST_SPECIAL",
        "PANOPTICON_SPECIAL",
        "PHILOSOPHER_SPECIAL",
        "PLC_ARTISAN_WORKSHOPS",
        "PLC_CONFEDERATION",
        "PLC_CONTINUOUS_SCANNING",
        "PLC_EXPLORATION",
        "PLC_ISOLATION",
        "PLC_NATIVE_APPROPRIATION",
        "PLC_DIVERSITY",
        "PLC_NO_SUPPLY",
    ],
    dislikes=[
        "BLD_GAS_GIANT_GEN",  # messes with their way of life
        "BLD_BIOTERROR_PROJECTOR",
        "BLD_SCANNING_FACILITY",
        "BLD_MEGALITH",
        "BLD_SPACE_ELEVATOR",
        "BLD_HYPER_DAM",
        "BLD_PLANET_BEACON",
        "BLD_SPATIAL_DISTORT_GEN",
        "ELERIUM_SPECIAL",
        "RESONANT_MOON_SPECIAL",
        "TEMPORAL_ANOMALY_SPECIAL",
        "SUPERCONDUCTOR_SPECIAL",
        "PLC_AUGMENTATION",
        "PLC_PROPAGANDA",
        "PLC_RACIAL_PURITY",
        "PLC_TERROR_SUPPRESSION",
        "PLC_CONFORMANCE",
        "PLC_CHECKPOINTS",
    ],
    effectsgroups=[
        *AVERAGE_INDUSTRY,
        *BAD_RESEARCH,
        *GOOD_INFLUENCE,
        *GREAT_STOCKPILE,
        *AVERAGE_POPULATION,
        *INDEPENDENT_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_SLY"),
        *VERY_BAD_SUPPLY,
        *GREAT_DEFENSE_TROOPS,
        *BAD_OFFENSE_TROOPS,
        *BAD_WEAPONS,
        *GOOD_DETECTION,
        *GREAT_STEALTH,
        # not for description,
        *AVERAGE_PLANETARY_SHIELDS,
        *AVERAGE_PLANETARY_DEFENSE,
        NARROW_EP,
        *STANDARD_SHIP_SHIELDS,
        # Extra refueling on gas giants,
        EffectsGroup(
            description="GASEOUS_DESC",
            scope=IsSource,
            activation=Ship
            & Stationary
            & ContainedBy(
                Object(id=Source.SystemID)
                & Contains(Planet(type=[GasGiantType]) & ~OwnedBy(affiliation=EnemyOf, empire=RootCandidate.Owner))
            ),
            stackinggroup="SP_SLY_FUEL_STACK",
            # Update ship_hulls.macros if this number changes
            effects=SetFuel(value=Value + 0.1),
        ),
        # Always repair and resupply at own planets,
        EffectsGroup(
            scope=Ship
            & InSystem(id=Source.SystemID)
            & Stationary
            & HasSpecies(name=[ThisSpecies])
            & (
                OwnedBy(empire=Source.Owner)
                | (
                    # either ally providing repair or owner of ship being repaired must adopt policy to share repairs
                    (
                        EmpireHasAdoptedPolicy(empire=Source.Owner, name="PLC_ALLIED_REPAIR")
                        | EmpireHasAdoptedPolicy(empire=LocalCandidate.Owner, name="PLC_ALLIED_REPAIR")
                    )
                    & OwnedBy(affiliation=AllyOf, empire=Source.Owner)
                )
            )
            & Turn(low=LocalCandidate.System.LastTurnBattleHere + 1)
            & Structure(high=LocalCandidate.MaxStructure - 0.001),
            activation=Planet(),
            stackinggroup="FLEET_REPAIR",
            effects=[
                SetStructure(value=Value + (Target.MaxStructure / 10)),
                SetFuel(value=Target.MaxFuel),
                # TODO: have a resupply effect that will also act on part meters without needing to list them all...
            ],
        ),
        EffectsGroup(
            description="SLY_STARTING_UNLOCKS_DESC",
            scope=IsSource,
            activation=Capital,
            effects=GiveEmpirePolicy(name="PLC_CONFEDERATION", empire=Source.Owner),
        ),
    ],
    environments=GASEOUS_STANDARD_EP,
    graphic="icons/species/amorphous-05.png",
)
