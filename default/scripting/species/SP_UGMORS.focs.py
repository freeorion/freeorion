from focs._species import *
from species.common.empire_opinions import COMMON_OPINION_EFFECTS
from species.common.env import INFERNO_NARROW_EP, NARROW_EP
from species.common.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.common.happiness import AVERAGE_HAPPINESS
from species.common.industry import GOOD_INDUSTRY
from species.common.influence import GOOD_INFLUENCE
from species.common.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.common.population import AVERAGE_POPULATION
from species.common.research import BAD_RESEARCH
from species.common.shields import STANDARD_SHIP_SHIELDS
from species.common.stockpile import AVERAGE_STOCKPILE
from species.common.supply import AVERAGE_SUPPLY
from species.common.troops import GOOD_DEFENSE_TROOPS, GOOD_OFFENSE_TROOPS

Species(
    name="SP_UGMORS",
    description="SP_UGMORS_DESC",
    gameplay_description="SP_UGMORS_GAMEPLAY_DESC",
    native=True,
    can_produce_ships=True,
    can_colonize=True,
    tags=[
        "LITHIC",
        "GOOD_INDUSTRY",
        "BAD_RESEARCH",
        "GOOD_INFLUENCE",
        "AVERAGE_SUPPLY",
        "GOOD_OFFENSE_TROOPS",
        "PEDIA_LITHIC_SPECIES_CLASS",
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
        "FOCUS_STOCKPILE",
        "FOCUS_GROWTH",
        "MIMETIC_ALLOY_SPECIAL",
        "SPARK_FOSSILS_SPECIAL",
        "ECCENTRIC_ORBIT_SPECIAL",
        "ELERIUM_SPECIAL",
        "RESONANT_MOON_SPECIAL",
        "TIDAL_LOCK_SPECIAL",
        "PLC_CENTRALIZATION",
        "PLC_CONFORMANCE",
        "PLC_INDOCTRINATION",
        "PLC_TRAFFIC_CONTROL",
    ],
    dislikes=[
        "FOCUS_RESEARCH",
        "FOCUS_INFLUENCE",
        "BLD_BIOTERROR_PROJECTOR",
        "SUPERCONDUCTOR_SPECIAL",
        "SPICE_SPECIAL",
        "PROBIOTIC_SPECIAL",
        "POSITRONIUM_SPECIAL",
        "PLC_LIBERTY",
        "PLC_MARINE_RECRUITMENT",
        "PLC_DIVINE_AUTHORITY",
        "PLC_CONFEDERATION",
    ],
    effectsgroups=[
        *GOOD_INDUSTRY,
        *BAD_RESEARCH,
        *GOOD_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *AVERAGE_POPULATION,
        *AVERAGE_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_UGMORS"),
        *AVERAGE_SUPPLY,
        *GOOD_DEFENSE_TROOPS,
        *GOOD_OFFENSE_TROOPS,
        # not for description,
        *DEFAULT_NATIVE_DEFENSE,
        NARROW_EP,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=INFERNO_NARROW_EP,
    graphic="icons/species/amorphous-06.png",
)
