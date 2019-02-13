from logging import debug, warn, error, info
from operator import itemgetter

import freeOrionAIInterface as fo  # pylint: disable=import-error

from common.print_utils import Table, Text, Sequence, Bool, Float
import AIDependencies
import AIstate
import EspionageAI
import FleetUtilsAI
import InvasionAI
import PlanetUtilsAI
import PriorityAI
import ProductionAI
import MilitaryAI
from aistate_interface import get_aistate
from target import TargetPlanet
from turn_state import state
from EnumsAI import MissionType, FocusType, EmpireProductionTypes, ShipRoleType, PriorityType
from freeorion_tools import tech_is_complete, get_ai_tag_grade, cache_by_turn, AITimer, get_partial_visibility_turn
from AIDependencies import (INVALID_ID, OUTPOSTING_TECH, POP_CONST_MOD_MAP,
                            POP_SIZE_MOD_MAP_MODIFIED_BY_SPECIES, POP_SIZE_MOD_MAP_NOT_MODIFIED_BY_SPECIES)

colonization_timer = AITimer('getColonyFleets()')


empire_colonizers = {}
empire_ship_builders = {}
empire_shipyards = {}
available_growth_specials = {}
active_growth_specials = {}
empire_metabolisms = {}
planet_supply_cache = {}  # includes system supply
all_colony_opportunities = {}

pilot_ratings = {}
colony_status = {}
facilities_by_species_grade = {}
system_facilities = {}


NEST_VAL_MAP = {
    "SNOWFLAKE_NEST_SPECIAL": 15,
    "KRAKEN_NEST_SPECIAL": 40,
    "JUGGERNAUT_NEST_SPECIAL": 80,
}

AVG_PILOT_RATING = 2.0
GOOD_PILOT_RATING = 4.0
GREAT_PILOT_RATING = 6.0
ULT_PILOT_RATING = 12.0

# minimum evaluation score that a planet must reach so it is considered for outposting or colonizing
MINIMUM_COLONY_SCORE = 60


def colony_pod_cost():
    return AIDependencies.COLONY_POD_COST * (1 + state.get_number_of_colonies()*AIDependencies.COLONY_POD_UPKEEP)


def outpod_pod_cost():
    return AIDependencies.OUTPOST_POD_COST * (1 + state.get_number_of_colonies()*AIDependencies.COLONY_POD_UPKEEP)


def calc_max_pop(planet, species, detail):
    planet_size = planet.habitableSize
    planet_env = species.getPlanetEnvironment(planet.type)
    if planet_env == fo.planetEnvironment.uninhabitable:
        detail.append("Uninhabitable.")
        return 0

    for bldg_id in planet.buildingIDs:
        building = fo.getUniverse().getBuilding(bldg_id)
        if not building:
            continue
        if building.buildingTypeName == "BLD_GATEWAY_VOID":
            detail.append("Gateway to the void: Uninhabitable.")
            return 0

    tag_list = list(species.tags) if species else []
    pop_tag_mod = AIDependencies.SPECIES_POPULATION_MODIFIER.get(get_ai_tag_grade(tag_list, "POPULATION"), 1.0)
    if planet.type == fo.planetType.gasGiant and "GASEOUS" in tag_list:
        gaseous_adjustment = AIDependencies.GASEOUS_POP_FACTOR
        detail.append("GASEOUS adjustment: %.2f" % gaseous_adjustment)
    else:
        gaseous_adjustment = 1.0

    base_pop_modified_by_species = 0
    base_pop_not_modified_by_species = 0
    pop_const_mod = 0

    # first, account for the environment
    environment_mod = POP_SIZE_MOD_MAP_MODIFIED_BY_SPECIES["environment_bonus"][planet_env]
    detail.append("Base environment: %d" % environment_mod)
    base_pop_modified_by_species += environment_mod

    # find all applicable modifiers
    for tech in POP_SIZE_MOD_MAP_MODIFIED_BY_SPECIES:
        if tech != "environment_bonus" and tech_is_complete(tech):
            base_pop_modified_by_species += POP_SIZE_MOD_MAP_MODIFIED_BY_SPECIES[tech][planet_env]
            detail.append("%s_PSM_early(%d)" % (tech, POP_SIZE_MOD_MAP_MODIFIED_BY_SPECIES[tech][planet_env]))

    for tech in POP_SIZE_MOD_MAP_NOT_MODIFIED_BY_SPECIES:
        if tech_is_complete(tech):
            base_pop_not_modified_by_species += POP_SIZE_MOD_MAP_NOT_MODIFIED_BY_SPECIES[tech][planet_env]
            detail.append("%s_PSM_late(%d)" % (tech, POP_SIZE_MOD_MAP_NOT_MODIFIED_BY_SPECIES[tech][planet_env]))

    for tech in POP_CONST_MOD_MAP:
        if tech_is_complete(tech):
            pop_const_mod += POP_CONST_MOD_MAP[tech][planet_env]
            detail.append("%s_PCM(%d)" % (tech, POP_CONST_MOD_MAP[tech][planet_env]))

    for _special in set(planet.specials).intersection(AIDependencies.POP_FIXED_MOD_SPECIALS):
        this_mod = sum(AIDependencies.POP_FIXED_MOD_SPECIALS[_special].get(int(psize), 0)
                       for psize in [-1, planet.size])
        detail.append("%s_PCM(%d)" % (_special, this_mod))
        pop_const_mod += this_mod

    for _special in set(planet.specials).intersection(AIDependencies.POP_PROPORTIONAL_MOD_SPECIALS):
        this_mod = sum(AIDependencies.POP_PROPORTIONAL_MOD_SPECIALS[_special].get(int(psize), 0)
                       for psize in [-1, planet.size])
        detail.append("%s (maxPop%+.1f)" % (_special, this_mod))
        base_pop_not_modified_by_species += this_mod

    #  exobots can't ever get to good environ so no gaia bonus, for others we'll assume they'll get there
    if "GAIA_SPECIAL" in planet.specials and species.name != "SP_EXOBOT":
        base_pop_not_modified_by_species += 3
        detail.append("Gaia_PSM_late(3)")

    if "SELF_SUSTAINING" in tag_list:
        base_pop_not_modified_by_species += 3
        detail.append("SelfSustaining_PSM_late(3)")

    applicable_boosts = set()
    for this_tag in [tag for tag in tag_list if tag in AIDependencies.metabolismBoostMap]:
        metab_boosts = AIDependencies.metabolismBoostMap.get(this_tag, [])
        for key in active_growth_specials.keys():
            if len(active_growth_specials[key]) > 0 and key in metab_boosts:
                applicable_boosts.add(key)
                detail.append("%s boost active" % key)
        for boost in metab_boosts:
            if boost in planet.specials:
                applicable_boosts.add(boost)
                detail.append("%s boost present" % boost)

    n_boosts = len(applicable_boosts)
    if n_boosts:
        base_pop_not_modified_by_species += n_boosts
        detail.append("boosts_PSM(%d from %s)" % (n_boosts, applicable_boosts))

    if planet.id in species.homeworlds:
        detail.append('Homeworld (2)')
        base_pop_not_modified_by_species += 2

    if AIDependencies.TAG_LIGHT_SENSITIVE in tag_list:
        star_type = fo.getUniverse().getSystem(planet.systemID).starType
        star_pop_mod = AIDependencies.POP_MOD_LIGHTSENSITIVE_STAR_MAP.get(star_type, 0)
        base_pop_not_modified_by_species += star_pop_mod
        detail.append("Lightsensitive Star Bonus_PSM_late(%.1f)" % star_pop_mod)

    def max_pop_size():
        species_effect = (pop_tag_mod - 1) * abs(base_pop_modified_by_species)
        gaseous_effect = (gaseous_adjustment - 1) * abs(base_pop_modified_by_species)
        base_pop = (base_pop_not_modified_by_species + base_pop_modified_by_species
                    + species_effect + gaseous_effect)
        return planet_size * base_pop + pop_const_mod

    if "PHOTOTROPHIC" in tag_list and max_pop_size() > 0:
        star_type = fo.getUniverse().getSystem(planet.systemID).starType
        star_pop_mod = AIDependencies.POP_MOD_PHOTOTROPHIC_STAR_MAP.get(star_type, 0)
        base_pop_not_modified_by_species += star_pop_mod
        detail.append("Phototropic Star Bonus_PSM_late(%0.1f)" % star_pop_mod)

    detail.append("max_pop = base + size*[psm_early + species_mod*abs(psm_early) + psm_late]")
    detail.append("        = %.2f + %d * [%.2f + %.2f*abs(%.2f) + %.2f]" % (
                    pop_const_mod, planet_size, base_pop_modified_by_species,
                    (pop_tag_mod+gaseous_adjustment-2), base_pop_modified_by_species,
                    base_pop_not_modified_by_species))
    detail.append("        = %.2f" % max_pop_size())
    return max_pop_size()


def galaxy_is_sparse():
    setup_data = fo.getGalaxySetupData()
    avg_empire_systems = setup_data.size / len(fo.allEmpireIDs())
    return ((setup_data.monsterFrequency <= fo.galaxySetupOption.low) and
            ((avg_empire_systems >= 40) or
             ((avg_empire_systems >= 35) and (setup_data.shape != fo.galaxyShape.elliptical))))


def rate_piloting_tag(tag_list):
    grades = {'NO': 1e-8, 'BAD': 0.75, 'GOOD': GOOD_PILOT_RATING, 'GREAT': GREAT_PILOT_RATING,
              'ULTIMATE': ULT_PILOT_RATING}
    return grades.get(get_ai_tag_grade(tag_list, "WEAPONS"), 1.0)


