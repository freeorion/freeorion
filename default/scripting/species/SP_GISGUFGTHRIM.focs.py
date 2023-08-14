from focs._species import *
from species.common.empire_opinions import COMMON_OPINION_EFFECTS
from species.common.env import TOXIC_STANDARD_EP
from species.common.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.common.happiness import GOOD_HAPPINESS
from species.common.industry import BAD_INDUSTRY
from species.common.influence import BAD_INFLUENCE
from species.common.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.common.population import AVERAGE_POPULATION
from species.common.research import GOOD_RESEARCH
from species.common.shields import STANDARD_SHIP_SHIELDS
from species.common.stockpile import AVERAGE_STOCKPILE
from species.common.supply import AVERAGE_SUPPLY
from species.common.troops import BAD_DEFENSE_TROOPS, BAD_OFFENSE_TROOPS

Species(
    name="SP_GISGUFGTHRIM",
    description="SP_GISGUFGTHRIM_DESC",
    gameplay_description="SP_GISGUFGTHRIM_GAMEPLAY_DESC",
    native=True,
    can_produce_ships=True,
    tags=[
        "ORGANIC",
        "BAD_INDUSTRY",
        "GOOD_RESEARCH",
        "BAD_INFLUENCE",
        "GOOD_HAPPINESS",
        "AVERAGE_SUPPLY",
        "BAD_OFFENSE_TROOPS",
        "PEDIA_ORGANIC_SPECIES_CLASS",
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
        "FRACTAL_GEODES_SPECIAL",
        "SPARK_FOSSILS_SPECIAL",
        "SUCCULENT_BARNACLES_SPECIAL",
        "MONOPOLE_SPECIAL",
        "ELERIUM_SPECIAL",
        "PROBIOTIC_SPECIAL",
        "TEMPORAL_ANOMALY_SPECIAL",
        "PLC_CAPITAL_MARKETS",
        "PLC_ENVIRONMENTALISM",
        "PLC_TECHNOCRACY",
    ],
    dislikes=[
        "BLD_ENCLAVE_VOID",
        "BLD_PLANET_DRIVE",
        "BLD_PLANET_BEACON",
        "BLD_PLANET_CLOAK",
        "BLD_SPATIAL_DISTORT_GEN",
        "BLD_STARGATE",
        "BLD_HYPER_DAM",
        "KRAKEN_IN_THE_ICE_SPECIAL",
        "RESONANT_MOON_SPECIAL",
        "GAIA_SPECIAL",
        "HONEYCOMB_SPECIAL",
        "PLC_INDUSTRIALISM",
        "PLC_NO_SUPPLY",
    ],
    effectsgroups=[
        *BAD_INDUSTRY,
        *GOOD_RESEARCH,
        *BAD_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *AVERAGE_POPULATION,
        *GOOD_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_GISGUFGTHRIM"),
        *AVERAGE_SUPPLY,
        *BAD_DEFENSE_TROOPS,
        *BAD_OFFENSE_TROOPS,
        # not for description,
        *DEFAULT_NATIVE_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=TOXIC_STANDARD_EP,
    graphic="icons/species/gis-guf-gthrim.png",
)
