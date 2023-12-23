from focs._species import *
from species.species_macros.empire_opinions import COMMON_OPINION_EFFECTS
from species.species_macros.env import SWAMP_STANDARD_EP
from species.species_macros.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.species_macros.happiness import AVERAGE_HAPPINESS
from species.species_macros.industry import AVERAGE_INDUSTRY
from species.species_macros.influence import GREAT_INFLUENCE
from species.species_macros.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.species_macros.population import GOOD_POPULATION
from species.species_macros.research import AVERAGE_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stockpile import AVERAGE_STOCKPILE
from species.species_macros.supply import AVERAGE_SUPPLY
from species.species_macros.troops import BAD_DEFENSE_TROOPS, BAD_OFFENSE_TROOPS
from species.species_macros.weapons import BAD_WEAPONS

Species(
    name="SP_TAEGHIRUS",
    description="SP_TAEGHIRUS_DESC",
    gameplay_description="SP_TAEGHIRUS_GAMEPLAY_DESC",
    native=True,
    can_produce_ships=True,
    can_colonize=True,
    tags=[
        "ORGANIC",
        "TELEPATHIC",
        "GOOD_POPULATION",
        "GREAT_INFLUENCE",
        "BAD_WEAPONS",
        "AVERAGE_SUPPLY",
        "BAD_OFFENSE_TROOPS",
        "PEDIA_ORGANIC_SPECIES_CLASS",
        "PEDIA_TELEPATHIC_TITLE",
    ],
    foci=[
        HAS_INDUSTRY_FOCUS,
        HAS_RESEARCH_FOCUS,
        HAS_INFLUENCE_FOCUS,
        HAS_GROWTH_FOCUS,
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_INFLUENCE",
    likes=[
        "FOCUS_INFLUENCE",
        "SHIMMER_SILK_SPECIAL",
        "SPARK_FOSSILS_SPECIAL",
        "WORLDTREE_SPECIAL",
        "MINERALS_SPECIAL",
        "FRUIT_SPECIAL",
        "HONEYCOMB_SPECIAL",
        "PLC_CONFORMANCE",
        "PLC_MODERATION",
        "PLC_POPULATION",
        "PLC_CENTRALIZATION",
        "PLC_INDOCTRINATION",
    ],
    dislikes=[
        "BLD_INTERSPECIES_ACADEMY",
        "BLD_CULTURE_ARCHIVES",
        "BLD_CULTURE_LIBRARY",
        "BLD_COLLECTIVE_NET",
        "BLD_PLANET_BEACON",
        "BLD_GENOME_BANK",
        "BLD_COLLECTIVE_NET",
        "RESONANT_MOON_SPECIAL",
        "TIDAL_LOCK_SPECIAL",
        "ECCENTRIC_ORBIT_SPECIAL",
        "KRAKEN_NEST_SPECIAL",
        "PLC_MARINE_RECRUITMENT",
        "PLC_TERROR_SUPPRESSION",
        "PLC_MARTIAL_LAW",
        "PLC_DIVINE_AUTHORITY",
        "SP_NIGHTSIDERS",
    ],
    effectsgroups=[
        *AVERAGE_INDUSTRY,
        *AVERAGE_RESEARCH,
        *GREAT_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *GOOD_POPULATION,
        *AVERAGE_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_TAEGHIRUS"),
        *AVERAGE_SUPPLY,
        *BAD_DEFENSE_TROOPS,
        *BAD_OFFENSE_TROOPS,
        *BAD_WEAPONS,
        # not for description,
        *DEFAULT_NATIVE_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=SWAMP_STANDARD_EP,
    graphic="icons/species/t-aeghirus.png",
)