def rate_planetary_piloting(pid):
    universe = fo.getUniverse()
    planet = universe.getPlanet(pid)
    if not planet or not planet.speciesName:
        return 0.0
    this_spec = fo.getSpecies(planet.speciesName)
    if not this_spec:
        return 0.0
    return rate_piloting_tag(this_spec.tags)


@cache_by_turn
def get_supply_tech_range():
    return sum(_range for _tech, _range in AIDependencies.supply_range_techs.iteritems() if tech_is_complete(_tech))


def survey_universe():
    colonization_timer.start("Categorizing Visible Planets")
    universe = fo.getUniverse()
    empire_id = fo.empireID()
    current_turn = fo.currentTurn()

    # set up / reset various variables; the 'if' is purely for code folding convenience
    if True:
        colony_status['colonies_under_attack'] = []
        colony_status['colonies_under_threat'] = []
        AIstate.empireStars.clear()
        empire_colonizers.clear()
        empire_ship_builders.clear()
        empire_shipyards.clear()
        empire_metabolisms.clear()
        available_growth_specials.clear()
        active_growth_specials.clear()
        if tech_is_complete(AIDependencies.EXOBOT_TECH_NAME):
            empire_colonizers["SP_EXOBOT"] = []  # get it into colonizer list even if no colony yet
        for spec_name in AIDependencies.EXTINCT_SPECIES:
            if tech_is_complete("TECH_COL_" + spec_name):
                empire_colonizers["SP_" + spec_name] = []  # get it into colonizer list even if no colony yet
        pilot_ratings.clear()
        facilities_by_species_grade.clear()
        system_facilities.clear()

    # var setup done
    aistate = get_aistate()
    for sys_id in universe.systemIDs:
        system = universe.getSystem(sys_id)
        if not system:
            continue
        local_ast = False
        local_gg = False
        empire_has_qualifying_planet = False
        best_local_pilot_val = 0
        if sys_id in AIstate.colonyTargetedSystemIDs:
            empire_has_qualifying_planet = True
        for pid in system.planetIDs:
            planet = universe.getPlanet(pid)
            if not planet:
                continue
            if pid in aistate.colonisablePlanetIDs:
                empire_has_qualifying_planet = True
            if planet.size == fo.planetSize.asteroids:
                local_ast = True
            elif planet.size == fo.planetSize.gasGiant:
                local_gg = True
            spec_name = planet.speciesName
            this_spec = fo.getSpecies(spec_name)
            owner_id = planet.owner
            planet_population = planet.currentMeterValue(fo.meterType.population)
            buildings_here = [universe.getBuilding(bldg).buildingTypeName for bldg in planet.buildingIDs]
            ship_facilities = set(AIDependencies.SHIP_FACILITIES).intersection(buildings_here)
            weapons_grade = "WEAPONS_0.0"
            if owner_id == empire_id:
                if planet_population > 0.0 and this_spec:
                    empire_has_qualifying_planet = True
                    for metab in [tag for tag in this_spec.tags if tag in AIDependencies.metabolismBoostMap]:
                        empire_metabolisms[metab] = (empire_metabolisms.get(metab, 0.0) + planet.habitableSize)
                    if this_spec.canProduceShips:
                        pilot_val = rate_piloting_tag(list(this_spec.tags))
                        if spec_name == "SP_ACIREMA":
                            pilot_val += 1
                        weapons_grade = "WEAPONS_%.1f" % pilot_val
                        pilot_ratings[pid] = pilot_val
                        best_local_pilot_val = max(best_local_pilot_val, pilot_val)
                        yard_here = []
                        if "BLD_SHIPYARD_BASE" in buildings_here:
                            empire_ship_builders.setdefault(spec_name, []).append(pid)
                            empire_shipyards[pid] = pilot_val
                            yard_here = [pid]
                        if this_spec.canColonize and planet.currentMeterValue(fo.meterType.targetPopulation) >= 3:
                            empire_colonizers.setdefault(spec_name, []).extend(yard_here)

                this_grade_facilities = facilities_by_species_grade.setdefault(weapons_grade, {})
                for facility in ship_facilities:
                    this_grade_facilities.setdefault(facility, []).append(pid)
                    if facility in AIDependencies.SYSTEM_SHIP_FACILITIES:
                        this_facility_dict = system_facilities.setdefault(facility, {})
                        this_facility_dict.setdefault("systems", set()).add(sys_id)
                        this_facility_dict.setdefault("planets", set()).add(pid)

                for special in planet.specials:
                    if special in NEST_VAL_MAP:
                        state.set_have_nest()
                    if special in AIDependencies.metabolismBoosts:
                        available_growth_specials.setdefault(special, []).append(pid)
                        if planet.focus == FocusType.FOCUS_GROWTH:
                            active_growth_specials.setdefault(special, []).append(pid)
            elif owner_id != -1:
                if get_partial_visibility_turn(pid) >= current_turn - 1:  # only interested in immediately recent data
                    aistate.misc.setdefault('enemies_sighted', {}).setdefault(current_turn, []).append(pid)

        if empire_has_qualifying_planet:
            if local_ast:
                state.set_have_asteroids()
            elif local_gg:
                state.set_have_gas_giant()

        if sys_id in state.get_empire_planets_by_system():
            AIstate.empireStars.setdefault(system.starType, []).append(sys_id)
            sys_status = aistate.systemStatus.setdefault(sys_id, {})
            if sys_status.get('fleetThreat', 0) > 0:
                colony_status['colonies_under_attack'].append(sys_id)
            if sys_status.get('neighborThreat', 0) > 0:
                colony_status['colonies_under_threat'].append(sys_id)

    _print_empire_species_roster()

    if pilot_ratings:
        rating_list = sorted(pilot_ratings.values(), reverse=True)
        state.set_best_pilot_rating(rating_list[0])
        if len(pilot_ratings) == 1:
            state.set_medium_pilot_rating(rating_list[0])
        else:
            state.set_medium_pilot_rating(rating_list[1 + int(len(rating_list) / 5)])
    # the idea behind this was to note systems that the empire has claimed-- either has a current colony or has targeted
    # for making/invading a colony
    # claimedStars = {}
    # for sType in AIstate.empireStars:
    # claimedStars[sType] = list( AIstate.empireStars[sType] )
    # for sysID in set( AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs):
    # tSys = universe.getSystem(sysID)
    # if not tSys: continue
    # claimedStars.setdefault( tSys.starType, []).append(sysID)
    # get_aistate().misc['claimedStars'] = claimedStars
    colonization_timer.stop()


