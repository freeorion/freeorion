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
from species.common.happiness import BAD_HAPPINESS
from species.common.industry import AVERAGE_INDUSTRY
from species.common.influence import GREAT_INFLUENCE
from species.common.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.common.population import AVERAGE_POPULATION
from species.common.research import AVERAGE_RESEARCH
from species.common.shields import STANDARD_SHIP_SHIELDS
from species.common.stockpile import AVERAGE_STOCKPILE
from species.common.supply import AVERAGE_SUPPLY
from species.common.troops import AVERAGE_DEFENSE_TROOPS, NO_OFFENSE_TROOPS
from species.common.weapons import BAD_WEAPONS

Species(
    name="SP_DERTHREAN",
    description="SP_DERTHREAN_DESC",
    gameplay_description="SP_DERTHREAN_GAMEPLAY_DESC",
    native=True,
    can_produce_ships=True,
    can_colonize=True,
    tags=[
        "ORGANIC",
        "TELEPATHIC",
        "GREAT_INFLUENCE",
        "NO_OFFENSE_TROOPS",
        "BAD_WEAPONS",
        "BAD_HAPPINESS",
        "AVERAGE_SUPPLY",
        "PEDIA_ORGANIC_SPECIES_CLASS",
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
        "FOCUS_RESEARCH",
        "MIMETIC_ALLOY_SPECIAL",
        "SPARK_FOSSILS_SPECIAL",
        "GAIA_SPECIAL",
        "PHILOSOPHER_SPECIAL",
        "RESONANT_MOON_SPECIAL",
        "COMPUTRONIUM_SPECIAL",
        "PLC_DIVINE_AUTHORITY",
        "PLC_DIVERSITY",
        "PLC_CONFEDERATION",
        "PLC_MODERATION",
        "PLC_INDOCTRINATION",
        "PLC_INDUSTRIALISM",
    ],
    dislikes=[
        "BLD_SHIPYARD_ORG_CELL_GRO_CHAMB",
        "BLD_SHIPYARD_ENRG_COMP",
        "BLD_NEUTRONIUM_SYNTH",
        "BLD_NEUTRONIUM_FORGE",
        "BLD_NEUTRONIUM_EXTRACTOR",
        "MINERALS_SPECIAL",
        "CRYSTALS_SPECIAL",
        "ANCIENT_RUINS_DEPLETED_SPECIAL",
        "ECCENTRIC_ORBIT_SPECIAL",
        "FORTRESS_SPECIAL",
        "PLC_CONFORMANCE",
        "PLC_CENTRALIZATION",
        "PLC_TERROR_SUPPRESSION",
        "PLC_NO_GROWTH",
        "SP_NIGHTSIDERS",
    ],
    effectsgroups=[
        *AVERAGE_INDUSTRY,
        *AVERAGE_RESEARCH,
        *GREAT_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *AVERAGE_POPULATION,
        *BAD_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_DERTHREAN"),
        *AVERAGE_SUPPLY,
        *AVERAGE_DEFENSE_TROOPS,
        *NO_OFFENSE_TROOPS,
        *BAD_WEAPONS,
        # not for description,
        *DEFAULT_NATIVE_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=TOXIC_STANDARD_EP,
    graphic="icons/species/derthrean.png",
)
