from common.misc import DESCRIPTION_EFFECTSGROUP_MACRO
from focs._species import *
from species.common.empire_opinions import COMMON_OPINION_EFFECTS
from species.common.env import TERRAN_STANDARD_EP
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
from species.common.planet_defense import AVERAGE_PLANETARY_DEFENSE
from species.common.planet_shields import AVERAGE_PLANETARY_SHIELDS
from species.common.planet_size import LARGE_PLANET
from species.common.population import AVERAGE_POPULATION
from species.common.research import AVERAGE_RESEARCH
from species.common.shields import STANDARD_SHIP_SHIELDS
from species.common.stockpile import AVERAGE_STOCKPILE
from species.common.supply import AVERAGE_SUPPLY
from species.common.troops import AVERAGE_DEFENSE_TROOPS

Species(
    name="SP_HUMAN",
    description="SP_HUMAN_DESC",
    gameplay_description="SP_HUMAN_GAMEPLAY_DESC",
    playable=True,
    can_produce_ships=True,
    can_colonize=True,
    tags=["ORGANIC", "STYLISH", "ARTISTIC", "GOOD_INFLUENCE", "PEDIA_ORGANIC_SPECIES_CLASS", "PEDIA_ARTISTIC"],
    foci=[
        HAS_INDUSTRY_FOCUS,
        HAS_RESEARCH_FOCUS,
        HAS_INFLUENCE_FOCUS,
        HAS_GROWTH_FOCUS,
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_RESEARCH",
    likes=[
        "PLC_MODERATION",
        "FOCUS_INFLUENCE",
        "FOCUS_RESEARCH",
        "WORLDTREE_SPECIAL",
        "SHIMMER_SILK_SPECIAL",
        "FRACTAL_GEODES_SPECIAL",
        "ANCIENT_RUINS_DEPLETED_SPECIAL",
        "GAIA_SPECIAL",
    ],
    dislikes=[
        "PLC_CENTRALIZATION",
        "TIDAL_LOCK_SPECIAL",
        "FOCUS_BIOTERROR",
        "FOCUS_PROTECTION",
        "BLD_MILITARY_COMMAND",
        "BLD_BIOTERROR_PROJECTOR",
        "BLD_SPATIAL_DISTORT_GEN",
        "FORTRESS_SPECIAL",
        "TEMPORAL_ANOMALY_SPECIAL",
        "KRAKEN_NEST_SPECIAL",
        "SP_NIGHTSIDERS",
    ],
    effectsgroups=[
        *AVERAGE_INDUSTRY,
        *AVERAGE_RESEARCH,
        *GOOD_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *AVERAGE_POPULATION,
        *AVERAGE_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_HUMAN"),
        *AVERAGE_SUPPLY,
        *AVERAGE_DEFENSE_TROOPS,
        DESCRIPTION_EFFECTSGROUP_MACRO("AVERAGE_INDUSTRY_DESC"),
        DESCRIPTION_EFFECTSGROUP_MACRO("AVERAGE_RESEARCH_DESC"),
        DESCRIPTION_EFFECTSGROUP_MACRO("AVERAGE_STOCKPILE_DESC"),
        DESCRIPTION_EFFECTSGROUP_MACRO("AVERAGE_POPULATION_DESC"),
        DESCRIPTION_EFFECTSGROUP_MACRO("AVERAGE_SUPPLY_DESC"),
        DESCRIPTION_EFFECTSGROUP_MACRO("AVERAGE_FUEL_DESC"),
        DESCRIPTION_EFFECTSGROUP_MACRO("AVERAGE_DEFENSE_TROOPS_DESC"),
        # not for description,
        *AVERAGE_PLANETARY_SHIELDS,
        *AVERAGE_PLANETARY_DEFENSE,
        *LARGE_PLANET,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=TERRAN_STANDARD_EP,
    graphic="icons/species/human.png",
)