def get_colony_fleets():
    """examines known planets, collects various colonization data, to be later used to send colony fleets"""
    universe = fo.getUniverse()
    empire = fo.getEmpire()

    colonization_timer.start('Identify Existing colony/outpost targets')
    colony_targeted_planet_ids = FleetUtilsAI.get_targeted_planet_ids(universe.planetIDs, MissionType.COLONISATION)
    all_colony_targeted_system_ids = PlanetUtilsAI.get_systems(colony_targeted_planet_ids)
    colony_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.COLONISATION)
    num_colony_fleets = len(FleetUtilsAI.extract_fleet_ids_without_mission_types(colony_fleet_ids))

    outpost_targeted_planet_ids = FleetUtilsAI.get_targeted_planet_ids(universe.planetIDs, MissionType.OUTPOST)
    outpost_targeted_planet_ids.extend(FleetUtilsAI.get_targeted_planet_ids(universe.planetIDs,
                                                                            MissionType.ORBITAL_OUTPOST))
    all_outpost_targeted_system_ids = PlanetUtilsAI.get_systems(outpost_targeted_planet_ids)
    outpost_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.OUTPOST)
    num_outpost_fleets = len(FleetUtilsAI.extract_fleet_ids_without_mission_types(outpost_fleet_ids))

    debug("Colony Targeted SystemIDs: %s" % all_colony_targeted_system_ids)
    debug("Colony Targeted PlanetIDs: %s" % colony_targeted_planet_ids)
    debug(colony_fleet_ids and "Colony Fleet IDs: %s" % colony_fleet_ids or "Available Colony Fleets: 0")
    debug("Colony Fleets Without Missions: %s" % num_colony_fleets)
    debug('')
    debug("Outpost Targeted SystemIDs: %s" % all_outpost_targeted_system_ids)
    debug("Outpost Targeted PlanetIDs: %s" % outpost_targeted_planet_ids)
    debug(outpost_fleet_ids and "Outpost Fleet IDs: %s" % outpost_fleet_ids or "Available Outpost Fleets: 0")
    debug("Outpost Fleets Without Missions: %s" % num_outpost_fleets)
    debug('')

    # export targeted systems for other AI modules
    AIstate.colonyTargetedSystemIDs = all_colony_targeted_system_ids
    AIstate.outpostTargetedSystemIDs = all_outpost_targeted_system_ids

    colonization_timer.start('Identify colony base targets')
    # keys are sets of ints; data is doubles
    available_pp = {tuple(el.key()): el.data() for el in empire.planetsWithAvailablePP}

    avail_pp_by_sys = {}
    for p_set in available_pp:
        avail_pp_by_sys.update([(sys_id, available_pp[p_set]) for sys_id in set(PlanetUtilsAI.get_systems(p_set))])

    evaluated_colony_planet_ids = list(state.get_unowned_empty_planets().union(state.get_empire_outposts()) - set(
        colony_targeted_planet_ids))  # places for possible colonyBase

    aistate = get_aistate()
    outpost_base_manager = aistate.orbital_colonization_manager

    for pid in evaluated_colony_planet_ids:  # TODO: reorganize
        planet = universe.getPlanet(pid)
        if not planet:
            continue
        sys_id = planet.systemID
        for pid2 in state.get_empire_planets_by_system(sys_id, include_outposts=False):
            planet2 = universe.getPlanet(pid2)
            if not (planet2 and planet2.speciesName in empire_colonizers):
                continue
            if planet.unowned:
                outpost_base_manager.create_new_plan(pid, pid2)

    colonization_timer.start('Initiate outpost base construction')

    reserved_outpost_base_targets = outpost_base_manager.get_targets()
    debug("Current qualifyingOutpostBaseTargets: %s" % reserved_outpost_base_targets)
    outpost_base_manager.build_bases()
    colonization_timer.start('Evaluate Primary Colony Opportunities')

    evaluated_outpost_planet_ids = list(state.get_unowned_empty_planets() - set(outpost_targeted_planet_ids) - set(
            colony_targeted_planet_ids) - set(reserved_outpost_base_targets))

    evaluated_colony_planets = assign_colonisation_values(evaluated_colony_planet_ids, MissionType.COLONISATION, None)
    colonization_timer.stop('Evaluate %d Primary Colony Opportunities' % (len(evaluated_colony_planet_ids)))
    colonization_timer.start('Evaluate All Colony Opportunities')
    all_colony_opportunities.clear()
    all_colony_opportunities.update(
        assign_colonisation_values(evaluated_colony_planet_ids, MissionType.COLONISATION, None, [], True))
    colonization_timer.start('Evaluate Outpost Opportunities')

    sorted_planets = evaluated_colony_planets.items()
    sorted_planets.sort(lambda x, y: cmp(x[1], y[1]), reverse=True)

    _print_colony_candidate_table(sorted_planets, show_detail=False)

    sorted_planets = [(planet_id, score[:2]) for planet_id, score in sorted_planets if score[0] > 0]
    # export planets for other AI modules
    aistate.colonisablePlanetIDs.clear()
    aistate.colonisablePlanetIDs.update(sorted_planets)

    evaluated_outpost_planets = assign_colonisation_values(evaluated_outpost_planet_ids, MissionType.OUTPOST, None)
    # if outposted planet would be in supply range, let outpost value be best of outpost value or colonization value
    for pid in set(evaluated_outpost_planets).intersection(evaluated_colony_planets):
        if planet_supply_cache.get(pid, -99) >= 0:
            evaluated_outpost_planets[pid] = (
                max(evaluated_colony_planets[pid][0], evaluated_outpost_planets[pid][0]), '')

    colonization_timer.stop()

    sorted_outposts = evaluated_outpost_planets.items()
    sorted_outposts.sort(lambda x, y: cmp(x[1], y[1]), reverse=True)

    _print_outpost_candidate_table(sorted_outposts)

    sorted_outposts = [(planet_id, score[:2]) for planet_id, score in sorted_outposts if score[0] > 0]
    # export outposts for other AI modules
    aistate.colonisableOutpostIDs.clear()
    aistate.colonisableOutpostIDs.update(sorted_outposts)
    colonization_timer.stop_print_and_clear()


# TODO: clean up suppliable versus annexable
def assign_colonisation_values(planet_ids, mission_type, species, detail=None, return_all=False):
    """Creates a dictionary that takes planetIDs as key and their colonisation score as value."""
    if detail is None:
        detail = []
    orig_detail = detail
    planet_values = {}
    if mission_type == MissionType.OUTPOST:
        # print "\n=========\nAssigning Outpost Values\n========="
        try_species = [""]
    elif species is not None:
        # print "\n=========\nAssigning Colony Values\n========="
        if isinstance(species, str):
            try_species = [species]
        elif isinstance(species, list):
            try_species = species
        else:
            try_species = [species.name]
    else:
        # print "\n=========\nAssigning Colony Values\n========="
        try_species = list(empire_colonizers)
    for planet_id in planet_ids:
        pv = []
        for spec_name in try_species:
            detail = orig_detail[:]
            # appends (score, species_name, detail)
            pv.append((evaluate_planet(planet_id, mission_type, spec_name, detail), spec_name, detail))
        all_sorted = sorted(pv, reverse=True)
        best = all_sorted[:1]
        if best:
            if return_all:
                planet_values[planet_id] = all_sorted
            else:
                planet_values[planet_id] = best[0]
                # print best[0][2]
    return planet_values


def next_turn_pop_change(cur_pop, target_pop):
    """Population change calc taken from PopCenter.cpp."""
    if target_pop > cur_pop:
        pop_change = cur_pop * (target_pop + 1 - cur_pop) / 100
        return min(pop_change, target_pop - cur_pop)
    else:
        pop_change = -(cur_pop - target_pop) / 10
        return max(pop_change, target_pop - cur_pop)


def project_ind_val(init_pop, max_pop_size, init_industry, max_ind_factor, flat_industry, discount_multiplier):
    """returns a discouted value for a projected industry stream over time with changing population"""
    discount_factor = 0.95
    if discount_multiplier > 1.0:
        discount_factor = 1.0 - 1.0 / discount_multiplier
    cur_pop = float(init_pop)
    cur_ind = float(init_industry)
    ind_val = 0
    val_factor = 1.0
    for turn in range(50):
        cur_pop += next_turn_pop_change(cur_pop, max_pop_size)
        cur_ind = min(cur_ind + 1, max(0, cur_ind - 1, flat_industry + cur_pop * max_ind_factor))
        val_factor *= discount_factor
        ind_val += val_factor * cur_ind
    return ind_val


@cache_by_turn
def get_base_outpost_defense_value():
    """
    :return:planet defenses contribution towards planet evaluations
    :rtype float
    """
    # TODO: assess current AI defense technology, compare the resulting planetary rating to the current
    # best military ship design, derive a planet defenses value related to the cost of such a ship

    net_count = sum(tech_is_complete(tech_name) for tech_name in AIDependencies.DEFENSE_DEFENSE_NET_TECHS)
    regen_count = sum(tech_is_complete(tech_name) for tech_name in AIDependencies.DEFENSE_REGEN_TECHS)
    garrison_count = sum(tech_is_complete(tech_name) for tech_name in AIDependencies.DEFENSE_GARRISON_TECHS)
    shield_count = sum(tech_is_complete(tech_name) for tech_name in AIDependencies.DEFENSE_SHIELDS_TECHS)
    # not counting mine techs because their effects are per-system, not per-planet

    # for now, just combing these for rough composite factor
    # since outposts have no infrastructure (until late game at least), many of these have less weight
    # than for colonies
    result = 3 * (0.1 + net_count) * (1 + regen_count/3.0) * (1 + garrison_count/6.0) * (1 + shield_count/3.0)

    return result


@cache_by_turn
def get_base_colony_defense_value():
    """
    :return:planet defenses contribution towards planet evaluations
    :rtype float
    """
    # TODO: assess current AI defense technology, compare the resulting planetary rating to the current
    # best military ship design, derive a planet defenses value related to the cost of such a ship

    net_count = sum(tech_is_complete(tech_name) for tech_name in AIDependencies.DEFENSE_DEFENSE_NET_TECHS)
    regen_count = sum(tech_is_complete(tech_name) for tech_name in AIDependencies.DEFENSE_REGEN_TECHS)
    garrison_count = sum(tech_is_complete(tech_name) for tech_name in AIDependencies.DEFENSE_GARRISON_TECHS)
    shield_count = sum(tech_is_complete(tech_name) for tech_name in AIDependencies.DEFENSE_SHIELDS_TECHS)
    # not counting mine techs because their effects are per-system, not per-planet

    # for now, just combing these for rough composite factor
    result = 4 * (0.1 + net_count) * (1 + regen_count/2.0) * (1 + garrison_count/4.0) * (1 + shield_count/2.0)
    return result


def get_defense_value(species_name):
    """
    :param species_name:
    :type species_name: str
    :return: planet defenses contribution towards planet evaluations
    :rtype: float
    """
    # TODO: assess species defense characteristics
    if species_name:
        return get_base_colony_defense_value()
    else:
        return get_base_outpost_defense_value()


def _base_asteroid_mining_val():
    return 5 if tech_is_complete("PRO_MICROGRAV_MAN") else 3


