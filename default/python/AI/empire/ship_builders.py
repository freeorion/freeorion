from typing import Dict, List, Mapping, Set, Union

from common.fo_typing import PlanetId, SpeciesName
from empire.survey_lock import survey_universe_lock
from freeorion_tools.caching import cache_for_current_turn


@survey_universe_lock
def can_build_ship_for_species(species_name: Union[SpeciesName, str]):
    return species_name in get_ship_builders()


@survey_universe_lock
def get_ship_builder_locations(species_name: SpeciesName) -> List[PlanetId]:
    return get_ship_builders().get(species_name, [])


def set_ship_builders(species_name: SpeciesName, pid: PlanetId):
    """
    Add planet where you can build ships for species.

    Warning! Temporal coupling.
    All calls of this function should be done before using of this information.
    """
    get_shipyards().add(pid)
    _get_ship_builders().setdefault(species_name, []).append(pid)


@survey_universe_lock
def get_ship_builders() -> Mapping[SpeciesName, List[PlanetId]]:
    """
    Return map from the species to list of the planet where you could build a ship with it.
    """
    return _get_ship_builders()


@cache_for_current_turn
def _get_ship_builders() -> Dict[SpeciesName, List[PlanetId]]:
    """
    Return mutable state.
    """
    return {}


@cache_for_current_turn
def get_shipyards() -> Set[PlanetId]:
    return set()


@survey_universe_lock
def has_shipyard(pid: PlanetId) -> bool:
    return pid in get_shipyards()
