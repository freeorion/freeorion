from species.common.detection import ULTIMATE_DETECTION
from species.common.env import BARREN_STANDARD_EP
from species.common.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.common.happiness import GOOD_HAPPINESS
from species.common.industry import GOOD_INDUSTRY
from species.common.influence import AVERAGE_INFLUENCE
from species.common.planet_defense import AVERAGE_PLANETARY_DEFENSE
from species.common.planet_shields import AVERAGE_PLANETARY_SHIELDS
from species.common.population import AVERAGE_POPULATION, LIGHT_SENSITIVE
from species.common.research import AVERAGE_RESEARCH
from species.common.shields import STANDARD_SHIP_SHIELDS
from species.common.stealth import GOOD_STEALTH
from species.common.stockpile import AVERAGE_STOCKPILE
from species.common.supply import AVERAGE_SUPPLY
from species.common.troops import AVERAGE_DEFENSE_TROOPS

Species(
    name="SP_BANFORO",
    description="SP_BANFORO_DESC",
    gameplay_description="SP_BANFORO_GAMEPLAY_DESC",
    can_produce_ships=True,
    can_colonize=True,
    tags=[
        "LITHIC",
        "LIGHT_SENSITIVE",
        "GOOD_INDUSTRY",
        "GOOD_HAPPINESS",
        "AVERAGE_SUPPLY",
        "ULTIMATE_DETECTION",
        "GOOD_STEALTH",
        "CTRL_EXTINCT",
        "PEDIA_LITHIC_SPECIES_CLASS",
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
        "SHIMMER_SILK_SPECIAL",
        "FRACTAL_GEODES_SPECIAL",
        "TIDAL_LOCK_SPECIAL",
        "WORLDTREE_SPECIAL",
        "MONOPOLE_SPECIAL",
        "SUPERCONDUCTOR_SPECIAL",
        "ELERIUM_SPECIAL",
        "PLC_CONTINUOUS_SCANNING",
        "PLC_CONFORMANCE",
        "PLC_ENVIRONMENTALISM",
        "PLC_ALLIED_REPAIR",
        "PLC_INTERSTELLAR_INFRA",
    ],
    dislikes=[
        "BLD_REGIONAL_ADMIN",
        "BLD_GENOME_BANK",
        "BLD_NEUTRONIUM_EXTRACTOR",
        "BLD_SPATIAL_DISTORT_GEN",
        "ECCENTRIC_ORBIT_SPECIAL",
        "KRAKEN_IN_THE_ICE_SPECIAL",
        "KRAKEN_NEST_SPECIAL",
        "RESONANT_MOON_SPECIAL",
        "PLC_CAPITAL_MARKETS" "PLC_DIVINE_AUTHORITY",
        "PLC_INDUSTRIALISM",
        "PLC_MARTIAL_LAW",
        "PLC_TERROR_SUPPRESSION",
    ],
    effectsgroups=[
        *GOOD_INDUSTRY,
        *AVERAGE_RESEARCH,
        *AVERAGE_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *AVERAGE_POPULATION,
        *GOOD_HAPPINESS,
        *AVERAGE_SUPPLY,
        *AVERAGE_DEFENSE_TROOPS,
        ULTIMATE_DETECTION,
        GOOD_STEALTH,
        *LIGHT_SENSITIVE,
        # not for description
        *AVERAGE_PLANETARY_SHIELDS,
        *AVERAGE_PLANETARY_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
    ],
    **BARREN_STANDARD_EP,
    graphic="icons/species/banforo.png",
)