def evaluate_planet(planet_id, mission_type, spec_name, detail=None):
    """returns the colonisation value of a planet"""
    empire = fo.getEmpire()
    if detail is None:
        detail = []
    retval = 0
    character = get_aistate().character
    discount_multiplier = character.preferred_discount_multiplier([30.0, 40.0])
    species = fo.getSpecies(spec_name or "")  # in case None is passed as specName
    species_foci = [] and species and list(species.foci)
    tag_list = list(species.tags) if species else []
    pilot_val = pilot_rating = 0
    if species and species.canProduceShips:
        pilot_val = pilot_rating = rate_piloting_tag(species.tags)
        if pilot_val > state.best_pilot_rating:
            pilot_val *= 2
        if pilot_val > 2:
            retval += discount_multiplier * 5 * pilot_val
            detail.append("Pilot Val %.1f" % (discount_multiplier * 5 * pilot_val))

    priority_scaling = 1.0
    max_gggs = 1  # if goes above 1 need some other adjustments below
    if empire.productionPoints < 100:
        backup_factor = 0.0
    else:
        backup_factor = min(1.0, (empire.productionPoints / 200.0) ** 2)

    universe = fo.getUniverse()
    capital_id = PlanetUtilsAI.get_capital()
    homeworld = universe.getPlanet(capital_id)
    planet = universe.getPlanet(planet_id)
    prospective_invasion_targets = [pid for pid, pscore, trp in
                                    AIstate.invasionTargets[:PriorityAI.allotted_invasion_targets()]
                                    if pscore > InvasionAI.MIN_INVASION_SCORE]

    if spec_name != planet.speciesName and planet.speciesName and mission_type != MissionType.INVASION:
        return 0

    this_sysid = planet.systemID
    if homeworld:
        home_system_id = homeworld.systemID
        eval_system_id = this_sysid
        if home_system_id != INVALID_ID and eval_system_id != INVALID_ID:
            least_jumps = universe.jumpDistance(home_system_id, eval_system_id)
            if least_jumps == -1:  # indicates no known path
                return 0.0
                # distanceFactor = 1.001 / (least_jumps + 1)

    claimed_stars = get_claimed_stars()

    empire_research_list = [element.tech for element in empire.researchQueue]
    if planet is None:
        vis_map = universe.getVisibilityTurnsMap(planet_id, empire.empireID)
        debug("Planet %d object not available; visMap: %s" % (planet_id, vis_map))
        return 0
    # only count existing presence if not target planet
    # TODO: consider neighboring sytems for smaller contribution, and bigger contributions for
    # local colonies versus local outposts
    locally_owned_planets = [lpid for lpid in state.get_empire_planets_by_system(this_sysid) if lpid != planet_id]
    planets_with_species = state.get_inhabited_planets()
    locally_owned_pop_ctrs = [lpid for lpid in locally_owned_planets if lpid in planets_with_species]
    # triple count pop_ctrs
    existing_presence = len(locally_owned_planets) + 2 * len(locally_owned_pop_ctrs)
    system = universe.getSystem(this_sysid)

    sys_supply = state.get_system_supply(this_sysid)
    planet_supply = AIDependencies.supply_by_size.get(planet.size, 0)
    bld_types = set(universe.getBuilding(bldg).buildingTypeName for bldg in planet.buildingIDs).intersection(
        AIDependencies.building_supply)
    planet_supply += sum(AIDependencies.building_supply[bld_type].get(int(psize), 0)
                         for psize in [-1, planet.size] for bld_type in bld_types)
    planet_supply += sum(AIDependencies.SUPPLY_MOD_SPECIALS[_special].get(int(psize), 0)
                         for psize in [-1, planet.size] for _special in
                         set(planet.specials).union(system.specials).intersection(AIDependencies.SUPPLY_MOD_SPECIALS))

    ind_tag_mod = AIDependencies.SPECIES_INDUSTRY_MODIFIER.get(get_ai_tag_grade(tag_list, "INDUSTRY"), 1.0)
    res_tag_mod = AIDependencies.SPECIES_RESEARCH_MODIFIER.get(get_ai_tag_grade(tag_list, "RESEARCH"), 1.0)
    if species:
        supply_tag_mod = AIDependencies.SPECIES_SUPPLY_MODIFIER.get(get_ai_tag_grade(tag_list, "SUPPLY"), 1)
    else:
        supply_tag_mod = 0

    # determine the potential supply provided by owning this planet, and if the planet is currently populated by
    # the evaluated species, then save this supply value in a cache.
    # The system supply value can be negative (indicates the respective number of starlane jumps to the closest supplied
    # system).  So if this total planet supply value is non-negative, then the planet would be supply connected (to at
    # least a portion of the existing empire) if the planet becomes owned by the AI.

    planet_supply += supply_tag_mod
    planet_supply = max(planet_supply, 0)  # planets can't have negative supply
    if planet.speciesName == (spec_name or ""):  # adding or "" in case None was passed as spec_name
        planet_supply_cache[planet_id] = planet_supply + sys_supply

    threat_factor = determine_colony_threat_factor(planet_id, spec_name, existing_presence)

    sys_partial_vis_turn = get_partial_visibility_turn(this_sysid)
    planet_partial_vis_turn = get_partial_visibility_turn(planet_id)

    if planet_partial_vis_turn < sys_partial_vis_turn:
        # last time we had partial vis of the system, the planet was stealthed to us
        # print "Colonization AI couldn't get current info on planet id %d (was stealthed at last sighting)" % planet_id
        return 0  # TODO: track detection strength, order new scouting when it goes up

    star_bonus = 0
    colony_star_bonus = 0
    research_bonus = 0
    growth_val = 0
    fixed_ind = 0
    fixed_res = 0
    if system:
        already_got_this_one = this_sysid in state.get_empire_planets_by_system()
        # TODO: Should probably consider pilot rating also for Phototropic species
        if "PHOTOTROPHIC" not in tag_list and pilot_rating >= state.best_pilot_rating:
            if system.starType == fo.starType.red and tech_is_complete("LRN_STELLAR_TOMOGRAPHY"):
                star_bonus += 40 * discount_multiplier  # can be used for artif'l black hole and solar hull
                detail.append("Red Star for Art Black Hole for solar hull %.1f" % (40 * discount_multiplier))
            elif system.starType == fo.starType.blackHole and tech_is_complete("SHP_FRC_ENRG_COMP"):
                star_bonus += 100 * discount_multiplier  # can be used for solar hull
                detail.append("Black Hole for solar hull %.1f" % (100 * discount_multiplier))

        if tech_is_complete("PRO_SOL_ORB_GEN") or "PRO_SOL_ORB_GEN" in empire_research_list[:5]:
            if system.starType in [fo.starType.blue, fo.starType.white]:
                if not claimed_stars.get(fo.starType.blue, []) + claimed_stars.get(fo.starType.white, []):
                    star_bonus += 20 * discount_multiplier
                    detail.append("PRO_SOL_ORB_GEN BW %.1f" % (20 * discount_multiplier))
                elif not already_got_this_one:
                    # still has extra value as an alternate location for solar generators
                    star_bonus += 10 * discount_multiplier * backup_factor
                    detail.append(
                        "PRO_SOL_ORB_GEN BW Backup Location %.1f" % (10 * discount_multiplier * backup_factor))
                elif fo.currentTurn() > 100:  # lock up this whole system
                    pass
                    # starBonus += 5  # TODO: how much?
                    # detail.append("PRO_SOL_ORB_GEN BW LockingDownSystem %.1f"%5)
            if system.starType in [fo.starType.yellow, fo.starType.orange]:
                if not (claimed_stars.get(fo.starType.blue, []) + claimed_stars.get(fo.starType.white, []) +
                        claimed_stars.get(fo.starType.yellow, []) + claimed_stars.get(fo.starType.orange, [])):
                    star_bonus += 10 * discount_multiplier
                    detail.append("PRO_SOL_ORB_GEN YO %.1f" % (10 * discount_multiplier))
                else:
                    pass
                    # starBonus +=2  # still has extra value as an alternate location for solar generators
                    # detail.append("PRO_SOL_ORB_GEN YO Backup %.1f" % 2)
        if system.starType in [fo.starType.blackHole] and fo.currentTurn() > 100:
            if not already_got_this_one:
                # whether have tech yet or not, assign some base value
                star_bonus += 10 * discount_multiplier * backup_factor
                detail.append("Black Hole %.1f" % (10 * discount_multiplier * backup_factor))
            else:
                star_bonus += 5 * discount_multiplier * backup_factor
                detail.append("Black Hole Backup %.1f" % (5 * discount_multiplier * backup_factor))
        if tech_is_complete(AIDependencies.PRO_SOL_ORB_GEN):  # start valuing as soon as PRO_SOL_ORB_GEN done
            if system.starType in [fo.starType.blackHole]:
                # pretty rare planets, good for generator
                this_val = 0.5 * max(state.population_with_industry_focus(), 20) * discount_multiplier
                if not claimed_stars.get(fo.starType.blackHole, []):
                    star_bonus += this_val
                    detail.append("PRO_SINGULAR_GEN %.1f" % this_val)
                elif this_sysid not in claimed_stars.get(fo.starType.blackHole, []):
                    # still has extra value as an alternate location for generators & for blocking enemies generators
                    star_bonus += this_val * backup_factor
                    detail.append("PRO_SINGULAR_GEN Backup %.1f" % (this_val * backup_factor))
            elif system.starType in [fo.starType.red] and not claimed_stars.get(fo.starType.blackHole, []):
                rfactor = (1.0 + len(claimed_stars.get(fo.starType.red, []))) ** -2
                star_bonus += 40 * discount_multiplier * backup_factor * rfactor  # can be used for artif'l black hole
                detail.append("Red Star for Art Black Hole %.1f" % (40 * discount_multiplier * backup_factor * rfactor))
        if tech_is_complete("PRO_NEUTRONIUM_EXTRACTION") or "PRO_NEUTRONIUM_EXTRACTION" in empire_research_list[:8]:
            if system.starType in [fo.starType.neutron]:
                if not claimed_stars.get(fo.starType.neutron, []):
                    star_bonus += 80 * discount_multiplier  # pretty rare planets, good for armor
                    detail.append("PRO_NEUTRONIUM_EXTRACTION %.1f" % (80 * discount_multiplier))
                else:
                    # still has extra value as an alternate location for generators & for bnlocking enemies generators
                    star_bonus += 20 * discount_multiplier * backup_factor
                    detail.append("PRO_NEUTRONIUM_EXTRACTION Backup %.1f" % (20 * discount_multiplier * backup_factor))
        if tech_is_complete("SHP_ENRG_BOUND_MAN") or "SHP_ENRG_BOUND_MAN" in empire_research_list[:6]:
            # TODO: base this on pilot val, and also consider red stars
            if system.starType in [fo.starType.blackHole, fo.starType.blue]:
                init_val = 100 * discount_multiplier * (pilot_val or 1)
                if not claimed_stars.get(fo.starType.blackHole, []) + claimed_stars.get(fo.starType.blue, []):
                    colony_star_bonus += init_val  # pretty rare planets, good for energy shipyards
                    detail.append("SHP_ENRG_BOUND_MAN %.1f" % init_val)
                elif this_sysid not in (claimed_stars.get(fo.starType.blackHole, []) +
                                        claimed_stars.get(fo.starType.blue, [])):
                    # still has extra value as an alternate location for energy shipyard
                    colony_star_bonus += 0.5 * init_val * backup_factor
                    detail.append("SHP_ENRG_BOUND_MAN Backup %.1f" % (0.5 * init_val * backup_factor))
    retval += star_bonus

    planet_specials = list(planet.specials)
    if "ECCENTRIC_ORBIT_SPECIAL" in planet.specials:
        fixed_res += discount_multiplier * 6
        detail.append("ECCENTRIC_ORBIT_SPECIAL %.1f" % (discount_multiplier * 6))

    if (mission_type == MissionType.OUTPOST or
            (mission_type == MissionType.INVASION and not spec_name)):

        if "ANCIENT_RUINS_SPECIAL" in planet.specials:  # TODO: add value for depleted ancient ruins
            retval += discount_multiplier * 30
            detail.append("Undepleted Ruins %.1f" % discount_multiplier * 30)

        for special in planet_specials:
            if "_NEST_" in special:
                nest_val = NEST_VAL_MAP.get(special,
                                            5) * discount_multiplier  # get an outpost on the nest quick
                retval += nest_val
                detail.append("%s %.1f" % (special, nest_val))
            elif special == "FORTRESS_SPECIAL":
                fort_val = 10 * discount_multiplier
                retval += fort_val
                detail.append("%s %.1f" % (special, fort_val))
            elif special == "HONEYCOMB_SPECIAL":
                honey_val = 0.3 * (AIDependencies.HONEYCOMB_IND_MULTIPLIER * AIDependencies.INDUSTRY_PER_POP *
                                   state.population_with_industry_focus() * discount_multiplier)
                retval += honey_val
                detail.append("%s %.1f" % (special, honey_val))
        if planet.size == fo.planetSize.asteroids:
            ast_val = 0
            if system:
                for pid in system.planetIDs:
                    other_planet = universe.getPlanet(pid)
                    if other_planet.size == fo.planetSize.asteroids:
                        if pid == planet_id:
                            continue
                        elif pid < planet_id and planet.unowned:
                            ast_val = 0
                            break
                    elif other_planet.speciesName:
                        if other_planet.owner == empire.empireID:
                            ownership_factor = 1.0
                        elif pid in prospective_invasion_targets:
                            ownership_factor = character.secondary_valuation_factor_for_invasion_targets()
                        else:
                            ownership_factor = 0.0
                        ast_val += ownership_factor * _base_asteroid_mining_val() * discount_multiplier
                retval += ast_val
                if ast_val > 0:
                    detail.append("AsteroidMining %.1f" % ast_val)
            ast_val = 0
            if tech_is_complete("SHP_ASTEROID_HULLS"):
                per_ast = 20
            elif tech_is_complete("CON_ORBITAL_CON"):
                per_ast = 5
            else:
                per_ast = 0.1
            if system:
                for pid in system.planetIDs:
                    other_planet = universe.getPlanet(pid)
                    if other_planet.size == fo.planetSize.asteroids:
                        if pid == planet_id:
                            continue
                        elif pid < planet_id and planet.unowned:
                            ast_val = 0
                            break
                    elif other_planet.speciesName:
                        other_species = fo.getSpecies(other_planet.speciesName)
                        if other_species and other_species.canProduceShips:
                            if other_planet.owner == empire.empireID:
                                ownership_factor = 1.0
                            elif pid in prospective_invasion_targets:
                                ownership_factor = character.secondary_valuation_factor_for_invasion_targets()
                            else:
                                ownership_factor = 0.0
                            ast_val += ownership_factor * per_ast * discount_multiplier
                retval += ast_val
                if ast_val > 0:
                    detail.append("AsteroidShipBuilding %.1f" % ast_val)
        # We will assume that if any GG in the system is populated, they all will wind up populated; cannot then hope
        # to build a GGG on a non-populated GG
        populated_gg_factor = 1.0
        per_gg = 0
        if planet.size == fo.planetSize.gasGiant:
            # TODO: Given current industry calc approach, consider bringing this max val down to actual max val of 10
            if tech_is_complete("PRO_ORBITAL_GEN"):
                per_gg = 20
            elif tech_is_complete("CON_ORBITAL_CON"):
                per_gg = 10
            if spec_name:
                populated_gg_factor = 0.5
        else:
            per_gg = 5
        if system:
            gg_list = []
            orb_gen_val = 0
            gg_detail = []
            for pid in system.planetIDs:
                other_planet = universe.getPlanet(pid)
                if other_planet.size == fo.planetSize.gasGiant:
                    gg_list.append(pid)
                    if other_planet.speciesName:
                        populated_gg_factor = 0.5
                if pid != planet_id and other_planet.owner == empire.empireID and FocusType.FOCUS_INDUSTRY in list(
                        other_planet.availableFoci) + [other_planet.focus]:
                    orb_gen_val += per_gg * discount_multiplier
                    # Note, this reported value may not take into account a later adjustment from a populated gg
                    gg_detail.append("GGG for %s %.1f" % (other_planet.name, discount_multiplier * per_gg *
                                                          populated_gg_factor))
            if planet_id in sorted(gg_list)[:max_gggs]:
                retval += orb_gen_val * populated_gg_factor
                detail.extend(gg_detail)
            else:
                detail.append("Won't GGG")
        if existing_presence:
            detail.append("preexisting system colony")
            retval = (retval + existing_presence * get_defense_value(spec_name)) * 1.5

        # Fixme - sys_supply is always <= 0 leading to incorrect supply bonus score
        supply_val = 0
        if sys_supply < 0:
            if sys_supply + planet_supply >= 0:
                supply_val += 30 * (planet_supply - max(-3, sys_supply))
            else:
                retval += 30 * (planet_supply + sys_supply)  # a penalty
        elif planet_supply > sys_supply and (sys_supply < 2):  # TODO: check min neighbor supply
            supply_val += 25 * (planet_supply - sys_supply)
        detail.append("sys_supply: %d, planet_supply: %d, supply_val: %.0f" % (sys_supply, planet_supply, supply_val))
        retval += supply_val

        if threat_factor < 1.0:
            threat_factor = revise_threat_factor(threat_factor, retval, this_sysid, MINIMUM_COLONY_SCORE)
            retval *= threat_factor
            detail.append("threat reducing value by %3d %%" % (100 * (1 - threat_factor)))
        return int(retval)
    else:  # colonization mission
        if not species:
            return 0
        supply_val = 0
        if "ANCIENT_RUINS_SPECIAL" in planet.specials:
            retval += discount_multiplier * 50
            detail.append("Undepleted Ruins %.1f" % discount_multiplier * 50)
        if "HONEYCOMB_SPECIAL" in planet.specials:
            honey_val = (AIDependencies.HONEYCOMB_IND_MULTIPLIER * AIDependencies.INDUSTRY_PER_POP *
                         state.population_with_industry_focus() * discount_multiplier)
            if FocusType.FOCUS_INDUSTRY not in species_foci:
                honey_val *= -0.3  # discourage settlement by colonizers not able to use Industry Focus
            retval += honey_val
            detail.append("%s %.1f" % ("HONEYCOMB_SPECIAL", honey_val))

        # Fixme - sys_supply is always <= 0 leading to incorrect supply bonus score
        if sys_supply <= 0:
            if sys_supply + planet_supply >= 0:
                supply_val = 40 * (planet_supply - max(-3, sys_supply))
            else:
                supply_val = 200 * (planet_supply + sys_supply)  # a penalty
                if spec_name == "SP_SLY":
                    # Sly are essentially stuck with lousy supply, so don't penalize for that
                    supply_val = 0
        elif planet_supply > sys_supply == 1:  # TODO: check min neighbor supply
            supply_val = 20 * (planet_supply - sys_supply)
        detail.append("sys_supply: %d, planet_supply: %d, supply_val: %.0f" % (sys_supply, planet_supply, supply_val))

        # if AITags != "":
        # print "Species %s has AITags %s"%(specName, AITags)

        retval += fixed_res
        retval += colony_star_bonus
        asteroid_bonus = 0
        gas_giant_bonus = 0
        flat_industry = 0
        mining_bonus = 0
        per_ggg = 10

        asteroid_factor = 0.0
        gg_factor = 0.0
        ast_shipyard_name = ""
        if system and FocusType.FOCUS_INDUSTRY in species.foci:
            for pid in [temp_id for temp_id in system.planetIDs if temp_id != planet_id]:
                p2 = universe.getPlanet(pid)
                if p2:
                    if p2.size == fo.planetSize.asteroids:
                        this_factor = 0.0
                        if p2.owner == empire.empireID:
                            this_factor = 1.0
                        elif p2.unowned:
                            this_factor = 0.5
                        elif pid in prospective_invasion_targets:
                            this_factor = character.secondary_valuation_factor_for_invasion_targets()
                        if this_factor > asteroid_factor:
                            asteroid_factor = this_factor
                            ast_shipyard_name = p2.name
                    if p2.size == fo.planetSize.gasGiant:
                        if p2.owner == empire.empireID:
                            gg_factor = max(gg_factor, 1.0)
                        elif p2.unowned:
                            gg_factor = max(gg_factor, 0.5)
                        elif pid in prospective_invasion_targets:
                            gg_factor = max(gg_factor, character.secondary_valuation_factor_for_invasion_targets())
        if asteroid_factor > 0.0:
            if tech_is_complete("PRO_MICROGRAV_MAN") or "PRO_MICROGRAV_MAN" in empire_research_list[:10]:
                flat_industry += 5 * asteroid_factor  # will go into detailed industry projection
                detail.append("Asteroid mining ~ %.1f" % (5 * asteroid_factor * discount_multiplier))
            if tech_is_complete("SHP_ASTEROID_HULLS") or "SHP_ASTEROID_HULLS" in empire_research_list[:11]:
                if species and species.canProduceShips:
                    asteroid_bonus = 30 * discount_multiplier * pilot_val
                    detail.append(
                        "Asteroid ShipBuilding from %s %.1f" % (
                            ast_shipyard_name, discount_multiplier * 30 * pilot_val))
        if gg_factor > 0.0:
            if tech_is_complete("PRO_ORBITAL_GEN") or "PRO_ORBITAL_GEN" in empire_research_list[:5]:
                flat_industry += per_ggg * gg_factor  # will go into detailed industry projection
                detail.append("GGG ~ %.1f" % (per_ggg * gg_factor * discount_multiplier))

        # calculate the maximum population of the species on that planet.
        if planet.speciesName not in AIDependencies.SPECIES_FIXED_POPULATION:
            max_pop_size = calc_max_pop(planet, species, detail)
        else:
            max_pop_size = AIDependencies.SPECIES_FIXED_POPULATION[planet.speciesName]
            detail.append("Fixed max population of %.2f" % max_pop_size)

        if max_pop_size <= 0:
            detail.append("Non-positive population projection for species '%s', so no colonization value" % (
                species and species.name))
            return 0

        for special in ["MINERALS_SPECIAL", "CRYSTALS_SPECIAL", "ELERIUM_SPECIAL"]:
            if special in planet_specials:
                mining_bonus += 1

        has_blackhole = len(claimed_stars.get(fo.starType.blackHole, [])) > 0
        ind_tech_map_before_species_mod = AIDependencies.INDUSTRY_EFFECTS_PER_POP_MODIFIED_BY_SPECIES
        ind_tech_map_after_species_mod = AIDependencies.INDUSTRY_EFFECTS_PER_POP_NOT_MODIFIED_BY_SPECIES

        ind_mult = 1
        for tech in ind_tech_map_before_species_mod:
            if tech_is_complete(tech) and (tech != AIDependencies.PRO_SINGULAR_GEN or has_blackhole):
                ind_mult += ind_tech_map_before_species_mod[tech]

        ind_mult = ind_mult * max(ind_tag_mod,
                                  0.5 * (ind_tag_mod + res_tag_mod))  # TODO: report an actual calc for research value

        for tech in ind_tech_map_after_species_mod:
            if tech_is_complete(tech) and (tech != AIDependencies.PRO_SINGULAR_GEN or has_blackhole):
                ind_mult += ind_tech_map_after_species_mod[tech]

        max_ind_factor = 0
        if tech_is_complete("PRO_SENTIENT_AUTOMATION"):
            fixed_ind += discount_multiplier * 5
        if FocusType.FOCUS_INDUSTRY in species.foci:
            if 'TIDAL_LOCK_SPECIAL' in planet.specials:
                ind_mult += 1
            max_ind_factor += AIDependencies.INDUSTRY_PER_POP * mining_bonus
            max_ind_factor += AIDependencies.INDUSTRY_PER_POP * ind_mult
        cur_pop = 1.0  # assume an initial colonization value
        if planet.speciesName != "":
            cur_pop = planet.currentMeterValue(fo.meterType.population)
        elif tech_is_complete("GRO_LIFECYCLE_MAN"):
            cur_pop = 3.0
        cur_industry = planet.currentMeterValue(fo.meterType.industry)
        ind_val = project_ind_val(cur_pop, max_pop_size, cur_industry, max_ind_factor, flat_industry,
                                  discount_multiplier)
        detail.append("ind_val %.1f" % ind_val)
        # used to give preference to closest worlds

        for special in [spec for spec in planet_specials if spec in AIDependencies.metabolismBoosts]:
            # TODO: also consider potential future benefit re currently unpopulated planets
            gbonus = discount_multiplier * AIDependencies.INDUSTRY_PER_POP * ind_mult * empire_metabolisms.get(
                AIDependencies.metabolismBoosts[special], 0)  # due to growth applicability to other planets
            growth_val += gbonus
            detail.append("Bonus for %s: %.1f" % (special, gbonus))

        if FocusType.FOCUS_RESEARCH in species.foci:
            research_bonus += discount_multiplier * 2 * AIDependencies.RESEARCH_PER_POP * max_pop_size
            if "ANCIENT_RUINS_SPECIAL" in planet.specials or "ANCIENT_RUINS_DEPLETED_SPECIAL" in planet.specials:
                research_bonus += discount_multiplier * 2 * AIDependencies.RESEARCH_PER_POP * max_pop_size * 5
                detail.append("Ruins Research")
            if "TEMPORAL_ANOMALY_SPECIAL" in planet.specials:
                research_bonus += discount_multiplier * 2 * AIDependencies.RESEARCH_PER_POP * max_pop_size * 25
                detail.append("Temporal Anomaly Research")
            if AIDependencies.COMPUTRONIUM_SPECIAL in planet.specials:
                comp_bonus = (0.5 * AIDependencies.TECH_COST_MULTIPLIER * AIDependencies.RESEARCH_PER_POP *
                              AIDependencies.COMPUTRONIUM_RES_MULTIPLIER * state.population_with_research_focus() *
                              discount_multiplier)
                if state.have_computronium:
                    comp_bonus *= backup_factor
                research_bonus += comp_bonus
                detail.append(AIDependencies.COMPUTRONIUM_SPECIAL)

        retval += max(ind_val + asteroid_bonus + gas_giant_bonus, research_bonus,
                      growth_val) + fixed_ind + fixed_res + supply_val
        retval *= priority_scaling
        if existing_presence:
            detail.append("preexisting system colony")
            retval = (retval + existing_presence * get_defense_value(spec_name)) * 2
        if threat_factor < 1.0:
            threat_factor = revise_threat_factor(threat_factor, retval, this_sysid, MINIMUM_COLONY_SCORE)
            retval *= threat_factor
            detail.append("threat reducing value by %3d %%" % (100 * (1 - threat_factor)))
    return retval


