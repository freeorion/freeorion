from focs._species import *
from species.species_macros.detection import GOOD_DETECTION
from species.species_macros.empire_opinions import COMMON_OPINION_EFFECTS
from species.species_macros.env import NARROW_EP, OCEAN_NARROW_EP
from species.species_macros.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.species_macros.general import SLOW_COLONIZATION
from species.species_macros.happiness import BAD_HAPPINESS
from species.species_macros.industry import BAD_INDUSTRY
from species.species_macros.influence import BAD_INFLUENCE
from species.species_macros.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.species_macros.population import BAD_POPULATION
from species.species_macros.research import GOOD_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stealth import BAD_STEALTH
from species.species_macros.stockpile import AVERAGE_STOCKPILE
from species.species_macros.supply import GREAT_SUPPLY
from species.species_macros.troops import BAD_DEFENSE_TROOPS, BAD_OFFENSE_TROOPS

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
