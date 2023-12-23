from focs._effects import (
    EffectsGroup,
    GenerateSitRepMessage,
    IsSource,
    Population,
    SetPopulation,
    Source,
    Target,
    Value,
)
from focs._species import *
from species.species_macros.empire_opinions import COMMON_OPINION_EFFECTS
from species.species_macros.env import GASEOUS_NARROW_EP, NARROW_EP
from species.species_macros.focus import (
    HAS_ADVANCED_FOCI,
    HAS_GROWTH_FOCUS,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.species_macros.happiness import AVERAGE_HAPPINESS
from species.species_macros.industry import NO_INDUSTRY
from species.species_macros.influence import AVERAGE_INFLUENCE
from species.species_macros.native_fortification import DEFAULT_NATIVE_DEFENSE
from species.species_macros.population import AVERAGE_POPULATION
from species.species_macros.research import AVERAGE_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stealth import GOOD_STEALTH
from species.species_macros.stockpile import AVERAGE_STOCKPILE
from species.species_macros.supply import VERY_BAD_SUPPLY
from species.species_macros.troops import GOOD_DEFENSE_TROOPS, NO_OFFENSE_TROOPS

Species(
    name="SP_THENIAN",
    description="SP_THENIAN_DESC",
    gameplay_description="SP_THENIAN_GAMEPLAY_DESC",
    native=True,
    tags=[
        "GASEOUS",
        "NO_TERRAFORM",
        "ARTISTIC",
        "AVERAGE_POPULATION",
        "AVERAGE_HAPPINESS",
        "VERY_BAD_SUPPLY",
        "NO_INDUSTRY",
        "GOOD_STEALTH",
        "NO_OFFENSE_TROOPS",
        "PEDIA_GASEOUS_SPECIES_CLASS",
        "PEDIA_ARTISTIC",
    ],
    foci=[
        HAS_RESEARCH_FOCUS,
        HAS_INFLUENCE_FOCUS,
        HAS_GROWTH_FOCUS,
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_GROWTH",
    likes=[
        "FOCUS_RESEARCH",  # arty farty
    ],
    dislikes=[
        "BLD_GAS_GIANT_GEN",  # messes with their habitat
        "BLD_SPACE_ELEVATOR",  # see above
    ],
    effectsgroups=[
        NO_INDUSTRY,
        *AVERAGE_RESEARCH,
        *AVERAGE_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *AVERAGE_POPULATION,
        *AVERAGE_HAPPINESS,
        COMMON_OPINION_EFFECTS("SP_THENIAN"),
        *VERY_BAD_SUPPLY,
        *NO_OFFENSE_TROOPS,
        *GOOD_DEFENSE_TROOPS,
        *GOOD_STEALTH,
        # not for description,
        *DEFAULT_NATIVE_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
        NARROW_EP,
        EffectsGroup(
            scope=IsSource & Population(low=Source.TargetPopulation - 0.01),  # when maximum population is reached
            effects=[
                SetPopulation(value=Value * 0.5),  # then dieoff to half
                GenerateSitRepMessage(  # and tell the owner it happened
                    message="EFFECT_THENIAN_NATURAL_DIEOFF",
                    label="EFFECT_THENIAN_NATURAL_DIEOFF_LABEL",
                    icon="icons/sitrep/colony_starvation.png",
                    parameters={"tag": "planet", "data": Target.ID},
                    empire=Target.Owner,
                ),
            ],
        ),
    ],
    environments=GASEOUS_NARROW_EP,
    graphic="icons/species/thenian.png",
)
