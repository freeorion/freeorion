from focs._species import *
from species.species_macros.empire_opinions import COMMON_OPINION_EFFECTS
from species.species_macros.env import OCEAN_STANDARD_EP
from species.species_macros.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.species_macros.fuel import BAD_FUEL
from species.species_macros.happiness import AVERAGE_HAPPINESS
from species.species_macros.industry import AVERAGE_INDUSTRY
from species.species_macros.influence import GOOD_INFLUENCE
from species.species_macros.planet_defense import AVERAGE_PLANETARY_DEFENSE
from species.species_macros.planet_shields import AVERAGE_PLANETARY_SHIELDS
from species.species_macros.population import GOOD_POPULATION
from species.species_macros.research import GREAT_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stockpile import AVERAGE_STOCKPILE
from species.species_macros.supply import BAD_SUPPLY
from species.species_macros.troops import AVERAGE_DEFENSE_TROOPS

Species(
    name="SP_SCYLIOR",
    description="SP_SCYLIOR_DESC",
    gameplay_description="SP_SCYLIOR_GAMEPLAY_DESC",
    playable=True,
    can_produce_ships=True,
    can_colonize=True,
    tags=[
        "ORGANIC",
        "ARTISTIC",
        "GREAT_RESEARCH",
        "GOOD_INFLUENCE",
        "GOOD_POPULATION",
        "BAD_SUPPLY",
        "BAD_FUEL",
        "PEDIA_ORGANIC_SPECIES_CLASS",
        "PEDIA_ARTISTIC",
    ],
    foci=[
        HAS_INDUSTRY_FOCUS,
        HAS_RESEARCH_FOCUS,
        HAS_INFLUENCE_FOCUS,
        HAS_GROWTH_FOCUS,
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_RESEARCH",
    likes=[
        "FOCUS_RESEARCH",
        "SHIMMER_SILK_SPECIAL",
        "FRACTAL_GEODES_SPECIAL",
        "CRYSTALS_SPECIAL",
        "MONOPOLE_SPECIAL",
        "SPICE_SPECIAL",
        "ECCENTRIC_ORBIT_SPECIAL",
        "RESONANT_MOON_SPECIAL",
        "PLC_DIVERSITY",
        "PLC_ENVIRONMENTALISM",
        "PLC_EXOBOT_PRODUCTIVITY",
        "PLC_TECHNOCRACY",
    ],
    dislikes=[
        "BLD_SHIPYARD_CON_NANOROBO",
        "BLD_BIOTERROR_PROJECTOR",
        "BLD_REGIONAL_ADMIN",
        "BLD_SPACE_ELEVATOR",
        "BLD_HYPER_DAM",
        "BLD_PLANET_DRIVE",
        "BLD_NEUTRONIUM_FORGE",
        "BLD_NEUTRONIUM_SYNTH",
        "KRAKEN_IN_THE_ICE_SPECIAL",
        "PHILOSOPHER_SPECIAL",
        "TEMPORAL_ANOMALY_SPECIAL",
        "WORLDTREE_SPECIAL",
        "PLC_MARTIAL_LAW",
        "PLC_RACIAL_PURITY",
        "PLC_TERRAFORMING",
        "PLC_TERROR_SUPPRESSION",
    ],
    effectsgroups=[
        *AVERAGE_INDUSTRY,
        *GREAT_RESEARCH,
        *GOOD_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *GOOD_POPULATION,
        *AVERAGE_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_SCYLIOR"),
        *BAD_SUPPLY,
        *AVERAGE_DEFENSE_TROOPS,
        *BAD_FUEL,
        # not for description,
        *AVERAGE_PLANETARY_SHIELDS,
        *AVERAGE_PLANETARY_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=OCEAN_STANDARD_EP,
    graphic="icons/species/scylior.png",
)
