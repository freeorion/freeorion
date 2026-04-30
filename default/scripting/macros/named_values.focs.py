from focs._effects import (
    GameRule,
    NamedInteger,
    NamedReal,
    UsedInDesignID,
)
from focs._value_refs import NumPartClassesInShipDesign, PartOfClassInShipDesign
from macros.base_prod import RESEARCH_PER_POP
from macros.misc import MIN_RECOLONIZING_HAPPINESS, MIN_RECOLONIZING_SIZE, SHIP_STRUCTURE_FACTOR

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
