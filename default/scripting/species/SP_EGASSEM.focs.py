from focs._species import *
from species.species_macros.empire_opinions import COMMON_OPINION_EFFECTS
from species.species_macros.env import INFERNO_NARROW_EP, NARROW_EP
from species.species_macros.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.species_macros.happiness import BAD_HAPPINESS
from species.species_macros.industry import GREAT_INDUSTRY
from species.species_macros.influence import BAD_INFLUENCE
from species.species_macros.planet_defense import AVERAGE_PLANETARY_DEFENSE
from species.species_macros.planet_shields import AVERAGE_PLANETARY_SHIELDS
from species.species_macros.planet_size import LARGE_PLANET
from species.species_macros.population import BAD_POPULATION
from species.species_macros.research import BAD_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stockpile import AVERAGE_STOCKPILE
from species.species_macros.supply import GREAT_SUPPLY
from species.species_macros.troops import GREAT_DEFENSE_TROOPS, GREAT_OFFENSE_TROOPS

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
        "PLC_CONFORMANCE",
        "PLC_FEUDALISM",
        "PLC_ALGORITHMIC_RESEARCH",
        "PLC_DESIGN_SIMPLICITY",
        "PLC_MARINE_RECRUITMENT",
        "PLC_MARTIAL_LAW",
        "PLC_PROPAGANDA",
    ],
    dislikes=[
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
        "PLC_BOOTSTRAPPING",
        "PLC_CENTRALIZATION",
        "PLC_LIBERTY",
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
        *LARGE_PLANET,
        NARROW_EP,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=INFERNO_NARROW_EP,
    graphic="icons/species/egassem.png",
)
