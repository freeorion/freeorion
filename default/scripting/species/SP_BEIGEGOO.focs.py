from focs._species import *
from macros.misc import MINIMUM_DISTANCE_EMPIRE_CHECK
from species.species_macros.empire_opinions import COMMON_OPINION_EFFECTS
from species.species_macros.env import RADIATED_STANDARD_EP
from species.species_macros.focus import (
    HAS_ADVANCED_FOCI,
    HAS_INDUSTRY_FOCUS,
)
from species.species_macros.happiness import AVERAGE_HAPPINESS
from species.species_macros.industry import GREAT_INDUSTRY
from species.species_macros.influence import NO_INFLUENCE
from species.species_macros.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.species_macros.population import GOOD_POPULATION
from species.species_macros.research import NO_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stockpile import AVERAGE_STOCKPILE
from species.species_macros.supply import AVERAGE_SUPPLY
from species.species_macros.troops import GREAT_DEFENSE_TROOPS

Species(
    name="SP_BEIGEGOO",
    description="SP_BEIGEGOO_DESC",
    gameplay_description="SP_BEIGEGOO_GAMEPLAY_DESC",
    native=True,
    tags=[
        "ROBOTIC",
        "GOOD_POPULATION",
        "GREAT_INDUSTRY",
        "NO_INFLUENCE",
        "NO_RESEARCH",
        "AVERAGE_SUPPLY",
        "PEDIA_ROBOTIC_SPECIES_CLASS",
    ],
    foci=[
        HAS_INDUSTRY_FOCUS,
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_INDUSTRY",
    likes=[
        "FOCUS_INDUSTRY",
        "MIMETIC_ALLOY_SPECIAL",
        "PLC_ALGORITHMIC_RESEARCH",
        "PLC_CENTRALIZATION",
        "PLC_CONTINUOUS_SCANNING",
        "PLC_DESIGN_SIMPLICITY",
        "PLC_ENGINEERING",
        "PLC_ISOLATION",
        "PLC_RACIAL_PURITY",
        "RESONANT_MOON_SPECIAL",
        "SPARK_FOSSILS_SPECIAL",
        "TEMPORAL_ANOMALY_SPECIAL",
        "TIDAL_LOCK_SPECIAL",
        "WORLDTREE_SPECIAL",
    ],
    dislikes=[
        "BLD_COLLECTIVE_NET",
        "BLD_SCRYING_SPHERE",
        "BLD_SPACE_ELEVATOR",
        "BLD_STOCKPILING_CENTER",
        "CRYSTALS_SPECIAL",
        "FRUIT_SPECIAL",
        "PLC_ALLIED_REPAIR",
        "PLC_ENVIRONMENTALISM",
        "PLC_INTERSTELLAR_INFRA",
        "PLC_MODERATION",
        "PLC_NATIVE_APPROPRIATION",
        "PROBIOTIC_SPECIAL",
        "SPICE_SPECIAL",
    ],
    effectsgroups=[
        *GREAT_INDUSTRY,
        NO_RESEARCH,
        *NO_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *GOOD_POPULATION,
        *AVERAGE_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_BEIGEGOO"),
        *AVERAGE_SUPPLY,
        *GREAT_DEFENSE_TROOPS,
        # not for description
        *DEFAULT_NATIVE_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
        EffectsGroup(
            scope=IsSource,
            activation=Planet() & Turn(high=0) & (GalaxyMaxAIAggression >= 1) & (GalaxyMonsterFrequency >= 1),
            effects=Conditional(
                condition=MINIMUM_DISTANCE_EMPIRE_CHECK & Star(type=[Blue, White, Yellow, Orange, Red]),
                effects=[CreateShip(designname="SM_TREE")],
                else_=[AddSpecial(name="VOLCANIC_ASH_SLAVE_SPECIAL"), AddSpecial(name="VOLCANIC_ASH_MASTER_SPECIAL")],
            ),
        ),
    ],
    environments=RADIATED_STANDARD_EP,
    graphic="icons/species/beige-goo.png",
)
