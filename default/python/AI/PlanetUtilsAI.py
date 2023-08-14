import freeOrionAIInterface as fo
from collections.abc import Iterable, Mapping, Sequence
from enum import Enum
from logging import debug, error
from typing import (
    NamedTuple,
    Optional,
    Union,
)

from AIDependencies import INVALID_ID, STABILITY_PER_LIKED_FOCUS
from aistate_interface import get_aistate
from common.fo_typing import PlanetId, SpeciesName, SystemId
from empire.colony_builders import get_colony_builders, get_extra_colony_builders
from empire.ship_builders import get_ship_builders
from EnumsAI import FocusType
from freeorion_tools import get_game_rule_int, get_named_real, ppstring
from freeorion_tools.caching import cache_for_current_turn


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
def get_capital() -> PlanetId:  # noqa: C901
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


def get_planets_in__systems_ids(system_ids: Iterable[SystemId]) -> list[PlanetId]:
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
def get_owned_planets_by_empire() -> list[PlanetId]:
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


@cache_for_current_turn
def get_empire_populated_planets() -> tuple[fo.planet]:
    universe = fo.getUniverse()
    return tuple(universe.getPlanet(pid) for pid in get_populated_planet_ids(get_owned_planets_by_empire()))


def get_systems(planet_ids: Sequence[PlanetId]) -> Sequence[SystemId]:
    """
    Return list of systems containing planet_ids.
    """
    # TODO discuss change return type to set
    universe = fo.getUniverse()
    return [universe.getPlanet(pid).systemID for pid in planet_ids]


class Opinion(NamedTuple):
    likes: set[PlanetId]
    neutral: set[PlanetId]
    dislikes: set[PlanetId]

    def value(self, pid: PlanetId, like_value: float, neutral_value: float, dislike_value: float) -> float:
        """Returns like_value if pid is in likes, dislike_value if pid is in dislikes, else neutral_value"""
        if pid in self.likes:
            return like_value
        elif pid in self.dislikes:
            return dislike_value
        return neutral_value


def get_planet_opinion(feature: Union[str, Enum]) -> Opinion:
    """
    Returns sets of empire planets that like, are neutral and dislike the given feature
    """
    # default: feature not in any like or dislike set, all neutral
    if isinstance(feature, Enum):
        feature = feature.value()
    default = Opinion(set(), set(get_owned_planets_by_empire()), set())
    return _calculate_get_planet_opinions().get(feature, default)


def _get_species_from_colony_building(name: str) -> Optional[SpeciesName]:
    """Extract a species if name is the name of a colony building"""
    building_prefix = "BLD_COL_"
    species_prefix = "SP_"
    if name.startswith(building_prefix):
        return species_prefix + name[len(building_prefix) :]


@cache_for_current_turn
def _planned_species() -> Mapping[PlanetId, SpeciesName]:
    universe = fo.getUniverse()
    production_queue = fo.getEmpire().productionQueue
    planned_species = {}
    colonisation_plans = get_aistate().colonisablePlanetIDs
    for element in production_queue:
        species_name = _get_species_from_colony_building(element.name)
        if species_name:
            planned_species[element.locationID] = species_name
    for pid in get_owned_planets_by_empire():
        planet = universe.getPlanet(pid)
        if planet.speciesName:
            # skip already populated planets
            continue
        # Finished colony buildings are normal buildings for one turn before turning the outpost into a colony
        for building in map(universe.getBuilding, planet.buildingIDs):
            species_name = _get_species_from_colony_building(building.name)
            if species_name:
                planned_species[pid] = species_name
                break
        if pid not in planned_species:
            # Without checking the colonisation plans, the AI may start building a colony and buildings
            # the future species wouldn't like in the same turn.
            plan = colonisation_plans.get(pid)
            if plan:
                planned_species[pid] = plan[1]
    debug(f"Planned species: {planned_species}")
    return planned_species


def _planet_species(pid: PlanetId) -> Optional[fo.species]:
    universe = fo.getUniverse()
    planet = universe.getPlanet(pid)
    species_name = planet.speciesName
    if not species_name:
        species_name = _planned_species().get(pid)
    if species_name:
        return fo.getSpecies(species_name)
    return None


