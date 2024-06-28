try:
    from focs._effects import EffectsGroup, GiveEmpireTech, IsSource, Target, Turn
    from focs._species import *
except ModuleNotFoundError:
    pass
from species.species_macros.env import BROAD_EP, INFERNO_BROAD_EP
from species.species_macros.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.species_macros.happiness import GOOD_HAPPINESS
from species.species_macros.industry import AVERAGE_INDUSTRY
from species.species_macros.influence import GREAT_INFLUENCE
from species.species_macros.planet_defense import AVERAGE_PLANETARY_DEFENSE
from species.species_macros.planet_shields import AVERAGE_PLANETARY_SHIELDS
from species.species_macros.planet_size import LARGE_PLANET
from species.species_macros.population import AVERAGE_POPULATION
from species.species_macros.research import BAD_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stockpile import AVERAGE_STOCKPILE
from species.species_macros.supply import AVERAGE_SUPPLY
from species.species_macros.troops import BAD_DEFENSE_TROOPS

Species(
    name="SP_ABADDONI",
    description="SP_ABADDONI_DESC",
    gameplay_description="SP_ABADDONI_GAMEPLAY_DESC",
    playable=True,
    can_produce_ships=True,
    can_colonize=True,
    tags=[
        "LITHIC",
        "BAD_RESEARCH",
        "GREAT_INFLUENCE",
        "GOOD_HAPPINESS",
        "AVERAGE_SUPPLY",
        "PEDIA_LITHIC_SPECIES_CLASS",
    ],
    foci=[
        HAS_INDUSTRY_FOCUS,
        HAS_RESEARCH_FOCUS,
        HAS_INFLUENCE_FOCUS,
        HAS_GROWTH_FOCUS,
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_INDUSTRY",
    likes=[
        "FOCUS_INDUSTRY",
        "SHIMMER_SILK_SPECIAL",
        "FRACTAL_GEODES_SPECIAL",
        "SUPERCONDUCTOR_SPECIAL",
        "PROBIOTIC_SPECIAL",
        "MINERALS_SPECIAL",
        "CRYSTALS_SPECIAL",
        "PLC_DIVINE_AUTHORITY",
        "PLC_CONFORMANCE",
        "PLC_TERROR_SUPPRESSION",
        "PLC_INDOCTRINATION",
        "PLC_BUREAUCRACY",
    ],
    dislikes=[
        "BLD_SCRYING_SPHERE",
        "BLD_MEGALITH",
        "BLD_PLANET_DRIVE",
        "BLD_GATEWAY_VOID",
        "BLD_GAS_GIANT_GEN",
        "FORTRESS_SPECIAL",
        "HONEYCOMB_SPECIAL",
        "PHILOSOPHER_SPECIAL",
        "TIDAL_LOCK_SPECIAL",
        "PLC_DIVERSITY",
        "PLC_LIBERTY",
        "PLC_ARTISAN_WORKSHOPS",
        "PLC_CONFEDERATION",
    ],
    effectsgroups=[
        *AVERAGE_INDUSTRY,
        *BAD_RESEARCH,
        *GREAT_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *AVERAGE_POPULATION,
        *GOOD_HAPPINESS,
        *AVERAGE_SUPPLY,
        *BAD_DEFENSE_TROOPS,
        EffectsGroup(
            scope=IsSource,
            activation=Turn(high=1),
            effects=[GiveEmpireTech(name="GRO_SUBTER_HAB", empire=Target.Owner)],
        ),
        # not for description
        *AVERAGE_PLANETARY_SHIELDS,
        *AVERAGE_PLANETARY_DEFENSE,
        *LARGE_PLANET,
        BROAD_EP,
        *STANDARD_SHIP_SHIELDS,
    ],
    environments=INFERNO_BROAD_EP,
    graphic="icons/species/abaddonnian.png",
)
