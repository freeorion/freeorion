GameRuleDefinition(
    name="RULE_SHIP_HULL_COST_FACTOR",
    description="RULE_SHIP_HULL_COST_FACTOR_DESC",
    category="BALANCE",
    type=float,
    default=1.0,
    min=0.1,
    max=10.0,
)

GameRuleDefinition(
    name="RULE_SHIP_PART_COST_FACTOR",
    description="RULE_SHIP_PART_COST_FACTOR_DESC",
    category="BALANCE",
    type=float,
    default=1.0,
    min=0.1,
    max=10.0,
)

GameRuleDefinition(
    name="RULE_TECH_COST_FACTOR",
    description="RULE_TECH_COST_FACTOR_DESC",
    category="BALANCE",
    type=float,
    default=2.0,
    min=0.1,
    max=10.0,
)

GameRuleDefinition(
    name="RULE_BUILDING_COST_FACTOR",
    description="RULE_BUILDING_COST_FACTOR_DESC",
    category="BALANCE",
    type=float,
    default=1.0,
    min=0.1,
    max=10.0,
)

GameRuleDefinition(
    name="RULE_ENABLE_EXPERIMENTORS",
    description="RULE_ENABLE_EXPERIMENTORS_DESC",
    category="CONTENT",
    type=bool,
    default=True,
)

GameRuleDefinition(
    name="RULE_ENABLE_SUPER_TESTER",
    description="RULE_ENABLE_SUPER_TESTER_DESC",
    category="CONTENT",
    type=bool,
    default=True,
)

GameRuleDefinition(
    name="RULE_TEST_STRING",
    description="RULE_TEST_STRING_DESC",
    category="TEST",
    type=str,
    default="PLAYER",
    allowed=["MODERATOR", "OBSERVER", "PLAYER", "AI_PLAYER"],
)

GameRuleDefinition(
    name="RULE_HABITABLE_SIZE_TINY",
    description="RULE_HABITABLE_SIZE_DESC",
    category="PLANET_SIZE",
    type=int,
    default=1,
    min=0,
    max=999,
)

GameRuleDefinition(
    name="RULE_HABITABLE_SIZE_SMALL",
    description="RULE_HABITABLE_SIZE_DESC",
    category="PLANET_SIZE",
    type=int,
    default=2,
    min=0,
    max=999,
)

GameRuleDefinition(
    name="RULE_HABITABLE_SIZE_MEDIUM",
    description="RULE_HABITABLE_SIZE_DESC",
    category="PLANET_SIZE",
    type=int,
    default=3,
    min=0,
    max=999,
)

GameRuleDefinition(
    name="RULE_HABITABLE_SIZE_LARGE",
    description="RULE_HABITABLE_SIZE_DESC",
    category="PLANET_SIZE",
    type=int,
    default=4,
    min=0,
    max=999,
)

GameRuleDefinition(
    name="RULE_HABITABLE_SIZE_HUGE",
    description="RULE_HABITABLE_SIZE_DESC",
    category="PLANET_SIZE",
    type=int,
    default=5,
    min=0,
    max=999,
)

GameRuleDefinition(
    name="RULE_HABITABLE_SIZE_ASTEROIDS",
    description="RULE_HABITABLE_SIZE_DESC",
    category="PLANET_SIZE",
    type=int,
    default=3,
    min=0,
    max=999,
)

GameRuleDefinition(
    name="RULE_HABITABLE_SIZE_GASGIANT",
    description="RULE_HABITABLE_SIZE_DESC",
    category="PLANET_SIZE",
    type=int,
    default=6,
    min=0,
    max=999,
)

GameRuleDefinition(
    name="RULE_SHIP_PART_BASED_UPKEEP",
    description="RULE_SHIP_PART_BASED_UPKEEP_DESC",
    category="BALANCE",
    type=bool,
    default=False,
)

GameRuleDefinition(
    name="RULE_ENABLE_ALLIED_REPAIR",
    description="RULE_ENABLE_ALLIED_REPAIR_DESC",
    category="MULTIPLAYER",
    type=bool,
    default=False,
)

GameRuleDefinition(
    name="RULE_FIRST_COMBAT_ROUND_IN_CLOSE_TARGETING_RANGE",
    description="RULE_FIRST_COMBAT_ROUND_IN_CLOSE_TARGETING_RANGE_DESC",
    category="BALANCE",
    type=int,
    default=3,
    min=1,
    max=20,
)
