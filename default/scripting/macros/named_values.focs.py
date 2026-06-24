from focs._value_refs import (
    GameRule,
    NamedInteger,
    NamedIntegerLookup,
    NamedReal,
    NumPartClassesInShipDesign,
    PartOfClassInShipDesign,
    Round,
    StaticCast,
    UsedInDesignID,
)
from macros.base_prod import RESEARCH_PER_POP
from macros.misc_pre import MIN_RECOLONIZING_HAPPINESS, MIN_RECOLONIZING_SIZE, SHIP_STRUCTURE_FACTOR
from ship_hulls.hull_structures import HULL_STRUCTURES

# Proposed naming convention: <EFFECT_NAME> _ <METER_NAME> _ FLAT/PERPOP/<others>
# Examples:
# - PLC_CONFEDERATION_TARGET_HAPPINESS_FLAT (flat bonus/malus to TargetHappiness from policy PLC_CONFEDERATION)
# - GARRISON_2_TROOPREGEN_FLAT (flat bonus/malus to troop regen from tech Garrison_2
# - GRO_CYBORGS_MAX_TROOPS_PERPOP (bonus/malus per population to MaxTroops from tech GRO_CYBORGS)

NamedReal(name="ANCIENT_RUINS_TARGET_RESEARCH_PERPOP", value=5 * RESEARCH_PER_POP)

NamedReal(name="ANCIENT_RUINS_MIN_STABILITY", value=12)

NamedReal(name="IMPERIAL_GARRISON_MAX_TROOPS_FLAT", value=6)

NamedInteger(name="XENOPHOBIC_MAX_JUMPS", value=4)

NamedReal(name="PROTECION_FOCUS_STABILITY_BONUS", value=GameRule(type=float, name="RULE_PROTECTION_FOCUS_STABILITY"))

NamedReal(name="FU_RAMSCOOP_REFUEL", value=0.4)
NamedReal(name="BLD_NEST_RESERVE_WILD_SPAWN_FACTOR", value=0.3)

NamedReal(name="SHP_REINFORCED_HULL_BONUS", value=5 * SHIP_STRUCTURE_FACTOR)

NamedInteger(name="NUM_COMBAT_ROUNDS", value=GameRule(type=int, name="RULE_NUM_COMBAT_ROUNDS"))

NamedInteger(name="NUM_COMBAT_ROUNDS_FIGHTERS", value=GameRule(type=int, name="RULE_NUM_COMBAT_ROUNDS") - 1)

NamedInteger(
    name="FIRST_COMBAT_ROUND_IN_CLOSE_TARGETING_RANGE",
    value=GameRule(type=int, name="RULE_FIRST_COMBAT_ROUND_IN_CLOSE_TARGETING_RANGE"),
)

NamedReal(name="AUGMENTATION_FULL_GROWTH_INFRASTRUCTURE_REQUIREMENT", value=40.0)

# For calculation we mostly need a real value
NamedReal(
    name="NUM_REAL_COMBAT_ROUNDS_IN_CLOSE_TARGETING_RANGE",
    value=GameRule(type=float, name="RULE_NUM_COMBAT_ROUNDS")
    - GameRule(type=float, name="RULE_FIRST_COMBAT_ROUND_IN_CLOSE_TARGETING_RANGE")
    + 1,
)

NamedInteger(name="MIN_COLONY_SIZE", value=MIN_RECOLONIZING_SIZE)

NamedInteger(name="MIN_COLONY_HAPPINESS", value=MIN_RECOLONIZING_HAPPINESS)

NamedInteger(name="MIN_MONSTER_DISTANCE", value=GameRule(type=int, name="RULE_MINIMUM_MONSTER_DISTANCE_CAPITAL") - 1)

