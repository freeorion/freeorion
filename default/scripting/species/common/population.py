from common.priorities import (
    POPULATION_FIRST_PRIORITY,
    TARGET_POPULATION_AFTER_SCALING_PRIORITY,
    TARGET_POPULATION_BEFORE_SCALING_PRIORITY,
    TARGET_POPULATION_LAST_BEFORE_OVERRIDE_PRIORITY,
    TARGET_POPULATION_OVERRIDE_PRIORITY,
    TARGET_POPULATION_SCALING_PRIORITY,
)
from species.common.advanced_focus import ADVANCED_FOCUS_EFFECTS
from species.common.general import (
    FOCUS_CHANGE_PENALTY,
    STANDARD_CONSTRUCTION,
    STANDARD_METER_GROWTH,
)

GROWTH_RATE_FACTOR = (
    0.1
    * (
        1 - StatisticIf(float, condition=IsTarget & EmpireHasAdoptedPolicy(name="PLC_NO_GROWTH"))
    )  # no growth with no-growth policy
    * (
        1 + 0.5 * StatisticIf(float, condition=IsTarget & EmpireHasAdoptedPolicy(name="PLC_POPULATION"))
    )  # +50% growth with population policy
    * (
        1
        + StatisticIf(float, condition=IsTarget & EmpireHasAdoptedPolicy(name="PLC_AUGMENTATION"))
        # slower growth with augmentation on low-infrastructure planets
        * MinOf(
            float,
            1.2,
            MaxOf(
                float,
                0.5,
                Target.Construction / NamedRealLookup(name="AUGMENTATION_FULL_GROWTH_INFRASTRUCTURE_REQUIREMENT"),
            ),
        )
    )
)

HOMEWORLD_BONUS_POPULATION = EffectsGroup(
    scope=IsSource & Homeworld(name=[Source.Species]),
    activation=Planet(),
    stackinggroup="HOMEWORLD_STACK",
    accountinglabel="HOMEWORLD_BONUS",
    priority=TARGET_POPULATION_AFTER_SCALING_PRIORITY,
    effects=SetTargetPopulation(value=Value + 2 * Target.HabitableSize),
)

# reduces max population in systems with bright stars
LIGHT_SENSITIVE = [
    EffectsGroup(
        description="LIGHT_SENSITIVE_DESC",
        scope=IsSource,
        activation=Planet() & Star(type=[Blue]),
        accountinglabel="VERY_BRIGHT_STAR",
        priority=TARGET_POPULATION_AFTER_SCALING_PRIORITY,
        effects=SetTargetPopulation(value=Value - 2 * Source.HabitableSize),
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Planet() & Star(type=[White]),
        accountinglabel="BRIGHT_STAR",
        priority=TARGET_POPULATION_AFTER_SCALING_PRIORITY,
        effects=SetTargetPopulation(value=Value - Source.HabitableSize),
    ),
]

# Implements environmental modifiers from Growth techs.
# Changes to the growth tree should take this into account.

ENVIRONMENT_MODIFIER = [
    EffectsGroup(
        scope=IsSource,
        activation=Planet() & Planet(environment=[Uninhabitable]),
        accountinglabel="UNINHABTIABLE_ENVIRONMENT_LABEL",
        priority=TARGET_POPULATION_OVERRIDE_PRIORITY,
        effects=SetTargetPopulation(value=-999),
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Planet() & Planet(environment=[Hostile]),
        accountinglabel="HOSTILE_ENVIRONMENT_LABEL",
        priority=TARGET_POPULATION_BEFORE_SCALING_PRIORITY,
        effects=SetTargetPopulation(value=Value - 4 * Source.HabitableSize),
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Planet() & Planet(environment=[Poor]),
        accountinglabel="POOR_ENVIRONMENT_LABEL",
        priority=TARGET_POPULATION_BEFORE_SCALING_PRIORITY,
        effects=SetTargetPopulation(value=Value - 2 * Source.HabitableSize),
    ),
    #           EffectsGroup
    #             scope = Source
    #             activation = And [
    #                 Planet
    #                 Planet environment = Adequate
    #             ]
    #             accountinglabel = "ADEQUATE_ENVIRONMENT_LABEL"
    #             priority = [[TARGET_POPULATION_BEFORE_SCALING_PRIORITY]]
    #             effects = SetTargetPopulation value = Value + 0 * Source.HabitableSize
    EffectsGroup(
        scope=IsSource,
        activation=Planet(environment=[Good]),
        accountinglabel="GOOD_ENVIRONMENT_LABEL",
        priority=TARGET_POPULATION_BEFORE_SCALING_PRIORITY,
        effects=SetTargetPopulation(value=Value + 3 * Target.HabitableSize),
    ),
]

# This is dependent on current placement in population effects calc,
# just after Homeworld and Environment
SELF_SUSTAINING_BONUS = EffectsGroup(
    scope=IsSource,
    activation=Planet(environment=[Good]) & HasTag(name="SELF_SUSTAINING"),
    accountinglabel="SELF_SUSTAINING_LABEL",
    priority=TARGET_POPULATION_AFTER_SCALING_PRIORITY,
    effects=SetTargetPopulation(value=Value + 3 * Target.HabitableSize),  # Gets the same bonus as three growth specials
)

