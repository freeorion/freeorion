from collections import defaultdict
from typing import DefaultDict, Dict, FrozenSet, Set, Union

import AIDependencies
from common.fo_typing import BuildingId, PlanetId, SystemId
from empire.pilot_rating import best_pilot_rating
from empire.survey_lock import survey_universe_lock
from freeorion_tools.caching import cache_for_current_turn


@survey_universe_lock
def get_best_pilot_facilities(building: Union[BuildingId, str]) -> FrozenSet[PlanetId]:
    best_pilot_facilities = _get_facilities().get("WEAPONS_%.1f" % best_pilot_rating(), {})

    return best_pilot_facilities.get(building, set())


@survey_universe_lock
def get_systems_with_building(bld_name: BuildingId):
    return _get_system_facilities()[bld_name]


@cache_for_current_turn
def _get_facilities() -> Dict[str, Dict[BuildingId, Set[PlanetId]]]:
    return {}


@cache_for_current_turn
def _get_system_facilities() -> DefaultDict[BuildingId, Set]:
    return defaultdict(set)


def set_building_locations(
    weapons_grade: str,
    ship_facilities: Set[BuildingId],
    pid: PlanetId,
    sid: SystemId,
):
    this_grade_facilities = _get_facilities().setdefault(weapons_grade, {})

    for facility in ship_facilities:
        this_grade_facilities.setdefault(facility, set()).add(pid)
        if facility in AIDependencies.SYSTEM_SHIP_FACILITIES:
            _get_system_facilities()[facility].add(sid)
