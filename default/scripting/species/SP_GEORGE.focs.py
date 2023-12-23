from focs._species import *
from species.species_macros.detection import COMMUNAL_VISION
from species.species_macros.empire_opinions import COMMON_OPINION_EFFECTS
from species.species_macros.env import BROAD_EP, TUNDRA_BROAD_EP
from species.species_macros.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.species_macros.happiness import GOOD_HAPPINESS
from species.species_macros.industry import GOOD_INDUSTRY
from species.species_macros.influence import AVERAGE_INFLUENCE
from species.species_macros.planet_defense import AVERAGE_PLANETARY_DEFENSE
from species.species_macros.planet_shields import AVERAGE_PLANETARY_SHIELDS
from species.species_macros.planet_size import LARGE_PLANET
from species.species_macros.population import AVERAGE_POPULATION
from species.species_macros.research import BAD_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stockpile import AVERAGE_STOCKPILE
from species.species_macros.supply import GREAT_SUPPLY
from species.species_macros.troops import AVERAGE_DEFENSE_TROOPS

Species(
    name="SP_GEORGE",
    description="SP_GEORGE_DESC",
    gameplay_description="SP_GEORGE_GAMEPLAY_DESC",
    playable=True,
    can_produce_ships=True,
    can_colonize=True,
    tags=[
        "LITHIC",
        "TELEPATHIC",
        "GOOD_INDUSTRY",
        "BAD_RESEARCH",
        "GOOD_HAPPINESS",
        "GREAT_SUPPLY",
        "PEDIA_LITHIC_SPECIES_CLASS",
        "PEDIA_TELEPATHIC_TITLE",
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
        "SUCCULENT_BARNACLES_SPECIAL",
        "SPARK_FOSSILS_SPECIAL",
        "MONOPOLE_SPECIAL",
        "PROBIOTIC_SPECIAL",
        "GAIA_SPECIAL",
        "HONEYCOMB_SPECIAL",
        "PANOPTICON_SPECIAL",
        "PLC_ARTISAN_WORKSHOPS",
        "PLC_CHARGE",
        "PLC_EXPLORATION",
        "PLC_NATIVE_APPROPRIATION",
        "PLC_THE_HUNT",
    ],
    dislikes=[
        "BLD_STARGATE",
        "BLD_GAS_GIANT_GEN",
        "BLD_SHIPYARD_CON_NANOROBO",
        "BLD_SHIPYARD_CON_GEOINT",
        "BLD_SHIPYARD_CON_ADV_ENGINE",
        "BLD_GATEWAY_VOID",
        "BLD_ENCLAVE_VOID",
        "PHILOSOPHER_SPECIAL",
        "ELERIUM_SPECIAL",
        "FRUIT_SPECIAL",
        "PLC_BUREAUCRACY",
        "PLC_CHECKPOINTS",
        "PLC_CONFORMANCE",
        "PLC_MODERATION",
        "PLC_NO_SUPPLY",
    ],
    effectsgroups=[
        *GOOD_INDUSTRY,
        *BAD_RESEARCH,
        *AVERAGE_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *AVERAGE_POPULATION,
        *GOOD_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_GEORGE"),
        *GREAT_SUPPLY,
        *AVERAGE_DEFENSE_TROOPS,
        # not for description,
        *AVERAGE_PLANETARY_SHIELDS,
        *AVERAGE_PLANETARY_DEFENSE,
        *LARGE_PLANET,
        BROAD_EP,
        *STANDARD_SHIP_SHIELDS,
        COMMUNAL_VISION("SP_GEORGE"),
    ],
    environments=TUNDRA_BROAD_EP,
    graphic="icons/species/george.png",
)
