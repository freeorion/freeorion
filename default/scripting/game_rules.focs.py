# comments hold info about internal engine rules defined in c++ side, but this was manually created in some effort to bring the two together
# and holds no power over what is on the C++ side
# for proper ordering ranks need to make sense together with GameRuleRanks.h
# categories are ranked by definitions in GameRuleCategories.h - add new category there in expected order, along with stringtable entry


def count(start=0, step=1):
    n = start
    while True:
        yield n
        n += step


#########################################
# Category "" aka "GENERAL"
#########################################
# -> only internal rules so far
#   internal RULE_RESEED_PRNG_SERVER
#   internal RULE_EXTRASOLAR_SHIP_DETECTION
#   internal RULE_BASIC_VIS_SYSTEM_INFO_SHOWN
#   internal RULE_NUM_COMBAT_ROUNDS
#   internal RULE_AGGRESSIVE_SHIPS_COMBAT_VISIBLE
#   internal RULE_STOCKPILE_IMPORT_LIMITED
#   internal RULE_PRODUCTION_QUEUE_FRONTLOAD_FACTOR
#   internal RULE_PRODUCTION_QUEUE_TOPPING_UP_FACTOR


#########################################
# Category "CONTENT"
#########################################
ranker = count(start=100, step=100)

GameRule(
    name="RULE_ALLOW_REPEATED_SPECIES",
    description="RULE_ALLOW_REPEATED_SPECIES_DESC",
    category="CONTENT",
    type=bool,
    default=False,
    rank=next(ranker),
)

GameRule(
    name="RULE_ENSURE_HABITABLE_PLANET_HW_VICINITY",
    description="RULE_ENSURE_HABITABLE_PLANET_HW_VICINITY_DESC",
    category="CONTENT",
    type=bool,
    default=False,
    rank=next(ranker),
)

GameRule(
    name="RULE_ENABLE_EXPERIMENTORS",
    description="RULE_ENABLE_EXPERIMENTORS_DESC",
    category="CONTENT",
    type=bool,
    default=True,
    rank=next(ranker),
)

GameRule(
    name="RULE_EXPERIMENTORS_SPAWN_BASE_TURN",
    description="RULE_EXPERIMENTORS_SPAWN_BASE_TURN_DESC",
    category="CONTENT",
    type=int,
    default=250,
    min=0,
    max=500,
    rank=next(ranker),
)

GameRule(
    name="RULE_NESTS_ALWAYS_SPAWN_WILD",
    description="RULE_NESTS_ALWAYS_SPAWN_WILD_DESC",
    category="CONTENT",
    type=bool,
    default=False,
    rank=next(ranker),
)

GameRule(
    name="RULE_WANDERING_MONSTERS_CAN_STOP_INSYSTEM",
    description="RULE_WANDERING_MONSTERS_CAN_STOP_INSYSTEM_DESC",
    category="CONTENT",
    type=bool,
    default=False,
    rank=next(ranker),
)

GameRule(
    name="RULE_WILD_NEST_MONSTER_SPAWN_FACTOR",
    description="RULE_WILD_NEST_MONSTER_SPAWN_FACTOR_DESC",
    category="CONTENT",
    type=float,
    default=1.0,
    min=0.0,
    max=10.0,
    rank=next(ranker),
)

GameRule(
    name="RULE_DOMESTIC_NEST_MONSTER_SPAWN_FACTOR",
    description="RULE_DOMESTIC_NEST_MONSTER_SPAWN_FACTOR_DESC",
    category="CONTENT",
    type=float,
    default=1.0,
    min=0.0,
    max=10.0,
    rank=next(ranker),
)

#########################################
# Category "BALANCE"
#########################################
ranker = count(start=100, step=100)

GameRule(
    name="RULE_SHIP_HULL_COST_FACTOR",
    description="RULE_SHIP_HULL_COST_FACTOR_DESC",
    category="BALANCE",
    type=float,
    default=1.0,
    min=0.1,
    max=10.0,
    rank=next(ranker),
)

