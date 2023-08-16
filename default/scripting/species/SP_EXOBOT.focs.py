from focs._effects import (
    Adequate,
    AsteroidsType,
    Barren,
    Desert,
    GasGiantType,
    Inferno,
    Ocean,
    Poor,
    Radiated,
    Swamp,
    Terran,
    Toxic,
    Tundra,
    Uninhabitable,
)
from focs._species import *
from species.common.empire_opinions import FIXED_OPINION_EFFECTS
from species.common.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.common.happiness import GREAT_HAPPINESS
from species.common.industry import VERY_BAD_INDUSTRY
from species.common.influence import NO_INFLUENCE
from species.common.planet_defense import AVERAGE_PLANETARY_DEFENSE
from species.common.planet_shields import AVERAGE_PLANETARY_SHIELDS
from species.common.population import BAD_POPULATION
from species.common.research import VERY_BAD_RESEARCH
from species.common.shields import BAD_SHIP_SHIELDS
from species.common.stockpile import NO_STOCKPILE
from species.common.supply import BAD_SUPPLY
from species.common.troops import BAD_DEFENSE_TROOPS, BAD_OFFENSE_TROOPS
from species.common.weapons import BAD_WEAPONS

Species(
    name="SP_EXOBOT",
    description="SP_EXOBOT_DESC",
    gameplay_description="SP_EXOBOT_GAMEPLAY_DESC",
    can_produce_ships=True,
    can_colonize=True,
    tags=[
        "ROBOTIC",
        "LOW_BRAINPOWER",
        "VERY_BAD_INDUSTRY",
        "VERY_BAD_RESEARCH",
        "NO_INFLUENCE",
        "BAD_POPULATION",
        "BAD_WEAPONS",
        "BAD_SHIP_SHIELDS",
        "GREAT_HAPPINESS",
        "BAD_SUPPLY",
        "CTRL_ALWAYS_REPORT",
        "BAD_OFFENSE_TROOPS",
        "PEDIA_ROBOTIC_SPECIES_CLASS",
        "NO_TERRAFORM",
    ],
    foci=[
        HAS_INDUSTRY_FOCUS,
        HAS_RESEARCH_FOCUS,
        # [[HAS_INFLUENCE_FOCUS]]
        HAS_GROWTH_FOCUS,
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_INDUSTRY",
    likes=[
        "PLC_ALLIED_REPAIR",
        "PLC_BUREAUCRACY",
        "PLC_DESIGN_SIMPLICITY",
        "PLC_ENGINEERING",
        "PLC_EXOBOT_PRODUCTIVITY",
    ],
    dislikes=[
        "BLD_BLACK_HOLE_POW_GEN",
        "BLD_INDUSTRY_CENTER",
        "BLD_LIGHTHOUSE",
        "BLD_SCANNING_FACILITY",
        "BLD_SHIPYARD_ENRG_COMP",
        "BLD_SOL_ORB_GEN",
    ],
    effectsgroups=[
        *VERY_BAD_INDUSTRY,
        *VERY_BAD_RESEARCH,
        *NO_INFLUENCE,
        *NO_STOCKPILE,
        *BAD_POPULATION,
        *GREAT_HAPPINESS,
        FIXED_OPINION_EFFECTS("SP_EXOBOT", 20),
        *BAD_SUPPLY,
        *BAD_DEFENSE_TROOPS,
        *BAD_OFFENSE_TROOPS,
        *BAD_WEAPONS,
        # not for description,
        *AVERAGE_PLANETARY_SHIELDS,
        *AVERAGE_PLANETARY_DEFENSE,
        *BAD_SHIP_SHIELDS,
    ],
    environments={
        Swamp: Adequate,
        Toxic: Adequate,
        Inferno: Adequate,
        Radiated: Adequate,
        Barren: Adequate,
        Tundra: Adequate,
        Desert: Adequate,
        Terran: Adequate,
        Ocean: Adequate,
        AsteroidsType: Poor,
        GasGiantType: Uninhabitable,
    },
    graphic="icons/species/robotic-01.png",
)
