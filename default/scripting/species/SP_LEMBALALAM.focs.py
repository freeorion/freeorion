from focs._effects import (
    AddSpecial,
    EffectsGroup,
    GenerateSitRepMessage,
    GiveEmpireTech,
    HasSpecial,
    IsSource,
    OwnerHasTech,
    Planet,
    RemoveSpecial,
    SetPopulation,
    SetTargetPopulation,
    Source,
    Target,
    Turn,
    Uninhabitable,
)
from focs._species import *
from macros.priorities import POPULATION_OVERRIDE_PRIORITY, TARGET_POPULATION_OVERRIDE_PRIORITY
from species.species_macros.advanced_focus import ADVANCED_FOCUS_EFFECTS
from species.species_macros.env import DESERT_STANDARD_EP
from species.species_macros.focus import (
    HAS_ADVANCED_FOCI,
    HAS_INFLUENCE_FOCUS,
    HAS_RESEARCH_FOCUS,
)
from species.species_macros.general import FOCUS_CHANGE_PENALTY, STANDARD_CONSTRUCTION, STANDARD_METER_GROWTH
from species.species_macros.happiness import AVERAGE_HAPPINESS
from species.species_macros.industry import NO_INDUSTRY
from species.species_macros.influence import VERY_BAD_INFLUENCE
from species.species_macros.planet_defense import AVERAGE_PLANETARY_DEFENSE
from species.species_macros.planet_shields import AVERAGE_PLANETARY_SHIELDS
from species.species_macros.research import GOOD_RESEARCH
from species.species_macros.shields import STANDARD_SHIP_SHIELDS
from species.species_macros.stealth import GOOD_STEALTH
from species.species_macros.stockpile import AVERAGE_STOCKPILE
from species.species_macros.supply import AVERAGE_SUPPLY
from species.species_macros.troops import AVERAGE_DEFENSE_TROOPS

