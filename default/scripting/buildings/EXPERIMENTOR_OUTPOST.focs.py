from buildings.buildings_macros import CAN_ADD_STARLANE_TO_SOURCE
from focs._effects import (
    AddStarlanes,
    AnyEmpire,
    ContainedBy,
    Contains,
    CreateShip,
    Destroy,
    DirectDistanceBetween,
    EffectsGroup,
    GalaxyMaxAIAggression,
    GalaxyPlanetDensity,
    GalaxySize,
    GameRule,
    GenerateSitRepMessage,
    HasDesign,
    HasSpecies,
    InSystem,
    IsAnyObject,
    IsSource,
    LocalCandidate,
    MaxOf,
    MinimumNumberOf,
    Monster,
    NumberOf,
    Object,
    OwnedBy,
    Planet,
    Random,
    RemoveStarlanes,
    SetDefense,
    SetDetection,
    SetMaxDefense,
    SetMaxShield,
    SetMaxTroops,
    SetShield,
    SetStealth,
    Ship,
    Source,
    System,
    Turn,
    Unowned,
    Value,
    Victory,
    WithinStarlaneJumps,
)
from macros.misc import PLANET_DEFENSE_FACTOR, PLANET_SHIELD_FACTOR

try:
    from focs._buildings import *
except ModuleNotFoundError:
    pass

# the highest current MaxAIAggression is 5, so this would range from 0.2 to 1.2
EXPERIMENTOR_MONSTER_FREQ_FACTOR: float = (1 + GalaxyMaxAIAggression) / 5.0

EXPERIMENTOR_SPAWN_BASE_TURN: int = GameRule(type=int, name="RULE_EXPERIMENTORS_SPAWN_BASE_TURN")

EXPERIMENTOR_SPAWN_AI_AGGRESSION_CHECK: bool = GalaxyMaxAIAggression <= 2

# the following intermediate value is turn 200 for Manaical Max AI Aggression, and 50 turns later for each aggression tier less,
# for AI aggressions above Typical, otherwise adding 1000 turns
EXPERIMENTOR_SPAWN_CALC_A: int = (
    EXPERIMENTOR_SPAWN_BASE_TURN + 50 * (5 - GalaxyMaxAIAggression) + 1000 * EXPERIMENTOR_SPAWN_AI_AGGRESSION_CHECK
)

# the following modifies Experimentor spawn start by a factor of 0.1 up for low planet density, and 0.1 down for high planet density
# for galaxy sizes above 200 increases it somewhat as the galaxy size grows
EXPERIMENTOR_SPAWN_START_TURN = (
    EXPERIMENTOR_SPAWN_CALC_A * (1.2 - GalaxyPlanetDensity / 10) * (MaxOf(float, 1, GalaxySize / 200)) ** 0.4
)

# of the 5 closest systems to which a lane could be added, pick one
EXPERIMENTOR_ADD_STARLANE = AddStarlanes(
    endpoint=NumberOf(
        number=1,
        condition=MinimumNumberOf(
            number=5,
            sortkey=DirectDistanceBetween(Source.ID, LocalCandidate.ID),
            condition=System & ~Contains(IsSource),
        )
        & CAN_ADD_STARLANE_TO_SOURCE,
    ),
)