GameRule(
    name="RULE_SHIP_PART_COST_FACTOR",
    description="RULE_SHIP_PART_COST_FACTOR_DESC",
    category="BALANCE",
    type=float,
    default=1.0,
    min=0.1,
    max=10.0,
    rank=next(ranker),
)

GameRule(
    name="RULE_BUILDING_COST_FACTOR",
    description="RULE_BUILDING_COST_FACTOR_DESC",
    category="BALANCE",
    type=float,
    default=1.0,
    min=0.1,
    max=10.0,
    rank=next(ranker),
)

GameRule(
    name="RULE_TECH_COST_FACTOR",
    description="RULE_TECH_COST_FACTOR_DESC",
    category="BALANCE",
    type=float,
    default=1.0,
    min=0.1,
    max=10.0,
    rank=next(ranker),
)

GameRule(
    name="RULE_SINGULARITY_COST_FACTOR",
    description="RULE_SINGULARITY_COST_FACTOR_DESC",
    category="BALANCE",
    type=float,
    default=1.0,
    min=0.1,
    max=10.0,
    rank=next(ranker),
)

GameRule(
    name="RULE_MINIMUM_MONSTER_DISTANCE_CAPITAL",
    description="RULE_MINIMUM_MONSTER_DISTANCE_CAPITAL_DESC",
    category="BALANCE",
    type=int,
    default=3,
    min=1,
    max=10,
    rank=next(ranker),
)

GameRule(
    name="RULE_SHIP_PART_BASED_UPKEEP",
    description="RULE_SHIP_PART_BASED_UPKEEP_DESC",
    category="BALANCE",
    type=bool,
    default=False,
    rank=next(ranker),
)

GameRule(
    name="RULE_FIRST_COMBAT_ROUND_IN_CLOSE_TARGETING_RANGE",
    description="RULE_FIRST_COMBAT_ROUND_IN_CLOSE_TARGETING_RANGE_DESC",
    category="BALANCE",
    type=int,
    default=3,
    min=1,
    max=20,
    rank=next(ranker),
)

# internal RULE_SHIP_WEAPON_DAMAGE_FACTOR rank = 1100,
# internal RULE_FIGHTER_DAMAGE_FACTOR rank = 1200,
# internal RULE_SHIP_STRUCTURE_FACTOR rank = 1300,
# internal RULE_SHIP_SPEED_FACTOR rank = 1400,


#########################################
# category "TEST"
#########################################
ranker = count(start=100, step=100)

GameRule(
    name="RULE_ENABLE_SUPER_TESTER",
    description="RULE_ENABLE_SUPER_TESTER_DESC",
    category="TEST",
    type=bool,
    default=False,
    rank=next(ranker),
)

# internal RULE_CHEAP_AND_FAST_BUILDING_PRODUCTION, rank = 110,
# internal RULE_CHEAP_AND_FAST_SHIP_PRODUCTION, rank = 111,
# internal RULE_CHEAP_AND_FAST_TECH_RESEARCH, rank = 112,
# internal RULE_CHEAP_POLICIES, rank = 113,
# internal RULE_ALL_OBJECTS_VISIBLE, rank = 130,
# internal RULE_ALL_SYSTEMS_VISIBLE, rank = 131,
# internal RULE_UNSEEN_STEALTHY_PLANETS_INVISIBLE, rank = 132,
# internal RULE_STARLANES_EVERYWHERE, rank = 135,

GameRule(
    name="RULE_TEST_STRING",
    description="RULE_TEST_STRING_DESC",
    category="TEST",
    type=str,
    default="PLAYER",
    allowed=["MODERATOR", "OBSERVER", "PLAYER", "AI_PLAYER"],
    rank=next(ranker),
)


#########################################
# category "BALANCE_STABILITY"
#########################################
ranker = count(start=100, step=100)

GameRule(
    name="RULE_BASELINE_PLANET_STABILITY",
    description="RULE_BASELINE_PLANET_STABILITY_DESC",
    category="BALANCE_STABILITY",
    type=int,
    default=0,
    min=-20,
    max=20,
    rank=next(ranker),
)