def revise_threat_factor(threat_factor, planet_value, system_id, min_planet_value=MINIMUM_COLONY_SCORE):
    """
    Check if the threat_factor should be made less severe.

    If the AI does have enough total miltary to secure this system, and the target has more than minimal value,
    don't let the threat_factor discount the adjusted value below min_planet_value +1, so that if there are no
    other targets the AI could still pursue this one.  Otherwise, scoring pressure from
    MilitaryAI.get_preferred_max_military_portion_for_single_battle might prevent the AI from pursuing a heavily
    defended but still obtainable target even if it has no softer locations available.

    :param threat_factor: the base threat factor
    :type threat_factor: float
    :param planet_value: the planet score
    :type planet_value: float
    :param system_id: the system ID of subject planet
    :type system_id: int
    :param min_planet_value: a floor planet value if the AI has enough military to secure the system
    :type min_planet_value: float
    :return: the (potentially) revised threat_factor
    :rtype: float
    """

    # the default value below for fleetThreat shouldn't come in to play, but is just to be absolutely sure we don't
    # send colony ships into some system for which we have not evaluated fleetThreat
    system_status = get_aistate().systemStatus.get(system_id, {})
    system_fleet_treat = system_status.get('fleetThreat', 1000)
    # TODO: consider taking area threat into account here.  Arguments can be made both ways, see discussion in PR2069
    sys_total_threat = system_fleet_treat + system_status.get('monsterThreat', 0) + system_status.get('planetThreat', 0)
    if (MilitaryAI.get_concentrated_tot_mil_rating() > sys_total_threat) and (planet_value > 2 * min_planet_value):
        threat_factor = max(threat_factor, (min_planet_value + 1) / planet_value)
    return threat_factor


