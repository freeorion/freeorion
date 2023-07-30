from focs._species import *
from species.common.empire_opinions import COMMON_OPINION_EFFECTS
from species.common.env import BARREN_BROAD_EP, BROAD_EP
from species.common.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.common.happiness import GREAT_HAPPINESS
from species.common.industry import BAD_INDUSTRY
from species.common.influence import GOOD_INFLUENCE
from species.common.planet_defense import AVERAGE_PLANETARY_DEFENSE
from species.common.planet_shields import AVERAGE_PLANETARY_SHIELDS
from species.common.planet_size import LARGE_PLANET
from species.common.population import AVERAGE_POPULATION
from species.common.research import GOOD_RESEARCH
from species.common.shields import STANDARD_SHIP_SHIELDS
from species.common.stockpile import AVERAGE_STOCKPILE
from species.common.supply import AVERAGE_SUPPLY
from species.common.troops import GOOD_DEFENSE_TROOPS

Species(
    name="SP_CRAY",
    description="SP_CRAY_DESC",
    gameplay_description="SP_CRAY_GAMEPLAY_DESC",
    playable=True,
    can_produce_ships=True,
    can_colonize=True,
    tags=[
        "ROBOTIC",
        "ARTISTIC",
        "BAD_INDUSTRY",
        "GOOD_RESEARCH",
        "GOOD_INFLUENCE",
        "GREAT_HAPPINESS",
        "AVERAGE_SUPPLY",
        "PEDIA_ROBOTIC_SPECIES_CLASS",
        "PEDIA_ARTISTIC",
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
        "SUCCULENT_BARNACLES_SPECIAL",
        "SHIMMER_SILK_SPECIAL",
        "TEMPORAL_ANOMALY_SPECIAL",
        "MONOPOLE_SPECIAL",
        "CRYSTALS_SPECIAL",
        "FORTRESS_SPECIAL",
        "PLC_ALGORITHMIC_RESEARCH",
        "PLC_DIVINE_AUTHORITY",
        "PLC_DIVERSITY",
        "PLC_CONFEDERATION",
        "PLC_ARTISAN_WORKSHOPS",
        "PLC_ALLIED_REPAIR",
    ],
    dislikes=[
        "BLD_HYPER_DAM",
        "BLD_GAS_GIANT_GEN",
        "BLD_SCANNING_FACILITY",
        "BLD_SOL_ORB_GEN",
        "BLD_INDUSTRY_CENTER",
        "KRAKEN_NEST_SPECIAL",
        "WORLDTREE_SPECIAL",
        "HONEYCOMB_SPECIAL",
        "PLC_CONFORMANCE",
        "PLC_DESIGN_SIMPLICITY",
        "PLC_CENTRALIZATION",
        "PLC_TERROR_SUPPRESSION",
    ],
    effectsgroups=[
        *BAD_INDUSTRY,
        *GOOD_RESEARCH,
        *GOOD_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *AVERAGE_POPULATION,
        *GREAT_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_CRAY"),
        *AVERAGE_SUPPLY,
        *GOOD_DEFENSE_TROOPS,
        # not for description,
        *AVERAGE_PLANETARY_SHIELDS,
        *AVERAGE_PLANETARY_DEFENSE,
        LARGE_PLANET,
        BROAD_EP,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=BARREN_BROAD_EP,
    graphic="icons/species/cray.png",
)
