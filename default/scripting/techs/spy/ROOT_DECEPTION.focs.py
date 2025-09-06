from focs._effects import (
    BlackHole,
    EffectsGroup,
    InSystem,
    Min,
    NamedReal,
    Neutron,
    NoOpCondition,
    NoStar,
    OwnedBy,
    Red,
    SetSpecialCapacity,
    SetStealth,
    Ship,
    Source,
    SpecialCapacity,
    Star,
    Statistic,
    StatisticCount,
    StatisticIf,
    Stealth,
    Target,
    Value,
)
from focs._tech import *
from macros.priorities import (
    AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
    EARLY_AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
    LATE_AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
)

lower_stealth_count_special = "LOWER_STEALTH_COUNT_SPECIAL"
base_stealth_special = "BASE_STEALTH_SPECIAL"

def count_lower_stealth_ships_statistic_valref():
    return StatisticCount(
        float,
        condition=Ship
        & InSystem(id=Target.SystemID)
        & OwnedBy(empire=Source.Owner)
        & (Value(Target.Stealth) >= Value(LocalCandidate.Stealth)),
    )

target_has_lower_stealth_cond = (
    Ship
    & InSystem(id=Target.SystemID)
    & OwnedBy(empire=Source.Owner)
    & (Value(Target.Stealth) < Value(LocalCandidate.Stealth))
)

target_has_less_or_equal_stealth_cond = (
    Ship
    & InSystem(id=Target.SystemID)
    & OwnedBy(empire=Source.Owner)
    & (SpecialCapacity(name=base_stealth_special,object=Target.ID) <= SpecialCapacity(name=base_stealth_special,object=LocalCandidate.ID)))

target_has_at_least_stealth_cond = (
    Ship
    & InSystem(id=Target.SystemID)
    & OwnedBy(empire=Source.Owner)
    #& (Value(Target.Stealth) >= Value(LocalCandidate.Stealth))
    & (SpecialCapacity(name=base_stealth_special,object=Target.ID) >= SpecialCapacity(name=base_stealth_special,object=LocalCandidate.ID))
)

target_has_more_stealth_cond = (
    Ship
    & InSystem(id=Target.SystemID)
    & OwnedBy(empire=Source.Owner)
    #& (Value(Target.Stealth) > Value(LocalCandidate.Stealth))
    & (SpecialCapacity(name=base_stealth_special,object=Target.ID) > SpecialCapacity(name=base_stealth_special,object=LocalCandidate.ID))
)

bla = "bla"

 # setting highest base stealth ships, always has ( base_stealth - unstealthiness )
def min_effective_stealth_of_more_stealthy_ships_valref():
    return (StatisticIf(float, condition=target_has_less_or_equal_stealth_cond)
            * MinOf(float, Statistic(
                float,
                Min,
                value= SpecialCapacity(name=base_stealth_special, object=LocalCandidate.ID)
                - SpecialCapacity(name=lower_stealth_count_special, object=LocalCandidate.ID),
                condition=target_has_lower_stealth_cond&NoOpCondition,
            ),
            (SpecialCapacity(name=base_stealth_special,object=Target.ID) - SpecialCapacity(name=lower_stealth_count_special,object=LocalCandidate.ID))
            )
    )
    + (StatisticIf(float, condition=target_has_at_least_stealth_cond)*1000)