PHOTOTROPHIC_BONUS = [
    EffectsGroup(
        scope=Contains(IsSource),
        activation=Planet()
        & (
            OwnedBy(affiliation=AnyEmpire)
            | Star(type=[BlackHole, NoStar])  # Natives are allowed to have bright or dim stars
        )
        & HasTag(name="PHOTOTROPHIC")
        & Turn(high=0)
        & ~Star(type=[Orange]),
        effects=SetStarType(type=Yellow),  # start with a normal star to be balanced
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Planet() & HasTag(name="PHOTOTROPHIC") & Star(type=[Blue]) & TargetPopulation(low=0),
        accountinglabel="VERY_BRIGHT_STAR",
        priority=TARGET_POPULATION_LAST_BEFORE_OVERRIDE_PRIORITY,
        effects=SetTargetPopulation(value=Value + 3 * Source.HabitableSize),
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Planet() & HasTag(name="PHOTOTROPHIC") & Star(type=[White]) & TargetPopulation(low=0),
        accountinglabel="BRIGHT_STAR",
        priority=TARGET_POPULATION_LAST_BEFORE_OVERRIDE_PRIORITY,
        effects=SetTargetPopulation(value=Value + 1.5 * Source.HabitableSize),
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Planet() & HasTag(name="PHOTOTROPHIC") & Star(type=[Red, Neutron]),
        accountinglabel="DIM_STAR",
        priority=TARGET_POPULATION_LAST_BEFORE_OVERRIDE_PRIORITY,
        effects=SetTargetPopulation(value=Value - 1 * Source.HabitableSize),
    ),
    EffectsGroup(
        scope=IsSource,
        activation=Planet() & HasTag(name="PHOTOTROPHIC") & Star(type=[BlackHole, NoStar]),
        accountinglabel="NO_STAR",
        priority=TARGET_POPULATION_LAST_BEFORE_OVERRIDE_PRIORITY,
        effects=SetTargetPopulation(value=Value - 10 * Source.HabitableSize),
    ),
]

GASEOUS_BONUS = EffectsGroup(
    scope=IsSource,
    activation=Planet(type=[GasGiantType]) & HasTag(name="GASEOUS"),
    accountinglabel="GASEOUS_LABEL",
    priority=TARGET_POPULATION_SCALING_PRIORITY,
    effects=SetTargetPopulation(value=Value - 0.5 * Abs(float, Value)),
)

HOMEWORLD_GROWTH_FOCUS_BOOST = EffectsGroup(
    scope=Planet()
    & OwnedBy(empire=Source.Owner)
    & HasSpecies(name=[Source.Species])
    & ~Homeworld(name=[Source.Species])
    & ResourceSupplyConnected(empire=Source.Owner, condition=IsSource),
    activation=Planet() & Focus(type=["FOCUS_GROWTH"]) & Homeworld(),
    stackinggroup="HOMEWORLD_STACK",
    accountinglabel="HOMEWORLD_SUPPLY",
    priority=TARGET_POPULATION_AFTER_SCALING_PRIORITY,
    effects=SetTargetPopulation(value=Value + 1 * Target.HabitableSize),
)

BASIC_POPULATION = [
    HOMEWORLD_BONUS_POPULATION,
    *ENVIRONMENT_MODIFIER,
    SELF_SUSTAINING_BONUS,
    *PHOTOTROPHIC_BONUS,
    GASEOUS_BONUS,
    HOMEWORLD_GROWTH_FOCUS_BOOST,
    # population growth or decay towards to target population
    EffectsGroup(
        scope=IsSource,
        activation=Planet() & (LocalCandidate.LastTurnConquered < CurrentTurn),
        priority=POPULATION_FIRST_PRIORITY,
        effects=SetPopulation(
            # growth is GROWTH_RATE_FACTOR * population * (maximum + 1 - population) / maximum
            # which gives factor for population 1, best growth around half full and aproaching
            # factor again when almost full
            value=(
                (Value < Value(Target.TargetPopulation))
                * MinOf(
                    float,
                    Value(Target.TargetPopulation),
                    Value + Value * GROWTH_RATE_FACTOR * (1 + (1 - Value) / Value(Target.TargetPopulation)),
                )
            )
            # shrink if overpopulated
            + (
                (Value >= Value(Target.TargetPopulation))
                * MaxOf(float, Value(Target.TargetPopulation), Value - 0.1 * (Value - Value(Target.TargetPopulation)))
            )
        ),
    ),
    # Since all species have the same advanced focus effects and
    # infrastructure, the macros are stashed here, where they don't
    # need to be manually included in each species' macros.
    *FOCUS_CHANGE_PENALTY,
    *ADVANCED_FOCUS_EFFECTS,
    *STANDARD_CONSTRUCTION,
    *STANDARD_METER_GROWTH,
]

AVERAGE_POPULATION = BASIC_POPULATION
