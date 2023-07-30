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
from species.common.happiness import BAD_HAPPINESS
from species.common.industry import GREAT_INDUSTRY
from species.common.influence import BAD_INFLUENCE
from species.common.planet_defense import AVERAGE_PLANETARY_DEFENSE
from species.common.planet_shields import AVERAGE_PLANETARY_SHIELDS
from species.common.planet_size import LARGE_PLANET
from species.common.population import BAD_POPULATION
from species.common.research import BAD_RESEARCH
from species.common.shields import STANDARD_SHIP_SHIELDS
from species.common.stockpile import AVERAGE_STOCKPILE
from species.common.supply import GREAT_SUPPLY
from species.common.troops import GREAT_DEFENSE_TROOPS, GREAT_OFFENSE_TROOPS

Species(
    name="SP_EGASSEM",
    description="SP_EGASSEM_DESC",
    gameplay_description="SP_EGASSEM_GAMEPLAY_DESC",
    playable=True,
    can_produce_ships=True,
    can_colonize=True,
    tags=[
        "LITHIC",
        "ARTISTIC",
        "BAD_RESEARCH",
        "GREAT_INDUSTRY",
        "BAD_INFLUENCE",
        "BAD_POPULATION",
        "BAD_HAPPINESS",
        "GREAT_SUPPLY",
        "GREAT_OFFENSE_TROOPS",
        "PEDIA_LITHIC_SPECIES_CLASS",
        "PEDIA_ARTISTIC",
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
        "FRACTAL_GEODES_SPECIAL",
        "SHIMMER_SILK_SPECIAL",
        "PROBIOTIC_SPECIAL",
        "MONOPOLE_SPECIAL",
        "ELERIUM_SPECIAL",
        "FORTRESS_SPECIAL",
        "PLC_CONFEDERATION",
        "PLC_CAPITAL_MARKETS",
        "PLC_ALGORITHMIC_RESEARCH",
        "PLC_DESIGN_SIMPLICITY",
        "PLC_MARINE_RECRUITMENT",
        "PLC_MARTIAL_LAW",
    ],
    dislikes=[
        "BLD_AUTO_HISTORY_ANALYSER",
        "BLD_NEUTRONIUM_EXTRACTOR",
        "BLD_BLACK_HOLE_POW_GEN",
        "BLD_STARGATE",
        "BLD_SHIPYARD_AST_REF",
        "BLD_SHIPYARD_AST",
        "HONEYCOMB_SPECIAL",
        "WORLDTREE_SPECIAL",
        "KRAKEN_IN_THE_ICE_SPECIAL",
        "KRAKEN_NEST_SPECIAL",
        "PLC_ENVIRONMENTALISM",
        "PLC_CENTRALIZATION",
        "PLC_NO_GROWTH",
        "PLC_DREAM_RECURSION",
        "PLC_ENGINEERING",
    ],
    effectsgroups=[
        *GREAT_INDUSTRY,
        *BAD_RESEARCH,
        *BAD_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *BAD_POPULATION,
        *BAD_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_EGASSEM"),
        *GREAT_SUPPLY,
        *GREAT_DEFENSE_TROOPS,
        *GREAT_OFFENSE_TROOPS,
        # not for description,
        *AVERAGE_PLANETARY_SHIELDS,
        *AVERAGE_PLANETARY_DEFENSE,
        LARGE_PLANET,
        NARROW_EP,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=INFERNO_NARROW_EP,
    graphic="icons/species/egassem.png",
)
