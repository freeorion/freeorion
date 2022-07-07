import freeOrionAIInterface as fo
from logging import debug

from common.option_tools import check_bool, get_option_dict
from freeorion_tools.caching import cache_for_session

# Minimum evaluation score that a planet must reach to it is considered for outposting or colonizing.
MINIMUM_COLONY_SCORE = 60
# Resource production is evaluated as production * priority_for_the_resource * RESOURCE_PRIORITY_MULTIPLIER.
# So raising this makes it easier for planets to reach the minimum score and in relation reduces the value of
# other scores, which are rated as factor * MINIMUM_COLONY_SCORE
RESOURCE_PRIORITY_MULTIPLIER = 0.5

DEBUG_COLONY_RATING = check_bool(get_option_dict().get("debug_planet_rating", "False"))


def debug_rating(*args, **kwargs):
    if DEBUG_COLONY_RATING:
        debug(*args, **kwargs)


@cache_for_session
def use_new_rating() -> bool:
    config_value = int(get_option_dict().get("use_new_colony_rating", "1"))
    if config_value < 2:
        result = bool(config_value)
    else:
        result = fo.empireID() <= config_value
    debug(f"use_new_rating: {result}")
    return result
