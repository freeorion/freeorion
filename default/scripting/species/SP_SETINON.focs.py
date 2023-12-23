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
from species.species_macros.industry import BAD_INDUSTRY
from species.species_macros.influence import AVERAGE_INFLUENCE
from species.species_macros.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.species_macros.population import GOOD_POPULATION
from species.species_macros.research import AVERAGE_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stealth import GREAT_STEALTH
from species.species_macros.stockpile import AVERAGE_STOCKPILE
from species.species_macros.supply import AVERAGE_SUPPLY
from species.species_macros.troops import NO_DEFENSE_TROOPS, NO_OFFENSE_TROOPS

Species(
    name="SP_SETINON",
    description="SP_SETINON_DESC",
    gameplay_description="SP_SETINON_GAMEPLAY_DESC",
    native=True,
    can_produce_ships=True,
    can_colonize=True,
    tags=[
        "ORGANIC",
        "BAD_INDUSTRY",
        "GOOD_POPULATION",
        "AVERAGE_SUPPLY",
        "GREAT_STEALTH",
        "NO_OFFENSE_TROOPS",
        "PEDIA_ORGANIC_SPECIES_CLASS",
    ],
    foci=[
        HAS_INDUSTRY_FOCUS,
        HAS_RESEARCH_FOCUS,
        HAS_INFLUENCE_FOCUS,
        HAS_GROWTH_FOCUS,
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_PROTECTION",
    likes=[
        "FOCUS_PROTECTION",
        "FRACTAL_GEODES_SPECIAL",
        "SUCCULENT_BARNACLES_SPECIAL",
        "WORLDTREE_SPECIAL",
        "FORTRESS_SPECIAL",
        "CRYSTALS_SPECIAL",
        "COMPUTRONIUM_SPECIAL",
        "PLC_MARTIAL_LAW",
        "PLC_CONFEDERATION",
        "PLC_ARTISAN_WORKSHOPS",
    ],
    dislikes=[
        "BLD_SHIPYARD_ENRG_SOLAR",
        "BLD_SCANNING_FACILITY",
        "BLD_INDUSTRY_CENTER",
        "BLD_INTERSPECIES_ACADEMY",
        "BLD_STOCKPILING_CENTER",
        "BLD_MEGALITH",
        "BLD_SPACE_ELEVATOR",
        "SUPERCONDUCTOR_SPECIAL",
        "SPICE_SPECIAL",
        "PROBIOTIC_SPECIAL",
        "POSITRONIUM_SPECIAL",
        "PLC_CENTRALIZATION",
        "PLC_INDUSTRIALISM",
        "PLC_CONFORMANCE",
        "PLC_DIVINE_AUTHORITY",
    ],
    effectsgroups=[
        *BAD_INDUSTRY,
        *AVERAGE_RESEARCH,
        *AVERAGE_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *GOOD_POPULATION,
        *AVERAGE_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_SETINON"),
        *AVERAGE_SUPPLY,
        *NO_DEFENSE_TROOPS,
        *NO_OFFENSE_TROOPS,
        *GREAT_STEALTH,
        # not for description,
        *DEFAULT_NATIVE_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=SWAMP_STANDARD_EP,
    graphic="icons/species/amorphous-02.png",
)
