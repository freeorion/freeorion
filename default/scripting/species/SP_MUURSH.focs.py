from focs._species import *
from species.common.empire_opinions import COMMON_OPINION_EFFECTS
from species.common.env import DESERT_STANDARD_EP
from species.common.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.common.happiness import AVERAGE_HAPPINESS
from species.common.industry import AVERAGE_INDUSTRY
from species.common.influence import GOOD_INFLUENCE
from species.common.native_fortification import ADVANCED_NATIVE_DEFENSE
from species.common.population import AVERAGE_POPULATION
from species.common.research import BAD_RESEARCH
from species.common.shields import STANDARD_SHIP_SHIELDS
from species.common.stockpile import AVERAGE_STOCKPILE
from species.common.supply import BAD_SUPPLY
from species.common.troops import GREAT_DEFENSE_TROOPS
from species.common.weapons import GREAT_WEAPONS

Species(
    name="SP_MUURSH",
    description="SP_MUURSH_DESC",
    gameplay_description="SP_MUURSH_GAMEPLAY_DESC",
    native=True,
    can_produce_ships=True,
    can_colonize=True,
    tags=["ORGANIC", "BAD_RESEARCH", "GOOD_INFLUENCE", "GREAT_WEAPONS", "BAD_SUPPLY", "PEDIA_ORGANIC_SPECIES_CLASS"],
    foci=[
        HAS_INDUSTRY_FOCUS,
        HAS_RESEARCH_FOCUS,
        HAS_INFLUENCE_FOCUS,
        HAS_GROWTH_FOCUS,
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_INDUSTRY",
    likes=[
        "FOCUS_INFLUENCE",
        "SUCCULENT_BARNACLES_SPECIAL",
        "MIMETIC_ALLOY_SPECIAL",
        "MINERALS_SPECIAL",
        "FRUIT_SPECIAL",
        "PLC_DIVERSITY",
        "PLC_TERROR_SUPPRESSION",
    ],
    dislikes=[
        "PLC_CONFEDERATION",
        "PLC_CONTINUOUS_SCANNING",
        "PLC_BLACK_MARKET",
        "PLC_RACIAL_PURITY",
        "PLC_THE_HUNT",
        "BLD_SHIPYARD_ORBITAL_DRYDOCK",
        "BLD_SHIPYARD_ORG_ORB_INC",
        "BLD_SHIPYARD_AST",
        "BLD_SHIPYARD_CON_NANOROBO",
        "BLD_SHIPYARD_CON_ADV_ENGINE",
        "BLD_SHIPYARD_CON_GEOINT",
        "BLD_SHIPYARD_AST_REF",
        "BLD_SHIPYARD_ORG_XENO_FAC",
        "BLD_SHIPYARD_ENRG_COMP",
        "BLD_SHIPYARD_ENRG_SOLAR",
        "BLD_PLANET_CLOAK",
        "BLD_SPATIAL_DISTORT_GEN",
        "BLD_STARGATE",
        "BLD_MILITARY_COMMAND",
        "BLD_CONC_CAMP",
        "SP_NIGHTSIDERS",
    ],
    effectsgroups=[
        *AVERAGE_INDUSTRY,
        *BAD_RESEARCH,
        *GOOD_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *AVERAGE_POPULATION,
        *AVERAGE_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_MUURSH"),
        *BAD_SUPPLY,
        *GREAT_DEFENSE_TROOPS,
        *GREAT_WEAPONS,
        *ADVANCED_NATIVE_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=DESERT_STANDARD_EP,
    graphic="icons/species/muursh.png",
)
