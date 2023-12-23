from focs._effects import (
    AnyEmpire,
    AsteroidsType,
    Barren,
    Desert,
    EffectsGroup,
    GasGiantType,
    GenerateSitRepMessage,
    Good,
    Inferno,
    IsSource,
    NamedReal,
    Ocean,
    OwnedBy,
    Planet,
    Radiated,
    SetMaxShield,
    SetMaxTroops,
    SetPopulation,
    Source,
    Swamp,
    Target,
    Terran,
    Toxic,
    Tundra,
    Turn,
    Value,
)
from focs._species import *
from macros.misc import PLANET_SHIELD_FACTOR
from macros.priorities import POPULATION_OVERRIDE_PRIORITY
from species.species_macros.empire_opinions import FIXED_OPINION_EFFECTS
from species.species_macros.focus import HAS_ADVANCED_FOCI
from species.species_macros.happiness import AVERAGE_HAPPINESS
from species.species_macros.industry import NO_INDUSTRY
from species.species_macros.influence import NO_INFLUENCE
from species.species_macros.planet_defense import AVERAGE_PLANETARY_DEFENSE
from species.species_macros.planet_shields import AVERAGE_PLANETARY_SHIELDS
from species.species_macros.population import AVERAGE_POPULATION
from species.species_macros.research import NO_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stockpile import NO_STOCKPILE
from species.species_macros.supply import AVERAGE_SUPPLY
from species.species_macros.troops import AVERAGE_DEFENSE_TROOPS

Species(
    name="SP_ANCIENT_GUARDIANS",
    description="SP_ANCIENT_GUARDIANS_DESC",
    gameplay_description="SP_ANCIENT_GUARDIANS_GAMEPLAY_DESC",
    tags=[
        "ROBOTIC",
        "LOW_BRAINPOWER",
        "NO_RESEARCH",
        "NO_INFLUENCE",
        "AVERAGE_SUPPLY",
        "DESTROYED_ON_CONQUEST",
        "CTRL_STAT_SKIP_DEPOP",
        "PEDIA_ROBOTIC_SPECIES_CLASS",
    ],
    foci=HAS_ADVANCED_FOCI,
    defaultfocus="FOCUS_PROTECTION",
    effectsgroups=[
        # just to make the tags consistent...
        NO_INDUSTRY,
        NO_RESEARCH,
        *NO_INFLUENCE,
        *NO_STOCKPILE,
        *AVERAGE_POPULATION,
        *AVERAGE_HAPPINESS,
        FIXED_OPINION_EFFECTS("SP_ANCIENT_GUARDIANS", 0.0),
        *AVERAGE_SUPPLY,
        *AVERAGE_DEFENSE_TROOPS,
        # not for description
        *AVERAGE_PLANETARY_SHIELDS,
        *AVERAGE_PLANETARY_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
        # huge bonus to planetary shields
        EffectsGroup(
            description="ANCIENT_PLANETARY_SHIELD_DESC",
            scope=IsSource & Planet(),
            effects=SetMaxShield(
                value=Value + NamedReal(name="ANCIENT_PLANETARY_SHIELD", value=250 * PLANET_SHIELD_FACTOR)
            ),
        ),
        # huge bonus to ground troops
        EffectsGroup(
            description="ANCIENT_DEFENSE_TROOPS_DESC",
            scope=IsSource & Planet(),
            effects=SetMaxTroops(
                value=Value + (Target.Population * NamedReal(name="ANCIENT_PLANETARY_TROOPS", value=5))
            ),
        ),
        # set population to max
        EffectsGroup(
            scope=IsSource & Planet(),
            activation=Turn(high=1),
            priority=POPULATION_OVERRIDE_PRIORITY,
            effects=SetPopulation(value=Target.TargetPopulation),
        ),
        # self-destruct when captured
        EffectsGroup(
            scope=IsSource,
            activation=Planet() & OwnedBy(affiliation=AnyEmpire),
            priority=POPULATION_OVERRIDE_PRIORITY,
            effects=[
                SetPopulation(value=0),
                GenerateSitRepMessage(
                    message="EFFECT_ANCIENT_GUARDIANS_CAPTURED",
                    label="EFFECT_ANCIENT_GUARDIANS_CAPTURED_LABEL",
                    icon="icons/species/robotic-08.png",
                    parameters={"planet": Source.ID},
                    empire=Source.Owner,
                ),
            ],
        ),
    ],
    environments={
        Swamp: Good,
        Toxic: Good,
        Inferno: Good,
        Radiated: Good,
        Barren: Good,
        Tundra: Good,
        Desert: Good,
        Terran: Good,
        Ocean: Good,
        AsteroidsType: Good,
        GasGiantType: Good,
    },
    graphic="icons/species/robotic-08.png",
    annexation_condition=NoObject,
)
