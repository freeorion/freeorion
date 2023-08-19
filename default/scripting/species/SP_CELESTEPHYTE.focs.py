from focs._species import *
from species.common.empire_opinions import COMMON_OPINION_EFFECTS
from species.common.env import ASTEROIDAL_NARROW_EP, NARROW_EP
from species.common.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.common.general import FAST_COLONIZATION
from species.common.happiness import BAD_HAPPINESS
from species.common.industry import VERY_BAD_INDUSTRY
from species.common.influence import AVERAGE_INFLUENCE
from species.common.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.common.population import VERY_BAD_POPULATION
from species.common.research import AVERAGE_RESEARCH
from species.common.shields import STANDARD_SHIP_SHIELDS
from species.common.stockpile import AVERAGE_STOCKPILE
from species.common.supply import BAD_SUPPLY
from species.common.troops import BAD_DEFENSE_TROOPS, NO_OFFENSE_TROOPS

Species(
    name="SP_CELESTEPHYTE",
    description="SP_CELESTEPHYTE_DESC",
    gameplay_description="SP_CELESTEPHYTE_GAMEPLAY_DESC",
    native=True,
    tags=[
        "PHOTOTROPHIC",
        "TELEPATHIC",
        "VERY_BAD_POPULATION",
        "BAD_HAPPINESS",
        "BAD_SUPPLY",
        "VERY_BAD_INDUSTRY",
        "NO_OFFENSE_TROOPS",
        "PEDIA_PHOTOTROPHIC_SPECIES_CLASS",
        "PEDIA_TELEPATHIC_TITLE",
    ],
    foci=[
        HAS_INDUSTRY_FOCUS,
        HAS_RESEARCH_FOCUS,
        HAS_INFLUENCE_FOCUS,
        HAS_GROWTH_FOCUS,
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_GROWTH",
    likes=[
        "BLD_LIGHTHOUSE",  # if there were a greenhouse they would like that too
        "FOCUS_GROWTH",  # what else would a plant want?
        "PLC_POPULATION",  # grow, grow, grow your load, gently down the ..
    ],
    dislikes=[  # dislikes anything disturbing their fragile habitat or irritating their telepathic minds
        "PLC_NO_GROWTH",  # is a plant, they like to grow
        "BLD_SHIPYARD_AST",  # disturbs the rocks
        "BLD_SHIPYARD_AST_REF",  # same
        "JUGGERNAUT_NEST_SPECIAL",  # compete for rocks
        "ECCENTRIC_ORBIT_SPECIAL",  # messes up all those orbital resonances, quite inharmonious
        "FLD_METEOR_BLIZZARD",  # same
        "FLD_VOID_RIFT",  # same
    ],
    effectsgroups=[
        *VERY_BAD_INDUSTRY,
        *AVERAGE_RESEARCH,
        *AVERAGE_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *VERY_BAD_POPULATION,
        *BAD_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_CELESTEPHYTE"),
        *BAD_SUPPLY,
        *NO_OFFENSE_TROOPS,
        *BAD_DEFENSE_TROOPS,
        FAST_COLONIZATION,
        # not for description
        *DEFAULT_NATIVE_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
        NARROW_EP,
    ],
    environments=ASTEROIDAL_NARROW_EP,
    graphic="icons/species/celestephyte.png",
)
