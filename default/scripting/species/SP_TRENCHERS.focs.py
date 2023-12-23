from focs._species import *
from species.species_macros.empire_opinions import COMMON_OPINION_EFFECTS
from species.species_macros.env import BARREN_STANDARD_EP
from species.species_macros.focus import (
    HAS_ADVANCED_FOCI,
    HAS_INDUSTRY_FOCUS,
)
from species.species_macros.happiness import AVERAGE_HAPPINESS
from species.species_macros.industry import GOOD_INDUSTRY
from species.species_macros.influence import NO_INFLUENCE
from species.species_macros.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.species_macros.population import AVERAGE_POPULATION
from species.species_macros.research import NO_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stockpile import AVERAGE_STOCKPILE
from species.species_macros.supply import BAD_SUPPLY
from species.species_macros.troops import BAD_DEFENSE_TROOPS

Species(
    name="SP_TRENCHERS",
    description="SP_TRENCHERS_DESC",
    gameplay_description="SP_TRENCHERS_GAMEPLAY_DESC",
    native=True,
    tags=["ROBOTIC", "GOOD_INDUSTRY", "NO_RESEARCH", "NO_INFLUENCE", "BAD_SUPPLY", "PEDIA_ROBOTIC_SPECIES_CLASS"],
    foci=[
        HAS_INDUSTRY_FOCUS,
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_INDUSTRY",
    likes=[
        "BLD_LIGHTHOUSE",
        "BLD_SCANNING_FACILITY",
        "BLD_SCRYING_SPHERE",
        "BLD_SPATIAL_DISTORT_GEN",
        "BLD_STARGATE",
        "FOCUS_PROTECTION",
        "FRACTAL_GEODES_SPECIAL",
        "KRAKEN_NEST_SPECIAL",
        "PANOPTICON_SPECIAL",
        "PHILOSOPHER_SPECIAL",
        "PLC_ALGORITHMIC_RESEARCH",
        "PLC_ARTISAN_WORKSHOPS",
        "PLC_MODERATION",
        "PLC_TERRAFORMING",
        "PLC_TRAFFIC_CONTROL",
        "SPARK_FOSSILS_SPECIAL",
        "WORLDTREE_SPECIAL",
    ],
    dislikes=[
        "BLD_AUTO_HISTORY_ANALYSER",
        "BLD_CULTURE_LIBRARY",
        "BLD_INDUSTRY_CENTER",
        "BLD_MEGALITH",
        "BLD_PLANET_BEACON",
        "BLD_REGIONAL_ADMIN",
        "BLD_STOCKPILING_CENTER",
        "COMPUTRONIUM_SPECIAL",
        "KRAKEN_IN_THE_ICE_SPECIAL",
        "PLC_CONFORMANCE",
        "PLC_CONTINUOUS_SCANNING",
        "PLC_DESIGN_SIMPLICITY",
        "PLC_ENVIRONMENTALISM",
        "PLC_NATIVE_APPROPRIATION",
        "SPICE_SPECIAL",
    ],
    effectsgroups=[
        *GOOD_INDUSTRY,
        NO_RESEARCH,
        *NO_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *AVERAGE_POPULATION,
        *AVERAGE_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_TRENCHERS"),
        *BAD_SUPPLY,
        *BAD_DEFENSE_TROOPS,
        # not for description,
        *DEFAULT_NATIVE_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=BARREN_STANDARD_EP,
    graphic="icons/species/robotic-02.png",
)
