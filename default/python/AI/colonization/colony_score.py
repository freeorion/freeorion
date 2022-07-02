from common.option_tools import check_bool, get_option_dict

# minimum evaluation score that a planet must reach so it is considered for outposting or colonizing
MINIMUM_COLONY_SCORE = 60
# resource production is evaluated as production * priority_for_the_resource * RESOURCE_PRIORITY_MULTIPLIER
RESOURCE_PRIORITY_MULTIPLIER = 0.5

DEBUG_COLONY_RATING = check_bool(get_option_dict().get("debug_planet_rating", "False"))
