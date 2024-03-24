from focs._effects import Blue, EffectsGroup, InSystem, IsSource, SetFuel, Ship, Star, Stationary, Value, White
from focs._species import *
from species.species_macros.detection import GREAT_DETECTION
from species.species_macros.empire_opinions import COMMON_OPINION_EFFECTS
from species.species_macros.env import BROAD_EP, OCEAN_BROAD_EP
from species.species_macros.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.species_macros.happiness import INDEPENDENT_HAPPINESS
from species.species_macros.industry import AVERAGE_INDUSTRY
from species.species_macros.influence import BAD_INFLUENCE
from species.species_macros.planet_defense import AVERAGE_PLANETARY_DEFENSE
from species.species_macros.planet_shields import AVERAGE_PLANETARY_SHIELDS
from species.species_macros.planet_size import LARGE_PLANET
from species.species_macros.population import GOOD_POPULATION
from species.species_macros.research import AVERAGE_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stealth import GOOD_STEALTH
from species.species_macros.stockpile import GOOD_STOCKPILE
from species.species_macros.supply import AVERAGE_SUPPLY
from species.species_macros.troops import BAD_DEFENSE_TROOPS, BAD_OFFENSE_TROOPS

Species(
    name="SP_LAENFA",
    description="SP_LAENFA_DESC",
    gameplay_description="SP_LAENFA_GAMEPLAY_DESC",
    playable=True,
    can_produce_ships=True,
    can_colonize=True,
    tags=[
        "PHOTOTROPHIC",
        "SNEAKY",
        "TELEPATHIC",
        "BAD_INFLUENCE",
        "GOOD_POPULATION",
        "AVERAGE_SUPPLY",
        "GREAT_DETECTION",
        "GOOD_STEALTH",
        "BAD_OFFENSE_TROOPS",
        "PEDIA_PC_FUEL",
        "PEDIA_PHOTOTROPHIC_SPECIES_CLASS",
        "INDEPENDENT_HAPPINESS",
        "PEDIA_TELEPATHIC_TITLE",
        "PEDIA_SNEAKY_TITLE",
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
        "FOCUS_STOCKPILE",
        "FRUIT_SPECIAL",
        "PANOPTICON_SPECIAL",
        "BLD_INTERSPECIES_ACADEMY",
        "BLD_REGIONAL_ADMIN",
        "PLC_BUREAUCRACY",
        "PLC_CENTRALIZATION",
        "PLC_CONFORMANCE",
        "PLC_CONTINUOUS_SCANNING",
        "PLC_DIVINE_AUTHORITY",
        "PLC_DREAM_RECURSION",
        "PLC_ENVIRONMENTALISM",
        "PLC_EXPLORATION",
        "PLC_FLANKING",
        "PLC_INSURGENCY",
        "PLC_NATIVE_APPROPRIATION",
        "POSITRONIUM_SPECIAL",
        "SPARK_FOSSILS_SPECIAL",
        "SP_CELESTEPHYTE",  # because they too are telepathic plants
        "SUCCULENT_BARNACLES_SPECIAL",
        "TIDAL_LOCK_SPECIAL",
    ],
    dislikes=[
        "ANCIENT_RUINS_DEPLETED_SPECIAL",
        "BLD_BIOTERROR_PROJECTOR",
        "BLD_CULTURE_LIBRARY",
        "BLD_GAS_GIANT_GEN",
        "BLD_INDUSTRY_CENTER",
        "BLD_PLANET_BEACON",
        "BLD_SOL_ORB_GEN",
        "ELERIUM_SPECIAL",
        "PLC_EXOBOT_PRODUCTIVITY",
        "PLC_INDUSTRIALISM",
        "PLC_MARINE_RECRUITMENT",
        "RESONANT_MOON_SPECIAL",
        "WORLDTREE_SPECIAL",
    ],
    effectsgroups=[
        *AVERAGE_INDUSTRY,
        *AVERAGE_RESEARCH,
        *BAD_INFLUENCE,
        *GOOD_STOCKPILE,
        *GOOD_POPULATION,
        *INDEPENDENT_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_LAENFA"),
        *AVERAGE_SUPPLY,
        *BAD_DEFENSE_TROOPS,
        *BAD_OFFENSE_TROOPS,
        *GREAT_DETECTION,
        *GOOD_STEALTH,
        # not for description,
        *AVERAGE_PLANETARY_SHIELDS,
        *AVERAGE_PLANETARY_DEFENSE,
        *LARGE_PLANET,
        BROAD_EP,
        *STANDARD_SHIP_SHIELDS,
        # replenish fuel in suitably coloured star systems,
        EffectsGroup(
            description="PHOTOTROP_DESC",
            scope=IsSource,
            activation=Ship & Stationary & InSystem() & Star(type=[Blue]),
            effects=SetFuel(value=Value + 1.9),  # sums with natural 0.1 per turn to a nice round 2.0 per turn
        ),
        EffectsGroup(
            scope=IsSource,
            activation=Ship & Stationary & InSystem() & Star(type=[White]),
            effects=SetFuel(value=Value + 0.9),
        ),
    ],
    environments=OCEAN_BROAD_EP,
    graphic="icons/species/laenfa.png",
)
