import freeOrionAIInterface as fo
from logging import debug
from typing import NamedTuple

import AIDependencies
import EspionageAI
import MilitaryAI
import PlanetUtilsAI
from aistate_interface import get_aistate
from buildings import BuildingType
from colonization import get_nest_rating, special_is_nest
from colonization.calculate_influence import calculate_influence
from colonization.calculate_population import calc_max_pop
from colonization.calculate_production import calculate_production
from colonization.calculate_research import calculate_research
from colonization.calculate_stability import calculate_stability
from colonization.claimed_stars import count_claimed_stars
from colonization.colony_score import MINIMUM_COLONY_SCORE, RESOURCE_PRIORITY_MULTIPLIER
from colonization.rate_pilots import detection_value, rate_colony_for_pilots
from common.fo_typing import PlanetId, SpeciesName, SystemId
from empire.growth_specials import get_growth_specials
from EnumsAI import FocusType, MissionType, PriorityType
from freeorion_tools import (
    assertion_fails,
    get_named_real,
    get_partial_visibility_turn,
    get_species_supply,
    tech_is_complete,
    tech_soon_available,
)
from freeorion_tools.caching import cache_for_current_turn
from PlanetUtilsAI import get_empire_populated_planets, stability_with_focus
from turn_state import (
    get_empire_outposts,
    get_empire_planets_by_species,
    get_owned_planets_in_system,
    get_supply_group,
    get_supply_group_id,
    get_system_supply,
    have_computronium,
    have_honeycomb,
    have_worldtree,
    owned_asteroid_coatings,
    supply_connected,
)
from universe.system_network import get_neighbors, within_n_jumps

empire_metabolisms = {}


def calculate_planet_colonization_rating(
    *,
    planet_id: PlanetId,
    mission_type: MissionType,
    spec_name: SpeciesName | None,
    detail: list | None,
) -> float:
    """Returns the colonisation value of a planet."""
    if spec_name is None:
        spec_name = ""

    planet = fo.getUniverse().getPlanet(planet_id)
    rating, new_detail = _calculate_planet_rating(planet, spec_name)
    owned = int(planet.owner == fo.empireID())
    debug(f"colrating {mission_type}-{owned} {planet} {spec_name} rating={rating}, detail={new_detail}")
    if detail:
        detail.clear()
        detail.extend(new_detail)
    return rating


def _determine_colony_threat_factor(planet, spec_name, existing_presence):
    sys_status = get_aistate().systemStatus.get(planet.systemID, {})
    cur_best_mil_ship_rating = max(MilitaryAI.cur_best_mil_ship_rating(), 0.001)
    local_defenses = sys_status.get("all_local_defenses", 0)
    local_threat = sys_status.get("fleetThreat", 0) + sys_status.get("monsterThreat", 0)
    neighbor_threat = sys_status.get("neighborThreat", 0)
    jump2_threat = 0.6 * max(0, sys_status.get("jump2_threat", 0) - sys_status.get("my_neighbor_rating", 0))
    area_threat = neighbor_threat + jump2_threat
    area_threat *= 2.0 / (existing_presence + 2)  # once we have a foothold be less scared off by area threats
    # TODO: evaluate detectability by specific source of area threat, also consider if the subject AI already has
    # researched techs that would grant a stealth bonus
    local_enemies = sys_status.get("enemies_nearly_supplied", [])
    # could more conservatively base detection on get_aistate().misc.get("observed_empires")
    if not EspionageAI.colony_detectable_by_empire(planet.id, spec_name, empire=local_enemies, default_result=True):
        area_threat *= 0.05
    net_threat = max(0, local_threat + area_threat - local_defenses)
    # even if our military has lost all warships, rate planets as if we have at least one
    reference_rating = max(
        cur_best_mil_ship_rating,
        MilitaryAI.get_preferred_max_military_portion_for_single_battle()
        * MilitaryAI.get_concentrated_tot_mil_rating(),
    )
    threat_factor = min(1.0, reference_rating / (net_threat + 0.001)) ** 2
    if threat_factor < 0.5:
        mil_ref_string = "Military rating reference: %.1f" % reference_rating
        debug(
            "Significant threat discounting %2d%% at %s, local defense: %.1f, local threat %.1f, area threat %.1f"
            % (100 * (1 - threat_factor), planet.name, local_defenses, local_threat, area_threat)
            + mil_ref_string
        )
    return threat_factor


