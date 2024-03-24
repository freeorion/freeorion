from focs._effects import (
    AddSpecial,
    Conditional,
    CreateShip,
    EffectsGroup,
    GalaxyMaxAIAggression,
    GalaxyMonsterFrequency,
    IsSource,
    Planet,
    Turn,
)
from focs._species import *
from macros.misc import MINIMUM_DISTANCE_EMPIRE_CHECK
from species.species_macros.empire_opinions import COMMON_OPINION_EFFECTS
from species.species_macros.env import SWAMP_STANDARD_EP
from species.species_macros.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.species_macros.happiness import AVERAGE_HAPPINESS
from species.species_macros.industry import NO_INDUSTRY
from species.species_macros.influence import NO_INFLUENCE
from species.species_macros.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.species_macros.planet_size import NOT_LARGE_PLANET
from species.species_macros.population import BAD_POPULATION
from species.species_macros.research import ULTIMATE_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stealth import GOOD_STEALTH
from species.species_macros.stockpile import AVERAGE_STOCKPILE
from species.species_macros.supply import AVERAGE_SUPPLY
from species.species_macros.troops import BAD_DEFENSE_TROOPS

Species(
    name="SP_FIFTYSEVEN",
    description="SP_FIFTYSEVEN_DESC",
    gameplay_description="SP_FIFTYSEVEN_GAMEPLAY_DESC",
    native=True,
    tags=[
        "ORGANIC",
        "ARTISTIC",
        "NO_INDUSTRY",
        "ULTIMATE_RESEARCH",
        "NO_INFLUENCE",
        "BAD_POPULATION",
        "AVERAGE_SUPPLY",
        "GOOD_STEALTH",
        "PEDIA_ORGANIC_SPECIES_CLASS",
        "PEDIA_ARTISTIC",
    ],
    foci=[
        # /*[[HAS_INDUSTRY_FOCUS]]*/
        HAS_RESEARCH_FOCUS,
        # /*[[HAS_INFLUENCE_FOCUS]]*/
        HAS_GROWTH_FOCUS,
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_RESEARCH",
    likes=[
        "FOCUS_RESEARCH",
        "SHIMMER_SILK_SPECIAL",
        "SPARK_FOSSILS_SPECIAL",
        "SPICE_SPECIAL",
        "POSITRONIUM_SPECIAL",
        "ELERIUM_SPECIAL",
        "RESONANT_MOON_SPECIAL",
        "PLC_ALGORITHMIC_RESEARCH",
        "PLC_DREAM_RECURSION",
        "PLC_CONTINUOUS_SCANNING",
        "PLC_CENTRALIZATION",
        "PLC_ISOLATION",
    ],
    dislikes=[
        "BLD_CULTURE_LIBRARY",
        "BLD_SCANNING_FACILITY",
        "BLD_GENOME_BANK",
        "BLD_COLLECTIVE_NET",
        "BLD_ENCLAVE_VOID",
        "BLD_GATEWAY_VOID",
        "BLD_SHIPYARD_ENRG_COMP",
        "COMPUTRONIUM_SPECIAL",
        "PANOPTICON_SPECIAL",
        "TEMPORAL_ANOMALY_SPECIAL",
        "CRYSTALS_SPECIAL",
        "PLC_DESIGN_SIMPLICITY",
        "PLC_EXPLORATION",
        "PLC_ENGINEERING",
        "PLC_MARINE_RECRUITMENT",
        "PLC_TRAFFIC_CONTROL",
        "SP_NIGHTSIDERS",
    ],
    effectsgroups=[
        NO_INDUSTRY,
        *ULTIMATE_RESEARCH,
        *NO_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *BAD_POPULATION,
        *AVERAGE_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_FIFTYSEVEN"),
        *AVERAGE_SUPPLY,
        *BAD_DEFENSE_TROOPS,
        *GOOD_STEALTH,
        # not for description,
        *DEFAULT_NATIVE_DEFENSE,
        *NOT_LARGE_PLANET,
        *STANDARD_SHIP_SHIELDS,
        EffectsGroup(
            scope=IsSource,
            activation=Planet() & Turn(high=0) & (GalaxyMaxAIAggression >= 1) & (GalaxyMonsterFrequency >= 1),
            effects=[
                Conditional(
                    condition=MINIMUM_DISTANCE_EMPIRE_CHECK,
                    effects=[CreateShip(designname="SM_DRAGON")],
                    else_=[
                        AddSpecial(name="VOLCANIC_ASH_SLAVE_SPECIAL"),
                        AddSpecial(name="VOLCANIC_ASH_MASTER_SPECIAL"),
                    ],
                )
            ],
        ),
    ],
    environments=SWAMP_STANDARD_EP,
    graphic="icons/species/fifty-seven.png",
)
