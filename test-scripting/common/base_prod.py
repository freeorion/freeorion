try:
    from focs._effects import GameRule
except ModuleNotFoundError:
    pass

INDUSTRY_PER_POP = 0.2

STOCKPILE_PER_POP = 0.02

RESEARCH_PER_POP = 0.2

TROOPS_PER_POP = 0.2

TECH_COST_MULTIPLIER = GameRule(type=float, name="RULE_TECH_COST_FACTOR")

BUILDING_COST_MULTIPLIER = GameRule(type=float, name="RULE_BUILDING_COST_FACTOR")