def _revise_threat_factor(
    threat_factor: float,
    planet_value: float,
    system_id: int,
    min_planet_value: float = MINIMUM_COLONY_SCORE,
) -> float:
    """
    Check if the threat_factor should be made less severe.

    If the AI does have enough total military to secure this system, and the target has more than minimal value,
    don't let the threat_factor discount the adjusted value below min_planet_value +1, so that if there are no
    other targets the AI could still pursue this one.  Otherwise, scoring pressure from
    MilitaryAI.get_preferred_max_military_portion_for_single_battle might prevent the AI from pursuing a heavily
    defended but still obtainable target even if it has no softer locations available.

    :param threat_factor: the base threat factor
    :param planet_value: the planet score
    :param system_id: the system ID of subject planet
    :param min_planet_value: a floor planet value if the AI has enough military to secure the system
    :return: the (potentially) revised threat_factor
    """

    # the default value below for fleetThreat shouldn't come in to play, but is just to be absolutely sure we don't
    # send colony ships into some system for which we have not evaluated fleetThreat
    system_status = get_aistate().systemStatus.get(system_id, {})
    system_fleet_treat = system_status.get("fleetThreat", 1000)
    # TODO: consider taking area threat into account here.  Arguments can be made both ways, see discussion in PR2069
    sys_total_threat = system_fleet_treat + system_status.get("monsterThreat", 0) + system_status.get("planetThreat", 0)
    if (MilitaryAI.get_concentrated_tot_mil_rating() > sys_total_threat) and (planet_value > 2 * min_planet_value):
        threat_factor = max(threat_factor, (min_planet_value + 1) / planet_value)
    return threat_factor


def _rate_production(pp: float) -> float:
    return pp * RESOURCE_PRIORITY_MULTIPLIER * get_aistate().get_priority(PriorityType.RESOURCE_PRODUCTION)


def _rate_research(rp: float) -> float:
    # Research priority starts rather high, but in the long term it is usually much lower than production.
    return (
        rp
        * RESOURCE_PRIORITY_MULTIPLIER
        * min(
            get_aistate().get_priority(PriorityType.RESOURCE_PRODUCTION),
            get_aistate().get_priority(PriorityType.RESOURCE_RESEARCH),
        )
    )


def _rate_influence(ip: float) -> float:
    return ip * RESOURCE_PRIORITY_MULTIPLIER * get_aistate().get_priority(PriorityType.RESOURCE_INFLUENCE)


def _path_to_capital(system_id: SystemId):
    """Returns whether we know a path from our capital to that system. Always returns true if we have no capital."""
    universe = fo.getUniverse()
    capital_id = PlanetUtilsAI.get_capital()
    homeworld = universe.getPlanet(capital_id)
    if homeworld:
        home_system_id = homeworld.systemID
        least_jumps = universe.jumpDistance(home_system_id, system_id)
        return least_jumps != -1  # -1 means no path
    return True


def _planet_supply(planet: fo.planet) -> int:
    universe = fo.getUniverse()
    planet_supply = AIDependencies.supply_by_size.get(planet.size, 0)
    bld_types = {universe.getBuilding(bldg).buildingTypeName for bldg in planet.buildingIDs}.intersection(
        AIDependencies.building_supply
    )
    planet_supply += sum(
        AIDependencies.building_supply[bld_type].get(int(psize), 0)
        for psize in [-1, planet.size]
        for bld_type in bld_types
    )
    system = universe.getSystem(planet.systemID)
    supply_specials = set(planet.specials).union(system.specials).intersection(AIDependencies.SUPPLY_MOD_SPECIALS)
    planet_supply += sum(
        AIDependencies.SUPPLY_MOD_SPECIALS[_special].get(int(psize), 0)
        for _special in supply_specials
        for psize in [-1, planet.size]
    )
    return planet_supply


@cache_for_current_turn
# hashing entire planets? Perhaps better us a tuple of pids?
def _count_producers(candidates: tuple[fo.planet], min_stability: float, number: bool) -> float:
    """
    Return number of planets (number) or population on planets (not number) amongst candidates,
    which have industry focus and at least min_stability. Actually, since foci may change, planet currently
    having the focus are count 80% and planets having other foci are counted 30%.
    """
    value = 0.0
    for planet in candidates:
        if stability_with_focus(planet, FocusType.FOCUS_INDUSTRY) >= min_stability:
            focus_factor = 0.8 if planet.focus == FocusType.FOCUS_INDUSTRY else 0.3
            # TBD: perhaps use something between current and target population?
            # Surely not current, it will take time until we get the bonus we're evaluating...
            value += focus_factor * (1.0 if number else planet.currentMeterValue(fo.meterType.targetPopulation))
    return value


