from focs._species import *
from species.species_macros.detection import ULTIMATE_DETECTION
from species.species_macros.empire_opinions import FIXED_OPINION_EFFECTS
from species.species_macros.env import VERY_TOLERANT_EP
from species.species_macros.focus import (
    HAS_ADVANCED_FOCI,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.species_macros.happiness import AVERAGE_HAPPINESS
from species.species_macros.industry import NO_INDUSTRY
from species.species_macros.influence import ULTIMATE_INFLUENCE
from species.species_macros.planet_defense import AVERAGE_PLANETARY_DEFENSE
from species.species_macros.planet_shields import AVERAGE_PLANETARY_SHIELDS
from species.species_macros.population import GOOD_POPULATION
from species.species_macros.research import ULTIMATE_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stealth import ULTIMATE_STEALTH
from species.species_macros.stockpile import ULTIMATE_STOCKPILE
from species.species_macros.supply import AVERAGE_SUPPLY
from species.species_macros.troops import ULTIMATE_DEFENSE_TROOPS

Species(
    name="SP_EXPERIMENTOR",
    description="SP_EXPERIMENTOR_DESC",
    gameplay_description="SP_EXPERIMENTOR_GAMEPLAY_DESC",
    tags=[
        "SELF_SUSTAINING",
        "NO_INDUSTRY",
        "ULTIMATE_INFLUENCE",
        "ULTIMATE_RESEARCH",
        "GOOD_POPULATION",
        "ULTIMATE_STOCKPILE",
        "ULTIMATE_DETECTION",
        "ULTIMATE_STEALTH",
        "PEDIA_SELF_SUSTAINING_SPECIES_CLASS",
    ],
    foci=[
        # [[HAS_INDUSTRY_FOCUS]]
        HAS_RESEARCH_FOCUS,
        HAS_INFLUENCE_FOCUS,
        # [[HAS_GROWTH_FOCUS]]
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_RESEARCH",
    likes=[
        "FOCUS_RESEARCH",
        "FRACTAL_GEODES_SPECIAL",
        "SPARK_FOSSILS_SPECIAL",
        "BLD_SHIPYARD_ENRG_COMP",
        "BLD_SHIPYARD_BASE",
        "BLD_SHIPYARD_ORBITAL_DRYDOCK",
        "BLD_BIOTERROR_PROJECTOR",
        "TIDAL_LOCK_SPECIAL",
        "ANCIENT_RUINS_DEPLETED_SPECIAL",
        "HONEYCOMB_SPECIAL",
        "ELERIUM_SPECIAL",
    ],
    dislikes=[
        "BLD_SHIPYARD_ENRG_COMP",
        "BLD_CULTURE_ARCHIVES",
        "BLD_CULTURE_LIBRARY",
        "MONOPOLE_SPECIAL",
        "PANOPTICON_SPECIAL",
        "TEMPORAL_ANOMALY_SPECIAL",
        "SPICE_SPECIAL",
    ],
    annexation_condition=NoObject,
    effectsgroups=[
        NO_INDUSTRY,
        *ULTIMATE_RESEARCH,
        *ULTIMATE_INFLUENCE,
        *ULTIMATE_STOCKPILE,
        *GOOD_POPULATION,
        *AVERAGE_HAPPINESS,
        FIXED_OPINION_EFFECTS("SP_EXPERIMENTOR", 0),
        *AVERAGE_SUPPLY,
        *ULTIMATE_DEFENSE_TROOPS,
        *ULTIMATE_STEALTH,
        *ULTIMATE_DETECTION,
        # not for description,
        *AVERAGE_PLANETARY_SHIELDS,
        *AVERAGE_PLANETARY_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=VERY_TOLERANT_EP,
    graphic="icons/species/two-headed-03.png",
)