# For reduced/increased ship cost
# value is used in upkeep.focs.txt, can be migrated to upkeep.focs.py as soon as that gets used when parsing ships
# Note: colony part class increases complexity by 4; every other classe by 1
colony_extra_complexity = 3
NamedInteger(name="DESIGN_SIMPLICITY_SOURCE_COMPLEXITY_PC_COLONY", value=1 + colony_extra_complexity)
NamedInteger(
    name="DESIGN_SIMPLICITY_SOURCE_COMPLEXITY_COUNT_VREF",
    value=NumPartClassesInShipDesign(design=UsedInDesignID)
    + PartOfClassInShipDesign(name="PC_COLONY", design=UsedInDesignID),
)

# Racial Purity

NamedReal(name="PLC_RACIAL_PURITY_TARGET_STABILITY_SELF", value=5)

NamedReal(name="PLC_RACIAL_PURITY_STABILITY_PER_TURN", value=0.5)

NamedReal(name="PLC_RACIAL_PURITY_TARGET_INFLUENCE_SELF", value=1)

NamedReal(name="PLC_RACIAL_PURITY_TARGET_STABILITY_OTHER", value=-10)

NamedReal(name="PLC_RACIAL_PURITY_TARGET_INFLUENCE_OTHER", value=-1)

NamedReal(name="PLC_RACIAL_PURITY_TARGET_INDUSTRY_OTHER", value=0.25 * RESEARCH_PER_POP)

# Spatial Flux Stealth section

SPATIAL_FLUX_STEALTH_HULL_BASE = 15
SPATIAL_FLUX_STEALTH_NON_AGGRESSIVE_BONUS = 10

NamedReal(name="SPATIAL_FLUX_STEALTH_HULL_BASE", value=SPATIAL_FLUX_STEALTH_HULL_BASE)
NamedReal(name="SPATIAL_FLUX_STEALTH_ARRIVAL_MALUS", value=30)
NamedReal(name="SPATIAL_FLUX_STEALTH_NON_AGGRESSIVE_BONUS", value=SPATIAL_FLUX_STEALTH_NON_AGGRESSIVE_BONUS)
NamedReal(
    name="SPATIAL_FLUX_STEALTH_NON_AGGRESSIVE",
    value=SPATIAL_FLUX_STEALTH_HULL_BASE + SPATIAL_FLUX_STEALTH_NON_AGGRESSIVE_BONUS,
)
NamedReal(name="SPATIAL_FLUX_STEALTH_TECH_BONUS", value=10)

# Named values for living hulls - LIVING_HULL_NAMED macro and base values come from ship_hulls/hull_structures.macros


def LIVING_HULL_NAMED(hull: str):
    NamedReal(name=hull + "_STRUCTURE", value=HULL_STRUCTURES[hull].BASE_STRUCTURE * SHIP_STRUCTURE_FACTOR)
    NamedInteger(
        name=hull + "_GROWTH_STRUCTURE",
        value=Round(int, HULL_STRUCTURES[hull].GROWTH_STRUCTURE * SHIP_STRUCTURE_FACTOR),
    )
    NamedInteger(name=hull + "_GROWTH_TURNS", value=HULL_STRUCTURES[hull].GROWTH_TURNS)
    NamedReal(
        name=hull + "_GROWTH_PER_TURN",
        value=StaticCast(float, NamedIntegerLookup(name=hull + "_GROWTH_STRUCTURE"))
        / StaticCast(float, NamedIntegerLookup(name=hull + "_GROWTH_TURNS")),
    )


LIVING_HULL_NAMED("SH_BIOADAPTIVE")
LIVING_HULL_NAMED("SH_ENDOMORPHIC")
LIVING_HULL_NAMED("SH_ENDOSYMBIOTIC")
LIVING_HULL_NAMED("SH_ORGANIC")
LIVING_HULL_NAMED("SH_PROTOPLASMIC")
LIVING_HULL_NAMED("SH_RAVENOUS")
LIVING_HULL_NAMED("SH_SENTIENT")
LIVING_HULL_NAMED("SH_SYMBIOTIC")