def _already_have_types(locally_owned_planets: list[fo.planet]) -> tuple[bool, bool]:
    """
    Determine whether list of planets contains asteroids and a gas giant.
    Gas giants are not considered, if they are populated by a species that dislikes GGG and the
    planet does not already have one built or enqueued.
    """
    have_asteroids = False
    have_gas_giant = False
    for planet in locally_owned_planets:
        if planet.type == fo.planetType.asteroids:
            have_asteroids = True
        if planet.type == fo.planetType.gasGiant:
            species = fo.getSpecies(planet.speciesName)
            ggg = BuildingType.GAS_GIANT_GEN
            # Currently, all gaseous beings dislike GGG, another one to place a GGG may be good
            if not species or ggg.value not in species.dislikes or planet.id in ggg.built_or_queued_at():
                have_gas_giant = True
    return have_asteroids, have_gas_giant


class _LocalPresence(NamedTuple):
    number: int  # number outposts + 3 x number of colonies
    asteroids_rating: float  # estimate how many colonies would profit from Microgravity Industry
    ggg_rating: float  # estimate how many colonies would profit from a gas giant generator


def _determine_local_presence(planet: fo.planet, details: list) -> _LocalPresence:
    universe = fo.getUniverse()
    planets = [universe.getPlanet(pid) for pid in get_owned_planets_in_system(planet.systemID) if pid != planet.id]
    populated = tuple(planet for planet in planets if planet.speciesName)
    details.append(f"local presence {len(planets)}({len(populated)} pop.)")
    asteroids_rating = ggg_rating = 0.0
    have_asteroids, have_gas_giant = _already_have_types(planets)
    # Check the pre-requisite tech for Microgravity Industry and Gas Giant Generators to allow for parallel research/production
    # TODO consider checking the pre-requisites directly from the tech
    if planet.type in (fo.planetType.gasGiant, fo.planetType.asteroids):
        if planet.type == fo.planetType.asteroids:
            if not have_asteroids and tech_is_complete(AIDependencies.CON_ORBITAL_CON):
                users = _count_producers(populated, get_named_real("PRO_MICROGRAV_MAN_MIN_STABILITY"), True)
                bonus = get_named_real("PRO_MICROGRAV_MAN_TARGET_INDUSTRY_FLAT")
                asteroids_rating = _rate_production(users * bonus)
                details.append(f"Mirco-gravity users={users}, rating={asteroids_rating}")
        else:  # gas giant
            if not have_gas_giant and tech_is_complete(AIDependencies.CON_ORBITAL_CON):
                users = _count_producers(populated, get_named_real("BLD_GAS_GIANT_GEN_MIN_STABILITY"), True)
                bonus = get_named_real("BLD_GAS_GIANT_GEN_OUTPOST_TARGET_INDUSTRY_FLAT")
                ggg_rating = _rate_production(users * bonus)
                details.append(f"GGG users={users}, rating={ggg_rating}")
    return _LocalPresence(len(planets) + 2 * len(populated), asteroids_rating, ggg_rating)


def _check_solar_orbital_generator(system: fo.system, details: list) -> float:
    """
    Checks whether a planet in this system could allow us building a solar orbital generator.
    If possible, return a value for that possibility, otherwise return 0.
    """
    tech = AIDependencies.PRO_SOL_ORB_GEN
    rating = 0.0
    if system.starType in [fo.starType.blue, fo.starType.white]:
        factor = 0.0
        if tech_soon_available(tech, 5):
            bw_stars_claimed = count_claimed_stars(fo.starType.blue) + count_claimed_stars(fo.starType.white)
            if not bw_stars_claimed:
                factor = 1.0
                details.append(f"SOG(bright): {tech} and first bw star")
            else:
                # give a little bonus as backup
                factor = 1.0 / (bw_stars_claimed + 3)
                details.append(f"SOG(bright): {tech}, having {bw_stars_claimed} stars")
        if factor > 0.0:
            min_stability = get_named_real("BLD_SOL_ORB_GEN_MIN_STABILITY")
            gain_per_pop = get_named_real("BLD_SOL_ORB_GEN_BRIGHT_TARGET_INDUSTRY_PERPOP")
            users = _count_producers(get_empire_populated_planets(), min_stability, False)
            rating = _rate_production(factor * users * gain_per_pop)
            details.append(f"SOG(bright): users={users}, rating={rating}")
    return rating


