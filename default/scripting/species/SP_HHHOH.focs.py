from focs._species import *
from species.common.empire_opinions import COMMON_OPINION_EFFECTS
from species.common.env import TUNDRA_STANDARD_EP
from species.common.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.common.happiness import GOOD_HAPPINESS
from species.common.industry import AVERAGE_INDUSTRY
from species.common.influence import GOOD_INFLUENCE
from species.common.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.common.population import AVERAGE_POPULATION
from species.common.research import AVERAGE_RESEARCH
from species.common.shields import STANDARD_SHIP_SHIELDS
from species.common.stockpile import AVERAGE_STOCKPILE
from species.common.supply import AVERAGE_SUPPLY
from species.common.troops import GREAT_DEFENSE_TROOPS

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
