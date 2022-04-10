from collections import defaultdict
from typing import DefaultDict, Dict, FrozenSet, Set, Union

import AIDependencies
from common.fo_typing import BuildingId, PlanetId, SystemId
from empire.pilot_rating import best_pilot_rating
from empire.survey_lock import survey_universe_lock
from freeorion_tools.caching import cache_for_current_turn


@survey_universe_lock
def get_best_pilot_facilities(facility: Union[BuildingId, str]) -> FrozenSet[PlanetId]:
    """Gives list of planets with the best available pilots have the given ship building facility"""
    # TBD: this is only for battle ships and ignores things like fuel and vision range.
    # For scouts, vision and fuel would be more important
    best_pilot_facilities = _get_facilities().get("WEAPONS_%.1f" % best_pilot_rating(), {})

    return best_pilot_facilities.get(facility, set())


@survey_universe_lock
def get_systems_with_facilities(bld_name: BuildingId) -> FrozenSet[SystemId]:
    """Give list of systems with system-wide ship facilities (asteroid processors)"""
    return _get_system_facilities()[bld_name]


@survey_universe_lock
def get_planets_with_building(bld_name: BuildingId) -> FrozenSet[PlanetId]:
    """Give list of planets containing the given building"""
    return _get_building_locations()[bld_name]


@cache_for_current_turn
def _get_facilities() -> Dict[str, Dict[BuildingId, Set[PlanetId]]]:
    return {}


@cache_for_current_turn
def _get_system_facilities() -> DefaultDict[BuildingId, Set]:
    return defaultdict(set)


@cache_for_current_turn
def _get_building_locations() -> DefaultDict[BuildingId, Set]:
    return defaultdict(set)


def set_building_locations(
    weapons_grade: str,
    buildings_here: Set[BuildingId],
    pid: PlanetId,
    sid: SystemId,
):
    this_grade_facilities = _get_facilities().setdefault(weapons_grade, {})

    for building in buildings_here:
        _get_building_locations().setdefault(building, set()).add(pid)
        if building in AIDependencies.SHIP_FACILITIES:
            this_grade_facilities.setdefault(building, set()).add(pid)
        if building in AIDependencies.SYSTEM_SHIP_FACILITIES:
            _get_system_facilities()[building].add(sid)