def _check_blackhole_generator(system: fo.system, details: list) -> float:
    """
    Checks whether a planet in this system could allow us building a black hole generator.
    If possible, return a value for that possibility, otherwise return 0.
    """
    # Wait for Theory of Everything, because that is really expensive.
    # Once we have that, Singularity Generation shouldn't take too long, and we may as well prepare for it.
    tech1 = AIDependencies.LRN_EVERYTHING
    # red planets are only considered, if we could turn them into black holes
    tech2 = AIDependencies.LRN_ART_BLACK_HOLE
    tech3 = AIDependencies.PRO_SINGULAR_GEN
    rating = 0.0
    if tech_is_complete(tech1) and tech_soon_available(tech3, 5):
        factor = 0.0
        black_holes_claimed = count_claimed_stars(fo.starType.blackHole)
        if system.starType == fo.starType.blackHole:
            # blackholes with planets are pretty rare
            factor = 1.0 / (black_holes_claimed + 1)
            details.append(f"BHG: {tech1} and {black_holes_claimed} black holes yet")
        elif tech_is_complete(tech2):
            red_stars_claimed = count_claimed_stars(fo.starType.red)
            # we could turn this into a black hole
            factor = 1.0 / (2 * black_holes_claimed + red_stars_claimed + 1)
            details.append(f"BHG: {tech2}, {black_holes_claimed} black holes, {red_stars_claimed} reds")
        if not tech_is_complete(tech3):
            factor *= 0.5  # could not start building the BHG yet
        if factor:
            min_stability = get_named_real("BLD_BLACK_HOLE_POW_GEN_MIN_STABILITY")
            gain_per_pop = get_named_real("BLD_BLACK_HOLE_POW_GEN_TARGET_INDUSTRY_PERPOP")
            users = _count_producers(get_empire_populated_planets(), min_stability, False)
            rating = _rate_production(factor * gain_per_pop * users)
            details.append(f"BHG: users={users}, rating={rating}")
    return rating


def _check_neutronium_extraction(system: fo.system, details: list) -> float:
    """
    Checks whether a planet in this system could allow us building a neutronium extractor.
    If possible, return a value for that possibility, otherwise return 0.
    """
    rating = 0.0
    if system.starType == fo.starType.neutron:
        if tech_soon_available(AIDependencies.PRO_NEUTRONIUM_EXTRACTION, 5):
            neutron_stars_claimed = count_claimed_stars(fo.starType.neutron)
            # Note that a NEUTRONIUM_EXTRACTOR is much cheaper than a NEUTRONIUM_SYNTH, so even if can build
            # a synthesizer, getting a neutron star may help.
            built = len(BuildingType.NEUTRONIUM_SYNTH.built_at()) + len(BuildingType.NEUTRONIUM_EXTRACTOR.built_at())
            rating = 1.0 / (neutron_stars_claimed + built + 1)
        rating *= 5 * MINIMUM_COLONY_SCORE
        details.append(f"neutronium extraction={rating}")
    return rating


def _check_ruins(planet: fo.planet, details: list) -> float:
    """Check whether the planet has still unexplored ancients ruins and return a value for that."""
    rating = 0.0
    if AIDependencies.ANCIENT_RUINS_SPECIAL in planet.specials:
        rating = 1.0 if tech_is_complete(AIDependencies.LRN_XENOARCH) else 0.5
        # this can be very valuable!
        rating *= 10 * MINIMUM_COLONY_SCORE
        details.append(f"unexplored ancient ruins={rating}")
    return rating


def _check_asteroid_coating(planet: fo.planet, details: list) -> float:
    """Check whether the planet has an asteroid_coating special and return a value for that."""
    rating = 0.0
    # do we really need two different specials? Well, at the moment we have two...
    if (
        AIDependencies.ASTEROID_COATING_OWNED_SPECIAL in planet.specials
        or AIDependencies.ASTEROID_COATING_SPECIAL in planet.specials
    ):
        rating = 2 * MINIMUM_COLONY_SCORE
        if owned_asteroid_coatings():
            rating /= 3 * owned_asteroid_coatings()
        if not tech_is_complete(AIDependencies.SHP_ASTEROID_HULLS):
            rating /= 2
        details.append(f"asteroid coating={rating}")
    return rating


def _check_worldtree(planet: fo.planet, details: list) -> float:
    """Check whether the planet has an worldtree special and return a value for that."""
    rating = 0.0
    if not have_worldtree and AIDependencies.WORLDTREE_SPECIAL in planet.specials:
        rating = 0.5 * MINIMUM_COLONY_SCORE
        details.append(f"worldtree={rating}")
    return rating


def _check_nests(planet: fo.planet, details: list) -> float:
    """Check whether the planet has a nest special and return a value for that."""
    for special in planet.specials:
        if special_is_nest(special):
            nest_val = get_nest_rating(special)
            details.append(f"{special}={nest_val}")
            return nest_val
    return 0.0