Species(
    name="SP_LEMBALALAM",
    description="SP_LEMBALALAM_DESC",
    gameplay_description="SP_LEMBALALAM_GAMEPLAY_DESC",
    native=True,
    tags=[
        "ORGANIC",
        "NO_INDUSTRY",
        "GOOD_RESEARCH",
        "VERY_BAD_INFLUENCE",
        "AVERAGE_SUPPLY",
        "GOOD_STEALTH",
        "PEDIA_ORGANIC_SPECIES_CLASS",
    ],
    foci=[
        # HAS_INDUSTRY_FOCUS
        HAS_RESEARCH_FOCUS,
        HAS_INFLUENCE_FOCUS,
        # HAS_GROWTH_FOCUS
        *HAS_ADVANCED_FOCI,
    ],
    defaultfocus="FOCUS_RESEARCH",
    likes=[
        "FOCUS_RESEARCH",
        "SPARK_FOSSILS_SPECIAL",
        "SUCCULENT_BARNACLES_SPECIAL",
        "ANCIENT_RUINS_DEPLETED_SPECIAL",
        "PLC_BUREAUCRACY",
        "PLC_AUGMENTATION",
        "PLC_CHECKPOINTS",
        "PLC_EXOBOT_PRODUCTIVITY",
        "PLC_ISOLATION",
        "PLC_NO_GROWTH",
    ],
    dislikes=[
        "SUPERCONDUCTOR_SPECIAL",
        "SPICE_SPECIAL",
        "PROBIOTIC_SPECIAL",
        "POSITRONIUM_SPECIAL",
        "MONOPOLE_SPECIAL",
        "MINERALS_SPECIAL",
        "PLC_MARTIAL_LAW",
        "PLC_SYSTEM_INFRA",
        "PLC_INTERSTELLAR_INFRA",
        "PLC_METROPOLES",
        "PLC_POPULATION",
        "SP_NIGHTSIDERS",
    ],
    effectsgroups=[
        NO_INDUSTRY,
        *GOOD_RESEARCH,
        *VERY_BAD_INFLUENCE,
        *AVERAGE_STOCKPILE,
        *AVERAGE_HAPPINESS,
        *AVERAGE_SUPPLY,
        *AVERAGE_DEFENSE_TROOPS,
        *GOOD_STEALTH,
        # other species get these via BASIC_POPULATION,
        *FOCUS_CHANGE_PENALTY,
        *ADVANCED_FOCUS_EFFECTS,
        *STANDARD_CONSTRUCTION,
        *STANDARD_METER_GROWTH,
        # not for description,
        *AVERAGE_PLANETARY_SHIELDS,
        *AVERAGE_PLANETARY_DEFENSE,
        *STANDARD_SHIP_SHIELDS,
        # Start the game with Volcanic Ash stealth special,
        EffectsGroup(
            scope=IsSource,
            activation=Turn(high=0),
            # stackinggroup = "GAME_START_MOD_STACK",
            effects=[
                AddSpecial(name="VOLCANIC_ASH_SLAVE_SPECIAL"),
                AddSpecial(name="VOLCANIC_ASH_MASTER_SPECIAL"),
            ],
        ),
        # Always start the game as High Tech natives,
        EffectsGroup(
            scope=IsSource & HasSpecial(name="MODERATE_TECH_NATIVES_SPECIAL"),
            activation=Turn(high=0),
            effects=RemoveSpecial(name="MODERATE_TECH_NATIVES_SPECIAL"),
        ),
        EffectsGroup(
            scope=IsSource & ~HasSpecial(name="HIGH_TECH_NATIVES_SPECIAL"),
            activation=Turn(high=0),
            # stackinggroup = "GAME_START_MOD_STACK",
            effects=AddSpecial(name="HIGH_TECH_NATIVES_SPECIAL"),
        ),
        # Max Population is fixed at 5,
        EffectsGroup(
            description="FIXED_LOW_POPULATION_DESC",
            scope=IsSource,
            activation=Planet() & ~Planet(environment=[Uninhabitable]),
            priority=TARGET_POPULATION_OVERRIDE_PRIORITY,
            effects=SetTargetPopulation(value=5, accountinglabel="IMMORTAL_LABEL"),
        ),
        # set population to max,
        EffectsGroup(
            scope=IsSource & Planet(),
            activation=Turn(high=1),
            priority=POPULATION_OVERRIDE_PRIORITY,
            effects=SetPopulation(value=Target.TargetPopulation),
        ),
        # Give techs to empire,
        EffectsGroup(
            scope=IsSource,
            activation=~OwnerHasTech(name="SPY_STEALTH_1"),
            stackinggroup="LEMBALALAM_TECH_UNLOCK",
            effects=[
                GiveEmpireTech(name="SPY_STEALTH_1", empire=Target.Owner),
                GenerateSitRepMessage(
                    message="EFFECT_NATIVES_TECH",
                    label="EFFECT_NATIVES_TECH_LABEL",
                    icon="icons/species/other-01.png",
                    parameters={
                        "planet": Source.ID,
                        "species": "SP_LEMBALALAM",
                        "tech": "SPY_STEALTH_1",
                    },
                    empire=Source.Owner,
                ),
            ],
        ),
        EffectsGroup(
            scope=IsSource,
            activation=~OwnerHasTech(name="SPY_STEALTH_2") & OwnerHasTech(name="SPY_STEALTH_1"),
            stackinggroup="LEMBALALAM_TECH_UNLOCK",
            effects=[
                GiveEmpireTech(name="SPY_STEALTH_2", empire=Target.Owner),
                GenerateSitRepMessage(
                    message="EFFECT_NATIVES_TECH",
                    label="EFFECT_NATIVES_TECH_LABEL",
                    icon="icons/species/other-01.png",
                    parameters={
                        "planet": Source.ID,
                        "species": "SP_LEMBALALAM",
                        "tech": "SPY_STEALTH_2",
                    },
                    empire=Source.Owner,
                ),
            ],
        ),
        EffectsGroup(
            scope=IsSource,
            activation=~OwnerHasTech(name="GRO_LIFECYCLE_MAN"),
            stackinggroup="LEMBALALAM_TECH_UNLOCK",
            effects=[
                GiveEmpireTech(name="GRO_LIFECYCLE_MAN", empire=Target.Owner),
                GenerateSitRepMessage(
                    message="EFFECT_NATIVES_TECH",
                    label="EFFECT_NATIVES_TECH_LABEL",
                    icon="icons/species/other-01.png",
                    parameters={
                        "planet": Source.ID,
                        "species": "SP_LEMBALALAM",
                        "tech": "GRO_LIFECYCLE_MAN",
                    },
                    empire=Source.Owner,
                ),
            ],
        ),
    ],
    environments=DESERT_STANDARD_EP,
    graphic="icons/species/other-01.png",
)
