from focs._species import *
from species.common.empire_opinions import COMMON_OPINION_EFFECTS
from species.common.env import BARREN_STANDARD_EP
from species.common.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.common.happiness import AVERAGE_HAPPINESS
from species.common.industry import AVERAGE_INDUSTRY
from species.common.influence import GOOD_INFLUENCE
from species.common.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.common.planet_size import HUGE_PLANET
from species.common.population import GOOD_POPULATION
from species.common.research import AVERAGE_RESEARCH
from species.common.shields import STANDARD_SHIP_SHIELDS
from species.common.stockpile import AVERAGE_STOCKPILE
from species.common.supply import ULTIMATE_SUPPLY
from species.common.troops import AVERAGE_DEFENSE_TROOPS

Species(
    name="SP_VOLP",
    description="SP_VOLP_DESC",
    gameplay_description="SP_VOLP_GAMEPLAY_DESC",
    native=True,
    tags=["LITHIC", "GOOD_INFLUENCE", "GOOD_POPULATION", "ULTIMATE_SUPPLY", "PEDIA_LITHIC_SPECIES_CLASS"],
    foci=[
        HAS_INDUSTRY_FOCUS,
        HAS_RESEARCH_FOCUS,
        HAS_INFLUENCE_FOCUS,
        HAS_GROWTH_FOCUS,
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_PROTECTION",
    likes=[
        "FOCUS_INFLUENCE",
        "FOCUS_LOGISTICS",
        "FRACTAL_GEODES_SPECIAL",
        "MIMETIC_ALLOY_SPECIAL",
        "BLD_IMPERIAL_PALACE",
        "BLD_REGIONAL_ADMIN",
        "SUPERCONDUCTOR_SPECIAL",
        "SPICE_SPECIAL",
        "PROBIOTIC_SPECIAL",
        "POSITRONIUM_SPECIAL",
        "PLC_CONFORMANCE",
        "PLC_TERRAFORMING",
        "PLC_CENTRALIZATION",
        "PLC_INDOCTRINATION",
        "PLC_CAPITAL_MARKETS",
        "PLC_BUREAUCRACY",
    ],
    dislikes=[
        "BLD_GENOME_BANK",
        "BLD_BLACK_HOLE_POW_GEN",
        "BLD_STOCKPILING_CENTER",
        "BLD_MEGALITH",
        "BLD_SCRYING_SPHERE",
        "BLD_SHIPYARD_ENRG_COMP",
        "BLD_SHIPYARD_ENRG_SOLAR",
        "HONEYCOMB_SPECIAL",
        "KRAKEN_IN_THE_ICE_SPECIAL",
        "KRAKEN_NEST_SPECIAL",
        "PANOPTICON_SPECIAL",
        "PLC_COLONIZATION",
        "PLC_DIVERSITY",
        "PLC_ALGORITHMIC_RESEARCH",
        "PLC_ENVIRONMENTALISM",
    ],
    effectsgroups=[
        *AVERAGE_INDUSTRY,
        *AVERAGE_RESEARCH,
        *GOOD_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *GOOD_POPULATION,
        *AVERAGE_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_VOLP"),
        *ULTIMATE_SUPPLY,
        *AVERAGE_DEFENSE_TROOPS,
        # not for description,
        *DEFAULT_NATIVE_DEFENSE,
        *HUGE_PLANET,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=BARREN_STANDARD_EP,
    graphic="icons/species/volp.png",
)
