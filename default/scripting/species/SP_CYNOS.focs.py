from focs._species import *
from species.common.empire_opinions import COMMON_OPINION_EFFECTS
from species.common.env import BROAD_EP, SWAMP_BROAD_EP
from species.common.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.common.happiness import GOOD_HAPPINESS
from species.common.industry import NO_INDUSTRY
from species.common.influence import AVERAGE_INFLUENCE
from species.common.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.common.population import AVERAGE_POPULATION
from species.common.research import AVERAGE_RESEARCH
from species.common.shields import STANDARD_SHIP_SHIELDS
from species.common.stockpile import AVERAGE_STOCKPILE
from species.common.supply import AVERAGE_SUPPLY
from species.common.troops import GOOD_DEFENSE_TROOPS

Species(
    name="SP_CYNOS",
    description="SP_CYNOS_DESC",
    gameplay_description="SP_CYNOS_GAMEPLAY_DESC",
    native=True,
    can_produce_ships=True,
    tags=[
        "PHOTOTROPHIC",
        "ARTISTIC",
        "NO_INDUSTRY",
        "GOOD_HAPPINESS",
        "AVERAGE_SUPPLY",
        "PEDIA_PHOTOTROPHIC_SPECIES_CLASS",
        "PEDIA_ARTISTIC",
    ],
    foci=[
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
        "HONEYCOMB_SPECIAL",
        "ECCENTRIC_ORBIT_SPECIAL",
        "ANCIENT_RUINS_DEPLETED_SPECIAL",
        "FRUIT_SPECIAL",
        "PLC_DIVINE_AUTHORITY",
        "PLC_COLONIZATION",
        "PLC_ALLIED_REPAIR",
        "PLC_TERRAFORMING",
        "PLC_DESIGN_SIMPLICITY",
    ],
    dislikes=[
        "BLD_INDUSTRY_CENTER",
        "BLD_INTERSPECIES_ACADEMY",
        "BLD_ENCLAVE_VOID",
        "BLD_SOL_ORB_GEN",
        "BLD_PLANET_CLOAK",
        "PHILOSOPHER_SPECIAL",
        "RESONANT_MOON_SPECIAL",
        "TEMPORAL_ANOMALY_SPECIAL",
        "TIDAL_LOCK_SPECIAL",
        "WORLDTREE_SPECIAL",
        "PLC_NO_GROWTH",
        "PLC_EXPLORATION",
        "PLC_NATIVE_APPROPRIATION",
    ],
    effectsgroups=[
        NO_INDUSTRY,
        *AVERAGE_RESEARCH,
        *AVERAGE_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *AVERAGE_POPULATION,
        *GOOD_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_CYNOS"),
        *AVERAGE_SUPPLY,
        *GOOD_DEFENSE_TROOPS,
        # not for description,
        *DEFAULT_NATIVE_DEFENSE,
        BROAD_EP,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=SWAMP_BROAD_EP,
    graphic="icons/species/cynos.png",
)