GameRule(
    name="RULE_PROTECTION_FOCUS_STABILITY",
    description="RULE_PROTECTION_FOCUS_STABILITY_DESC",
    category="BALANCE_STABILITY",
    type=int,
    default=15,
    min=-20,
    max=20,
    rank=next(ranker),
)

GameRule(
    name="RULE_IMPERIAL_PALACE_INFLUENCE",
    description="RULE_IMPERIAL_PALACE_INFLUENCE_DESC",
    category="BALANCE_STABILITY",
    type=int,
    default=3,
    min=-20,
    max=20,
    rank=next(ranker),
)

GameRule(
    name="RULE_GOOD_ENVIRONMENT_STABILITY",
    description="RULE_GOOD_ENVIRONMENT_STABILITY_DESC",
    category="BALANCE_STABILITY",
    type=int,
    default=2,
    min=-20,
    max=20,
    rank=next(ranker),
)

GameRule(
    name="RULE_ADEQUATE_ENVIRONMENT_STABILITY",
    description="RULE_ADEQUATE_ENVIRONMENT_STABILITY_DESC",
    category="BALANCE_STABILITY",
    type=int,
    default=1,
    min=-20,
    max=20,
    rank=next(ranker),
)

GameRule(
    name="RULE_POOR_ENVIRONMENT_STABILITY",
    description="RULE_POOR_ENVIRONMENT_STABILITY_DESC",
    category="BALANCE_STABILITY",
    type=int,
    default=0,
    min=-20,
    max=20,
    rank=next(ranker),
)

GameRule(
    name="RULE_HOSTILE_ENVIRONMENT_STABILITY",
    description="RULE_HOSTILE_ENVIRONMENT_STABILITY_DESC",
    category="BALANCE_STABILITY",
    type=int,
    default=-1,
    min=-20,
    max=20,
    rank=next(ranker),
)

GameRule(
    name="RULE_TINY_SIZE_STABILITY",
    description="RULE_TINY_SIZE_STABILITY_DESC",
    category="BALANCE_STABILITY",
    type=int,
    default=2,
    min=-20,
    max=20,
    rank=next(ranker),
)

GameRule(
    name="RULE_SMALL_SIZE_STABILITY",
    description="RULE_SMALL_SIZE_STABILITY_DESC",
    category="BALANCE_STABILITY",
    type=int,
    default=1,
    min=-20,
    max=20,
    rank=next(ranker),
)

GameRule(
    name="RULE_MEDIUM_SIZE_STABILITY",
    description="RULE_MEDIUM_SIZE_STABILITY_DESC",
    category="BALANCE_STABILITY",
    type=int,
    default=0,
    min=-20,
    max=20,
    rank=next(ranker),
)

GameRule(
    name="RULE_LARGE_SIZE_STABILITY",
    description="RULE_LARGE_SIZE_STABILITY_DESC",
    category="BALANCE_STABILITY",
    type=int,
    default=-1,
    min=-20,
    max=20,
    rank=next(ranker),
)

GameRule(
    name="RULE_HUGE_SIZE_STABILITY",
    description="RULE_HUGE_SIZE_STABILITY_DESC",
    category="BALANCE_STABILITY",
    type=int,
    default=-2,
    min=-20,
    max=20,
    rank=next(ranker),
)

GameRule(
    name="RULE_GAS_GIANT_SIZE_STABILITY",
    description="RULE_GAS_GIANT_SIZE_STABILITY_DESC",
    category="BALANCE_STABILITY",
    type=int,
    default=0,
    min=-20,
    max=20,
    rank=next(ranker),
)

GameRule(
    name="RULE_BASELINE_SPECIES_EMPIRE_OPINION",
    description="RULE_BASELINE_SPECIES_EMPIRE_OPINION_DESC",
    category="BALANCE_STABILITY",
    type=int,
    default=0,
    min=-20,
    max=20,
    rank=next(ranker),
)

