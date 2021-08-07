from typing import Dict, Sequence

from common.fo_typing import PlanetId
from freeorion_tools.caching import cache_for_current_turn


def set_pilot_rating_for_planet(pid: PlanetId, pilot_rating: float):
    """
    Set pilot rating for planet.

    Warning! Temporal coupling.
    All calls of this function should be done before using of this information.
    """
    get_pilot_ratings()[pid] = pilot_rating


def get_rating_for_planet(pid: PlanetId) -> float:
    return get_pilot_ratings().get(pid, 0)


@cache_for_current_turn
def get_pilot_ratings() -> Dict[PlanetId, float]:
    return {}


class _Summary:
    def __init__(self):
        self.best_pilot_rating = 1e-8
        self.medium_pilot_rating = 1e-8


@cache_for_current_turn
def _get_pilot_summary() -> _Summary:
    return _Summary()


def best_pilot_rating() -> float:
    return _get_pilot_summary().best_pilot_rating


def medium_pilot_rating() -> float:
    return _get_pilot_summary().medium_pilot_rating


def summarize_pilot_ratings(ratings: Sequence[float]):
    if not ratings:
        return
    summary = _get_pilot_summary()
    summary.best_pilot_rating = ratings[0]
    if len(ratings) == 1:
        summary.medium_pilot_rating = ratings[0]
    else:
        summary.medium_pilot_rating = ratings[(1 + int(len(ratings) // 5))]