@cache_for_current_turn
def _calculate_get_planet_opinions() -> dict[str, Opinion]:
    universe = fo.getUniverse()
    all_species = [universe.getPlanet(pid).speciesName for pid in get_owned_planets_by_empire()]
    all_species += get_extra_colony_builders()
    all_features = set()
    for species_name in all_species:
        if species_name:
            species = fo.getSpecies(species_name)
            all_features.update(species.likes)
            all_features.update(species.dislikes)

    result = {feature: Opinion(set(), set(), set()) for feature in all_features}
    for pid in get_owned_planets_by_empire():
        species = _planet_species(pid)
        for feature, opinion in result.items():
            if species:
                if feature in species.likes:
                    opinion.likes.add(pid)
                elif feature in species.dislikes:
                    opinion.dislikes.add(pid)
                else:
                    opinion.neutral.add(pid)
            # else: no species -> neutral
            opinion.neutral.add(pid)
    return result


def dislike_factor() -> float:
    """Returns multiplier for dislike effects."""
    # See opinion.macros
    has_liberty = fo.getEmpire().policyAdopted("PLC_LIBERTY")
    has_conformance = fo.getEmpire().policyAdopted("PLC_CONFORMANCE")
    liberty_factor = get_named_real("PLC_LIBERTY_DISLIKE_FACTOR") if has_liberty else 1.0
    conformance_factor = get_named_real("PLC_CONFORMANCE_DISLIKE_FACTOR") if has_conformance else 1.0
    return liberty_factor * conformance_factor


def focus_stability_effect(species: fo.species, focus: str) -> float:
    """How does the focus affect the stability of a planet with the species."""
    result = 0.0
    if focus in species.likes:
        result += STABILITY_PER_LIKED_FOCUS
    if focus in species.dislikes:
        result += STABILITY_PER_LIKED_FOCUS * dislike_factor()
    if focus == FocusType.FOCUS_PROTECTION:
        result += get_game_rule_int("RULE_PROTECTION_FOCUS_STABILITY", 15)
    empire = fo.getEmpire()
    # TODO move policy definitions to AIDependency? Or used an EnumClass like BuildingType?
    if focus == FocusType.FOCUS_INDUSTRY and empire.policyAdopted("PLC_INDUSTRIALISM"):
        result += get_named_real("PLC_INDUSTRIALISM_TARGET_HAPPINESS_FLAT")
    if focus == FocusType.FOCUS_RESEARCH and empire.policyAdopted("PLC_TECHNOCRACY"):
        result += get_named_real("PLC_TECHNOCRACY_TARGET_HAPPINESS_FLAT")
    return result


def stability_with_focus(planet: fo.planet, focus: str) -> float:
    """
    What would the planets target stability be when switched to the given focus.
    Returns -99 if species cannot use the specified focus.
    """
    stability = planet.currentMeterValue(fo.meterType.targetHappiness)
    species = fo.getSpecies(planet.speciesName)
    if focus not in species.foci:
        # The actual value here is not important. If some part of the AI asks for a stability,
        # returning a big negative value here should stop it from considering that focus for anything.
        return -99.0
    return stability - focus_stability_effect(species, planet.focus) + focus_stability_effect(species, focus)


def adjust_liberty(planet: fo.planet, population: float) -> float:
    """
    Adjust liberty research output for changed stability.
    UpdateMeterEstimate calculates liberty based on current stability, not on target stability.
    So this may return a positive or negative adjustment, depending on whether stability goes up or down.
    """
    # This one is relatively easy to calculate. Still it would be better to have an adjustMeterUpdate with
    # modified stability...
    current_stability = planet.currentMeterValue(fo.meterType.happiness)
    target_stability = planet.currentMeterValue(fo.meterType.targetHappiness)
    low = get_named_real("PLC_LIBERTY_MIN_STABILITY")
    high = get_named_real("PLC_LIBERTY_MAX_STABILITY")
    difference = max(low, min(high, target_stability)) - max(low, min(high, current_stability))
    debug(f"adjust_liberty on {planet}: difference={difference}, population={population}")
    return difference * population * get_named_real("PLC_LIBERTY_RESEARCH_BONUS_SCALING")
