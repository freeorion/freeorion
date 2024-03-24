from focs._species import *
from species.species_macros.empire_opinions import COMMON_OPINION_EFFECTS
from species.species_macros.env import SWAMP_STANDARD_EP
from species.species_macros.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.species_macros.happiness import AVERAGE_HAPPINESS
from species.species_macros.industry import GOOD_INDUSTRY
from species.species_macros.influence import BAD_INFLUENCE
from species.species_macros.planet_defense import AVERAGE_PLANETARY_DEFENSE
from species.species_macros.planet_shields import AVERAGE_PLANETARY_SHIELDS
from species.species_macros.planet_size import LARGE_PLANET
from species.species_macros.population import AVERAGE_POPULATION
from species.species_macros.research import GOOD_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stockpile import AVERAGE_STOCKPILE
from species.species_macros.supply import AVERAGE_SUPPLY
from species.species_macros.troops import BAD_DEFENSE_TROOPS, BAD_OFFENSE_TROOPS
from species.species_macros.weapons import BAD_WEAPONS

Species(
    name="SP_GYSACHE",
    description="SP_GYSACHE_DESC",
    gameplay_description="SP_GYSACHE_GAMEPLAY_DESC",
    playable=True,
    can_produce_ships=True,
    can_colonize=True,
    tags=[
        "AVERAGE_POPULATION",
        "ORGANIC",
        "GOOD_INDUSTRY",
        "GOOD_RESEARCH",
        "BAD_INFLUENCE",
        "BAD_WEAPONS",
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
    defaultfocus="FOCUS_INDUSTRY",
    likes=[
        "FOCUS_INDUSTRY",
        "FRACTAL_GEODES_SPECIAL",
        "SUCCULENT_BARNACLES_SPECIAL",
        "GAIA_SPECIAL",
        "HONEYCOMB_SPECIAL",
        "ELERIUM_SPECIAL",
        "CRYSTALS_SPECIAL",
        "PLC_BOOTSTRAPPING",
        "PLC_CHECKPOINTS",
        "PLC_COLONIZATION",
        "PLC_CONFORMANCE",
        "PLC_CONTINUOUS_SCANNING",
        "PLC_INSURGENCY",
    ],
    dislikes=[
        "BLD_SPACE_ELEVATOR",
        "BLD_NEUTRONIUM_SYNTH",
        "BLD_NEUTRONIUM_FORGE",
        "BLD_NEUTRONIUM_EXTRACTOR",
        "BLD_SCRYING_SPHERE",
        "BLD_SHIPYARD_ENRG_COMP",
        "RESONANT_MOON_SPECIAL",
        "TIDAL_LOCK_SPECIAL",
        "WORLDTREE_SPECIAL",
        "MONOPOLE_SPECIAL",
        "PLC_CENTRALIZATION",
        "PLC_EXPLORATION",
        "PLC_ISOLATION",
        "PLC_MARINE_RECRUITMENT",
        "PLC_MARTIAL_LAW",
        "PLC_THE_HUNT",
        "SP_NIGHTSIDERS",
    ],
    effectsgroups=[
        *GOOD_INDUSTRY,
        *GOOD_RESEARCH,
        *BAD_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *AVERAGE_POPULATION,
        *AVERAGE_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_GYSACHE"),
        *AVERAGE_SUPPLY,
        *BAD_DEFENSE_TROOPS,
        *BAD_OFFENSE_TROOPS,
        *BAD_WEAPONS,
        # not for description,
        *AVERAGE_PLANETARY_SHIELDS,
        *AVERAGE_PLANETARY_DEFENSE,
        *LARGE_PLANET,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=SWAMP_STANDARD_EP,
    graphic="icons/species/gysache.png",
)