def determine_colony_threat_factor(planet_id, spec_name, existing_presence):
    universe = fo.getUniverse()
    planet = universe.getPlanet(planet_id)
    if not planet:  # should have been checked previously, but belt-and-suspenders
        error("Can't retrieve planet ID %d" % planet_id)
        return 0
    sys_status = get_aistate().systemStatus.get(planet.systemID, {})
    cur_best_mil_ship_rating = max(MilitaryAI.cur_best_mil_ship_rating(), 0.001)
    local_defenses = sys_status.get('all_local_defenses', 0)
    local_threat = sys_status.get('fleetThreat', 0) + sys_status.get('monsterThreat', 0)
    neighbor_threat = sys_status.get('neighborThreat', 0)
    jump2_threat = 0.6 * max(0, sys_status.get('jump2_threat', 0) - sys_status.get('my_neighbor_rating', 0))
    area_threat = neighbor_threat + jump2_threat
    area_threat *= 2.0 / (existing_presence + 2)  # once we have a foothold be less scared off by area threats
    # TODO: evaluate detectability by specific source of area threat, also consider if the subject AI already has
    # researched techs that would grant a stealth bonus
    local_enemies = sys_status.get("enemies_nearly_supplied", [])
    # could more conservatively base detection on get_aistate().misc.get("observed_empires")
    if not EspionageAI.colony_detectable_by_empire(planet_id, spec_name, empire=local_enemies, default_result=True):
        area_threat *= 0.05
    net_threat = max(0, local_threat + area_threat - local_defenses)
    # even if our military has lost all warships, rate planets as if we have at least one
    reference_rating = max(cur_best_mil_ship_rating, MilitaryAI.get_preferred_max_military_portion_for_single_battle() *
                           MilitaryAI.get_concentrated_tot_mil_rating())
    threat_factor = min(1.0, reference_rating / (net_threat + 0.001)) ** 2
    if threat_factor < 0.5:
        mil_ref_string = "Military rating reference: %.1f" % reference_rating
        debug("Significant threat discounting %2d%% at %s, local defense: %.1f, local threat %.1f, area threat %.1f" %
              (100 * (1 - threat_factor), planet.name, local_defenses, local_threat, area_threat) + mil_ref_string)
    return threat_factor


