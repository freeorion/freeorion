from common.base_prod import STOCKPILE_PER_POP
from common.priorities import (
    AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
    TARGET_EARLY_BEFORE_SCALING_PRIORITY,
)


def STOCKPILE_PER_POP_EFFECTSGROUP__SNIP(label: str, value) -> dict:
    return {
        "scope": Source,
        "activation": Planet(),
        "accountinglabel": label + "_STOCKPILE_LABEL",
        "priority": TARGET_EARLY_BEFORE_SCALING_PRIORITY,
        "effects": SetMaxStockpile(value=value),
    }


STANDARD_STOCKPILE = [
    EffectsGroup(  # increase or decrease towards target meter, when not recently conquered
        scope=Source,
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
        scope=ProductionCenter
        & OwnedBy(empire=Source.Owner)
        & HasSpecies(name=[Source.Species])
        & ~Homeworld(name=[Source.Species]),
        activation=Planet() & Focus(type=["FOCUS_STOCKPILE"]) & Homeworld(),
        stackinggroup="HOMEWORLD_STOCKPILE_FOCUS_BONUS_LABEL",
        accountinglabel="HOMEWORLD_STOCKPILE_FOCUS_BONUS_LABEL",
        effects=SetMaxStockpile(value=(Value + 2 * Target.Population * STOCKPILE_PER_POP)),
    ),
    # removes residual stockpile capacity from a dead planet
    EffectsGroup(scope=Source, activation=Planet() & TargetPopulation(high=0), effects=SetStockpile(value=0)),
]

AVERAGE_STOCKPILE = [
    EffectsGroup(
        # Skip the AVERAGE_STOCKPILE_DESC, same as for the other *_STOCKPILE macros
        **STOCKPILE_PER_POP_EFFECTSGROUP__SNIP("AVERAGE", Value + 1 * Target.Population * STOCKPILE_PER_POP)
    ),
    *STANDARD_STOCKPILE,
]
