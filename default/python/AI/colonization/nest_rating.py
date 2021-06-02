_nest_ratings = {
    "SNOWFLAKE_NEST_SPECIAL": 15,
    "KRAKEN_NEST_SPECIAL": 40,
    "JUGGERNAUT_NEST_SPECIAL": 80,
}


def special_is_nest(special_name: str) -> bool:
    return special_name in _nest_ratings


def get_nest_rating(special_name: str, default: float) -> float:
    return _nest_ratings.get(special_name, default)
