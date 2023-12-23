from focs._species import *
from species.species_macros.empire_opinions import COMMON_OPINION_EFFECTS
from species.species_macros.env import TERRAN_STANDARD_EP
from species.species_macros.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.species_macros.happiness import AVERAGE_HAPPINESS
from species.species_macros.industry import AVERAGE_INDUSTRY
from species.species_macros.influence import AVERAGE_INFLUENCE
from species.species_macros.planet_defense import AVERAGE_PLANETARY_DEFENSE
from species.species_macros.planet_shields import AVERAGE_PLANETARY_SHIELDS
from species.species_macros.planet_size import LARGE_PLANET
from species.species_macros.population import AVERAGE_POPULATION
from species.species_macros.research import AVERAGE_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stockpile import AVERAGE_STOCKPILE
from species.species_macros.supply import AVERAGE_SUPPLY
from species.species_macros.troops import AVERAGE_DEFENSE_TROOPS
from species.species_macros.weapons import GREAT_WEAPONS
from species.species_macros.xenophobic import XENOPHOBIC_OTHER, XENOPHOBIC_SELF

Species(
    name="SP_EAXAW",
    description="SP_EAXAW_DESC",
    gameplay_description="SP_EAXAW_GAMEPLAY_DESC",
    playable=True,
    can_produce_ships=True,
    can_colonize=True,
    tags=[
        "ORGANIC",
        "XENOPHOBIC",
        "GREAT_WEAPONS",
        "AVERAGE_SUPPLY",
        "PEDIA_ORGANIC_SPECIES_CLASS",
        "PEDIA_XENOPHOBIC_SPECIES_TITLE",
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
        "SHIMMER_SILK_SPECIAL",
        "FRACTAL_GEODES_SPECIAL",
        "SUCCULENT_BARNACLES_SPECIAL",
        "CRYSTALS_SPECIAL",
        "ANCIENT_RUINS_DEPLETED_SPECIAL",
        "PANOPTICON_SPECIAL",
        "PHILOSOPHER_SPECIAL",
        "PLC_CONTINUOUS_SCANNING",
        "PLC_TERRAFORMING",
        "PLC_CAPITAL_MARKETS",
        "PLC_DESIGN_SIMPLICITY",
        "PLC_CONFEDERATION",
    ],
    dislikes=[
        "PLC_DIVERSITY",
        "BLD_INTERSPECIES_ACADEMY",
        "BLD_XENORESURRECTION_LAB",
        "BLD_COLLECTIVE_NET",
        "BLD_LIGHTHOUSE",
        "BLD_REGIONAL_ADMIN",
        "RESONANT_MOON_SPECIAL",
        "TEMPORAL_ANOMALY_SPECIAL",
        "MINERALS_SPECIAL",
        "PROBIOTIC_SPECIAL",
        "PLC_DIVERSITY",
        "PLC_ENVIRONMENTALISM",
        "PLC_ALLIED_REPAIR",
        "PLC_MODERATION",
        "PLC_NATIVE_APPROPRIATION",
        "SP_NIGHTSIDERS",
    ],
    effectsgroups=[
        *AVERAGE_INDUSTRY,
        *AVERAGE_RESEARCH,
        *AVERAGE_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *AVERAGE_POPULATION,
        *AVERAGE_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_EAXAW"),
        *AVERAGE_SUPPLY,
        *AVERAGE_DEFENSE_TROOPS,
        *GREAT_WEAPONS,
        *XENOPHOBIC_SELF,
        XENOPHOBIC_OTHER("EAXAW"),
        # not for description,
        *AVERAGE_PLANETARY_SHIELDS,
        *AVERAGE_PLANETARY_DEFENSE,
        *LARGE_PLANET,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=TERRAN_STANDARD_EP,
    graphic="icons/species/eaxaw.png",
)
