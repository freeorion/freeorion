import freeOrionAIInterface as fo
from typing import Dict, Tuple

import AIDependencies
import PolicyAI
from buildings import BuildingType, iterate_buildings_types
from colonization.colony_score import debug_rating
from freeorion_tools import get_named_real, get_species_stability
from freeorion_tools.caching import cache_for_current_turn
from PlanetUtilsAI import dislike_factor
from turn_state import get_colonized_planets, have_worldtree
from universe.system_network import within_n_jumps


def calculate_stability(planet: fo.planet, species: fo.species) -> float:
    """
    Calculate the focus-independent stability species should have on planet.
    Distance to capital never give negatives, since we could build a regional admin, if it would.
    Supply-connection to nearest regional admin still TBD.
    """
    baseline = fo.getGameRules().getDouble("RULE_BASELINE_PLANET_STABILITY")
    species_mod = get_species_stability(species.name)
    home_bonus = AIDependencies.STABILITY_HOMEWORLD_BONUS if planet.id in species.homeworlds else 0.0
    policies = _evaluate_policies(species)
    specials = _evaluate_specials(planet, species)
    buildings = _evaluate_buildings(planet, species)
    xenophobia = _evaluate_xenophobia(planet, species)
    administration = _evaluate_administration(planet, species)
    result = baseline + species_mod + home_bonus + policies + specials + buildings + xenophobia + administration
    # missing: supply connection check, artisan bonus, anything else?
    debug_rating(
        f"stability of {species.name} on {planet} would be {result:.1f} (base={baseline:.1f}, "
        f"species={species_mod:.1f}, specials={specials:.1f}, home={home_bonus:.1f}, policies={policies:.1f}, "
        f"buildings={buildings:.1f}, xeno={xenophobia:.1f}, admin={administration:.1f})"
    )
    return result


@cache_for_current_turn
def _evaluate_policies(species: fo.species) -> float:
    empire = fo.getEmpire()
    like_value = AIDependencies.STABILITY_PER_LIKED_FOCUS
    dislike_value = like_value * dislike_factor()
    result = sum(like_value for p in empire.adoptedPolicies if p in species.likes)
    result -= sum(dislike_value for p in empire.adoptedPolicies if p in species.dislikes)
    if PolicyAI.bureaucracy in empire.adoptedPolicies:
        result += get_named_real("PLC_BUREAUCRACY_STABILITY_FLAT")
    # TBD: add conformance, indoctrination, etc. when the AI learns to use them
    return result


def _evaluate_specials(planet: fo.planet, species: fo.species) -> float:
    universe = fo.getUniverse()
    system = universe.getSystem(planet.systemID)
    result = 0.0
    for pid in system.planetIDs:
        if pid == planet.id:
            value = AIDependencies.STABILITY_PER_LIKED_SPECIAL_ON_PLANET
            eval_planet = planet
        else:
            value = AIDependencies.STABILITY_PER_LIKED_SPECIAL_IN_SYSTEM
            eval_planet = universe.getPlanet(pid)
        for special in eval_planet.specials:
            if special in species.likes:
                result += value
            if special in species.dislikes:
                result -= value
    if have_worldtree() and not AIDependencies.not_affect_by_special(AIDependencies.WORLDTREE_SPECIAL, species.name):
        result += AIDependencies.STABILITY_BY_WORLDTREE
    gaia = AIDependencies.GAIA_SPECIAL
    if gaia in planet.specials and not AIDependencies.not_affect_by_special(gaia, species):
        result += 5  # TBD add named real
    return result


@cache_for_current_turn
def _count_building(planet: fo.planet) -> Dict[str, Tuple[int, int, int]]:
    """Returns Mapping from BuildingType to number of buildings on planet, in system and elsewhere."""
    universe = fo.getUniverse()
    system = universe.getSystem(planet.systemID)
    planet_pid = {planet.id}
    system_pids = set(pid for pid in system.planetIDs if pid != planet.id)
    # TODO: add all buildings to BuildingType, so we get them all here
    result = {}
    for building_type in iterate_buildings_types():
        # So far we only consider conquering / settling this planet. By the time we have it,
        # queued buildings are likely finished as well, so we consider them here already.
        all_pids = building_type.built_or_queued_at()
        result[building_type.value] = (
            len(all_pids & planet_pid),
            len(all_pids & system_pids),
            len(all_pids - system_pids - planet_pid),
        )
    return result


def _evaluate_buildings(planet: fo.planet, species: fo.species) -> float:
    result = 0.0
    for name, numbers in _count_building(planet).items():
        if name in species.likes:
            result += (
                numbers[0] * AIDependencies.STABILITY_PER_LIKED_BUILDING_ON_PLANET
                + numbers[1] * AIDependencies.STABILITY_PER_LIKED_BUILDING_IN_SYSTEM
                + numbers[2] ** 0.5 * AIDependencies.STABILITY_BASE_LIKED_BUILDING_ELSEWHERE
            )
        elif name in species.dislikes:
            result -= dislike_factor() * (
                numbers[0] * AIDependencies.STABILITY_PER_LIKED_BUILDING_ON_PLANET
                + numbers[1] * AIDependencies.STABILITY_PER_LIKED_BUILDING_IN_SYSTEM
                + numbers[2] ** 0.5 * AIDependencies.STABILITY_BASE_LIKED_BUILDING_ELSEWHERE
            )
    return result


def _evaluate_xenophobia(planet, species) -> float:
    if AIDependencies.Tags.XENOPHOBIC not in species.tags:
        return 0.0
    colonies = get_colonized_planets()
    relevant_systems = within_n_jumps(planet.systemID, 5) & set(colonies.keys())
    result = 0.0
    universe = fo.getUniverse()
    value = get_named_real("XENOPHOBIC_SELF_TARGET_HAPPINESS_COUNT")
    for sys_id in relevant_systems:
        for pid in colonies[sys_id]:
            planet_species = universe.getPlanet(pid).speciesName
            if planet_species not in ("SP_EXOBOT", species.name, ""):
                result += value  # value is negative
    return result


def _evaluate_administration(planet: fo.planet, species: fo.species) -> float:
    result = 0.0
    palace = BuildingType.PALACE
    universe = fo.getUniverse()
    if AIDependencies.Tags.INDEPENDENT not in species.tags:
        admin_systems = BuildingType.REGIONAL_ADMIN.built_or_queued_at_sys() | palace.built_or_queued_at_sys()
        jumps_to_admin = min((universe.jumpDistance(planet.systemID, admin) for admin in admin_systems), default=99)
        # cap at 5, if it is 6 or more, we could build a Regional Admin on this planet
        # TODO: check if the planet would be supply-connected to the nearest administration
        # disconnected gives namedReal("DISCONNECTED_FROM_CAPITAL_AND_REGIONAL_ADMIN_STABILITY_PENALTY")
        result += 5 - min(jumps_to_admin, 5)
    if palace.built_at():  # bonus is only given the capital actually contains the palace
        capital = universe.getPlanet(fo.getEmpire().capitalID)
        if species.name == capital.speciesName:
            result += 5.0
    return result
