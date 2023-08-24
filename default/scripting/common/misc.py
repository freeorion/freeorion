from focs._effects import (
    AnyEmpire,
    Contains,
    EffectsGroup,
    GameRule,
    IsSource,
    NamedIntegerLookup,
    NoEffect,
    OwnedBy,
    Planet,
    System,
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
UNOWNED_EMPIRE_ID = -1


MINIMUM_DISTANCE_EMPIRE_CHECK = ~WithinStarlaneJumps(
    jumps=NamedIntegerLookup(name="MIN_MONSTER_DISTANCE"),
    condition=System & Contains(Planet() & OwnedBy(affiliation=AnyEmpire)),
)
