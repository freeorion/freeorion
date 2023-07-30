from focs._species import *
from species.common.empire_opinions import COMMON_OPINION_EFFECTS
from species.common.env import SWAMP_STANDARD_EP
from species.common.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
)
from species.common.general import FAST_COLONIZATION
from species.common.happiness import GOOD_HAPPINESS
from species.common.industry import AVERAGE_INDUSTRY
from species.common.influence import GOOD_INFLUENCE
from species.common.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.common.population import AVERAGE_POPULATION
from species.common.research import NO_RESEARCH
from species.common.shields import STANDARD_SHIP_SHIELDS
from species.common.stockpile import AVERAGE_STOCKPILE
from species.common.supply import BAD_SUPPLY
from species.common.troops import AVERAGE_DEFENSE_TROOPS

Species(
    name="SP_PHINNERT",
    description="SP_PHINNERT_DESC",
    gameplay_description="SP_PHINNERT_GAMEPLAY_DESC",
    native=True,
    can_colonize=True,
    tags=[
        "ORGANIC",
        "NO_RESEARCH",
        "GOOD_INFLUENCE",
        "GOOD_HAPPINESS",
        "BAD_SUPPLY",
        "PRIMITIVE",
        "PEDIA_ORGANIC_SPECIES_CLASS",
    ],
    foci=[
        HAS_INDUSTRY_FOCUS,
        HAS_INFLUENCE_FOCUS,
        HAS_GROWTH_FOCUS,
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_INDUSTRY",
    likes=[
        "FOCUS_INDUSTRY",
        "SUCCULENT_BARNACLES_SPECIAL",
        "MIMETIC_ALLOY_SPECIAL",
        "MINERALS_SPECIAL",
        "CRYSTALS_SPECIAL",
        "PANOPTICON_SPECIAL",
        "TIDAL_LOCK_SPECIAL",
        "PLC_ARTISAN_WORKSHOPS",
        "PLC_COLONIZATION",
        "PLC_POPULATION",
        "PLC_STOCKPILE_LIQUIDATION",
    ],
    dislikes=[
        "BLD_CULTURE_ARCHIVES",
        "BLD_CULTURE_LIBRARY",
        "BLD_REGIONAL_ADMIN",
        "BLD_MILITARY_COMMAND",
        "BLD_MEGALITH",
        "BLD_GENOME_BANK",
        "BLD_BLACK_HOLE_POW_GEN",
        "BLD_GAS_GIANT_GEN",
        "SPICE_SPECIAL",
        "PROBIOTIC_SPECIAL",
        "POSITRONIUM_SPECIAL",
        "HONEYCOMB_SPECIAL",
        "PLC_MARINE_RECRUITMENT",
        "PLC_NO_GROWTH",
        "SP_NIGHTSIDERS",
    ],
    effectsgroups=[
        *AVERAGE_INDUSTRY,
        NO_RESEARCH,
        *GOOD_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *AVERAGE_POPULATION,
        *GOOD_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_PHINNERT"),
        *BAD_SUPPLY,
        *AVERAGE_DEFENSE_TROOPS,
        FAST_COLONIZATION,
        # not for description,
        *DEFAULT_NATIVE_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=SWAMP_STANDARD_EP,
    graphic="icons/species/phinnert.png",
)
