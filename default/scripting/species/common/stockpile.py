from common.base_prod import STOCKPILE_PER_POP
from common.priorities import (
    AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
    TARGET_EARLY_BEFORE_SCALING_PRIORITY,
)
from focs._effects import (
    Abs,
    CurrentTurn,
    EffectsGroup,
    Focus,
    HasSpecies,
    Homeworld,
    IsSource,
    LocalCandidate,
    MinOf,
    OwnedBy,
    Planet,
    SetMaxStockpile,
    SetStockpile,
    Source,
    Target,
    TargetPopulation,
    Value,
)


def STOCKPILE_PER_POP_EFFECTSGROUP__SNIP(label: str, value) -> dict:
    return {
        "scope": IsSource,
        "activation": Planet(),
        "accountinglabel": label + "_STOCKPILE_LABEL",
        "priority": TARGET_EARLY_BEFORE_SCALING_PRIORITY,
        "effects": SetMaxStockpile(value=value),
    }


def STOCKPILE_PER_POP_EFFECTSGROUP(label: str, value):
    return EffectsGroup(description=label + "_STOCKPILE_DESC", **STOCKPILE_PER_POP_EFFECTSGROUP__SNIP(label, value))


NO_STOCKPILE = []

STANDARD_STOCKPILE = [
    EffectsGroup(  # increase or decrease towards target meter, when not recently conquered
        scope=IsSource,
        activation=Planet()
        & (LocalCandidate.LastTurnConquered < CurrentTurn)
        & (LocalCandidate.System.LastTurnBattleHere < CurrentTurn),
        priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
        effects=SetStockpile(
            value=Value
            + MinOf(float, Abs(float, Value(Target.MaxStockpile) - Value), 1)
            * (1 - 2 * (Value > Value(Target.MaxStockpile)))
        ),
    ),
    # increase stockpile for species if Homeworld is set to stockpile focus
    EffectsGroup(
        scope=Planet()
        & OwnedBy(empire=Source.Owner)
        & HasSpecies(name=[Source.Species])
        & ~Homeworld(name=[Source.Species]),
        activation=Planet() & Focus(type=["FOCUS_STOCKPILE"]) & Homeworld(),
        stackinggroup="HOMEWORLD_STOCKPILE_FOCUS_BONUS_LABEL",
        accountinglabel="HOMEWORLD_STOCKPILE_FOCUS_BONUS_LABEL",
        effects=SetMaxStockpile(value=(Value + 2 * Target.Population * STOCKPILE_PER_POP)),
    ),
    # removes residual stockpile capacity from a dead planet
    EffectsGroup(scope=IsSource, activation=Planet() & TargetPopulation(high=0), effects=SetStockpile(value=0)),
]

BAD_STOCKPILE = [
    STOCKPILE_PER_POP_EFFECTSGROUP("BAD", Value + 0.5 * Target.Population * STOCKPILE_PER_POP),
    *STANDARD_STOCKPILE,
]


AVERAGE_STOCKPILE = [
    EffectsGroup(
        # Skip the AVERAGE_STOCKPILE_DESC, same as for the other *_STOCKPILE macros
        **STOCKPILE_PER_POP_EFFECTSGROUP__SNIP("AVERAGE", Value + 1 * Target.Population * STOCKPILE_PER_POP)
    ),
    *STANDARD_STOCKPILE,
]


GOOD_STOCKPILE = [
    STOCKPILE_PER_POP_EFFECTSGROUP("GOOD", Value + 3 * Target.Population * STOCKPILE_PER_POP),
    *STANDARD_STOCKPILE,
]


GREAT_STOCKPILE = [
    STOCKPILE_PER_POP_EFFECTSGROUP("GREAT", Value + 10 * Target.Population * STOCKPILE_PER_POP),
    *STANDARD_STOCKPILE,
]


ULTIMATE_STOCKPILE = [
    STOCKPILE_PER_POP_EFFECTSGROUP("ULTIMATE", Value + 15 * Target.Population * STOCKPILE_PER_POP),
    *STANDARD_STOCKPILE,
]
