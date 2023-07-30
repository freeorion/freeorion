from focs._species import *
from species.common.empire_opinions import COMMON_OPINION_EFFECTS
from species.common.env import RADIATED_STANDARD_EP
from species.common.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.common.happiness import VERY_BAD_HAPPINESS
from species.common.industry import AVERAGE_INDUSTRY
from species.common.influence import AVERAGE_INFLUENCE
from species.common.planet_defense import AVERAGE_PLANETARY_DEFENSE
from species.common.planet_shields import AVERAGE_PLANETARY_SHIELDS
from species.common.planet_size import LARGE_PLANET
from species.common.population import GREAT_POPULATION
from species.common.research import VERY_BAD_RESEARCH
from species.common.shields import STANDARD_SHIP_SHIELDS
from species.common.stockpile import AVERAGE_STOCKPILE
from species.common.supply import AVERAGE_SUPPLY
from species.common.troops import AVERAGE_DEFENSE_TROOPS

Species(
    name="SP_REPLICON",
    description="SP_REPLICON_DESC",
    gameplay_description="SP_REPLICON_GAMEPLAY_DESC",
    playable=True,
    can_produce_ships=True,
    can_colonize=True,
    tags=[
        "ROBOTIC",
        "VERY_BAD_RESEARCH",
        "VERY_BAD_HAPPINESS",
        "GREAT_POPULATION",
        "ADAPTIVE",
        "PEDIA_ADAPTIVE_TITLE",
        "PEDIA_ROBOTIC_SPECIES_CLASS",
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
        "FOCUS_INFLUENCE",
        "PANOPTICON_SPECIAL",
        "PHILOSOPHER_SPECIAL",
        "PLC_COLONIZATION",
        "PLC_COLONIALISM",
        "PLC_DESIGN_SIMPLICITY",
        "PLC_DIVINE_AUTHORITY",
        "PLC_EXPLORATION",
        "PLC_FEUDALISM",
        "PLC_MARINE_RECRUITMENT",
        "PLC_NATIVE_APPROPRIATION",
        "PLC_POPULATION",
        "PLC_VASSALIZATION",
        "SHIMMER_SILK_SPECIAL",
        "SPARK_FOSSILS_SPECIAL",
        "SPICE_SPECIAL",
    ],
    dislikes=[
        # "BLD_CLONING_CENTER"       // disabled content,
        "FOCUS_RESEARCH",
        "BLD_SHIPYARD_AST",
        "BLD_SHIPYARD_ENRG_COMP",
        "BLD_SHIPYARD_ORG_CELL_GRO_CHAMB",
        "CRYSTALS_SPECIAL",
        "GAIA_SPECIAL",
        "PLC_ALGORITHMIC_RESEARCH",
        "PLC_ENVIRONMENTALISM",
        "PLC_INDUSTRIALISM",
        "PLC_LIBERTY",
        "PLC_NO_GROWTH",
        "PLC_NO_SUPPLY",
        "PLC_TECHNOCRACY",
        "RESONANT_MOON_SPECIAL",
    ],
    effectsgroups=[
        *AVERAGE_INDUSTRY,
        *VERY_BAD_RESEARCH,
        *AVERAGE_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *GREAT_POPULATION,
        *VERY_BAD_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_REPLICON"),
        *AVERAGE_SUPPLY,
        *AVERAGE_DEFENSE_TROOPS,
        # not for description,
        *AVERAGE_PLANETARY_SHIELDS,
        *AVERAGE_PLANETARY_DEFENSE,
        LARGE_PLANET,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=RADIATED_STANDARD_EP,
    graphic="icons/species/replicon.png",
)
