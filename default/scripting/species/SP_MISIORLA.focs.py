from focs._species import *
from species.species_macros.detection import BAD_DETECTION
from species.species_macros.empire_opinions import COMMON_OPINION_EFFECTS
from species.species_macros.env import TOXIC_STANDARD_EP
from species.species_macros.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.species_macros.fuel import BAD_FUEL
from species.species_macros.happiness import GOOD_HAPPINESS
from species.species_macros.industry import BAD_INDUSTRY
from species.species_macros.influence import BAD_INFLUENCE
from species.species_macros.planet_defense import AVERAGE_PLANETARY_DEFENSE
from species.species_macros.planet_shields import AVERAGE_PLANETARY_SHIELDS
from species.species_macros.population import BAD_POPULATION
from species.species_macros.research import BAD_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stealth import BAD_STEALTH
from species.species_macros.stockpile import BAD_STOCKPILE
from species.species_macros.supply import BAD_SUPPLY
from species.species_macros.troops import BAD_DEFENSE_TROOPS
from species.species_macros.weapons import ULTIMATE_WEAPONS

Species(
    name="SP_MISIORLA",
    description="SP_MISIORLA_DESC",
    gameplay_description="SP_MISIORLA_GAMEPLAY_DESC",
    can_produce_ships=True,
    can_colonize=True,
    tags=[
        "ORGANIC",
        "ARTISTIC",
        "BAD_INDUSTRY",
        "BAD_RESEARCH",
        "BAD_INFLUENCE",
        "BAD_STOCKPILE",
        "BAD_POPULATION",
        "GOOD_HAPPINESS",
        "BAD_SUPPLY",
        "ULTIMATE_WEAPONS",
        "BAD_DETECTION",
        "BAD_STEALTH",
        "BAD_FUEL",
        "CTRL_EXTINCT",
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
    defaultfocus="FOCUS_INDUSTRY",
    likes=[
        "SUCCULENT_BARNACLES_SPECIAL",
        "SHIMMER_SILK_SPECIAL",
        "ECCENTRIC_ORBIT_SPECIAL",
        "FORTRESS_SPECIAL",
        "GAIA_SPECIAL",
        "HONEYCOMB_SPECIAL",
        "KRAKEN_IN_THE_ICE_SPECIAL",
        "PLC_ALLIED_REPAIR",
        "PLC_CONFEDERATION",
        "PLC_ENGINEERING",
        "PLC_EXPLORATION",
        "PLC_LIBERTY",
        "PLC_MARINE_RECRUITMENT",
    ],
    dislikes=[
        "BLD_INDUSTRY_CENTER",
        "BLD_INTERSPECIES_ACADEMY",
        "BLD_STOCKPILING_CENTER",
        "BLD_MEGALITH",
        "BLD_SPACE_ELEVATOR",
        "BLD_GENOME_BANK",
        "BLD_COLLECTIVE_NET",
        "BLD_GATEWAY_VOID",
        "BLD_ENCLAVE_VOID",
        "BLD_HYPER_DAM",
        "PLC_BUREAUCRACY",
        "PLC_CENTRALIZATION",
        "PLC_CONFORMANCE",
        "PLC_DIVINE_AUTHORITY",
        "SP_NIGHTSIDERS",
    ],
    effectsgroups=[
        *BAD_INDUSTRY,
        *BAD_RESEARCH,
        *BAD_INFLUENCE,
        *BAD_STOCKPILE,
        *BAD_POPULATION,
        *GOOD_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_MISIORLA"),
        *BAD_SUPPLY,
        *BAD_DEFENSE_TROOPS,
        *ULTIMATE_WEAPONS,
        *BAD_DETECTION,
        *BAD_STEALTH,
        *BAD_FUEL,
        # not for description,
        *AVERAGE_PLANETARY_SHIELDS,
        *AVERAGE_PLANETARY_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=TOXIC_STANDARD_EP,
    graphic="icons/species/misiorla.png",
)
