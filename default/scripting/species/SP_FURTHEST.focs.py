from focs._species import *
from species.common.detection import GOOD_DETECTION
from species.common.empire_opinions import COMMON_OPINION_EFFECTS
from species.common.env import TUNDRA_STANDARD_EP
from species.common.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.common.happiness import INDEPENDENT_HAPPINESS
from species.common.industry import AVERAGE_INDUSTRY
from species.common.influence import BAD_INFLUENCE
from species.common.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.common.population import BAD_POPULATION
from species.common.research import BAD_RESEARCH
from species.common.shields import STANDARD_SHIP_SHIELDS
from species.common.stealth import GREAT_STEALTH
from species.common.stockpile import AVERAGE_STOCKPILE
from species.common.supply import BAD_SUPPLY
from species.common.troops import GOOD_DEFENSE_TROOPS

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
        "BLD_INDUSTRY_CENTER",
        "BLD_STOCKPILING_CENTER",
        "BLD_MEGALITH",
        "BLD_SPACE_ELEVATOR",
        "BLD_SHIPYARD_ORBITAL_DRYDOCK",
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