@cache_by_turn
def get_claimed_stars():
    """
    Return dictionary of star type: list of colonised and planned to be colonized systems.
    Start type converted to int because `cache_by_turn` store its value in savegame
    and boost objects are not serializable.
    """
    claimed_stars = {}
    universe = fo.getUniverse()
    for s_type in AIstate.empireStars:
        claimed_stars[int(s_type)] = list(AIstate.empireStars[s_type])
    for sys_id in set(AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs):
        t_sys = universe.getSystem(sys_id)
        if not t_sys:
            continue
        claimed_stars.setdefault(int(t_sys.starType), []).append(sys_id)
    return claimed_stars


def assign_colony_fleets_to_colonise():

    aistate = get_aistate()
    aistate.orbital_colonization_manager.assign_bases_to_colonize()

    # assign fleet targets to colonisable planets
    all_colony_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.COLONISATION)
    send_colony_ships(FleetUtilsAI.extract_fleet_ids_without_mission_types(all_colony_fleet_ids),
                      aistate.colonisablePlanetIDs.items(),
                      MissionType.COLONISATION)

    # assign fleet targets to colonisable outposts
    all_outpost_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.OUTPOST)
    send_colony_ships(FleetUtilsAI.extract_fleet_ids_without_mission_types(all_outpost_fleet_ids),
                      aistate.colonisableOutpostIDs.items(),
                      MissionType.OUTPOST)


def send_colony_ships(colony_fleet_ids, evaluated_planets, mission_type):
    """sends a list of colony ships to a list of planet_value_pairs"""
    fleet_pool = colony_fleet_ids[:]
    try_all = False
    if mission_type == MissionType.OUTPOST:
        cost = 20 + outpod_pod_cost()
    else:
        try_all = True
        cost = 20 + colony_pod_cost()
        if fo.currentTurn() < 50:
            cost *= 0.4  # will be making fast tech progress so value is underestimated
        elif fo.currentTurn() < 80:
            cost *= 0.8  # will be making fast-ish tech progress so value is underestimated

    potential_targets = [(pid, (score, specName)) for (pid, (score, specName)) in evaluated_planets if
                         score > (0.8 * cost) and score > MINIMUM_COLONY_SCORE]

    debug("Colony/outpost ship matching: fleets %s to planets %s" % (fleet_pool, evaluated_planets))

    if try_all:
        debug("Trying best matches to current colony ships")
        best_scores = dict(evaluated_planets)
        potential_targets = []
        for pid, ratings in all_colony_opportunities.items():
            for rating in ratings:
                if rating[0] >= 0.75 * best_scores.get(pid, [9999])[0]:
                    potential_targets.append((pid, rating))
        potential_targets.sort(lambda x, y: cmp(x[1], y[1]), reverse=True)

    # added a lot of checking because have been getting mysterious exception, after too many recursions to get info
    fleet_pool = set(fleet_pool)
    universe = fo.getUniverse()
    for fid in fleet_pool:
        fleet = universe.getFleet(fid)
        if not fleet or fleet.empty:
            warn("Bad fleet ( ID %d ) given to colonization routine; will be skipped" % fid)
            fleet_pool.remove(fid)
            continue
        report_str = "Fleet ID (%d): %d ships; species: " % (fid, fleet.numShips)
        for sid in fleet.shipIDs:
            ship = universe.getShip(sid)
            if not ship:
                report_str += "NoShip, "
            else:
                report_str += "%s, " % ship.speciesName
        debug(report_str)
    debug('')
    already_targeted = []
    # for planetID_value_pair in evaluatedPlanets:
    aistate = get_aistate()
    while fleet_pool and potential_targets:
        target = potential_targets.pop(0)
        if target in already_targeted:
            continue
        planet_id = target[0]
        if planet_id in already_targeted:
            continue
        planet = universe.getPlanet(planet_id)
        sys_id = planet.systemID
        if aistate.systemStatus.setdefault(sys_id, {}).setdefault('monsterThreat', 0) > 2000 \
                or fo.currentTurn() < 20 and aistate.systemStatus[sys_id]['monsterThreat'] > 200:
            debug("Skipping colonization of system %s due to Big Monster, threat %d" % (
                universe.getSystem(sys_id), aistate.systemStatus[sys_id]['monsterThreat']))
            already_targeted.append(planet_id)
            continue
        this_spec = target[1][1]
        found_fleets = []
        try:
            this_fleet_list = FleetUtilsAI.get_fleets_for_mission(target_stats={}, min_stats={}, cur_stats={},
                                                                  starting_system=sys_id, species=this_spec,
                                                                  fleet_pool_set=fleet_pool, fleet_list=found_fleets)
        except Exception as e:
            error(e, exc_info=True)
            continue
        if not this_fleet_list:
            fleet_pool.update(found_fleets)  # just to be safe
            continue  # must have no compatible colony/outpost ships
        fleet_id = this_fleet_list[0]
        already_targeted.append(planet_id)
        ai_target = TargetPlanet(planet_id)
        aistate.get_fleet_mission(fleet_id).set_target(mission_type, ai_target)


def _print_empire_species_roster():
    """Print empire species roster in table format to log."""
    grade_map = {"ULTIMATE": "+++", "GREAT": "++", "GOOD": "+", "AVERAGE": "o", "BAD": "-", "NO": "---"}
    grade_tags = {'INDUSTRY': "Ind.", 'RESEARCH': "Res.", 'POPULATION': "Pop.",
                  'SUPPLY': "Supply", 'WEAPONS': "Pilots", 'ATTACKTROOPS': "Troops"}
    header = [Text('species'), Sequence('Planets'), Bool('Colonizer'), Text('Shipyards')]
    header.extend(Text(v) for v in grade_tags.values())
    header.append(Sequence('Tags'))
    species_table = Table(header, table_name="Empire species roster Turn %d" % fo.currentTurn())
    for species_name, planet_ids in state.get_empire_planets_by_species().items():
        species_tags = fo.getSpecies(species_name).tags
        is_colonizer = species_name in empire_colonizers
        number_of_shipyards = len(empire_ship_builders.get(species_name, []))
        this_row = [species_name, planet_ids, is_colonizer, number_of_shipyards]
        this_row.extend(grade_map.get(get_ai_tag_grade(species_tags, tag).upper(), "o") for tag in grade_tags)
        this_row.append([tag for tag in species_tags if not any(s in tag for s in grade_tags) and 'PEDIA' not in tag])
        species_table.add_row(this_row)
    print
    info(species_table)


def _print_outpost_candidate_table(candidates, show_detail=False):
    """Print a summary for the outpost candidates in a table format to log.

    :param candidates: list of (planet_id, (score, species, details)) tuples
    """
    __print_candidate_table(candidates, mission='Outposts', show_detail=show_detail)


def _print_colony_candidate_table(candidates, show_detail=False):
    """Print a summary for the colony candidates in a table format to log.

    :param candidates: list of (planet_id, (score, species, details)) tuples
    """
    __print_candidate_table(candidates, mission='Colonization', show_detail=show_detail)


def __print_candidate_table(candidates, mission, show_detail=False):
    universe = fo.getUniverse()
    if mission == 'Colonization':
        first_column = Text('(Score, Species)')

        def get_first_column_value(x):
            return round(x[0], 2), x[1]
    elif mission == 'Outposts':
        first_column = Float('Score')
        get_first_column_value = itemgetter(0)
    else:
        warn("__print_candidate_table(%s, %s): Invalid mission type" % (candidates, mission))
        return
    columns = [first_column, Text('Planet'), Text('System'), Sequence('Specials')]
    if show_detail:
        columns.append(Sequence('Detail'))
    candidate_table = Table(columns,
                            table_name='Potential Targets for %s in Turn %d' % (mission, fo.currentTurn()))
    for planet_id, score_tuple in candidates:
        if score_tuple[0] > 0.5:
            planet = universe.getPlanet(planet_id)
            entries = [get_first_column_value(score_tuple),
                       planet,
                       universe.getSystem(planet.systemID),
                       planet.specials,
                       ]
            if show_detail:
                entries.append(score_tuple[-1])
            candidate_table.add_row(entries)
    info(candidate_table)


