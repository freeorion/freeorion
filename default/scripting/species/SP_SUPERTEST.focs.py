from focs._effects import (
    Capital,
    EffectsGroup,
    IsSource,
    LocalCandidate,
    OwnedBy,
    Planet,
    Population,
    ResourceSupplyConnected,
    SetDetection,
    SetEmpireMeter,
    SetStealth,
    Source,
    Unowned,
    Value,
)
from focs._species import *
from species.species_macros.empire_opinions import FIXED_OPINION_EFFECTS
from species.species_macros.env import VERY_TOLERANT_EP
from species.species_macros.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INDUSTRY_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.species_macros.fuel import GREAT_FUEL
from species.species_macros.happiness import GREAT_HAPPINESS
from species.species_macros.industry import GREAT_INDUSTRY
from species.species_macros.influence import GREAT_INFLUENCE
from species.species_macros.planet_defense import AVERAGE_PLANETARY_DEFENSE
from species.species_macros.planet_shields import AVERAGE_PLANETARY_SHIELDS
from species.species_macros.population import GOOD_POPULATION
from species.species_macros.research import GREAT_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stockpile import GREAT_STOCKPILE
from species.species_macros.supply import ULTIMATE_SUPPLY
from species.species_macros.troops import GREAT_DEFENSE_TROOPS, GREAT_OFFENSE_TROOPS
from species.species_macros.weapons import ULTIMATE_WEAPONS

Species(
    name="SP_SUPER_TEST",
    description="SP_SUPER_TEST_DESC",
    gameplay_description="SP_SUPER_TEST_GAMEPLAY_DESC",
    can_produce_ships=True,
    can_colonize=True,
    tags=[
        "ORGANIC",
        "ROBOTIC",
        "LITHIC",
        "ARTISTIC",
        "PHOTOTROPHIC",
        "SELF_SUSTAINING",
        "TELEPATHIC",
        "GREAT_INDUSTRY",
        "GREAT_RESEARCH",
        "GREAT_INFLUENCE",
        "ULTIMATE_WEAPONS",
        "GREAT_FUEL",
        "GOOD_POPULATION",
        "GREAT_HAPPINESS",
        "ULTIMATE_SUPPLY",
        "INFINITE_DETECTION",
        "INFINITE_STEALTH",
        "GREAT_OFFENSE_TROOPS",
        "PEDIA_ARTISTIC",
        "PEDIA_TELEPATHIC_TITLE",
    ],
    foci=[
        HAS_INDUSTRY_FOCUS,
        HAS_RESEARCH_FOCUS,
        HAS_INFLUENCE_FOCUS,
        HAS_GROWTH_FOCUS,
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_RESEARCH",
    likes=["FOCUS_PROTECTION", "SPARK_FOSSILS_SPECIAL", "SUCCULENT_BARNACLES_SPECIAL"],
    dislikes=["FOCUS_GROWTH", "BLD_CULTURE_ARCHIVES", "BLD_CULTURE_LIBRARY"],
    effectsgroups=[
        *GREAT_INDUSTRY,
        *GREAT_RESEARCH,
        *GREAT_INFLUENCE,
        *GREAT_STOCKPILE,
        *GOOD_POPULATION,
        *GREAT_HAPPINESS,
        FIXED_OPINION_EFFECTS("SP_SUPER_TEST", 20),
        *ULTIMATE_SUPPLY,
        *GREAT_DEFENSE_TROOPS,
        *GREAT_OFFENSE_TROOPS,
        *ULTIMATE_WEAPONS,
        *GREAT_FUEL,
        # not for description,
        *AVERAGE_PLANETARY_SHIELDS,
        *AVERAGE_PLANETARY_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
        EffectsGroup(scope=IsSource, effects=SetDetection(value=Value + 10000)),
        EffectsGroup(scope=IsSource, effects=SetStealth(value=Value + 500)),
        EffectsGroup(
            scope=IsSource & Capital,
            effects=SetEmpireMeter(empire=Source.Owner, meter="METER_DETECTION_STRENGTH", value=200.0),
        ),  # Omniscient Detection
    ],
    environments=VERY_TOLERANT_EP,
    annexationcondition=Unowned
    & Population(low=0.001)
    & ResourceSupplyConnected(empire=Source.Owner, condition=Planet() & OwnedBy(empire=Source.Owner)),
    annexationcost=5.0 * LocalCandidate.Population,
    graphic="icons/species/other-04.png",
)