Tech(
    name="SPY_ROOT_DECEPTION",
    description="SPY_ROOT_DECEPTION_DESC",
    short_description="THEORY_SHORT_DESC",
    category="SPY_CATEGORY",
    researchcost=1,
    researchturns=1,
    tags=["PEDIA_SPY_CATEGORY", "THEORY"],
    effectsgroups=[
        EffectsGroup(
            scope=Ship & OwnedBy(empire=Source.Owner) & Star(type=[NoStar]),
            accountinglabel="SPY_DECEPTION_EMPTY_SPACE_PENALTY",
            effects=SetStealth(value=Value + NamedReal(name="SPY_DECEPTION_EMPTY_SPACE_PENALTY", value=-10.0)),
        ),
        EffectsGroup(
            scope=Ship & OwnedBy(empire=Source.Owner) & Star(type=[Red]),
            accountinglabel="SPY_DECEPTION_DIM_STAR_PENALTY",
            effects=SetStealth(value=Value + NamedReal(name="SPY_DECEPTION_DIM_STAR_PENALTY", value=-5.0)),
        ),
        EffectsGroup(
            scope=Ship & OwnedBy(empire=Source.Owner) & Star(type=[Neutron]),
            accountinglabel="SPY_DECEPTION_SUBSTELLAR_INTERFERENCE",
            effects=SetStealth(value=Value + NamedReal(name="SPY_DECEPTION_NEUTRON_INTERFERENCE", value=5.0)),
        ),
        EffectsGroup(
            scope=Ship & OwnedBy(empire=Source.Owner) & Star(type=[BlackHole]),
            accountinglabel="SPY_DECEPTION_SUBSTELLAR_INTERFERENCE",
            effects=SetStealth(value=Value + NamedReal(name="SPY_DECEPTION_BLACK_INTERFERENCE", value=10.0)),
        ),
        # temporarily note amount of fleet unstealthiness before capping
        EffectsGroup(
            scope=Ship & InSystem() & OwnedBy(empire=Source.Owner),
            priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
            effects=[
                SetSpecialCapacity(
                    name=lower_stealth_count_special, capacity=count_lower_stealth_ships_statistic_valref()
                ),
                AddSpecial(name=base_stealth_special,capacity=Value(Target.Stealth)),
            ]
        ),
        EffectsGroup(scope=All, priority=EARLY_AFTER_ALL_TARGET_MAX_METERS_PRIORITY, effects=[
            RemoveSpecial(name="INDEPENDENT_COLONY_TROOPS"),
            RemoveSpecial(name="INDEPENDENT_COLONY_SHIELDS_SPECIAL"),
            RemoveSpecial(name="INDEPENDENT_COLONY_TROOPS_SPECIAL"),
            RemoveSpecial(name="INDEPENDENT_COLONY_SHIELD_SPECIAL"),
            ]
        ),
        # apply the lowest resulting stealth of ships of higher/equal stealth
        EffectsGroup(
            scope=Ship & InSystem() & OwnedBy(empire=Source.Owner),
            accountinglabel="FLEET_UNSTEALTHINESS",
            priority=LATE_AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
            effects=[
                AddSpecial(name="INDEPENDENT_COLONY_TROOPS_SPECIAL",capacity=min_effective_stealth_of_more_stealthy_ships_valref()),
                SetStealth(value=min_effective_stealth_of_more_stealthy_ships_valref()),
                AddSpecial(name="INDEPENDENT_COLONY_SHIELD_SPECIAL",capacity=min_effective_stealth_of_more_stealthy_ships_valref()),
                SetShield(value=Value+min_effective_stealth_of_more_stealthy_ships_valref()),
            ]
        ),
        # Do test a) ships going via different starlanes to/from the same system
        EffectsGroup(
            scope=Ship & ~InSystem() & OwnedBy(empire=Source.Owner),
            accountinglabel="FLEET_UNSTEALTHINESS",
            effects=[
                SetStealth(
                    value=Value
                    - StatisticCount(
                        float,
                        condition=Ship
                        & ~InSystem()
                        & Stealth(high=Target.Stealth)
                        & (
                            (
                                (LocalCandidate.Fleet.NextSystemID == Target.Fleet.NextSystemID)
                                & (LocalCandidate.Fleet.PreviousSystemID == Target.Fleet.PreviousSystemID)
                            )
                            | (
                                (LocalCandidate.Fleet.NextSystemID == Target.Fleet.PreviousSystemID)
                                & (LocalCandidate.Fleet.PreviousSystemID == Target.Fleet.NextSystemID)
                            )
                        )
                        & OwnedBy(empire=Source.Owner),
                    )
                ),
            ],
        ),
    ],
)
