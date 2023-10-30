from focs._species import *
from species.common.detection import ULTIMATE_DETECTION
from species.common.empire_opinions import COMMON_OPINION_EFFECTS
from species.common.env import OCEAN_STANDARD_EP
from species.common.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.common.happiness import AVERAGE_HAPPINESS
from species.common.industry import BAD_INDUSTRY
from species.common.influence import VERY_BAD_INFLUENCE
from species.common.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.common.population import AVERAGE_POPULATION
from species.common.research import AVERAGE_RESEARCH
from species.common.shields import STANDARD_SHIP_SHIELDS
from species.common.stockpile import AVERAGE_STOCKPILE
from species.common.supply import BAD_SUPPLY
from species.common.troops import AVERAGE_DEFENSE_TROOPS

Species(
    name="SP_OURBOOLS",
    description="SP_OURBOOLS_DESC",
    gameplay_description="SP_OURBOOLS_GAMEPLAY_DESC",
    native=True,
    tags=[
        "ORGANIC",
        "ARTISTIC",
        "BAD_INDUSTRY",
        "VERY_BAD_INFLUENCE",
        "BAD_SUPPLY",
        "ULTIMATE_DETECTION",
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
        "ELERIUM_SPECIAL",
        "FOCUS_RESEARCH",
        "FORTRESS_SPECIAL",
        "FRACTAL_GEODES_SPECIAL",
        "MIMETIC_ALLOY_SPECIAL",
        "PLC_ENVIRONMENTALISM",
        "PLC_ISOLATION",
        "PLC_NO_SUPPLY",
        "RESONANT_MOON_SPECIAL",
    ],
    dislikes=[
        "BLD_BIOTERROR_PROJECTOR",
        # "BLD_CLONING_CENTER"       // disabled content,
        "BLD_CULTURE_ARCHIVES",
        "BLD_GATEWAY_VOID",
        "BLD_IMPERIAL_PALACE",
        "BLD_INTERSPECIES_ACADEMY",
        "BLD_MEGALITH",
        "BLD_STARGATE",
        "KRAKEN_NEST_SPECIAL",
        "PANOPTICON_SPECIAL",
        "PLC_BUREAUCRACY",
        "PLC_INDUSTRIALISM",
        "PLC_SYSTEM_INFRA",
        "TEMPORAL_ANOMALY_SPECIAL",
    ],
    effectsgroups=[
        *BAD_INDUSTRY,
        *AVERAGE_RESEARCH,
        *VERY_BAD_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *AVERAGE_POPULATION,
        *AVERAGE_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_OURBOOLS"),
        *BAD_SUPPLY,
        *AVERAGE_DEFENSE_TROOPS,
        *ULTIMATE_DETECTION,
        # not for description,
        *DEFAULT_NATIVE_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=OCEAN_STANDARD_EP,
    graphic="icons/species/ourbools.png",
)