def _rate_supply_extension(system: fo.system, planet_supply: int) -> float:
    """
    Determines how many supply groups an outpost with the given supply in the system would connect to.
    If this returns more than one, the outpost would potentially connect still disconnected planets.
    """
    if planet_supply < 1:
        return 0.0
    # planet_supply should be an int, better cast it just in case we get a float e.g. from a planet meter
    supplied = within_n_jumps(system.systemID, int(planet_supply))
    potentially_added = 0
    groups = set()
    for sys_id in supplied:
        if get_system_supply(sys_id) < 0:
            potentially_added += 1
        else:
            group = get_supply_group_id(sys_id)
            if not assertion_fails(group >= 0):
                groups.add(group)
    # supplying additional planets may be helpful, connecting planets to the capital helps a lot!
    groups_connected = max(len(groups) - 1, 0)
    capital_connected = 0
    if groups_connected:
        capital_group = get_supply_group_id(PlanetUtilsAI.get_capital_sys_id())
        if capital_group in groups:  # TODO: only count populated planets?
            capital_connected = sum(
                len(PlanetUtilsAI.get_planets_in__systems_ids(get_supply_group(g)))
                for g in groups
                if g != capital_group
            )
    if groups_connected:
        debug(
            f"Outpost at {system} with supply {planet_supply} connects {groups_connected} supply groups and "
            f"{capital_connected} planets to the capital"
        )
    return (0.05 * potentially_added + 0.2 * groups_connected + 0.5 * capital_connected) * MINIMUM_COLONY_SCORE


@cache_for_current_turn
def _rate_supply(system: fo.system, planet_supply: int) -> tuple[float, float]:
    """
    Evaluate supply situation. Return value for added supply and a factor to multiply the overall rating
    with that is lower for badly supplied planets.
    """
    sys_supply = get_system_supply(system.systemID)
    planet_supply = max(planet_supply, 0)  # there is no negative supply
    if sys_supply == 0:
        # get_system_supply == 0 means supplied, check if planet is on the border
        neighbors = get_neighbors(system.systemID)
        if all(supply_connected(sys_id, system.systemID) for sys_id in neighbors):
            factor = 1.0  # all neighbours are in the same supply group, so supply seems pretty save
        else:
            # planet is on the border of our supply network
            factor = 0.9 if planet_supply == 0 else 1.0
    else:
        # planet is outside the current supply network
        supply_sum = planet_supply + sys_supply
        if supply_sum >= 0:
            factor = min(1.0, 0.7 + 0.1 * supply_sum)  # >= 0.7
        else:
            factor = max(0.25, 0.6 + 0.1 * supply_sum)  # <= 0.5, planet would not be connected
    rating = _rate_supply_extension(system, planet_supply)
    return rating, factor


class _OutpostValues(NamedTuple):
    rating: float
    planet_supply: int
    supply_factor: float
    local_presence: int
    details: list


@cache_for_current_turn
def _calculate_outpost_rating(planet: fo.planet, supply_modifier: int) -> _OutpostValues:
    details = []
    empire = fo.getEmpire()
    universe = fo.getUniverse()
    system = universe.getSystem(planet.systemID)
    local_presence = _determine_local_presence(planet, details)
    planet_supply = _planet_supply(planet) + supply_modifier
    supply_rating, supply_factor = _rate_supply(system, planet_supply)
    if planet.owner == empire.empireID:
        # We already own it, so most outpost checks are pointless, but a species could extend supply range
        current_supply = int(planet.currentMeterValue(fo.meterType.maxSupply))
        current_supply_rating, _ = _rate_supply(system, current_supply)
        supply_rating -= current_supply_rating
        details.append(f"supply extension={planet_supply - current_supply}, rating={supply_rating}")
        return _OutpostValues(supply_rating, planet_supply, 1.0, local_presence.number, details)
    details.append(f"base_supply={planet_supply}, rating={supply_rating}, factor={supply_factor}")
    rating = (
        local_presence.asteroids_rating
        + local_presence.ggg_rating
        + local_presence.number * MINIMUM_COLONY_SCORE / 15
        + _check_solar_orbital_generator(system, details)
        + _check_blackhole_generator(system, details)
        + _check_neutronium_extraction(system, details)
        + _check_ruins(planet, details)  # TODO? AI could excavate them without supply connection
        + _check_asteroid_coating(planet, details)
        + _check_worldtree(planet, details)
        + _check_nests(planet, details)
        # TODO fortress?
    )
    rating = rating * supply_factor + supply_rating
    details.append(f"outpost_rating={rating}")
    return _OutpostValues(rating, planet_supply, supply_factor, local_presence.number, details)


