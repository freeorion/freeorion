from focs._species import *
from species.species_macros.empire_opinions import COMMON_OPINION_EFFECTS
from species.species_macros.env import TUNDRA_STANDARD_EP
from species.species_macros.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.species_macros.happiness import GOOD_HAPPINESS
from species.species_macros.industry import AVERAGE_INDUSTRY
from species.species_macros.influence import GOOD_INFLUENCE
from species.species_macros.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.species_macros.population import AVERAGE_POPULATION
from species.species_macros.research import AVERAGE_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stockpile import AVERAGE_STOCKPILE
from species.species_macros.supply import AVERAGE_SUPPLY
from species.species_macros.troops import GREAT_DEFENSE_TROOPS

Species(
    name="SP_HHHOH",
    description="SP_HHHOH_DESC",
    gameplay_description="SP_HHHOH_GAMEPLAY_DESC",
    native=True,
    can_produce_ships=True,
    can_colonize=True,
    tags=["ORGANIC", "GOOD_INFLUENCE", "GOOD_HAPPINESS", "AVERAGE_SUPPLY", "PEDIA_ORGANIC_SPECIES_CLASS"],
    foci=[
        HAS_INDUSTRY_FOCUS,
        HAS_RESEARCH_FOCUS,
        HAS_INFLUENCE_FOCUS,
        HAS_GROWTH_FOCUS,
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_INDUSTRY",
    likes=[
        "ECCENTRIC_ORBIT_SPECIAL",
        "FOCUS_INDUSTRY",
        "KRAKEN_NEST_SPECIAL",
        "MINERALS_SPECIAL",
        "PLC_CONFORMANCE",
        "PLC_MODERATION",
        "RESONANT_MOON_SPECIAL",
        "SHIMMER_SILK_SPECIAL",
        "SUCCULENT_BARNACLES_SPECIAL",
    ],
    dislikes=[
        "BLD_COLLECTIVE_NET",
        "BLD_SCANNING_FACILITY",
        "ELERIUM_SPECIAL",
        "FOCUS_PROTECTION",
        "FOCUS_RESEARCH",
        "FRUIT_SPECIAL",
        "GAIA_SPECIAL",
        "PLC_RACIAL_PURITY",
        "PLC_TECHNOCRACY",
        "POSITRONIUM_SPECIAL",
        "SP_NIGHTSIDERS",
    ],
    effectsgroups=[
        *AVERAGE_INDUSTRY,
        *AVERAGE_RESEARCH,
        *GOOD_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *AVERAGE_POPULATION,
        *GOOD_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_HHHOH"),
        *AVERAGE_SUPPLY,
        *GREAT_DEFENSE_TROOPS,
        # not for description,
        *DEFAULT_NATIVE_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=TUNDRA_STANDARD_EP,
    graphic="icons/species/hhhoh.png",
)
