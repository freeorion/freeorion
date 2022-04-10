import freeOrionAIInterface as fo
from logging import debug, error
from typing import Iterable, List, Sequence, Set, Union

from AIDependencies import INVALID_ID
from common.fo_typing import PlanetId, SystemId
from empire.colony_builders import get_colony_builders
from empire.ship_builders import get_ship_builders
from freeorion_tools import ppstring
from freeorion_tools.caching import cache_for_current_turn


def safe_name(univ_object):
    return (univ_object and univ_object.name) or "?"


def sys_name_ids(sys_ids: Iterable[int]) -> str:
    """
    Get a string representation of a list with system_ids.

    The returned string is of the form "[S_id<name>, ...]"

    :return: string representation of the systems in the list
    """
    universe = fo.getUniverse()
    return ppstring([str(universe.getSystem(sys_id)) for sys_id in sys_ids])


def planet_string(planet_ids: Union[PlanetId, Iterable[PlanetId]]) -> str:
    """
    Get a string representation of the passed planets.
    """

    def _safe_planet_name(planet_id):
        planet = fo.getUniverse().getPlanet(planet_id)
        return fo.to_str("P", planet_id, (planet and planet.name) or "?")

    if isinstance(planet_ids, int):
        return _safe_planet_name(planet_ids)
    return ppstring([_safe_planet_name(pid) for pid in planet_ids])


@cache_for_current_turn
def get_capital() -> PlanetId:
    """
    Return current empire capital id.

    If no current capital returns planet with biggest population in first not empty group.
    First check all planets with coloniser species, after that with ship builders and at last all inhabited planets.
    """
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empire_id = empire.empireID
    capital_id = empire.capitalID
    homeworld = universe.getPlanet(capital_id)
    if homeworld:
        if homeworld.owner == empire_id:
            return capital_id
        else:
            debug(
                "Nominal Capitol %s does not appear to be owned by empire %d %s"
                % (homeworld.name, empire_id, empire.name)
            )
    empire_owned_planet_ids = get_owned_planets_by_empire()
    peopled_planets = get_populated_planet_ids(empire_owned_planet_ids)
    if not peopled_planets:
        if empire_owned_planet_ids:
            return empire_owned_planet_ids[0]
        else:
            return INVALID_ID
    try:
        for spec_list in [get_colony_builders(), get_ship_builders(), None]:
            population_id_pairs = []
            for planet_id in peopled_planets:
                planet = universe.getPlanet(planet_id)
                if spec_list is None or planet.speciesName in spec_list:
                    population_id_pairs.append((planet.initialMeterValue(fo.meterType.population), planet_id))
            if population_id_pairs:
                return max(population_id_pairs)[-1]
    except Exception as e:
        error(e, exc_info=True)
    return INVALID_ID  # shouldn't ever reach here


def get_capital_sys_id() -> SystemId:
    """
    Return system id with empire capital.
    :return: system id
    """
    cap_id = get_capital()
    if cap_id == INVALID_ID:
        return INVALID_ID
    else:
        return fo.getUniverse().getPlanet(cap_id).systemID


def get_planets_in__systems_ids(system_ids: Iterable[SystemId]) -> List[PlanetId]:
    """
    Return list of planet ids for system ids list.
    """
    universe = fo.getUniverse()
    planet_ids = set()
    for system_id in system_ids:
        system = universe.getSystem(system_id)
        if system is not None:
            planet_ids.update(system.planetIDs)
    return list(planet_ids)


@cache_for_current_turn
def get_owned_planets_by_empire() -> List[PlanetId]:
    """
    Return list of all planets owned by empire.
    """
    universe = fo.getUniverse()
    empire_id = fo.getEmpire().empireID
    result = []
    for pid in universe.planetIDs:
        planet = universe.getPlanet(pid)
        # even if our universe says we own it, if we can't see it we must have lost it
        if (
            planet
            and not planet.unowned
            and planet.ownedBy(empire_id)
            and universe.getVisibility(pid, empire_id) >= fo.visibility.partial
        ):
            result.append(pid)
    return result


def get_all_owned_planet_ids(planet_ids: Sequence[PlanetId]) -> Sequence[PlanetId]:
    """
    Return list of all owned and populated planet_ids.
    """
    # TODO: remove after refactoring in invasionAI
    # this function result used only to filter out unpopulated planets,
    # after some changes in invasionAI it will became obsolete
    universe = fo.getUniverse()
    result = []
    for pid in planet_ids:
        planet = universe.getPlanet(pid)
        if planet:
            population = planet.initialMeterValue(fo.meterType.population)
            if not planet.unowned or population > 0:
                result.append(pid)
    return result


def get_populated_planet_ids(planet_ids: Sequence[PlanetId]) -> Sequence[PlanetId]:
    """
    Filter planets with population.
    :param planet_ids: list of planets ids
    :return: list of planets ids
    """
    universe = fo.getUniverse()
    return [pid for pid in planet_ids if universe.getPlanet(pid).initialMeterValue(fo.meterType.population) > 0]


def get_systems(planet_ids: Sequence[PlanetId]) -> Sequence[SystemId]:
    """
    Return list of systems containing planet_ids.
    """
    # TODO discuss change return type to set
    universe = fo.getUniverse()
    return [universe.getPlanet(pid).systemID for pid in planet_ids]


def get_planet_opinion(feature: str) -> (Set[PlanetId], Set[PlanetId], Set[PlanetId]):
    """
    Returns sets of empire planets that like, are neutral and dislike the given feature
    """
    # default: feature not in any like or dislike set, all neutral
    default = (set(), set(get_owned_planets_by_empire()), set())
    return _calculate_get_planet_opinions().get(feature, default)


@cache_for_current_turn
def _calculate_get_planet_opinions():  # -> Dict[str, (Set[PlanetId], Set[PlanetId], Set[PlanetId])]:
    universe = fo.getUniverse()
    all_species = {universe.getPlanet(pid).speciesName for pid in get_owned_planets_by_empire()}
    all_features = set()
    for species_name in all_species:
        if species_name:
            species = fo.getSpecies(species_name)
            all_features.update(species.likes)
            all_features.update(species.dislikes)

    result = {feature: (set(), set(), set()) for feature in all_features}
    for feature, sets in result.items():
        for pid in get_owned_planets_by_empire():
            species_name = universe.getPlanet(pid).speciesName
            if species_name:
                species = fo.getSpecies(species_name)
                if feature in species.likes:
                    sets[0].add(pid)
                elif feature in species.dislikes:
                    sets[2].add(pid)
                else:
                    sets[1].add(pid)
            # else: outposts are always neutral
            sets[1].add(pid)
    return result
