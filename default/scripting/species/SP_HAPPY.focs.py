from focs._species import *
from species.common.detection import GOOD_DETECTION
from species.common.empire_opinions import COMMON_OPINION_EFFECTS
from species.common.env import NARROW_EP, OCEAN_NARROW_EP
from species.common.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.common.general import SLOW_COLONIZATION
from species.common.happiness import BAD_HAPPINESS
from species.common.industry import BAD_INDUSTRY
from species.common.influence import BAD_INFLUENCE
from species.common.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.common.population import BAD_POPULATION
from species.common.research import GOOD_RESEARCH
from species.common.shields import STANDARD_SHIP_SHIELDS
from species.common.stealth import BAD_STEALTH
from species.common.stockpile import AVERAGE_STOCKPILE
from species.common.supply import GREAT_SUPPLY
from species.common.troops import BAD_DEFENSE_TROOPS, BAD_OFFENSE_TROOPS

Species(
    name="SP_HAPPY",
    description="SP_HAPPY_DESC",
    gameplay_description="SP_HAPPY_GAMEPLAY_DESC",
    native=True,
    can_produce_ships=True,
    can_colonize=True,
    tags=[
        "ROBOTIC",
        "TELEPATHIC",
        "GOOD_RESEARCH",
        "BAD_INDUSTRY",
        "BAD_INFLUENCE",
        "BAD_POPULATION",
        "BAD_HAPPINESS",
        "GREAT_SUPPLY",
        "GOOD_DETECTION",
        "BAD_STEALTH",
        "BAD_OFFENSE_TROOPS",
        "PEDIA_ROBOTIC_SPECIES_CLASS",
        "PEDIA_TELEPATHIC_TITLE",
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
        "FOCUS_RESEARCH",
        "MIMETIC_ALLOY_SPECIAL",
        "SPARK_FOSSILS_SPECIAL",
        "MONOPOLE_SPECIAL",
        "GAIA_SPECIAL",
        "PANOPTICON_SPECIAL",
        "RESONANT_MOON_SPECIAL",
        "PLC_ARTISAN_WORKSHOPS",
        "PLC_DIVERSITY",
    ],
    dislikes=["BLD_PLANET_CLOAK", "BLD_SPATIAL_DISTORT_GEN", "PLC_ISOLATION", "PLC_NO_SUPPLY", "PLC_RACIAL_PURITY"],
    effectsgroups=[
        *BAD_INDUSTRY,
        *GOOD_RESEARCH,
        *BAD_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *BAD_POPULATION,
        *BAD_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_HAPPY"),
        *GREAT_SUPPLY,
        *BAD_DEFENSE_TROOPS,
        *BAD_OFFENSE_TROOPS,
        *BAD_STEALTH,
        *GOOD_DETECTION,
        SLOW_COLONIZATION,
        # not for description,
        *DEFAULT_NATIVE_DEFENSE,
        NARROW_EP,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=OCEAN_NARROW_EP,
    graphic="icons/species/ichthyoid-06.png",
)
