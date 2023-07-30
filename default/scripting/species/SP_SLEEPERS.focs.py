from focs._effects import AddSpecial, EffectsGroup, IsSource, NoStar, Star, Turn
from focs._species import *
from species.common.empire_opinions import COMMON_OPINION_EFFECTS
from species.common.env import NARROW_EP, TUNDRA_NARROW_EP
from species.common.focus import (
    HAS_ADVANCED_FOCI,
    HAS_INDUSTRY_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.common.happiness import AVERAGE_HAPPINESS
from species.common.industry import GOOD_INDUSTRY
from species.common.influence import NO_INFLUENCE
from species.common.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.common.population import BAD_POPULATION
from species.common.research import VERY_BAD_RESEARCH
from species.common.shields import STANDARD_SHIP_SHIELDS
from species.common.stockpile import GOOD_STOCKPILE
from species.common.supply import AVERAGE_SUPPLY
from species.common.troops import BAD_DEFENSE_TROOPS, BAD_OFFENSE_TROOPS

Species(
    name="SP_SLEEPERS",
    description="SP_SLEEPERS_DESC",
    gameplay_description="SP_SLEEPERS_GAMEPLAY_DESC",
    native=True,
    tags=[
        "ORGANIC",
        "BAD_POPULATION",
        "VERY_BAD_RESEARCH",
        "NO_INFLUENCE",
        "BAD_OFFENSE_TROOPS",
        "GOOD_INDUSTRY",
        "GOOD_STOCKPILE",
        "PEDIA_ORGANIC_SPECIES_CLASS",
    ],
    foci=[
        HAS_INDUSTRY_FOCUS,
        HAS_RESEARCH_FOCUS,
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_INDUSTRY",
    likes=[  # not much really, because they are boring,
        "SHIMMER_SILK_SPECIAL",  # nice and comfy for the long sleep and if u have to get out to write the name in the snow u can c where u lay when u get back cuz is shimmering
        "BLD_SOL_ORB_GEN",  # bright and shiny and we likes the industrial benefits
        "PLC_DREAM_RECURSION",  # dreaming sounds nice. good night. sweet dream recursions.
        "PLC_INDUSTRIALISM",  # automation. also nice. just dont expect us to work ourselves..
        "FOCUS_RESEARCH",  # i.e. dreaming
    ],
    dislikes=[  # not much really, because that would make them interesting,
        "SP_NIGHTSIDERS",  # who doesnt?
        "FOCUS_INDUSTRY",  # just cuz we're good at it dont meen we like it. go away. let us sleep. we're not worth u'r trouble anyway.
        "MIMETIC_ALLOY_SPECIAL",  # too obtrusive
    ],
    # hibernate. dreaming. somnambulism. automation.
    effectsgroups=[
        *GOOD_INDUSTRY,
        *VERY_BAD_RESEARCH,  # because they like to sleep a lot and when not then sit around and do nothing on their eccentric worlds ..
        *NO_INFLUENCE,
        *GOOD_STOCKPILE,
        *BAD_POPULATION,
        *AVERAGE_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_SLEEPERS"),
        *AVERAGE_SUPPLY,
        *BAD_OFFENSE_TROOPS,
        *BAD_DEFENSE_TROOPS,
        # not for description,
        *DEFAULT_NATIVE_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
        NARROW_EP,
        # they live on eccentric worlds,
        EffectsGroup(
            scope=IsSource,
            activation=Turn(high=0) & ~Star(type=[NoStar]),
            effects=[AddSpecial(name="ECCENTRIC_ORBIT_SPECIAL")],
        ),
    ],
    environments=TUNDRA_NARROW_EP,
    graphic="icons/species/quadruped-05.png",
)
