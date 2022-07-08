_nest_ratings = {
    "SNOWFLAKE_NEST_SPECIAL": 35.0,
    "KRAKEN_NEST_SPECIAL": 60.0,
    "JUGGERNAUT_NEST_SPECIAL": 80.0,
}


def special_is_nest(special_name: str) -> bool:
    return special_name in _nest_ratings


def get_nest_rating(special_name: str) -> float:
    return _nest_ratings.get(special_name, 0.0)
