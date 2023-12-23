from focs._species import *
from species.species_macros.empire_opinions import COMMON_OPINION_EFFECTS
from species.species_macros.env import NARROW_EP, OCEAN_NARROW_EP
from species.species_macros.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.species_macros.happiness import AVERAGE_HAPPINESS
from species.species_macros.industry import BAD_INDUSTRY
from species.species_macros.influence import AVERAGE_INFLUENCE
from species.species_macros.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.species_macros.population import AVERAGE_POPULATION
from species.species_macros.research import AVERAGE_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stockpile import AVERAGE_STOCKPILE
from species.species_macros.supply import AVERAGE_SUPPLY
from species.species_macros.troops import BAD_DEFENSE_TROOPS, BAD_OFFENSE_TROOPS

Species(
    name="SP_SSLITH",
    description="SP_SSLITH_DESC",
    gameplay_description="SP_SSLITH_GAMEPLAY_DESC",
    native=True,
    can_produce_ships=True,
    can_colonize=True,
    tags=[
        "ORGANIC",
        "TELEPATHIC",
        "ARTISTIC",
        "BAD_INDUSTRY",
        "AVERAGE_SUPPLY",
        "BAD_OFFENSE_TROOPS",
        "PEDIA_ORGANIC_SPECIES_CLASS",
        "PEDIA_ARTISTIC",
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
        "ELERIUM_SPECIAL",
        "FOCUS_RESEARCH",
        "FRACTAL_GEODES_SPECIAL",
        "MIMETIC_ALLOY_SPECIAL",
        "MONOPOLE_SPECIAL",
        "PLC_CONFEDERATION",
        "PLC_DREAM_RECURSION",
        "PLC_LIBERTY",
        "PLC_MODERATION",
        "SUPERCONDUCTOR_SPECIAL",
        "TEMPORAL_ANOMALY_SPECIAL",
    ],
    dislikes=[
        # "BLD_CLONING_CENTER"       // disabled content,
        "BLD_ENCLAVE_VOID",
        "BLD_GATEWAY_VOID",
        "BLD_MILITARY_COMMAND",
        "BLD_SHIPYARD_AST",
        "BLD_SHIPYARD_ENRG_COMP",
        "BLD_SPATIAL_DISTORT_GEN",
        "BLD_STARGATE",
        "GAIA_SPECIAL",
        "HONEYCOMB_SPECIAL",
        "PLC_CHECKPOINTS",
        "PLC_CONFORMANCE",
        "PLC_DIVINE_AUTHORITY",
        "PLC_TRAFFIC_CONTROL",
        "PROBIOTIC_SPECIAL",
        "WORLDTREE_SPECIAL",
    ],
    effectsgroups=[
        *BAD_INDUSTRY,
        *AVERAGE_RESEARCH,
        *AVERAGE_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *AVERAGE_POPULATION,
        *AVERAGE_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_SSLITH"),
        *AVERAGE_SUPPLY,
        *BAD_DEFENSE_TROOPS,
        *BAD_OFFENSE_TROOPS,
        # not for description,
        *DEFAULT_NATIVE_DEFENSE,
        NARROW_EP,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=OCEAN_NARROW_EP,
    graphic="icons/species/sslith.png",
)
