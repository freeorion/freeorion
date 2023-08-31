from focs._species import *
from species.common.detection import BAD_DETECTION
from species.common.empire_opinions import COMMON_OPINION_EFFECTS
from species.common.env import DESERT_STANDARD_EP
from species.common.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.common.happiness import AVERAGE_HAPPINESS
from species.common.industry import BAD_INDUSTRY
from species.common.influence import AVERAGE_INFLUENCE
from species.common.planet_defense import AVERAGE_PLANETARY_DEFENSE
from species.common.planet_shields import AVERAGE_PLANETARY_SHIELDS
from species.common.planet_size import LARGE_PLANET
from species.common.population import AVERAGE_POPULATION
from species.common.research import AVERAGE_RESEARCH
from species.common.shields import STANDARD_SHIP_SHIELDS
from species.common.stealth import GOOD_STEALTH
from species.common.stockpile import AVERAGE_STOCKPILE
from species.common.supply import GREAT_SUPPLY
from species.common.troops import GOOD_DEFENSE_TROOPS
from species.common.weapons import GOOD_WEAPONS

Species(
    name="SP_ETTY",
    description="SP_ETTY_DESC",
    gameplay_description="SP_ETTY_GAMEPLAY_DESC",
    playable=True,
    can_produce_ships=True,
    can_colonize=True,
    tags=[
        "ROBOTIC",
        "BAD_INDUSTRY",
        "GOOD_WEAPONS",
        "GREAT_SUPPLY",
        "BAD_DETECTION",
        "GOOD_STEALTH",
        "PEDIA_ROBOTIC_SPECIES_CLASS",
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
        "SUPERCONDUCTOR_SPECIAL",
        "PROBIOTIC_SPECIAL",
        "COMPUTRONIUM_SPECIAL",
        "FORTRESS_SPECIAL",
        "PLC_ALLIED_REPAIR",
        "PLC_CENTRALIZATION",
        "PLC_LIBERTY",
        "PLC_CONTINUOUS_SCANNING",
        "PLC_BUREAUCRACY",
    ],
    dislikes=[
        "BLD_SHIPYARD_AST",
        "BLD_SHIPYARD_AST_REF",
        "BLD_SPATIAL_DISTORT_GEN",
        "BLD_SPACE_ELEVATOR",
        "BLD_STOCKPILING_CENTER",
        "BLD_LIGHTHOUSE",
        "FRUIT_SPECIAL",
        "PANOPTICON_SPECIAL",
        "TEMPORAL_ANOMALY_SPECIAL",
        "TIDAL_LOCK_SPECIAL",
        "PLC_CONFEDERATION",
        "PLC_NATIVE_APPROPRIATION",
        "PLC_INDUSTRIALISM",
        "PLC_EXOBOT_PRODUCTIVITY",
    ],
    effectsgroups=[
        *BAD_INDUSTRY,
        *AVERAGE_RESEARCH,
        *AVERAGE_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *AVERAGE_POPULATION,
        *AVERAGE_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_ETTY"),
        *GREAT_SUPPLY,
        *GOOD_DEFENSE_TROOPS,
        *GOOD_WEAPONS,
        *BAD_DETECTION,
        *GOOD_STEALTH,
        # not for description,
        *AVERAGE_PLANETARY_SHIELDS,
        *AVERAGE_PLANETARY_DEFENSE,
        LARGE_PLANET,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=DESERT_STANDARD_EP,
    graphic="icons/species/etty.png",
)
