from focs._species import *
from species.species_macros.empire_opinions import COMMON_OPINION_EFFECTS
from species.species_macros.env import TERRAN_STANDARD_EP
from species.species_macros.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
)
from species.species_macros.happiness import BAD_HAPPINESS
from species.species_macros.industry import BAD_INDUSTRY
from species.species_macros.influence import NO_INFLUENCE
from species.species_macros.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.species_macros.population import AVERAGE_POPULATION
from species.species_macros.research import NO_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stockpile import AVERAGE_STOCKPILE
from species.species_macros.supply import BAD_SUPPLY
from species.species_macros.troops import ULTIMATE_DEFENSE_TROOPS

Species(
    name="SP_RAAAGH",
    description="SP_RAAAGH_DESC",
    gameplay_description="SP_RAAAGH_GAMEPLAY_DESC",
    native=True,
    tags=[
        "ORGANIC",
        "BAD_INDUSTRY",
        "NO_RESEARCH",
        "NO_INFLUENCE",
        "BAD_HAPPINESS",
        "BAD_SUPPLY",
        "PRIMITIVE",
        "PEDIA_ORGANIC_SPECIES_CLASS",
    ],
    foci=[
        HAS_INDUSTRY_FOCUS,
        HAS_GROWTH_FOCUS,
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_PROTECTION",
    likes=[
        "FOCUS_PROTECTION",
        "MIMETIC_ALLOY_SPECIAL",
        "SPARK_FOSSILS_SPECIAL",
        "BLD_BLACK_HOLE_POW_GEN",
        "HONEYCOMB_SPECIAL",
        "COMPUTRONIUM_SPECIAL",
        "TIDAL_LOCK_SPECIAL",
        "FRUIT_SPECIAL",
        "PLC_CONFEDERATION",
        "PLC_CONFORMANCE",
        "PLC_DIVINE_AUTHORITY",
        "PLC_DREAM_RECURSION",
    ],
    dislikes=[
        "FOCUS_INDUSTRY",
        "BLD_GAS_GIANT_GEN",
        "BLD_AUTO_HISTORY_ANALYSER",
        "BLD_INTERSPECIES_ACADEMY",
        "BLD_GENOME_BANK",
        "BLD_PLANET_CLOAK",
        "BLD_SHIPYARD_CON_GEOINT",
        "BLD_SHIPYARD_ORG_ORB_INC",
        "PANOPTICON_SPECIAL",
        "SPICE_SPECIAL",
        "PROBIOTIC_SPECIAL",
        "PLC_INTERSTELLAR_INFRA",
        "SP_NIGHTSIDERS",
    ],
    effectsgroups=[
        *BAD_INDUSTRY,
        NO_RESEARCH,
        *NO_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *AVERAGE_POPULATION,
        *BAD_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_RAAAGH"),
        *BAD_SUPPLY,
        *ULTIMATE_DEFENSE_TROOPS,
        # not for description,
        *DEFAULT_NATIVE_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=TERRAN_STANDARD_EP,
    spawnrate=1,
    spawnlimit=1,
    graphic="icons/species/raaagh.png",
)