def _check_cultural_buildings(planet: fo.planet, stability: float, max_population: float) -> tuple[float, float]:
    """
    Determines production and research would could get from cultural buildings.
    Checks for Cultural Archive, Cultural Library and Automatic History Analyser.
    """
    have_archive = have_library = have_analyser = False
    for building_id in planet.buildingIDs:
        have_archive |= BuildingType.CULTURE_ARCHIVES.is_this_type(building_id)
        have_library |= BuildingType.CULTURE_LIBRARY.is_this_type(building_id)
        have_analyser |= BuildingType.AUTO_HISTORY_ANALYSER.is_this_type(building_id)
    research = production = 0.0
    if have_archive and stability >= get_named_real("BLD_CULTURE_ARCHIVES_MIN_STABILITY"):
        research += get_named_real("BLD_CULTURE_ARCHIVES_TARGET_RESEARCH_FLAT")
        production += get_named_real("BLD_CULTURE_ARCHIVES_TARGET_INDUSTRY_PERPOP") * max_population
    # Most likely every planet having an archive also has a history analyser, but we could build one if it
    # only has the archive.
    if have_analyser or have_archive:
        research += get_named_real("BLD_AUTO_HISTORY_ANALYSER_TARGET_RESEARCH_FLAT")
    if have_library and stability >= get_named_real("BLD_CULTURE_LIBRARY_MIN_STABILITY"):
        research += get_named_real("BLD_CULTURE_LIBRARY_TARGET_RESEARCH_FLAT")
    return research, production


def _rate_focus_independent(planet: fo.planet, species: fo.species, stability: float, max_population: float) -> float:
    """
    Rate focus independent focuses production.
    """
    research = production = influence = 0.0
    if stability <= 0:
        return 0.0  # TBD: lower rating is barely above 0, since it would take very long to get the full bonuses?

    min_stability = get_named_real("PRO_ADAPTIVE_AUTO_MIN_STABILITY")
    if tech_soon_available(AIDependencies.PRO_AUTO_1, 3) and stability >= min_stability:
        production += get_named_real("PRO_ADAPTIVE_AUTO_TARGET_INDUSTRY_FLAT")

    # Nascent AI is such an early research, we will probably have it by the time the colony reached the stability
    if stability >= get_named_real("LRN_NASCENT_AI_MIN_STABILITY"):
        research += get_named_real("LRN_NASCENT_AI_TARGET_RESEARCH_FLAT")

    min_stability = get_named_real("ECCENTRIC_ORBIT_MIN_STABILITY")
    if "ECCENTRIC_ORBIT_SPECIAL" in planet.specials and stability >= min_stability:
        research += get_named_real("ECCENTRIC_ORBIT_TARGET_RESEARCH_FLAT")

    if (
        AIDependencies.Tags.ARTISTIC in species.tags
        and fo.getEmpire().policyAdopted("PLC_ARTISAN_WORKSHOPS")
        and stability >= get_named_real("ARTISANS_MIN_STABILITY_NO_FOCUS")
    ):
        # TBD? Strictly speaking, this is not focus independent, since it is not cumulative with the focused bonus
        influence += get_named_real("ARTISANS_INFLUENCE_FLAT_NO_FOCUS")

    cultural_research, cultural_production = _check_cultural_buildings(planet, stability, max_population)
    research += cultural_research
    production += cultural_production

    # TODO Philosopher Planet, goes both ways...

    rating = _rate_research(research) + _rate_production(production) + _rate_influence(influence)
    return rating


def _check_honeycomb(planet: fo.planet) -> float:
    # TBD more sophisticated calculation
    value = get_aistate().get_priority(PriorityType.RESOURCE_PRODUCTION) * 2
    if AIDependencies.HONEYCOMB_SPECIAL in planet.specials:
        if not have_honeycomb():
            return value
        else:
            return value / 3
    return 0


def _rate_with_industry_focus(
    planet: fo.planet,
    species: fo.species,
    max_pop: float,
    base_stability: float,
    details: list,
) -> float:
    focus = FocusType.FOCUS_INDUSTRY
    if focus not in species.foci:
        return 0.0
    stability = base_stability + PlanetUtilsAI.focus_stability_effect(species, focus)
    focus_rating = _rate_production(calculate_production(planet, species, max_pop, stability))
    flat_rating = _rate_focus_independent(planet, species, stability, max_pop)
    honey_rating = _check_honeycomb(planet)
    details.append(f"industry: stability={stability} focus={focus_rating}, flat={flat_rating}, honey={honey_rating}")
    return focus_rating + flat_rating + honey_rating


def _check_computronium(planet: fo.planet) -> float:
    # TBD more sophisticated calculation
    value = get_aistate().get_priority(PriorityType.RESOURCE_RESEARCH) * 1.5
    if AIDependencies.COMPUTRONIUM_SPECIAL in planet.specials:
        if not have_computronium():
            return value
        else:
            return value / 3
    return 0


