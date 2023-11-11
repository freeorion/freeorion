from focs._effects import (
    DirectDistanceBetween,
    EffectsGroup,
    IsField,
    IsSource,
    LocalCandidate,
    MinimumNumberOf,
    MoveTowards,
    Source,
)
from focs._species import *
from species.common.detection import GREAT_DETECTION
from species.common.empire_opinions import COMMON_OPINION_EFFECTS
from species.common.env import DESERT_STANDARD_EP
from species.common.focus import (
    HAS_ADVANCED_FOCI,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.common.happiness import AVERAGE_HAPPINESS
from species.common.industry import NO_INDUSTRY
from species.common.influence import BAD_INFLUENCE
from species.common.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.common.population import BAD_POPULATION
from species.common.research import AVERAGE_RESEARCH
from species.common.shields import STANDARD_SHIP_SHIELDS
from species.common.stealth import GOOD_STEALTH
from species.common.stockpile import AVERAGE_STOCKPILE
from species.common.supply import BAD_SUPPLY
from species.common.troops import BAD_DEFENSE_TROOPS

Species(
    name="SP_NYMNMN",
    description="SP_NYMNMN_DESC",
    gameplay_description="SP_NYMNMN_GAMEPLAY_DESC",
    native=True,
    tags=[
        "SELF_SUSTAINING",
        "NO_INDUSTRY",
        "BAD_INFLUENCE",
        "BAD_POPULATION",
        "BAD_SUPPLY",
        "GREAT_DETECTION",
        "GOOD_STEALTH",
        "PEDIA_SELF_SUSTAINING_SPECIES_CLASS",
    ],
    foci=[
        HAS_RESEARCH_FOCUS,
        HAS_INFLUENCE_FOCUS,
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_RESEARCH",
    likes=[
        "BLD_NEUTRONIUM_EXTRACTOR",
        "BLD_NEUTRONIUM_FORGE",
        "BLD_NEUTRONIUM_SYNTH",
        "MINERALS_SPECIAL",
        "MONOPOLE_SPECIAL",
        "POSITRONIUM_SPECIAL",
    ],
    dislikes=[
        "BLD_GAS_GIANT_GEN",
        "BLD_SOL_ORB_GEN",
        "BLD_SPATIAL_DISTORT_GEN",
        "ECCENTRIC_ORBIT_SPECIAL",
        "FORTRESS_SPECIAL",
        "FRACTAL_GEODES_SPECIAL",
        "GAIA_SPECIAL",
        "PLC_DESIGN_SIMPLICITY",
        "SPARK_FOSSILS_SPECIAL",
    ],
    effectsgroups=[
        NO_INDUSTRY,
        *AVERAGE_RESEARCH,
        *BAD_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *BAD_POPULATION,
        *AVERAGE_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_NYMNMN"),
        *BAD_SUPPLY,
        *BAD_DEFENSE_TROOPS,
        *GREAT_DETECTION,
        *GOOD_STEALTH,
        EffectsGroup(
            scope=MinimumNumberOf(
                number=1,
                sortkey=DirectDistanceBetween(target=Source.ID, source=LocalCandidate.ID),
                condition=IsField(name=["FLD_ION_STORM", "FLD_METEOR_BLIZZARD", "FLD_NANITE_SWARM"]),
            ),
            effects=MoveTowards(speed=5, target=IsSource),
        ),
        # not for description,
        *DEFAULT_NATIVE_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=DESERT_STANDARD_EP,
    graphic="icons/species/intangible-05.png",
)
