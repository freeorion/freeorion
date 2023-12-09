from focs._effects import (
    Abs,
    CurrentTurn,
    EffectsGroup,
    Focus,
    IsSource,
    LocalCandidate,
    MaxOf,
    MinOf,
    Planet,
    Population,
    SetConstruction,
    SetIndustry,
    SetInfluence,
    SetResearch,
    SetStockpile,
    SetTargetConstruction,
    SetTargetIndustry,
    SetTargetInfluence,
    SetTargetResearch,
    Ship,
    Target,
    TargetPopulation,
    Value,
)
from macros.misc import DESCRIPTION_EFFECTSGROUP_MACRO
from macros.priorities import (
    AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
    END_CLEANUP_PRIORITY,
    FOCUS_CHANGE_PENALTY_PRIORITY,
    LATE_AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
)

STANDARD_METER_GROWTH = [
    # increase or decrease towards target meter of planets, when not recently conquered
    EffectsGroup(
        scope=IsSource,
        activation=Planet() & (LocalCandidate.LastTurnConquered < CurrentTurn),
        priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
        effects=[
            SetResearch(
                value=Value
                + MinOf(
                    float,
                    1.0,  # increase by (at most) 1 / turn up towards target
                    MaxOf(float, 0.0, Value(Target.Happiness)) / 5.0,  # limit rate of increase, dependent on Stability
                    MaxOf(
                        float,
                        MinOf(
                            float,
                            -1.0,  # decrease by 1 / turn down towards target, or:
                            (Value(Target.TargetResearch) - Value)
                            / 5.0,  # when (target>=value), this is larger than -1.0.  when (target<<value) this increases the rate of meter decay
                        ),
                        Value(Target.TargetResearch)
                        - Value,  # limit increase or decrease to the difference between current and target, to avoid overshooting target
                    ),
                )
            ),
            SetIndustry(
                value=Value
                + MinOf(
                    float,
                    1.0,
                    MaxOf(float, 0.0, Value(Target.Happiness)) / 5.0,
                    MaxOf(
                        float,
                        MinOf(float, -1.0, (Value(Target.TargetIndustry) - Value) / 5.0),
                        Value(Target.TargetIndustry) - Value,
                    ),
                )
            ),
            SetInfluence(
                value=Value
                + MinOf(
                    float,
                    1.0,  # increase by (at most) 1 / turn up towards target
                    MaxOf(
                        float,
                        MinOf(
                            float,
                            -1.0,  # decrease by 1 / turn down towards target, or:
                            (Value(Target.TargetInfluence) - Value)
                            / 5.0,  # when (target>=value), this is larger than -1.0.  when (target<<value) this increases the rate of meter decay
                        ),
                        Value(Target.TargetInfluence)
                        - Value,  # limit increase or decrease to the difference between current and target, to avoid overshooting target
                    ),
                )
            ),
        ],
    ),
    # increase or decrease towards target meter of ships
    EffectsGroup(
        scope=IsSource,
        activation=Ship,
        priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
        effects=[
            SetResearch(value=Value + MinOf(float, MaxOf(float, Value(Target.TargetResearch) - Value, -1), 1)),
            SetIndustry(value=Value + MinOf(float, MaxOf(float, Value(Target.TargetIndustry) - Value, -1), 1)),
            SetInfluence(value=Value + MinOf(float, MaxOf(float, Value(Target.TargetInfluence) - Value, -1), 1)),
        ],
    ),
    # removes residual production from a dead planet
    EffectsGroup(
        scope=IsSource,
        activation=Planet() & TargetPopulation(high=0) & Population(high=0.1),
        accountinglabel="DYING_POPULATION_LABEL",
        priority=END_CLEANUP_PRIORITY,
        effects=[
            SetResearch(value=0),
            SetIndustry(value=0),
            SetTargetIndustry(value=0),
            SetTargetResearch(value=0),
            SetTargetInfluence(value=0),
        ],
    ),
]

