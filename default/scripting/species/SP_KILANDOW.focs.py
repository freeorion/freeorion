from focs._effects import (
    AsteroidsType,
    ContainedBy,
    Contains,
    EffectsGroup,
    Focus,
    Happiness,
    IsSource,
    NamedReal,
    OwnedBy,
    Planet,
    SetTargetIndustry,
    Source,
    System,
    Target,
    Value,
)
from focs._species import *
from macros.base_prod import INDUSTRY_PER_POP
from species.species_macros.empire_opinions import COMMON_OPINION_EFFECTS
from species.species_macros.env import RADIATED_STANDARD_EP
from species.species_macros.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.species_macros.happiness import AVERAGE_HAPPINESS
from species.species_macros.industry import AVERAGE_INDUSTRY
from species.species_macros.influence import VERY_BAD_INFLUENCE
from species.species_macros.planet_defense import AVERAGE_PLANETARY_DEFENSE
from species.species_macros.planet_shields import AVERAGE_PLANETARY_SHIELDS
from species.species_macros.population import AVERAGE_POPULATION
from species.species_macros.research import GREAT_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stealth import BAD_STEALTH
from species.species_macros.stockpile import AVERAGE_STOCKPILE
from species.species_macros.supply import AVERAGE_SUPPLY
from species.species_macros.troops import AVERAGE_DEFENSE_TROOPS

Species(
    name="SP_KILANDOW",
    description="SP_KILANDOW_DESC",
    gameplay_description="SP_KILANDOW_GAMEPLAY_DESC",
    can_produce_ships=True,
    can_colonize=True,
    tags=[
        "PHOTOTROPHIC",
        "GREAT_RESEARCH",
        "VERY_BAD_INFLUENCE",
        "BAD_STEALTH",
        "CTRL_EXTINCT",
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
        "MIMETIC_ALLOY_SPECIAL",
        "CRYSTALS_SPECIAL",
        "ECCENTRIC_ORBIT_SPECIAL",
        "PHILOSOPHER_SPECIAL",
        "TIDAL_LOCK_SPECIAL",
        "PLC_EXPLORATION",
        "PLC_LIBERTY",
        "PLC_NATIVE_APPROPRIATION",
        "PLC_TECHNOCRACY",
    ],
    dislikes=[
        # "BLD_CLONING_CENTER"       // disabled content,
        "BLD_SCANNING_FACILITY",
        "BLD_SHIPYARD_CON_GEOINT",
        "BLD_SHIPYARD_CON_NANOROBO",
        "BLD_SHIPYARD_ORG_ORB_INC",
        "BLD_SPACE_ELEVATOR",
        "ELERIUM_SPECIAL",
        "MONOPOLE_SPECIAL",
        "PLC_BUREAUCRACY",
        "PLC_CHECKPOINTS",
        "PLC_DIVINE_AUTHORITY",
        "PLC_MARTIAL_LAW",
        "RESONANT_MOON_SPECIAL",
        "WORLDTREE_SPECIAL",
    ],
    effectsgroups=[
        *AVERAGE_INDUSTRY,
        *GREAT_RESEARCH,
        *VERY_BAD_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *AVERAGE_POPULATION,
        *AVERAGE_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_KILANDOW"),
        *AVERAGE_SUPPLY,
        *AVERAGE_DEFENSE_TROOPS,
        *BAD_STEALTH,
        EffectsGroup(
            description="GREAT_ASTEROID_INDUSTRY_DESC",
            scope=IsSource
            & Focus(type=["FOCUS_INDUSTRY"])
            & Happiness(low=0)
            & ContainedBy(System & Contains(Planet() & Planet(type=[AsteroidsType]) & OwnedBy(empire=Source.Owner))),
            effects=SetTargetIndustry(
                value=Value
                + Target.Population
                * NamedReal(name="GREAT_ASTEROID_INDUSTRY_TARGET_INDUSTRY_PERPOP", value=1.0 * INDUSTRY_PER_POP)
            ),
        ),
        # not for description,
        *AVERAGE_PLANETARY_SHIELDS,
        *AVERAGE_PLANETARY_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=RADIATED_STANDARD_EP,
    graphic="icons/species/kilandow.png",
)
