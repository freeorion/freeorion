from logging import info
from typing import Dict, Mapping, Sequence

from common.fo_typing import PlanetId
from empire.survey_lock import survey_universe_lock
from freeorion_tools.caching import cache_for_current_turn


def set_pilot_rating_for_planet(pid: PlanetId, pilot_rating: float):
    """
    Set pilot rating for planet.

    Warning! Temporal coupling.
    All calls of this function should be done before using of this information.
    """
    _get_pilot_ratings()[pid] = pilot_rating


@survey_universe_lock
def get_rating_for_planet(pid: PlanetId) -> float:
    return get_pilot_ratings().get(pid, 0)


@survey_universe_lock
def get_pilot_ratings() -> Mapping[PlanetId, float]:
    return _get_pilot_ratings()


@cache_for_current_turn
def _get_pilot_ratings() -> Dict[PlanetId, float]:
    """
    Return mutable state.
    """
    return {}


class _Summary:
    def __init__(self):
        self.best_pilot_rating = 1e-8
        self.medium_pilot_rating = 1e-8


@cache_for_current_turn
def _get_pilot_summary() -> _Summary:
    summary = _Summary()
    rating_list = sorted(get_pilot_ratings().values(), reverse=True)
    _summarize_pilot_ratings(summary, rating_list)
    return summary


@survey_universe_lock
def best_pilot_rating() -> float:
    return _get_pilot_summary().best_pilot_rating


@survey_universe_lock
def medium_pilot_rating() -> float:
    return _get_pilot_summary().medium_pilot_rating


def _summarize_pilot_ratings(summary: _Summary, ratings: Sequence[float]):
    if not ratings:
        return
    info("_summarize_pilot_ratings")
    summary.best_pilot_rating = ratings[0]
    if len(ratings) == 1:
        summary.medium_pilot_rating = ratings[0]
    else:
        summary.medium_pilot_rating = ratings[(1 + int(len(ratings) // 5))]
