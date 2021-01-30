from typing import Callable, FrozenSet, List, Mapping, Set, Tuple

import AIDependencies
import freeOrionAIInterface as fo
from freeorion_tools import cache_for_current_turn, ReadOnlyDict


class PlanetInfo:
    def __init__(self, pid: int):
        universe = fo.getUniverse()
        planet = universe.getPlanet(pid)
        self.pid = planet.id
        self.species_name = planet.speciesName
        self.owner = planet.owner
        self.system_id = planet.systemID

    def is_colonized(self) -> bool:
        return self.owner == fo.empireID() and self.species_name

    def is_owned_by_empire(self) -> bool:
        return self.owner == fo.empireID()

    def has_outpost(self) -> bool:
        return self.owner == fo.empireID() and not self.species_name

    def is_not_owned(self) -> bool:
        return self.owner == AIDependencies.INVALID_ID and not self.species_name


@cache_for_current_turn
def _get_planets_info() -> Mapping[int, PlanetInfo]:
    universe = fo.getUniverse()
    planets = (universe.getPlanet(pid) for pid in universe.planetIDs)
    return {planet.id: PlanetInfo(planet.id) for planet in planets}


def _get_system_planets_map(planet_filter: Callable[[PlanetInfo], bool]) -> Mapping[int, Tuple[int]]:
    result = {}
    for planet_info in (planet_info for planet_info in _get_planets_info().values() if planet_filter):
        result.setdefault(planet_info.system_id, []).append(planet_info.pid)
    return ReadOnlyDict({k: tuple(v) for k, v in result.items()})


@cache_for_current_turn
def get_owned_planets() -> Mapping[int, Tuple[int]]:
    """
    Return map from system id to list of planets with colony or outpost.
    """
    return _get_system_planets_map(PlanetInfo.is_owned_by_empire)


@cache_for_current_turn
def get_colonized_planets() -> Mapping[int, Tuple[int]]:
    """
    Return map from system id to list of planets with colony only.
    """
    return _get_system_planets_map(PlanetInfo.is_colonized)


def get_owned_planets_in_system(sys_id: int) -> Tuple[int]:
    """
    Return list of planets with colony or outpost in the system.
    """
    return get_owned_planets().get(sys_id)


def get_colonized_planets_in_system(sys_id: int) -> Tuple[int]:
    """
    Return list of planets with colony in the system.
     """
    return get_colonized_planets().get(sys_id)


@cache_for_current_turn
def get_inhabited_planets() -> FrozenSet[int]:
    """
    Return frozenset of empire planet ids with species.
    """
    return frozenset(planet_info.pid for planet_info in _get_planets_info().values() if planet_info.is_colonized())


@cache_for_current_turn
def get_empire_outposts() -> Tuple[int]:
    all_planets = _get_planets_info().values()
    return tuple(planet_info.pid for planet_info in all_planets if planet_info.has_outpost())


@cache_for_current_turn
def get_all_empire_planets() -> Tuple[int]:
    all_planets = _get_planets_info().values()
    return tuple(planet_info.pid for planet_info in all_planets if planet_info.is_owned_by_empire())


@cache_for_current_turn
def get_empire_planets_by_species() -> Mapping[str, List[int]]:
    """
    Return dict for empire from species to list of planet ids.
    """
    result = {}
    colonized = (planet_info for planet_info in _get_planets_info().values() if planet_info.is_colonized())
    for planet in colonized:
        result.setdefault(planet.species_name, []).append(planet.pid)
    return result


def get_unowned_empty_planets() -> Set[int]:
    """
    Return the set of planets that are not owned by any player and have no natives.
    """
    return {planet_info.pid for planet_info in _get_planets_info().values() if planet_info.is_not_owned()}


def get_number_of_colonies() -> int:
    return len(get_inhabited_planets())


def get_empire_planets_with_species(species_name: str) -> Tuple[int]:
    """
    Return empire planet ids with species.
    """
    if not species_name:
        return ()
    return tuple(get_empire_planets_by_species().get(species_name, []))