GameRule(
    name="RULE_INVASION_OPINION_PENALTY_SCALING",
    description="RULE_INVASION_OPINION_PENALTY_SCALING_DESC",
    category="BALANCE_STABILITY",
    type=float,
    default=-2.0,
    min=-50.0,
    max=25.0,
    rank=next(ranker),
)

GameRule(
    name="RULE_SHIPS_LOST_DESTROYED_PENALTY_SCALING",
    description="RULE_SHIPS_LOST_DESTROYED_PENALTY_SCALING_DESC",
    category="BALANCE_STABILITY",
    type=float,
    default=-0.5,
    min=-50.0,
    max=25.0,
    rank=next(ranker),
)

GameRule(
    name="RULE_COLONIES_FOUNDED_BONUS_SCALING",
    description="RULE_COLONIES_FOUNDED_BONUS_SCALING_DESC",
    category="BALANCE_STABILITY",
    type=float,
    default=1.0,
    min=-25.0,
    max=50.0,
    rank=next(ranker),
)

# internal RULE_ANNEX_COST_OPINION_EXP_BASE, rank = 5500,
# internal RULE_ANNEX_COST_STABILITY_EXP_BASE, rank = 5510,
# internal RULE_ANNEX_COST_SCALING, rank = 5520,
# internal RULE_BUILDING_ANNEX_COST_SCALING, rank = 5530,
# internal RULE_ANNEX_COST_MINIMUM, rank = 5540,

#########################################
# category "PLANET_SIZE"
#########################################
ranker = count(start=100, step=100)

GameRule(
    name="RULE_HABITABLE_SIZE_TINY",
    description="RULE_HABITABLE_SIZE_DESC",
    category="PLANET_SIZE",
    type=int,
    default=1,
    min=0,
    max=999,
    rank=next(ranker),
)

GameRule(
    name="RULE_HABITABLE_SIZE_SMALL",
    description="RULE_HABITABLE_SIZE_DESC",
    category="PLANET_SIZE",
    type=int,
    default=2,
    min=0,
    max=999,
    rank=next(ranker),
)

GameRule(
    name="RULE_HABITABLE_SIZE_MEDIUM",
    description="RULE_HABITABLE_SIZE_DESC",
    category="PLANET_SIZE",
    type=int,
    default=3,
    min=0,
    max=999,
    rank=next(ranker),
)

GameRule(
    name="RULE_HABITABLE_SIZE_LARGE",
    description="RULE_HABITABLE_SIZE_DESC",
    category="PLANET_SIZE",
    type=int,
    default=4,
    min=0,
    max=999,
    rank=next(ranker),
)

GameRule(
    name="RULE_HABITABLE_SIZE_HUGE",
    description="RULE_HABITABLE_SIZE_DESC",
    category="PLANET_SIZE",
    type=int,
    default=5,
    min=0,
    max=999,
    rank=next(ranker),
)

GameRule(
    name="RULE_HABITABLE_SIZE_ASTEROIDS",
    description="RULE_HABITABLE_SIZE_DESC",
    category="PLANET_SIZE",
    type=int,
    default=3,
    min=0,
    max=999,
    rank=next(ranker),
)

GameRule(
    name="RULE_HABITABLE_SIZE_GASGIANT",
    description="RULE_HABITABLE_SIZE_DESC",
    category="PLANET_SIZE",
    type=int,
    default=6,
    min=0,
    max=999,
    rank=next(ranker),
)


#########################################
# Category "MULTIPLAYER"
#########################################
# -> only internal rules so far
# internal RULE_DIPLOMACY, rank = 90000000,
# internal RULE_THRESHOLD_HUMAN_PLAYER_WIN, rank = 90000010,
# internal RULE_ONLY_ALLIANCE_WIN, rank = 90000020,
# internal RULE_ALLOW_CONCEDE, rank = 90000030,
# internal RULE_CONCEDE_COLONIES_THRESHOLD, rank = 90000040,
