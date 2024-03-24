from focs._species import *
from species.species_macros.detection import GOOD_DETECTION
from species.species_macros.empire_opinions import COMMON_OPINION_EFFECTS
from species.species_macros.env import TUNDRA_STANDARD_EP
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
from species.species_macros.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.species_macros.population import BAD_POPULATION
from species.species_macros.research import BAD_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stealth import GREAT_STEALTH
from species.species_macros.stockpile import AVERAGE_STOCKPILE
from species.species_macros.supply import BAD_SUPPLY
from species.species_macros.troops import GOOD_DEFENSE_TROOPS

Species(
    name="SP_FURTHEST",
    description="SP_FURTHEST_DESC",
    gameplay_description="SP_FURTHEST_GAMEPLAY_DESC",
    native=True,
    can_colonize=True,
    tags=[
        "ORGANIC",
        "BAD_RESEARCH",
        "BAD_POPULATION",
        "BAD_INFLUENCE",
        "BAD_SUPPLY",
        "GOOD_DETECTION",
        "GREAT_STEALTH",
        "PEDIA_ORGANIC_SPECIES_CLASS",
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
    defaultfocus="FOCUS_GROWTH",
    likes=[
        "FOCUS_GROWTH",
        "SHIMMER_SILK_SPECIAL",
        "SUCCULENT_BARNACLES_SPECIAL",
        "MIMETIC_ALLOY_SPECIAL",
        "ANCIENT_RUINS_DEPLETED_SPECIAL",
        "FRUIT_SPECIAL",
        "FORTRESS_SPECIAL",
        "TIDAL_LOCK_SPECIAL",
        "PLC_CHECKPOINTS",
        "PLC_ENVIRONMENTALISM",
        "PLC_ISOLATION",
        "PLC_NO_SUPPLY",
    ],
    dislikes=[
        "BLD_STOCKPILING_CENTER",
        "BLD_MEGALITH",
        "BLD_REGIONAL_ADMIN",
        "BLD_SPACE_ELEVATOR",
        "BLD_STARGATE",
        "BLD_PLANET_CLOAK",
        "WORLDTREE_SPECIAL",
        "COMPUTRONIUM_SPECIAL",
        "MONOPOLE_SPECIAL",
        "ECCENTRIC_ORBIT_SPECIAL",
        "PLC_ARTISAN_WORKSHOPS",
        "PLC_CAPITAL_MARKETS",
        "PLC_COLONIALISM",
        "PLC_INTERSTELLAR_INFRA",
        "SP_NIGHTSIDERS",
    ],
    effectsgroups=[
        *AVERAGE_INDUSTRY,
        *BAD_RESEARCH,
        *BAD_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *BAD_POPULATION,
        *INDEPENDENT_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_FURTHEST"),
        *BAD_SUPPLY,
        *GOOD_DEFENSE_TROOPS,
        *GOOD_DETECTION,
        *GREAT_STEALTH,
        # not for description,
        *DEFAULT_NATIVE_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=TUNDRA_STANDARD_EP,
    graphic="icons/species/furthest.png",
)