def _rate_with_research_focus(
    planet: fo.planet,
    species: fo.species,
    max_pop: float,
    base_stability: float,
    details: list,
) -> float:
    focus = FocusType.FOCUS_RESEARCH
    if focus not in species.foci:
        return 0.0
    stability = base_stability + PlanetUtilsAI.focus_stability_effect(species, focus)
    focus_rating = _rate_research(calculate_research(planet, species, max_pop, stability))
    flat_rating = _rate_focus_independent(planet, species, stability, max_pop)
    computronium_rating = _check_computronium(planet)
    details.append(
        f"research: stability={stability} focus={focus_rating}, flat={flat_rating}, comp={computronium_rating}"
    )
    return focus_rating + flat_rating + computronium_rating


def _rate_with_influence_focus(
    planet: fo.planet,
    species: fo.species,
    max_pop: float,
    base_stability: float,
    details: list,
) -> float:
    focus = FocusType.FOCUS_INFLUENCE
    if focus not in species.foci:
        return 0.0
    stability = base_stability + PlanetUtilsAI.focus_stability_effect(species, focus)
    focus_rating = _rate_influence(calculate_influence(planet, species, max_pop, stability))
    flat_rating = _rate_focus_independent(planet, species, stability, max_pop)
    details.append(f"influence: stability={stability} focus={focus_rating}, flat={flat_rating}")
    return focus_rating + flat_rating


def _rate_growth_special(special: str) -> float:
    # habitable size affected by this special
    boost_value = empire_metabolisms.get(AIDependencies.metabolismBoosts[special], 0)
    multiplier = 1.0
    universe = fo.getUniverse()
    for pid in get_growth_specials().get(special, []):
        planet = universe.getPlanet(pid)
        if planet.focus == FocusType.FOCUS_GROWTH:
            # TODO check supply
            multiplier = 0.4  # we already have it, but a backup may still be useful
        else:
            species = fo.getSpecies(planet.speciesName)
            if species and FocusType.FOCUS_GROWTH in species.foci:
                multiplier = min(multiplier, 0.9)  # planet is currently not exporting it, but it could
    return boost_value * multiplier * MINIMUM_COLONY_SCORE * 0.3


def _rate_with_growth_focus(
    planet: fo.planet,
    species: fo.species,
    max_pop: float,
    base_stability: float,
    details: list,
) -> float:
    focus = FocusType.FOCUS_GROWTH
    if focus not in species.foci:
        return 0.0
    stability = base_stability + PlanetUtilsAI.focus_stability_effect(species, focus)
    focus_rating = 0.0
    for special in [spec for spec in planet.specials if spec in AIDependencies.metabolismBoosts]:
        rating = _rate_growth_special(special)
        focus_rating += rating
        details.append(f"Bonus for {special}: {rating:.1f}")
    flat_rating = _rate_focus_independent(planet, species, stability, max_pop)
    details.append(f"growth: stability={stability} focus={focus_rating}, flat={flat_rating}")
    return focus_rating + flat_rating


def _rate_planet_vision(species: fo.species, supply_factor: float, details: list) -> float:
    """
    Give a small bonus to species with better detection.
    """
    detection_val = detection_value(species.name)  # -1 to 3
    # Well supplied planets are likely surrounded by other colonies. Vision is more important in the outskirts.
    # Could do some more sophisticated calculations here, but there or more important factors...
    supply_val = 1.05 - max(supply_factor, 0.8)  # 0.5 to 0.25
    scaling = MINIMUM_COLONY_SCORE * 0.3
    result = detection_val * supply_val * scaling
    if result:
        details.append(f"detection bonus:{result:.1f}")
    return result


def _adapt_population_for_rating(planet: fo.planet, max_population, details: list) -> float:
    current_population = planet.currentMeterValue(fo.meterType.population)
    if not current_population:
        current_population = 3.0 if tech_is_complete("GRO_LIFECYCLE_MAN") else 1.0
    # If more than half full, we consider its full potential population
    # TODO: could do some calculations to predict size in e.g. 20 turns
    result = min((2 * current_population + max_population) / 2, max_population)
    details.append(f"Adapted population: {result}")
    return result


def _calculate_colony_rating(planet: fo.planet, species: fo.species, supply_factor: float, details: list) -> float:
    dummy = []
    max_pop = calc_max_pop(planet, species, dummy)  # too much detail...
    if max_pop <= 0:
        details.append(f"{species.name} wouldn't survive")
        return 0.0
    details.append(f"max_pop={max_pop}")

    max_pop = _adapt_population_for_rating(planet, max_pop, details)
    base_stability = calculate_stability(planet, species)
    industry_rating = _rate_with_industry_focus(planet, species, max_pop, base_stability, details)
    research_rating = _rate_with_research_focus(planet, species, max_pop, base_stability, details)
    influence_rating = _rate_with_influence_focus(planet, species, max_pop, base_stability, details)
    growth_rating = _rate_with_growth_focus(planet, species, max_pop, base_stability, details)
    options = sorted((industry_rating, research_rating, influence_rating, growth_rating))
    best = options.pop()
    second = options.pop()
    rating = best + max(0.0, second - best * 0.5)  # if second best is more than half of best, increase rating
    pilot_rating = rate_colony_for_pilots(planet, species, details)
    vision_rating = _rate_planet_vision(species, supply_factor, details)
    rating += pilot_rating + vision_rating
    # TODO: Xenophobic harassment (by this planet and by others)
    details.append(
        f"industry: {industry_rating}, research: {research_rating}, influence: {influence_rating}, "
        f"growth: {growth_rating}, pilots: {pilot_rating} => rating={rating}"
    )
    return rating


