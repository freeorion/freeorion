from focs._species import *
from species.common.empire_opinions import COMMON_OPINION_EFFECTS
from species.common.env import BARREN_STANDARD_EP
from species.common.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.common.happiness import BAD_HAPPINESS
from species.common.industry import GREAT_INDUSTRY
from species.common.influence import GOOD_INFLUENCE
from species.common.native_fortification import ADVANCED_NATIVE_DEFENSE
from species.common.population import AVERAGE_POPULATION
from species.common.research import AVERAGE_RESEARCH
from species.common.shields import STANDARD_SHIP_SHIELDS
from species.common.stockpile import AVERAGE_STOCKPILE
from species.common.supply import AVERAGE_SUPPLY
from species.common.troops import BAD_OFFENSE_TROOPS, GOOD_DEFENSE_TROOPS

Species(
    name="SP_KOBUNTURA",
    description="SP_KOBUNTURA_DESC",
    gameplay_description="SP_KOBUNTURA_GAMEPLAY_DESC",
    native=True,
    can_produce_ships=True,
    can_colonize=True,
    tags=[
        "SELF_SUSTAINING",
        "GREAT_INDUSTRY",
        "BAD_HAPPINESS",
        "GOOD_INFLUENCE",
        "BAD_OFFENSE_TROOPS",
        "PEDIA_SELF_SUSTAINING_SPECIES_CLASS",
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
        "MIMETIC_ALLOY_SPECIAL",
        "SHIMMER_SILK_SPECIAL",
        "SUPERCONDUCTOR_SPECIAL",
        "PROBIOTIC_SPECIAL",
        "POSITRONIUM_SPECIAL",
        "GAIA_SPECIAL",
        "RESONANT_MOON_SPECIAL",
        "PLC_BUREAUCRACY",
        "PLC_CONTINUOUS_SCANNING",
        "PLC_DESIGN_SIMPLICITY",
        "PLC_EXPLORATION",
        "PLC_INDUSTRIALISM",
        "PLC_INTERSTELLAR_INFRA",
    ],
    dislikes=[
        "FOCUS_RESEARCH",
        "FOCUS_INFLUENCE",
        "BLD_SHIPYARD_BASE",
        "BLD_SHIPYARD_ORBITAL_DRYDOCK",
        "BLD_SHIPYARD_CON_NANOROBO",
        "BLD_SHIPYARD_CON_GEOINT",
        "BLD_SHIPYARD_CON_ADV_ENGINE",
        "KRAKEN_NEST_SPECIAL",
        "PANOPTICON_SPECIAL",
        "FRUIT_SPECIAL",
        "ELERIUM_SPECIAL",
        "PLC_CENTRALIZATION",
        "PLC_CHECKPOINTS",
        "PLC_ENVIRONMENTALISM",
        "PLC_NO_SUPPLY",
        "PLC_MODERATION",
    ],
    effectsgroups=[
        *GREAT_INDUSTRY,
        *AVERAGE_RESEARCH,
        *GOOD_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *AVERAGE_POPULATION,
        *BAD_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_KOBUNTURA"),
        *AVERAGE_SUPPLY,
        *GOOD_DEFENSE_TROOPS,
        *BAD_OFFENSE_TROOPS,
        # not for description,
        *ADVANCED_NATIVE_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=BARREN_STANDARD_EP,
    graphic="icons/species/intangible-04.png",
)
