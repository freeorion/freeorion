from focs._species import *
from species.species_macros.empire_opinions import COMMON_OPINION_EFFECTS
from species.species_macros.env import TERRAN_STANDARD_EP
from species.species_macros.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.species_macros.happiness import GOOD_HAPPINESS
from species.species_macros.industry import NO_INDUSTRY
from species.species_macros.influence import VERY_BAD_INFLUENCE
from species.species_macros.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.species_macros.population import AVERAGE_POPULATION
from species.species_macros.research import BAD_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stockpile import AVERAGE_STOCKPILE
from species.species_macros.supply import GREAT_SUPPLY
from species.species_macros.troops import AVERAGE_DEFENSE_TROOPS
from species.species_macros.weapons import BAD_WEAPONS

Species(
    name="SP_SILEXIAN",
    description="SP_SILEXIAN_DESC",
    gameplay_description="SP_SILEXIAN_GAMEPLAY_DESC",
    native=True,
    can_colonize=True,
    tags=[
        "ROBOTIC",
        "ARTISTIC",
        "NO_INDUSTRY",
        "BAD_RESEARCH",
        "VERY_BAD_INFLUENCE",
        "GOOD_HAPPINESS",
        "GREAT_SUPPLY",
        "BAD_WEAPONS",
        "PEDIA_ROBOTIC_SPECIES_CLASS",
        "PEDIA_ARTISTIC",
    ],
    foci=[
        HAS_RESEARCH_FOCUS,
        HAS_INFLUENCE_FOCUS,
        HAS_GROWTH_FOCUS,
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_RESEARCH",
    likes=[
        "FOCUS_RESEARCH",
        "MIMETIC_ALLOY_SPECIAL",
        "SHIMMER_SILK_SPECIAL",
        "MONOPOLE_SPECIAL",
        "COMPUTRONIUM_SPECIAL",
        "PHILOSOPHER_SPECIAL",
        "TEMPORAL_ANOMALY_SPECIAL",
        "PLC_DIVINE_AUTHORITY",
        "PLC_CENTRALIZATION",
        "PLC_MARTIAL_LAW",
        "PLC_ALGORITHMIC_RESEARCH",
        "PLC_INDUSTRIALISM",
        "PLC_ENGINEERING",
        "PLC_BUREAUCRACY",
    ],
    dislikes=[
        "BLD_AUTO_HISTORY_ANALYSER",
        "BLD_IMPERIAL_PALACE",
        "BLD_REGIONAL_ADMIN",
        "BLD_MILITARY_COMMAND",
        "BLD_BIOTERROR_PROJECTOR",
        "BLD_LIGHTHOUSE",
        "BLD_STARGATE",
        "MINERALS_SPECIAL",
        "FRUIT_SPECIAL",
        "ELERIUM_SPECIAL",
        "KRAKEN_IN_THE_ICE_SPECIAL",
        "PLC_LIBERTY",
        "PLC_CONFEDERATION",
        "PLC_ARTISAN_WORKSHOPS",
        "PLC_DESIGN_SIMPLICITY",
        "PLC_EXPLORATION",
    ],
    effectsgroups=[
        NO_INDUSTRY,
        *BAD_RESEARCH,
        *VERY_BAD_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *AVERAGE_POPULATION,
        *GOOD_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_SILEXIAN"),
        *GREAT_SUPPLY,
        *AVERAGE_DEFENSE_TROOPS,
        *BAD_WEAPONS,
        # not for description,
        *DEFAULT_NATIVE_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=TERRAN_STANDARD_EP,
    graphic="icons/species/robotic-06.png",
)