def _adapt_supply_factor(supply_factor: float, species: fo.species) -> float:
    """
    Limit the supply factor (penalty for bad supply) for INDEPENDENT species, especially Sly, which have basically
    no other chance then to settle in disconnected places for a long time.
    """
    if AIDependencies.Tags.INDEPENDENT in species.tags:
        current_turn = fo.currentTurn()
        if species.name == "SP_SLY":
            return max(supply_factor, 1.0 - current_turn * 0.003)  # minimum 0.4 at turn 200
        else:
            return max(supply_factor, 0.9 - current_turn * 0.005)  # minimum 0.4 at turn 100
    else:
        return supply_factor


def _colony_upkeep(num_exobots: int, num_normal: int, num_outpost: int) -> float:
    colonies = num_exobots + num_normal - 1  # assume we have a capital, calculation won't be too wrong otherwise
    per_colony = get_named_real("COLONY_ADMIN_COSTS_PER_PLANET")
    outpost_factor = get_named_real("OUTPOST_RELATIVE_ADMIN_COUNT")
    # see influence.macros, exobot colonies pay like a colony, but count as outposts for the cost increase
    return colonies * per_colony * (num_normal + outpost_factor * (num_exobots + num_outpost)) ** 0.5


def _rate_upkeep(planet: fo.planet, species_name: SpeciesName, details: list) -> float:
    exobot = SpeciesName("SP_EXOBOT")
    num_exobots = len(get_empire_planets_by_species().get(exobot, []))
    num_normal = len(get_empire_populated_planets()) - num_exobots
    num_outpost = len(get_empire_outposts())
    current_upkeep = _colony_upkeep(num_exobots, num_normal, num_outpost)
    if species_name == exobot:
        num_exobots += 1
    elif not species_name:
        num_outpost += 1
    else:
        num_normal += 1
    ip_cost = current_upkeep - _colony_upkeep(num_exobots, num_normal, num_outpost)
    species = fo.getSpecies(species_name)
    if species and planet.id in species.homeworlds:
        ip_cost -= AIDependencies.HOMEWORLD_INFLUENCE_COST
    result = _rate_influence(ip_cost)
    details.append(f"IP upkeep: {ip_cost} -> {result}")
    return result


def _calculate_planet_rating(planet: fo.planet, species_name: SpeciesName) -> tuple[float, list]:
    if not planet:
        return 0.0, ["unknown planet"]
    if not _path_to_capital(planet.systemID):
        return 0.0, ["no path from capital"]
    species = None
    if species_name:
        species = fo.getSpecies(species_name)
        if assertion_fails(species, f"unknown species {species_name}"):
            return 0.0, [f"unknown species {species_name}"]
        supply_modifier = get_species_supply(species_name)
    else:
        supply_modifier = 0
    sys_partial_vis_turn = get_partial_visibility_turn(planet.systemID)
    planet_partial_vis_turn = get_partial_visibility_turn(planet.id)
    # TODO: check whether this copied code makes sense
    if planet_partial_vis_turn < sys_partial_vis_turn:
        # last time we had partial vis of the system, the planet was stealthed to us
        return 0.0, ["Couldn't get current info, planet was stealthed at last sighting"]

    rating, planet_supply, supply_factor, local_presence, details = _calculate_outpost_rating(planet, supply_modifier)
    if species:
        colony_rating = _calculate_colony_rating(planet, species, supply_factor, details)
        supply_factor = _adapt_supply_factor(supply_factor, species)
        if colony_rating <= 0.0:
            return 0.0, details  # planet not habitable for species? skip the rest
        rating += colony_rating * supply_factor
    threat_factor = _determine_colony_threat_factor(planet, species_name, local_presence)
    if threat_factor < 1.0:
        threat_factor = _revise_threat_factor(threat_factor, rating, planet.systemID)
        rating *= threat_factor
        details.append("threat reducing value by %3d %%" % (100 * (1 - threat_factor)))
    rating += _rate_upkeep(planet, species_name, details)
    return rating, details
