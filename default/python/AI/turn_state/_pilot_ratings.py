from freeorion_tools import cache_for_current_turn


class PilotRatings:
    def __init__(self):
        self.best_pilot_rating = 1e-8
        self.medium_pilot_rating = 1e-8


@cache_for_current_turn
def _get_pilot_ratings() -> PilotRatings:
    return PilotRatings()


def best_pilot_rating() -> float:
    return _get_pilot_ratings().best_pilot_rating


def set_best_pilot_rating(value: float):
    _get_pilot_ratings().best_pilot_rating = value


def medium_pilot_rating() -> float:
    return _get_pilot_ratings().medium_pilot_rating


def set_medium_pilot_rating(value: float):
    _get_pilot_ratings().medium_pilot_rating = value
