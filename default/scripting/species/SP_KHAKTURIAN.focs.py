from focs._species import *
from species.species_macros.empire_opinions import COMMON_OPINION_EFFECTS
from species.species_macros.env import DESERT_NARROW_EP, NARROW_EP
from species.species_macros.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
)
from species.species_macros.happiness import AVERAGE_HAPPINESS
from species.species_macros.industry import BAD_INDUSTRY
from species.species_macros.influence import NO_INFLUENCE
from species.species_macros.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.species_macros.population import BAD_POPULATION
from species.species_macros.research import NO_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stockpile import GOOD_STOCKPILE
from species.species_macros.supply import AVERAGE_SUPPLY
from species.species_macros.troops import GREAT_DEFENSE_TROOPS, GREAT_OFFENSE_TROOPS

Species(
    name="SP_KHAKTURIAN",
    description="SP_KHAKTURIAN_DESC",
    gameplay_description="SP_KHAKTURIAN_GAMEPLAY_DESC",
    native=True,
    can_produce_ships=True,
    tags=[
        "PHOTOTROPHIC",
        "BAD_POPULATION",
        "BAD_INDUSTRY",
        "NO_RESEARCH",
        "NO_INFLUENCE",
        "GOOD_STOCKPILE",
        "GREAT_OFFENSE_TROOPS",
        "PEDIA_PHOTOTROPHIC_SPECIES_CLASS",
    ],
    foci=[
        HAS_INDUSTRY_FOCUS,
        # [[HAS_RESEARCH_FOCUS]]*/    // dumb plants after all
        # [[HAS_INFLUENCE_FOCUS]]*/   // see above
        HAS_GROWTH_FOCUS,
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_PROTECTION",
    likes=[
        "BLD_LIGHTHOUSE",  # if there were a greenhouse they would like that too
        "BLD_STOCKPILING_CENTER",  # why, to store all the seeds for the next rainy season of course
        "FOCUS_GROWTH",  # what else would a plant want?
        "FOCUS_PROTECTION",  # yep, that one too.. happily bristling with thorns, soaking sun, la la la
        "HEAD_ON_A_SPIKE_SPECIAL",  # we likes the spikes
        "PLC_POPULATION",  # grow, grow, grow your load, gently down the ..
        "TIDAL_LOCK_SPECIAL",  # eternal daylight yeah! nightsiders? not a problem for us prickly plants we got no blood
    ],
    dislikes=[  # dislikes are each on their own single line of code to facilitate commenting and merging,
        "CLOUD_COVER_MASTER_SPECIAL",  # stop blockin the sun over there
        "CLOUD_COVER_SLAVE_SPECIAL",  # same. emphasis.
        "PLC_NO_GROWTH",  # dumb plants, but looove to grow
        "VOLCANIC_ASH_MASTER_SPECIAL",  # no sunblock pls
        "VOLCANIC_ASH_SLAVE_SPECIAL",  # ...
        # see also: WORLDTREE.focs.txt, where only the SP_KHAKTURIAN are explicitely excluded from worldree bonus effects,
    ],
    effectsgroups=[
        *BAD_INDUSTRY,
        NO_RESEARCH,
        *NO_INFLUENCE,
        *GOOD_STOCKPILE,
        *BAD_POPULATION,
        *AVERAGE_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_KHAKTURIAN"),
        *AVERAGE_SUPPLY,
        *GREAT_OFFENSE_TROOPS,
        *GREAT_DEFENSE_TROOPS,
        # not for description,
        *DEFAULT_NATIVE_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
        NARROW_EP,
    ],
    environments=DESERT_NARROW_EP,
    graphic="icons/species/khakturian.png",
)
