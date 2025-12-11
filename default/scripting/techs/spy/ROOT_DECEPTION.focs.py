from focs._effects import (
    AddSpecial,
    BlackHole,
    EffectsGroup,
    InGame,
    InSystem,
    IsTarget,
    LocalCandidate,
    Min,
    MinOf,
    NamedReal,
    Neutron,
    NoOpValue,
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
    StatisticElse,
    Target,
    Value,
)
from focs._tech import *
from macros.priorities import (
    AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
    LATE_AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
)

lower_stealth_count_special = "LOWER_STEALTH_COUNT_SPECIAL"
base_stealth_special = "BASE_STEALTH_SPECIAL"


def count_lower_stealth_ships_statistic_valref(base_cond):
    return StatisticCount(
        float,
        condition=base_cond & (Value(Target.Stealth) >= Value(LocalCandidate.Stealth)),
    )


def target_has_less_stealth_cond(base_cond):
    return base_cond & (Value(Target.Stealth) < Value(LocalCandidate.Stealth))


own_ships_in_targetz_system = Ship & InSystem(id=Target.SystemID) & OwnedBy(empire=Source.Owner)
other_own_ships_in_targetz_system = Ship & InSystem(id=Target.SystemID) & ~IsTarget & OwnedBy(empire=Source.Owner)
own_ships_on_targetz_starlane = (
    Ship
    & ~InSystem()
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
    & OwnedBy(empire=Source.Owner)
)


# works: there are ships which have more stealth than the candidate
def candidate_has_less_stealth_cond(base_cond):
    return base_cond & (
        SpecialCapacity(name=base_stealth_special, object=Target.ID)
        < SpecialCapacity(name=base_stealth_special, object=LocalCandidate.ID)
    )


def stealth_result(obj, debug=False):
    if debug is True:
        return NoOpValue(
            float,
            NoOpValue(float, SpecialCapacity(name=base_stealth_special, object=obj))
            - NoOpValue(float, SpecialCapacity(name=lower_stealth_count_special, object=obj)),
        )
    else:
        return SpecialCapacity(name=base_stealth_special, object=obj) - SpecialCapacity(
            name=lower_stealth_count_special, object=obj
        )


#    iff the target does have the maximum stealth of all base_cond matches this returns 0
def min_effective_stealth_of_more_stealthy_ships_valref_for_not_max_stealth_ships(base_cond):
    return MinOf(
        float,
        Statistic(
            float,
            Min,
            value=stealth_result(LocalCandidate.ID),
            condition=target_has_less_stealth_cond(base_cond),
        ),
        (
            SpecialCapacity(name=base_stealth_special, object=Target.ID)
            - SpecialCapacity(name=lower_stealth_count_special, object=Target.ID)
        ),
    )


# returns a valueref<float> of target stealth resulting from perfect ignorance linear unstealthiness given the base_cond
#    normal linear unstealthiness reduces a target stealth by the count of ships which do not have higher stealth
#    this results in a stealth decrease which does not leak information about unseen higher-stealth ships
#    if there are a lot higher-stealth ships, normal linear unstealthiness would lead to the higher-stealth ships ending with lower stealth
#    perfect ignorance linear unstealthiness solves this weirdness by lowering stealth to the lowest stealth of initially-higher stealth ships
def min_effective_stealth_of_more_stealthy_ships_valref(base_cond):
    return StatisticElse(float, condition=candidate_has_less_stealth_cond(base_cond)) * stealth_result(
        Target.ID
    ) + min_effective_stealth_of_more_stealthy_ships_valref_for_not_max_stealth_ships(base_cond)


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
                    name=lower_stealth_count_special,
                    capacity=count_lower_stealth_ships_statistic_valref(own_ships_in_targetz_system),
                ),
                AddSpecial(name=base_stealth_special, capacity=Value(Target.Stealth)),
            ],
        ),
        # Check InGame(), this should not trigger in e.g. ShipDesigner (where the ship is ~InSystem). No meter effects - not strictly necessary
        EffectsGroup(
            scope=Ship & InGame() & ~InSystem() & OwnedBy(empire=Source.Owner),
            priority=AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
            effects=[
                SetSpecialCapacity(
                    name=lower_stealth_count_special,
                    capacity=count_lower_stealth_ships_statistic_valref(own_ships_on_targetz_starlane),
                ),
                AddSpecial(name=base_stealth_special, capacity=Value(Target.Stealth)),
            ],
        ),
        # apply the lowest resulting stealth of ships of higher/equal stealth
        EffectsGroup(
            scope=Ship & InSystem() & OwnedBy(empire=Source.Owner),
            accountinglabel="FLEET_UNSTEALTHINESS_INSYSTEM_LABEL",
            priority=LATE_AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
            effects=[
                SetStealth(
                    value=min_effective_stealth_of_more_stealthy_ships_valref(other_own_ships_in_targetz_system)
                ),
            ],
        ),
        # Needs InGame(), this should not trigger in e.g. ShipDesigner (where the ship is ~InSystem).
        # Do test a) ships going via different starlanes to/from the same system
        EffectsGroup(
            scope=Ship & InGame() & ~InSystem() & OwnedBy(empire=Source.Owner),
            accountinglabel="FLEET_UNSTEALTHINESS_ON_STARLANE_LABEL",
            priority=LATE_AFTER_ALL_TARGET_MAX_METERS_PRIORITY,
            effects=[
                SetStealth(
                    value=min_effective_stealth_of_more_stealthy_ships_valref(own_ships_on_targetz_starlane & ~IsTarget)
                ),
            ],
        ),
    ],
)
