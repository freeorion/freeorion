from focs._effects import (
    AnyEmpire,
    Contains,
    EffectsGroup,
    EmpireHasAdoptedPolicy,
    GameRule,
    IsSource,
    IsTarget,
    MaxOf,
    MinOf,
    NamedIntegerLookup,
    NamedRealLookup,
    NoEffect,
    OwnedBy,
    Planet,
    StatisticIf,
    System,
    Target,
    WithinStarlaneJumps,
)

MIN_RECOLONIZING_SIZE = 3

MIN_RECOLONIZING_HAPPINESS = 5

IMPOSSIBLY_LARGE_TURN = 2**15


def DESCRIPTION_EFFECTSGROUP_MACRO(desc: str):
    return EffectsGroup(description=desc, scope=IsSource, activation=None, effects=NoEffect)


FIGHTER_DAMAGE_FACTOR = GameRule(type=float, name="RULE_FIGHTER_DAMAGE_FACTOR")

PLANET_DEFENSE_FACTOR = GameRule(type=float, name="RULE_SHIP_WEAPON_DAMAGE_FACTOR")

PLANET_SHIELD_FACTOR = GameRule(type=float, name="RULE_SHIP_STRUCTURE_FACTOR")

SHIP_WEAPON_DAMAGE_FACTOR = GameRule(type=float, name="RULE_SHIP_WEAPON_DAMAGE_FACTOR")

SHIP_SHIELD_FACTOR = GameRule(type=float, name="RULE_SHIP_WEAPON_DAMAGE_FACTOR")

SHIP_STRUCTURE_FACTOR = GameRule(type=float, name="RULE_SHIP_STRUCTURE_FACTOR")

SYSTEM_MINES_DAMAGE_FACTOR = GameRule(type=float, name="RULE_SHIP_STRUCTURE_FACTOR")

SUPPLY_DISCONNECTED_INFLUENCE_MALUS = 1

# empire id used for unowned planets/ships - as defined in Universe.cpp(?)
UNOWNED_EMPIRE_ID = -1  # type: ignore[assignment]


MINIMUM_DISTANCE_EMPIRE_CHECK = ~WithinStarlaneJumps(
    jumps=NamedIntegerLookup(name="MIN_MONSTER_DISTANCE"),
    condition=System & Contains(Planet() & OwnedBy(affiliation=AnyEmpire)),
)

# Range from -0.5(1/2 population growth) to +1(double population growth)
AUGMENTATION_GROWTH_MODIFIER = (
    MinOf(
        float,
        2,
        MaxOf(
            float,
            0.5,
            Target.Construction / NamedRealLookup(name="AUGMENTATION_FULL_GROWTH_INFRASTRUCTURE_REQUIREMENT"),
        ),
    )
    - 1
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
        * AUGMENTATION_GROWTH_MODIFIER
    )
)
