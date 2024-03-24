from focs._effects import (
    AddSpecial,
    DirectDistanceBetween,
    EffectsGroup,
    IsField,
    IsSource,
    LocalCandidate,
    MinimumNumberOf,
    MoveTowards,
    NoStar,
    Source,
    Star,
    Turn,
)
from focs._species import *
from species.species_macros.detection import GREAT_DETECTION
from species.species_macros.empire_opinions import COMMON_OPINION_EFFECTS
from species.species_macros.env import INFERNO_NARROW_EP, NARROW_EP
from species.species_macros.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.species_macros.happiness import AVERAGE_HAPPINESS
from species.species_macros.industry import AVERAGE_INDUSTRY
from species.species_macros.influence import NO_INFLUENCE
from species.species_macros.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.species_macros.population import EXTREMELY_BAD_POPULATION, LIGHT_SENSITIVE
from species.species_macros.research import AVERAGE_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stealth import GOOD_STEALTH
from species.species_macros.stockpile import AVERAGE_STOCKPILE
from species.species_macros.supply import AVERAGE_SUPPLY
from species.species_macros.telepathic import HAEMAESTHETIC_DETECTION
from species.species_macros.troops import BAD_OFFENSE_TROOPS, GREAT_DEFENSE_TROOPS

Species(
    name="SP_NIGHTSIDERS",
    description="SP_NIGHTSIDERS_DESC",
    gameplay_description="SP_NIGHTSIDERS_GAMEPLAY_DESC",
    native=True,
    can_produce_ships=True,
    tags=[
        "ORGANIC",
        "NO_INFLUENCE",
        "EXTREMELY_BAD_POPULATION",
        "GREAT_DETECTION",
        "GOOD_STEALTH",
        "PEDIA_ORGANIC_SPECIES_CLASS",
        "HAEMAESTHETIC_DETECTION",
        "BAD_OFFENSE_TROOPS",
    ],
    foci=[
        HAS_INDUSTRY_FOCUS,
        HAS_RESEARCH_FOCUS,
        HAS_GROWTH_FOCUS,
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_PROTECTION",
    likes=[
        "MIMETIC_ALLOY_SPECIAL",  # because it works in the dark
        "TIDAL_LOCK_SPECIAL",  # no place like home
        "FOCUS_PROTECTION",  # good at it and like it. why not. go away. nothing to see here anyway. we'll come to you when we need your blood.
        "BLD_INTERSPECIES_ACADEMY",  # sure, show us all the species you got, maybe tasty
        "PLC_DIVINE_AUTHORITY",  # we were kind of in god's plan from the beginning ...
        "PLC_ISOLATION",
        "PLC_INSURGENCY",
        "SP_DERTHREAN",  # now following: whats on the menu – all those tasty organic non-aquatic species
        "SP_EAXAW",
        "SP_FIFTYSEVEN",
        "SP_FURTHEST",
        "SP_GYSACHE",
        "SP_HHHOH",
        "SP_HUMAN",
        "SP_LEMBALALAM",
        "SP_MISIORLA",
        "SP_MUURSH",
        "SP_PHINNERT",
        "SP_RAAAGH",
        "SP_SLEEPERS",
    ],
    dislikes=[
        "PANOPTICON_SPECIAL",  # can see well enough ourselves so no thx
        "BLD_SCANNING_FACILITY",  # same. emphasis.
        "SHIMMER_SILK_SPECIAL",  # draws unnecessary attention
        "BLD_LIGHTHOUSE",  # seriously?
        "SP_SETINON",  # cause we can feel they are there, but too small to be sucked. suckers!
        "SP_SSLITH",  # see above
        "SP_GISGUFGTHRIM",  # shell too hard and that amber sap makes their blood taste bitter
        "SP_OURBOOLS",
        "SP_SCYLIOR",  # and of course we hate all the other inedible organic species as well.
    ],
    effectsgroups=[
        *AVERAGE_INDUSTRY,
        *AVERAGE_RESEARCH,
        *NO_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *EXTREMELY_BAD_POPULATION,
        *AVERAGE_HAPPINESS,
        COMMON_OPINION_EFFECTS(
            "SP_NIGHTSIDERS"
        ),  # TODO: make like empires with lots of organic population? or just use a standard count of liked species in empire?
        *AVERAGE_SUPPLY,
        *BAD_OFFENSE_TROOPS,
        *GREAT_DEFENSE_TROOPS,
        *GREAT_DETECTION,
        *GOOD_STEALTH,
        *HAEMAESTHETIC_DETECTION(9),
        *LIGHT_SENSITIVE,
        # not for description,
        *DEFAULT_NATIVE_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
        NARROW_EP,
        # the nightsiders are so bad, they invite fire and brimstone,
        EffectsGroup(
            scope=MinimumNumberOf(
                number=1,
                sortkey=DirectDistanceBetween(Source.ID, LocalCandidate.ID),
                condition=IsField(name=["FLD_METEOR_BLIZZARD"]),
            ),
            effects=MoveTowards(speed=5, target=IsSource),
        ),
        # the nightsiders are so bad, they even repel good fields,
        EffectsGroup(
            scope=MinimumNumberOf(
                number=1,
                sortkey=DirectDistanceBetween(Source.ID, LocalCandidate.ID),
                condition=IsField(name=["FLD_NANITE_SWARM"]),
            ),
            effects=MoveTowards(speed=-5, target=IsSource),
        ),
        # they like to be kept in the dark, so either the night side,
        EffectsGroup(
            scope=IsSource,
            activation=Turn(high=0) & ~Star(type=[NoStar]),
            effects=[AddSpecial(name="TIDAL_LOCK_SPECIAL")],
        )
        # TODO: ... or a starless planet but then tidal_lock makes little sense,
    ],
    environments=INFERNO_NARROW_EP,
    graphic="icons/species/nightsiders.png",
)
