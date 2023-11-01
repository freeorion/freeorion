from focs._effects import Capital, EffectsGroup, GiveEmpirePolicy, IsSource, Planet, Unowned
from focs._species import *
from species.common.detection import GOOD_DETECTION
from species.common.empire_opinions import COMMON_OPINION_EFFECTS
from species.common.env import RADIATED_STANDARD_EP
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
from species.common.planet_defense import AVERAGE_PLANETARY_DEFENSE
from species.common.planet_shields import AVERAGE_PLANETARY_SHIELDS
from species.common.population import AVERAGE_POPULATION
from species.common.research import AVERAGE_RESEARCH
from species.common.shields import GOOD_SHIP_SHIELDS
from species.common.stealth import GOOD_STEALTH
from species.common.stockpile import AVERAGE_STOCKPILE
from species.common.supply import AVERAGE_SUPPLY
from species.common.telepathic import TELEPATHIC_DETECTION
from species.common.troops import AVERAGE_DEFENSE_TROOPS
from species.common.xenophobic import XENOPHOBIC_OTHER, XENOPHOBIC_SELF

Species(
    name="SP_TRITH",
    description="SP_TRITH_DESC",
    gameplay_description="SP_TRITH_GAMEPLAY_DESC",
    playable=True,
    can_produce_ships=True,
    can_colonize=True,
    tags=[
        "SELF_SUSTAINING",
        "TELEPATHIC",
        "XENOPHOBIC",
        "GREAT_INFLUENCE",
        "GOOD_SHIP_SHIELDS",
        "BAD_HAPPINESS",
        "AVERAGE_SUPPLY",
        "GOOD_DETECTION",
        "GOOD_STEALTH",
        "PEDIA_SELF_SUSTAINING_SPECIES_CLASS",
        "PEDIA_XENOPHOBIC_SPECIES_TITLE",
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
        "SPARK_FOSSILS_SPECIAL",
        "FRACTAL_GEODES_SPECIAL",
        "POSITRONIUM_SPECIAL",
        "MONOPOLE_SPECIAL",
        "MINERALS_SPECIAL",
        "ECCENTRIC_ORBIT_SPECIAL",
        "PLC_CHECKPOINTS",
        "PLC_RACIAL_PURITY",
        "PLC_CONFEDERATION",
        "PLC_TRAFFIC_CONTROL",
        "PLC_DREAM_RECURSION",
        "PLC_ISOLATION",
        "PLC_DIVINE_AUTHORITY",
    ],
    dislikes=[
        # "BLD_CLONING_CENTER"       // disabled content,
        "BLD_COLLECTIVE_NET",
        "BLD_GENOME_BANK",
        "BLD_INTERSPECIES_ACADEMY",
        "BLD_XENORESURRECTION_LAB",
        "FOCUS_INDUSTRY",
        "FORTRESS_SPECIAL",
        "PANOPTICON_SPECIAL",
        "PLC_ALLIED_REPAIR",
        "PLC_CONTINUOUS_SCANNING",
        "PLC_DIVERSITY",
        "PLC_EXPLORATION",
        "SPICE_SPECIAL",
        "WORLDTREE_SPECIAL",
    ],
    effectsgroups=[
        *AVERAGE_INDUSTRY,
        *AVERAGE_RESEARCH,
        *GREAT_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *AVERAGE_POPULATION,
        *BAD_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_TRITH"),
        *AVERAGE_SUPPLY,
        *AVERAGE_DEFENSE_TROOPS,
        *GOOD_DETECTION,
        *GOOD_STEALTH,
        *XENOPHOBIC_SELF,
        XENOPHOBIC_OTHER("TRITH"),
        *TELEPATHIC_DETECTION(5),
        # not for description,
        *AVERAGE_PLANETARY_SHIELDS,
        *AVERAGE_PLANETARY_DEFENSE,
        *GOOD_SHIP_SHIELDS,
        EffectsGroup(
            scope=IsSource, activation=Planet() & ~Unowned & Capital, effects=GiveEmpirePolicy(name="PLC_RACIAL_PURITY")
        ),
    ],
    environments=RADIATED_STANDARD_EP,
    graphic="icons/species/trith.png",
)