BuildingType(  # pyrefly: ignore[unbound-name]
    name="BLD_EXPERIMENTOR_OUTPOST",
    description="BLD_EXPERIMENTOR_OUTPOST_DESC",
    buildcost=1,
    buildtime=1,
    location=IsAnyObject,
    effectsgroups=[
        EffectsGroup(
            scope=IsSource | (Object(id=Source.PlanetID) & Planet()),
            effects=[SetStealth(value=Value + 60)],
        ),
        EffectsGroup(
            scope=(Object(id=Source.SystemID) | (InSystem(id=Source.SystemID) & Planet() & ~IsSource)),
            effects=[SetStealth(value=Value + 40)],
        ),
        EffectsGroup(
            scope=Object(id=Source.PlanetID),
            effects=[
                SetMaxShield(value=Value + (60000 * PLANET_SHIELD_FACTOR)),
                SetMaxDefense(value=Value + (EXPERIMENTOR_SPAWN_START_TURN * PLANET_DEFENSE_FACTOR)),
                SetMaxTroops(value=Value + 300),
                SetDetection(value=Value + 100),
            ],
        ),
        EffectsGroup(
            scope=Object(id=Source.PlanetID),
            activation=(~ContainedBy(Contains(Ship & OwnedBy(affiliation=AnyEmpire)))),
            effects=[
                # Regeneration
                SetDefense(value=Value + (10 * PLANET_DEFENSE_FACTOR)),
                SetShield(value=Value + (50 * PLANET_SHIELD_FACTOR)),
            ],
        ),
        EffectsGroup(
            scope=(IsSource & OwnedBy(affiliation=AnyEmpire) & HasSpecies(name=["SP_EXPERIMENTOR"])),
            effects=[
                Victory(reason="VICTORY_EXPERIMENTOR_CAPTURE"),
                EXPERIMENTOR_ADD_STARLANE,
                EXPERIMENTOR_ADD_STARLANE,
                EXPERIMENTOR_ADD_STARLANE,
                EXPERIMENTOR_ADD_STARLANE,
                EXPERIMENTOR_ADD_STARLANE,
                Destroy,
            ],
        ),
        EffectsGroup(
            scope=IsSource & ~OwnedBy(affiliation=AnyEmpire),
            activation=(
                # always remove lanes before spawn start turn
                Turn(high=EXPERIMENTOR_SPAWN_START_TURN)
                | ~ContainedBy(Contains(Monster & Unowned & ~HasDesign(name="SM_EXP_OUTPOST")))
            ),
            effects=[RemoveStarlanes(endpoint=WithinStarlaneJumps(jumps=1, condition=IsSource))],
        ),
        EffectsGroup(
            scope=IsSource,
            activation=(
                Turn(low=(EXPERIMENTOR_SPAWN_START_TURN), high=(EXPERIMENTOR_SPAWN_START_TURN + 35))
                & Random(probability=(0.2 * EXPERIMENTOR_MONSTER_FREQ_FACTOR))
            ),
            effects=[
                CreateShip(designname="SM_BLACK_KRAKEN", empire=Source.Owner),
                CreateShip(designname="SM_BLACK_KRAKEN", empire=Source.Owner),
                GenerateSitRepMessage(
                    message="EFFECT_EXPERIMENT_MONSTERS_LAUNCH",
                    label="EFFECT_EXPERIMENT_MONSTERS_LAUNCH_LABEL",
                    icon="icons/specials_huge/ancient_ruins.png",
                    parameters={
                        "system": Source.SystemID,
                        "predefinedshipdesign": "SM_BLACK_KRAKEN",
                        "species": "SP_EXPERIMENTOR",
                        "rawtext": "2",
                    },
                ),
                EXPERIMENTOR_ADD_STARLANE,
            ],
        ),
        EffectsGroup(
            scope=IsSource,
            activation=(
                Turn(low=(EXPERIMENTOR_SPAWN_START_TURN + 36))
                & Random(probability=(0.1 * EXPERIMENTOR_MONSTER_FREQ_FACTOR))
            ),
            effects=[
                CreateShip(designname="SM_BLACK_KRAKEN", empire=Source.Owner),
                CreateShip(designname="SM_BLACK_KRAKEN", empire=Source.Owner),
                GenerateSitRepMessage(
                    message="EFFECT_EXPERIMENT_MONSTERS_LAUNCH",
                    label="EFFECT_EXPERIMENT_MONSTERS_LAUNCH_LABEL",
                    icon="icons/specials_huge/ancient_ruins.png",
                    parameters={
                        "system": Source.SystemID,
                        "predefinedshipdesign": "SM_BLACK_KRAKEN",
                        "species": "SP_EXPERIMENTOR",
                        "rawtext": "2",
                    },
                ),
                EXPERIMENTOR_ADD_STARLANE,
            ],
        ),
        EffectsGroup(
            scope=IsSource,
            activation=(
                Turn(low=EXPERIMENTOR_SPAWN_START_TURN + 36, high=EXPERIMENTOR_SPAWN_START_TURN + 70)
                & Random(probability=0.2 * EXPERIMENTOR_MONSTER_FREQ_FACTOR)
            ),
            effects=[
                CreateShip(designname="SM_BLOATED_JUGGERNAUT", empire=Source.Owner),
                CreateShip(designname="SM_BLOATED_JUGGERNAUT", empire=Source.Owner),
                GenerateSitRepMessage(
                    message="EFFECT_EXPERIMENT_MONSTERS_LAUNCH",
                    label="EFFECT_EXPERIMENT_MONSTERS_LAUNCH_LABEL",
                    icon="icons/specials_huge/ancient_ruins.png",
                    parameters={
                        "system": Source.SystemID,
                        "predefinedshipdesign": "SM_BLOATED_JUGGERNAUT",
                        "species": "SP_EXPERIMENTOR",
                        "rawtext": "2",
                    },
                ),
                EXPERIMENTOR_ADD_STARLANE,
            ],
        ),
        EffectsGroup(
            scope=IsSource,
            activation=(
                Turn(low=EXPERIMENTOR_SPAWN_START_TURN + 71)
                & Random(probability=0.1 * EXPERIMENTOR_MONSTER_FREQ_FACTOR)
            ),
            effects=[
                CreateShip(designname="SM_BLOATED_JUGGERNAUT", empire=Source.Owner),
                CreateShip(designname="SM_BLOATED_JUGGERNAUT", empire=Source.Owner),
                GenerateSitRepMessage(
                    message="EFFECT_EXPERIMENT_MONSTERS_LAUNCH",
                    label="EFFECT_EXPERIMENT_MONSTERS_LAUNCH_LABEL",
                    icon="icons/specials_huge/ancient_ruins.png",
                    parameters={
                        "system": Source.SystemID,
                        "predefinedshipdesign": "SM_BLOATED_JUGGERNAUT",
                        "species": "SP_EXPERIMENTOR",
                        "rawtext": "2",
                    },
                ),
                EXPERIMENTOR_ADD_STARLANE,
            ],
        ),
        EffectsGroup(
            scope=IsSource,
            activation=(
                Turn(low=(EXPERIMENTOR_SPAWN_START_TURN + 71), high=(EXPERIMENTOR_SPAWN_START_TURN + 130))
                & Random(probability=(0.2 * EXPERIMENTOR_MONSTER_FREQ_FACTOR))
            ),
            effects=[
                CreateShip(designname="SM_PSIONIC_SNOWFLAKE", empire=Source.Owner),
                CreateShip(designname="SM_PSIONIC_SNOWFLAKE", empire=Source.Owner),
                GenerateSitRepMessage(
                    message="EFFECT_EXPERIMENT_MONSTERS_LAUNCH",
                    label="EFFECT_EXPERIMENT_MONSTERS_LAUNCH_LABEL",
                    icon="icons/specials_huge/ancient_ruins.png",
                    parameters={
                        "system": Source.SystemID,
                        "predefinedshipdesign": "SM_PSIONIC_SNOWFLAKE",
                        "species": "SP_EXPERIMENTOR",
                        "rawtext": "2",
                    },
                ),
                EXPERIMENTOR_ADD_STARLANE,
            ],
        ),
        EffectsGroup(
            scope=IsSource,
            activation=(
                Turn(low=EXPERIMENTOR_SPAWN_START_TURN + 131)
                & Random(probability=0.1 * EXPERIMENTOR_MONSTER_FREQ_FACTOR)
            ),
            effects=[
                CreateShip(designname="SM_PSIONIC_SNOWFLAKE", empire=Source.Owner),
                CreateShip(designname="SM_PSIONIC_SNOWFLAKE", empire=Source.Owner),
                GenerateSitRepMessage(
                    message="EFFECT_EXPERIMENT_MONSTERS_LAUNCH",
                    label="EFFECT_EXPERIMENT_MONSTERS_LAUNCH_LABEL",
                    icon="icons/specials_huge/ancient_ruins.png",
                    parameters={
                        "system": Source.SystemID,
                        "predefinedshipdesign": "SM_PSIONIC_SNOWFLAKE",
                        "species": "SP_EXPERIMENTOR",
                        "rawtext": "2",
                    },
                ),
                EXPERIMENTOR_ADD_STARLANE,
            ],
        ),
        EffectsGroup(
            scope=IsSource,
            activation=(
                Turn(low=(EXPERIMENTOR_SPAWN_START_TURN + 131))
                & Random(probability=(0.2 * EXPERIMENTOR_MONSTER_FREQ_FACTOR))
            ),
            effects=[
                CreateShip(designname="SM_COSMIC_DRAGON", empire=Source.Owner),
                GenerateSitRepMessage(
                    message="EFFECT_EXPERIMENT_MONSTERS_LAUNCH",
                    label="EFFECT_EXPERIMENT_MONSTERS_LAUNCH_LABEL",
                    icon="icons/specials_huge/ancient_ruins.png",
                    parameters={
                        "system": Source.SystemID,
                        "predefinedshipdesign": "SM_COSMIC_DRAGON",
                        "species": "SP_EXPERIMENTOR",
                        "rawtext": "1",
                    },
                ),
                EXPERIMENTOR_ADD_STARLANE,
            ],
        ),
    ],
    icon="",
)
