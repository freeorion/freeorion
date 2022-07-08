from typing import Dict, FrozenSet, Set

from buildings import Shipyard
from common.fo_typing import BuildingName, PlanetId
from empire.pilot_rating import best_pilot_rating
from empire.survey_lock import survey_universe_lock
from freeorion_tools.caching import cache_for_current_turn


@survey_universe_lock
def get_best_pilot_facilities(facility: BuildingName) -> FrozenSet[PlanetId]:
    """Gives list of planets with the best available pilots have the given ship building facility"""
    # TBD: this is only for battle ships and ignores things like fuel and vision range.
    # For scouts, vision and fuel would be more important
    best_pilot_facilities = _get_facilities().get("WEAPONS_%.1f" % best_pilot_rating(), {})

    return best_pilot_facilities.get(facility, set())


@cache_for_current_turn
def _get_facilities() -> Dict[str, Dict[BuildingName, Set[PlanetId]]]:
    return {}


def set_building_locations(
    weapons_grade: str,
    buildings_here: Set[BuildingName],
    pid: PlanetId,
):
    this_grade_facilities = _get_facilities().setdefault(weapons_grade, {})
    for building in buildings_here:
        try:
            Shipyard(building)
        except ValueError:
            continue
        this_grade_facilities.setdefault(building, set()).add(pid)