class OrbitalColonizationPlan(object):
    def __init__(self, target_id, source_id):
        """
        :param target_id: id of the target planet to colonize
        :type target_id: int
        :param source_id: id of the planet which should build the colony base
        :type source_id: int
        """
        self.target = target_id
        self.source = source_id
        self.base_enqueued = False
        self.fleet_id = INVALID_ID
        self.__score = 0
        self.__last_score_update = -1

    def assign_base(self, fleet_id):
        """
        Assign an outpost base fleet to execute the plan.

        It is expected that the fleet consists of only that one outpost base.

        :type fleet_id: int
        :return: True on success, False on failure
        :rtype: bool
        """
        if self.base_assigned:
            warn("Assigned a base to a plan that was already assigned a base to.")
            return False
        # give orders to perform the mission
        target = TargetPlanet(self.target)
        fleet_mission = get_aistate().get_fleet_mission(fleet_id)
        fleet_mission.set_target(MissionType.ORBITAL_OUTPOST, target)
        self.fleet_id = fleet_id
        return True

    def enqueue_base(self):
        """
        Enqueue the base according to the plan.

        :return: True on success, False on failure
        :rtype: bool
        """
        if self.base_enqueued:
            warn("Tried to enqueue a base eventhough already done that.")
            return False

        # find the best possible base design for the source planet
        universe = fo.getUniverse()
        best_ship, _, _ = ProductionAI.get_best_ship_info(PriorityType.PRODUCTION_ORBITAL_OUTPOST, self.source)
        if best_ship is None:
            warn("Can't find optimized outpost base design at %s" % (universe.getPlanet(self.source)))
            try:
                best_ship = next(design for design in fo.getEmpire().availableShipDesigns
                                 if "SD_OUTPOST_BASE" == fo.getShipDesign(design).name)
                debug("Falling back to base design SD_OUTPOST_BASE")
            except StopIteration:
                # fallback design not available
                return False

        # enqueue the design at the source planet
        retval = fo.issueEnqueueShipProductionOrder(best_ship, self.source)
        debug("Enqueueing Outpost Base at %s for %s with result %s" % (
            universe.getPlanet(self.source), universe.getPlanet(self.target), retval))

        if not retval:
            warn("Failed to enqueue outpost base at %s" % universe.getPlanet(self.source))
            return False

        self.base_enqueued = True
        return True

    @property
    def base_assigned(self):
        if self.fleet_id == INVALID_ID:
            return False

        fleet = fo.getUniverse().getFleet(self.fleet_id)
        if fleet:
            return True

        debug("The fleet assigned to the OrbitalColonizationPlan doesn't exist anymore.")
        self.fleet_id = INVALID_ID
        return False

    @property
    def score(self):
        if self.__last_score_update != fo.currentTurn():
            self.__update_score()
        return self.__score

    def __update_score(self):
        planet_score = evaluate_planet(self.target, MissionType.OUTPOST, None)
        for species in empire_colonizers:
            this_score = evaluate_planet(self.target, MissionType.COLONISATION, species)
            planet_score = max(planet_score, this_score)
        self.__last_score_update = fo.currentTurn()
        self.__score = planet_score

    def is_valid(self):
        """
        Check the colonization plan for validity, i.e. if it could be executed in the future.

        The plan is valid if it is possible to outpust the target planet
        and if the planet envisioned to build the outpost bases can still do so.

        :rtype: bool
        """
        universe = fo.getUniverse()

        # make sure target is valid
        target = universe.getPlanet(self.target)
        if target is None or (not target.unowned) or target.speciesName:
            return False

        # make sure source is valid
        source = universe.getPlanet(self.source)
        if not (source and source.ownedBy(fo.empireID()) and source.speciesName in empire_colonizers):
            return False

        # appears to be valid
        return True


class OrbitalColonizationManager(object):
    """
    The OrbitalColonizationManager handles orbital colonization for the AI.

    :type _colonization_plans: dict[int, OrbitalColonizationPlan]
    :type num_enqueued_bases: int
    """
    def __init__(self):
        self._colonization_plans = {}
        self.num_enqueued_bases = 0

    def get_targets(self):
        """
        Return all planets for which an orbital colonization plan exists.

        :rtype: list[int]
        """
        return self._colonization_plans.keys()

    def create_new_plan(self, target_id, source_id):
        """
        Create and keep track of a new colonization plan for a target planet.

        :param target_id: id of the target planet
        :type target_id: int
        :param source_id: id of the planet which is supposed to build the base
        :type source_id: int
        """
        if target_id in self._colonization_plans:
            warn("Already have a colonization plan for this planet. Doing nothing.")
            return
        self._colonization_plans[target_id] = OrbitalColonizationPlan(target_id, source_id)

    def turn_start_cleanup(self):
        universe = fo.getUniverse()
        # clean up invalid or finished plans
        for pid in self._colonization_plans.keys():
            if not self._colonization_plans[pid].is_valid():
                del self._colonization_plans[pid]

        # parse the production queue and find bases which no longer have valid
        # targets (e.g. the planet was colonized already).
        self.num_enqueued_bases = 0
        unaccounted_plans = dict(self._colonization_plans)

        # Check which plans still have valid bases assigned (possibly interrupted by combat last turn)
        for pid in unaccounted_plans.keys():
            if unaccounted_plans[pid].base_assigned:
                del unaccounted_plans[pid]

        # find enqueued bases which are no longer needed and dequeue those.
        items_to_dequeue = []
        aistate = get_aistate()
        for idx, element in enumerate(fo.getEmpire().productionQueue):
            if element.buildType != EmpireProductionTypes.BT_SHIP or element.turnsLeft == -1:
                continue

            role = aistate.get_ship_role(element.designID)
            if role != ShipRoleType.BASE_OUTPOST:
                continue

            self.num_enqueued_bases += 1
            # check if a target for this base remains
            original_target = next((target for target, plan in unaccounted_plans.iteritems() if
                                    plan.source == element.locationID and plan.base_enqueued), None)
            if original_target:
                debug("Base built at %d still has its original target." % element.locationID)
                del unaccounted_plans[original_target]
                continue

            # the original target may be no longer valid but maybe there is another
            # orbital colonization plan which wasn't started yet and has the same source planet
            alternative_target = next((target for target, plan in unaccounted_plans.iteritems()
                                       if plan.source == element.locationID), None)
            if alternative_target:
                debug("Reassigning base built at %d to new target %d as old target is no longer valid" % (
                    element.locationID, alternative_target))
                self._colonization_plans[alternative_target].base_enqueued = True
                del unaccounted_plans[alternative_target]
                continue

            # final try: unstarted plans with source in the same system
            target_system = universe.getSystem(universe.getPlanet(element.locationID).systemID)
            alternative_plan = next((plan for target, plan in unaccounted_plans.iteritems()
                                     if plan.source in target_system.planetIDs and not plan.base_enqueued
                                     and not plan.base_assigned), None)
            if alternative_plan:
                debug("Reassigning base enqueued at %d to new plan with target %d. Previous source was %d" % (
                    element.locationID, alternative_plan.target, alternative_plan.source))
                alternative_plan.source = element.locationID
                alternative_plan.base_enqueued = True
                del unaccounted_plans[alternative_plan.target]
                continue

            debug("Could not find a target for the outpost base enqueued at %s" %
                  universe.getPlanet(element.locationID))
            items_to_dequeue.append(idx)

        # TODO: Stop Building for targets with now insufficient colonization score

        # delete last items first so that queue index of remaining items
        # does not have to be adjusted
        # TODO: Only pause the production if could become valid again
        items_to_dequeue.sort(reverse=True)
        for idx in items_to_dequeue:
            fo.issueDequeueProductionOrder(idx)
            self.num_enqueued_bases -= 1

    def build_bases(self):
        empire = fo.getEmpire()
        if not empire.techResearched(OUTPOSTING_TECH):
            return

        considered_plans = [plan for plan in self._colonization_plans.itervalues()
                            if not plan.base_enqueued and plan.score > MINIMUM_COLONY_SCORE]
        queue_limit = max(1, int(2*empire.productionPoints / outpod_pod_cost()))
        for colonization_plan in sorted(considered_plans, key=lambda x: x.score, reverse=True):
            if self.num_enqueued_bases >= queue_limit:
                debug("Base enqueue limit (%d) reached." % queue_limit)
                return

            success = colonization_plan.enqueue_base()
            if success:
                self.num_enqueued_bases += 1

    def assign_bases_to_colonize(self):
        universe = fo.getUniverse()
        all_outpost_base_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.ORBITAL_OUTPOST)
        avail_outpost_base_fleet_ids = FleetUtilsAI.extract_fleet_ids_without_mission_types(all_outpost_base_fleet_ids)
        for fid in avail_outpost_base_fleet_ids:
            fleet = universe.getFleet(fid)
            if not fleet:
                continue
            sys_id = fleet.systemID
            system = universe.getSystem(sys_id)

            avail_plans = [plan for plan in self._colonization_plans.itervalues()
                           if plan.target in system.planetIDs and not plan.base_assigned]
            avail_plans.sort(key=lambda x: x.score, reverse=True)
            for plan in avail_plans:
                success = plan.assign_base(fid)
                if success:
                    break


def test_calc_max_pop():
    """
    Verify AI calculation of max population by comparing it with actual client
    queried values.

    This function may be called in debug mode in a running game and will compare
    the actual target population meters on all planets owned by this AI with the
    predicted maximum population. Any mismatch will be reported in chat.
    """
    from freeorion_tools import chat_human
    chat_human("Verifying calculation of ColonisationAI.calc_max_pop()")
    universe = fo.getUniverse()
    for spec_name, planets in state.get_empire_planets_by_species().iteritems():
        species = fo.getSpecies(spec_name)
        for pid in planets:
            planet = universe.getPlanet(pid)
            detail = []
            predicted = calc_max_pop(planet, species, detail)
            actual = planet.initialMeterValue(fo.meterType.targetPopulation)
            if actual != predicted:
                error("Predicted pop of %.2f on %s but actually is %.2f; Details: %s" %
                      (predicted, planet, actual, "\n".join(detail)))
    chat_human("Finished verification of ColonisationAI.calc_max_pop()")
