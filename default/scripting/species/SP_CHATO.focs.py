from focs._species import *
from species.common.empire_opinions import COMMON_OPINION_EFFECTS
from species.common.env import BROAD_EP, TOXIC_BROAD_EP
from species.common.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.common.happiness import AVERAGE_HAPPINESS
from species.common.industry import AVERAGE_INDUSTRY
from species.common.influence import BAD_INFLUENCE
from species.common.planet_defense import AVERAGE_PLANETARY_DEFENSE
from species.common.planet_shields import AVERAGE_PLANETARY_SHIELDS
from species.common.planet_size import LARGE_PLANET
from species.common.population import BAD_POPULATION
from species.common.research import GREAT_RESEARCH
from species.common.shields import GREAT_SHIP_SHIELDS
from species.common.stockpile import AVERAGE_STOCKPILE
from species.common.supply import AVERAGE_SUPPLY
from species.common.troops import BAD_DEFENSE_TROOPS, BAD_OFFENSE_TROOPS
from species.common.weapons import BAD_WEAPONS

Species(
    name="SP_CHATO",
    description="SP_CHATO_DESC",
    gameplay_description="SP_CHATO_GAMEPLAY_DESC",
    playable=True,
    can_produce_ships=True,
    can_colonize=True,
    tags=[
        "PHOTOTROPHIC",
        "GREAT_RESEARCH",
        "BAD_INFLUENCE",
        "BAD_WEAPONS",
        "GREAT_SHIP_SHIELDS",
        "BAD_POPULATION",
        "AVERAGE_SUPPLY",
        "BAD_OFFENSE_TROOPS",
        "PEDIA_PHOTOTROPHIC_SPECIES_CLASS",
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
        "SHIMMER_SILK_SPECIAL",
        "FRUIT_SPECIAL",
        "MONOPOLE_SPECIAL",
        "GAIA_SPECIAL",
        "ELERIUM_SPECIAL",
        "SPICE_SPECIAL",
        "PLC_CONFORMANCE",
        "PLC_DIVINE_AUTHORITY",
        "PLC_DREAM_RECURSION",
        "PLC_MODERATION",
    ],
    dislikes=[
        "BLD_PLANET_CLOAK",
        "BLD_SCANNING_FACILITY",
        "BLD_INTERSPECIES_ACADEMY",
        "BLD_SHIPYARD_CON_ADV_ENGINE",
        "BLD_NEUTRONIUM_FORGE",
        "BLD_STARGATE",
        "ECCENTRIC_ORBIT_SPECIAL",
        "KRAKEN_NEST_SPECIAL",
        "PANOPTICON_SPECIAL",
        "PLC_LIBERTY",
        "PLC_STOCKPILE_LIQUIDATION",
        "PLC_ALGORITHMIC_RESEARCH",
        "PLC_ARTISAN_WORKSHOPS",
    ],
    effectsgroups=[
        *AVERAGE_INDUSTRY,
        *GREAT_RESEARCH,
        *BAD_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *BAD_POPULATION,
        *AVERAGE_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_CHATO"),
        *AVERAGE_SUPPLY,
        *BAD_DEFENSE_TROOPS,
        *BAD_OFFENSE_TROOPS,
        *BAD_WEAPONS,
        # not for description
        *AVERAGE_PLANETARY_SHIELDS,
        *AVERAGE_PLANETARY_DEFENSE,
        LARGE_PLANET,
        BROAD_EP,
        *GREAT_SHIP_SHIELDS,
    ],
    environments=TOXIC_BROAD_EP,
    graphic="icons/species/chato-matou-gormoshk.png",
)