STANDARD_CONSTRUCTION = [
    EffectsGroup(
        scope=IsSource,
        activation=Planet(),
        accountinglabel="STANDARD_CONSTRUCTION_LABEL",
        effects=SetTargetConstruction(value=Value + 20),
    ),
    EffectsGroup(  # increase or decrease towards target meter, when not recently conquered
        scope=IsSource,
        activation=Planet()
        & (LocalCandidate.LastTurnConquered < CurrentTurn)
        & (LocalCandidate.LastTurnAttackedByShip < CurrentTurn),
        priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
        effects=SetConstruction(
            value=Value + MinOf(float, MaxOf(float, Value(Target.TargetConstruction) - Value, -1), 1)
        ),
    ),
    EffectsGroup(  # always ensure minimum value of one, as this is necessary for being attacked
        scope=IsSource,
        activation=Planet(),
        # has to happen after e.g. FORCE_ENERGY_STRC effects which also happens at AFTER_ALL_TARGET_MAX_METERS_PRIORITY
        priority=LATE_AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
        effects=SetConstruction(value=MaxOf(float, Value, 1)),
    ),
]

FOCUS_CHANGE_PENALTY = [
    # penalize changing focus by reducing the non-focused resource meters on the turns after the focus change.
    # these effects reduce the non-focused resource meters by 2 on the turn after a focus change.
    # eg. if changing to industry focus, if the new research target is 10 and the research is 20, research will
    # be reduced by 2 to 18. If research is 11, then it will be only reduced to the target (10).
    EffectsGroup(
        scope=IsSource,
        activation=Planet() & ~Focus(type=["FOCUS_INDUSTRY"]) & (0 == LocalCandidate.TurnsSinceFocusChange),
        priority=FOCUS_CHANGE_PENALTY_PRIORITY,
        effects=SetIndustry(
            value=Value
            - MaxOf(
                float,
                0,
                MinOf(
                    float,
                    MaxOf(float, 2, (Abs(float, Value - Value(Target.TargetIndustry))) ** 0.5),
                    Value - Value(Target.TargetIndustry),
                ),
            )
        ),
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Planet() & ~Focus(type=["FOCUS_RESEARCH"]) & (0 == LocalCandidate.TurnsSinceFocusChange),
        priority=FOCUS_CHANGE_PENALTY_PRIORITY,
        effects=SetResearch(
            value=Value
            - MaxOf(
                float,
                0,
                MinOf(
                    float,
                    MaxOf(float, 2, (Abs(float, Value - Value(Target.TargetResearch))) ** 0.5),
                    Value - Value(Target.TargetResearch),
                ),
            )
        ),
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Planet() & ~Focus(type=["FOCUS_INFLUENCE"]) & (0 == LocalCandidate.TurnsSinceFocusChange),
        priority=FOCUS_CHANGE_PENALTY_PRIORITY,
        effects=SetInfluence(
            value=Value
            - MaxOf(
                float,
                0,
                MinOf(
                    float,
                    MaxOf(float, 2, (Abs(float, Value - Value(Target.TargetInfluence))) ** 0.5),
                    Value - Value(Target.TargetInfluence),
                ),
            )
        ),
    ),
    # TODO Delete this whole Stockpile penalty clause if Stockpile is determined to remain as a Max rather than a Target type meter
    # If a decision is made to change Stockpile to a Target type meter, then simply change MaxStockpile below to TargetStockpile
    EffectsGroup(
        scope=IsSource,
        activation=Planet() & ~Focus(type=["FOCUS_STOCKPILE"]) & (0 == LocalCandidate.TurnsSinceFocusChange),
        priority=FOCUS_CHANGE_PENALTY_PRIORITY,
        effects=SetStockpile(
            value=Value
            - MaxOf(float, 0, MinOf(float, 1.0 - Target.TurnsSinceFocusChange, Value - Value(Target.MaxStockpile)))
        ),
    ),
]


# FAST_COLONIZATION and SLOW_COLONIZATION are stubs for applying the
# fast colonization building speed description to a species.
FAST_COLONIZATION = DESCRIPTION_EFFECTSGROUP_MACRO("FAST_COLONIZATION_DESC")
SLOW_COLONIZATION = DESCRIPTION_EFFECTSGROUP_MACRO("SLOW_COLONIZATION_DESC")
