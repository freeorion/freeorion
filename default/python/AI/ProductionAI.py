import freeOrionAIInterface as fo
import math
import random
from collections.abc import Iterable, Sequence
from logging import debug, error, warning
from operator import itemgetter
from typing import (
    NamedTuple,
    NewType,
    Union,
)

import AIDependencies
import AIstate
import FleetUtilsAI
import MilitaryAI
import PlanetUtilsAI
import PriorityAI
from AIDependencies import INVALID_ID, Tags
from aistate_interface import get_aistate
from buildings import BuildingType, BuildingTypeBase, Shipyard, get_empire_drydocks
from character.character_module import Aggression
from colonization import rate_planetary_piloting
from colonization.colony_score import MINIMUM_COLONY_SCORE
from colonization.rate_pilots import GREAT_PILOT_RATING
from common.fo_typing import BuildingName, PlanetId, SystemId
from empire.buildings_locations import get_best_pilot_facilities
from empire.colony_builders import (
    can_build_colony_for_species,
    can_build_only_sly_colonies,
    get_colony_builder_locations,
    get_colony_builders,
)
from empire.pilot_rating import (
    best_pilot_rating,
    get_pilot_ratings,
    get_rating_for_planet,
    medium_pilot_rating,
)
from empire.ship_builders import (
    can_build_ship_for_species,
    get_ship_builder_locations,
    has_shipyard,
)
from EnumsAI import (
    EmpireProductionTypes,
    FocusType,
    MissionType,
    PriorityType,
    ShipRoleType,
    get_priority_production_types,
)
from expansion_plans import get_colonisable_planet_ids
from freeorion_tools import get_named_real, ppstring, tech_is_complete
from production import print_building_list, print_capital_info, print_production_queue
from turn_state import (
    get_all_empire_planets,
    get_colonized_planets,
    get_colonized_planets_in_system,
    get_empire_outposts,
    get_empire_planets_by_species,
    get_empire_planets_with_species,
    get_inhabited_planets,
    get_owned_planets,
    get_owned_planets_in_system,
    population_with_industry_focus,
)
from turn_state.design import get_best_ship_info, get_best_ship_ratings
from universe.system_network import get_neighbors


def get_priority_locations() -> frozenset[PlanetId]:
    priority_facilities = [
        "BLD_SHIPYARD_ENRG_SOLAR",
        "BLD_SHIPYARD_CON_GEOINT",
        "BLD_SHIPYARD_AST_REF",
        Shipyard.ENRG_COMP.value,
    ]
    # TODO: also cover good troopship locations
    return frozenset(loc for building in priority_facilities for loc in get_best_pilot_facilities(building))


# set by ResourceAI
candidate_for_translator = None


def translators_wanted() -> int:
    """
    How many near universal translators we'd like to build.
    Function is also used by ResearchAI to determine when to research the prerequisites.
    """
    pid = PlanetUtilsAI.get_capital()
    # planet is needed to determine the cost. Without a capital we have bigger problems anyway...
    # At the beginning of the game influence_priority fluctuates strongly and that early in the game there are
    # surely more important things to build.
    if pid == INVALID_ID or fo.currentTurn() < 30:
        return 0.0
    building_type = BuildingType.TRANSLATOR
    translator_cost = building_type.production_cost(pid)
    influence_priority = get_aistate().get_priority(PriorityType.RESOURCE_INFLUENCE)
    num_species = len(get_empire_planets_by_species())
    num_enqueued = len(building_type.queued_at())
    num_built = len(building_type.built_at())
    # first one gives a policy slot
    first_bonus = 30 if num_enqueued + num_built == 0 else 0
    importance = 6 * (influence_priority + first_bonus) / translator_cost * num_species**0.25 - num_enqueued
    debug(
        f"translators_wanted: influence_priority: {influence_priority}, translator_cost: {translator_cost}, "
        f"built: {num_built}, enqueued: {num_enqueued}, num_species: {num_species}, turn: {fo.currentTurn()}, "
        f"importance: {importance}"
    )
    return int(importance)


def _get_capital_info() -> tuple[PlanetId, "fo.planet", SystemId]:
    capital_id = PlanetUtilsAI.get_capital()
    if capital_id is None or capital_id == INVALID_ID:
        homeworld = None
        capital_system_id = None
    else:
        homeworld = fo.getUniverse().getPlanet(capital_id)
        capital_system_id = homeworld.systemID
    return capital_id, homeworld, capital_system_id


def _first_turn_first_time():
    """
    Return true if generate production order at the first time.

    If game is loaded on the first turn this code should return false.
    """
    return fo.currentTurn() == 1 and len(AIstate.opponentPlanetIDs) == 0 and len(fo.getEmpire().productionQueue) == 0


def _first_turn_action():
    if not _first_turn_first_time():
        return

    init_build_nums = [(PriorityType.PRODUCTION_EXPLORATION, 2)]
    if can_build_only_sly_colonies():
        init_build_nums.append((PriorityType.PRODUCTION_COLONISATION, 1))
    else:
        init_build_nums.append((PriorityType.PRODUCTION_OUTPOST, 1))
    for ship_type, num_ships in init_build_nums:
        best_design_id, _, build_choices = get_best_ship_info(ship_type)
        if best_design_id is not None:
            for _ in range(num_ships):
                fo.issueEnqueueShipProductionOrder(best_design_id, build_choices[0])
            fo.updateProductionQueue()


def get_building_allocations() -> float:
    empire = fo.getEmpire()
    return sum(e.allocation for e in empire.productionQueue if e.buildType == EmpireProductionTypes.BT_BUILDING)


# TODO Move Building names to AIDependencies to avoid typos and for IDE-Support
def generate_production_orders():  # noqa: C901
    """Generate production orders."""
    # first check ship designs
    # next check for buildings etc, that could be placed on queue regardless of locally available PP
    # next loop over resource groups, adding buildings & ships
    print_production_queue()
    universe = fo.getUniverse()

    debug("Production Queue Management:")
    empire = fo.getEmpire()
    debug("")
    debug("  Total Available Production Points: %s" % empire.productionPoints)
    print_building_list()

    aistate = get_aistate()

    _first_turn_action()
    building_expense = 0.0
    building_ratio = aistate.character.preferred_building_ratio([0.4, 0.35, 0.30])
    capital_id, homeworld, capital_system_id = _get_capital_info()
    current_turn = fo.currentTurn()
    building_expense += get_building_allocations()
    if not homeworld:
        debug("if no capital, no place to build, should get around to capturing or colonizing a new one")  # TODO
    else:
        debug("Empire priority_id %d has current Capital %s:" % (empire.empireID, homeworld.name))
        print_capital_info(homeworld)
        capital_buildings = [universe.getBuilding(bldg).buildingTypeName for bldg in homeworld.buildingIDs]

        possible_building_type_ids = []
        for type_id in empire.availableBuildingTypes:
            # Apparently, when loading a saved game from another version, availableBuildingTypes may return
            # buildings that are not in our script.
            fo_building_type = fo.getBuildingType(type_id)
            if fo_building_type and fo_building_type.canBeProduced(empire.empireID, homeworld.id):
                possible_building_type_ids.append(type_id)

        if possible_building_type_ids:
            debug("Possible building types to build:")
            for type_id in possible_building_type_ids:
                building_type = fo.getBuildingType(type_id)
                debug(
                    "    {} cost: {}  time: {}".format(
                        building_type.name,
                        building_type.productionCost(empire.empireID, homeworld.id),
                        building_type.productionTime(empire.empireID, homeworld.id),
                    )
                )

            possible_building_types = [fo.getBuildingType(type_id).name for type_id in possible_building_type_ids]

            queued_building_names = _get_queued_buildings(capital_id)

            if "BLD_AUTO_HISTORY_ANALYSER" in possible_building_types:
                for pid in find_automatic_historic_analyzer_candidates():
                    res = fo.issueEnqueueBuildingProductionOrder("BLD_AUTO_HISTORY_ANALYSER", pid)
                    debug(
                        "Enqueueing BLD_AUTO_HISTORY_ANALYSER at planet %s - result %d" % (universe.getPlanet(pid), res)
                    )
                    if res:
                        cost, time = empire.productionCostAndTime(
                            empire.productionQueue[empire.productionQueue.size - 1]
                        )
                        building_expense += cost / time
                        res = fo.issueRequeueProductionOrder(empire.productionQueue.size - 1, 0)  # move to front
                        debug(
                            "Requeueing %s to front of build queue, with result %d" % ("BLD_AUTO_HISTORY_ANALYSER", res)
                        )

            # TODO: check existence of BLD_INDUSTRY_CENTER (and other buildings) in other locations in case we captured it
            if (
                (empire.productionPoints > 40 or ((current_turn > 40) and (population_with_industry_focus() >= 20)))
                and ("BLD_INDUSTRY_CENTER" in possible_building_types)
                and ("BLD_INDUSTRY_CENTER" not in (capital_buildings + queued_building_names))
                and (building_expense < building_ratio * empire.productionPoints)
            ):
                res = fo.issueEnqueueBuildingProductionOrder("BLD_INDUSTRY_CENTER", homeworld.id)
                debug("Enqueueing BLD_INDUSTRY_CENTER, with result %d" % res)
                if res:
                    cost, time = empire.productionCostAndTime(empire.productionQueue[empire.productionQueue.size - 1])
                    building_expense += cost / time

            if ("BLD_SHIPYARD_BASE" in possible_building_types) and (
                "BLD_SHIPYARD_BASE" not in (capital_buildings + queued_building_names)
            ):
                try:
                    res = fo.issueEnqueueBuildingProductionOrder("BLD_SHIPYARD_BASE", homeworld.id)
                    debug("Enqueueing BLD_SHIPYARD_BASE, with result %d" % res)
                except:  # noqa: E722
                    warning("Can't build shipyard at new capital, probably no population; we're hosed")

            for building_name in ["BLD_SHIPYARD_ORG_ORB_INC"]:
                if (
                    (building_name in possible_building_types)
                    and (building_name not in (capital_buildings + queued_building_names))
                    and (building_expense < building_ratio * empire.productionPoints)
                ):
                    try:
                        res = fo.issueEnqueueBuildingProductionOrder(building_name, homeworld.id)
                        debug("Enqueueing %s at capital, with result %d" % (building_name, res))
                        if res:
                            cost, time = empire.productionCostAndTime(
                                empire.productionQueue[empire.productionQueue.size - 1]
                            )
                            building_expense += cost / time
                            res = fo.issueRequeueProductionOrder(empire.productionQueue.size - 1, 0)  # move to front
                            debug("Requeueing %s to front of build queue, with result %d" % (building_name, res))
                    except:  # noqa: E722
                        error("Exception triggered and caught: ", exc_info=True)

            building_type = BuildingType.PALACE
            if building_type.available() and not building_type.built_or_queued_at():
                building_expense += _try_enqueue(building_type, capital_id, at_front=True, ignore_dislike=True)

            # ok, BLD_NEUTRONIUM_SYNTH is not currently unlockable, but just in case... ;-p
            if ("BLD_NEUTRONIUM_SYNTH" in possible_building_types) and (
                "BLD_NEUTRONIUM_SYNTH" not in (capital_buildings + queued_building_names)
            ):
                res = fo.issueEnqueueBuildingProductionOrder("BLD_NEUTRONIUM_SYNTH", homeworld.id)
                debug("Enqueueing BLD_NEUTRONIUM_SYNTH, with result %d" % res)
                if res:
                    res = fo.issueRequeueProductionOrder(empire.productionQueue.size - 1, 0)  # move to front
                    debug("Requeueing BLD_NEUTRONIUM_SYNTH to front of build queue, with result %d" % res)

    max_defense_portion = aistate.character.max_defense_portion()
    if aistate.character.check_orbital_production():
        sys_orbital_defenses = {}
        queued_defenses = {}
        defense_allocation = 0.0
        target_orbitals = aistate.character.target_number_of_orbitals()
        debug("Orbital Defense Check -- target Defense Orbitals: %s" % target_orbitals)
        for element in empire.productionQueue:
            if (element.buildType == EmpireProductionTypes.BT_SHIP) and (
                aistate.get_ship_role(element.designID) == ShipRoleType.BASE_DEFENSE
            ):
                planet = universe.getPlanet(element.locationID)
                if not planet:
                    error("Problem getting Planet for build loc %s" % element.locationID)
                    continue
                sys_id = planet.systemID
                queued_defenses[sys_id] = queued_defenses.get(sys_id, 0) + element.blocksize * element.remaining
                defense_allocation += element.allocation
        debug(
            "Queued Defenses: %s",
            ppstring([(str(universe.getSystem(sid)), num) for sid, num in queued_defenses.items()]),
        )
        for sys_id, pids in get_colonized_planets().items():
            if aistate.systemStatus.get(sys_id, {}).get("fleetThreat", 1) > 0:
                continue  # don't build orbital shields if enemy fleet present
            if defense_allocation > max_defense_portion * empire.productionPoints:
                break
            sys_orbital_defenses[sys_id] = 0
            fleets_here = aistate.systemStatus.get(sys_id, {}).get("myfleets", [])
            for fid in fleets_here:
                if aistate.get_fleet_role(fid) == MissionType.ORBITAL_DEFENSE:
                    debug(
                        "Found %d existing Orbital Defenses in %s :"
                        % (aistate.fleetStatus.get(fid, {}).get("nships", 0), universe.getSystem(sys_id))
                    )
                    sys_orbital_defenses[sys_id] += aistate.fleetStatus.get(fid, {}).get("nships", 0)
            for pid in pids:
                sys_orbital_defenses[sys_id] += queued_defenses.get(pid, 0)
            if sys_orbital_defenses[sys_id] < target_orbitals:
                num_needed = target_orbitals - sys_orbital_defenses[sys_id]
                for pid in pids:
                    best_design_id, col_design, build_choices = get_best_ship_info(
                        PriorityType.PRODUCTION_ORBITAL_DEFENSE, pid
                    )
                    if not best_design_id:
                        debug("no orbital defenses can be built at %s", PlanetUtilsAI.planet_string(pid))
                        continue
                    retval = fo.issueEnqueueShipProductionOrder(best_design_id, pid)
                    debug("queueing %d Orbital Defenses at %s" % (num_needed, PlanetUtilsAI.planet_string(pid)))
                    if retval != 0:
                        if num_needed > 1:
                            fo.issueChangeProductionQuantityOrder(empire.productionQueue.size - 1, 1, num_needed)
                        cost, time = empire.productionCostAndTime(
                            empire.productionQueue[empire.productionQueue.size - 1]
                        )
                        defense_allocation += (
                            empire.productionQueue[empire.productionQueue.size - 1].blocksize * cost / time
                        )
                        fo.issueRequeueProductionOrder(empire.productionQueue.size - 1, 0)  # move to front
                        break

    info = _build_basic_shipyards()
    queued_shipyard_pids = info.queued_shipyard_pids
    colony_systems = info.colony_systems
    top_pilot_systems = info.top_pilot_systems
    blackhole_pilots, red_pilots, building_expense = _build_energy_shipyards(
        queued_shipyard_pids, colony_systems, building_ratio, building_expense
    )
    _build_ship_facilities(Shipyard.ORG_ORB_INC)
    # gating by life cycle manipulation helps delay these until they are closer to being worthwhile
    if tech_is_complete(AIDependencies.GRO_LIFE_CYCLE) or empire.researchProgress(AIDependencies.GRO_LIFE_CYCLE) > 0:
        for building_type in (Shipyard.XENO_FACILITY, Shipyard.ORG_CELL_GRO_CHAMB):
            _build_ship_facilities(building_type)
    building_expense += _build_asteroid_processor(top_pilot_systems, queued_shipyard_pids)

    building_expense += _build_gas_giant_generator()
    building_expense += _build_translator()
    building_expense += _build_regional_administration()
    building_expense += _build_military_command()

    building_name = "BLD_SOL_ORB_GEN"
    if empire.buildingTypeAvailable(building_name) and aistate.character.may_build_building(building_name):
        already_got_one = 99
        for pid in get_all_empire_planets():
            planet = universe.getPlanet(pid)
            if planet and building_name in [
                bld.buildingTypeName for bld in map(universe.getBuilding, planet.buildingIDs)
            ]:
                system = universe.getSystem(planet.systemID)
                if system and system.starType < already_got_one:
                    already_got_one = system.starType
        best_type = fo.starType.white
        best_locs = AIstate.empireStars.get(fo.starType.blue, []) + AIstate.empireStars.get(fo.starType.white, [])
        if not best_locs:
            best_type = fo.starType.orange
            best_locs = AIstate.empireStars.get(fo.starType.yellow, []) + AIstate.empireStars.get(
                fo.starType.orange, []
            )
        if (not best_locs) or (already_got_one < 99 and already_got_one <= best_type):
            pass  # could consider building at a red star if have a lot of PP but somehow no better stars
        else:
            use_new_loc = True
            queued_building_locs = [
                element.locationID for element in (empire.productionQueue) if (element.name == building_name)
            ]
            if queued_building_locs:
                queued_star_types = {}
                for loc in queued_building_locs:
                    planet = universe.getPlanet(loc)
                    if not planet:
                        continue
                    system = universe.getSystem(planet.systemID)
                    queued_star_types.setdefault(system.starType, []).append(loc)
                if queued_star_types:
                    best_queued = sorted(queued_star_types.keys())[0]
                    if best_queued > best_type:  # i.e., best_queued is yellow, best_type available is blue or white
                        pass  # should probably evaluate cancelling the existing one under construction
                    else:
                        use_new_loc = False
            if use_new_loc:  # (of course, may be only loc, not really new)
                if not homeworld:
                    use_sys = best_locs[0]  # as good as any
                else:
                    distance_map = {}
                    for sys_id in best_locs:  # want to build close to capital for defense
                        if sys_id == INVALID_ID:
                            continue
                        try:
                            distance_map[sys_id] = universe.jumpDistance(homeworld.systemID, sys_id)
                        except:  # noqa: E722
                            pass
                    use_sys = ([(-1, INVALID_ID)] + sorted([(dist, sys_id) for sys_id, dist in distance_map.items()]))[
                        :2
                    ][-1][
                        -1
                    ]  # kinda messy, but ensures a value
                if use_sys != INVALID_ID:
                    try:
                        use_loc = get_owned_planets_in_system(use_sys)[0]
                        res = fo.issueEnqueueBuildingProductionOrder(building_name, use_loc)
                        debug(
                            "Enqueueing %s at planet %d (%s) , with result %d",
                            building_name,
                            use_loc,
                            universe.getPlanet(use_loc).name,
                            res,
                        )
                        if res:
                            cost, time = empire.productionCostAndTime(
                                empire.productionQueue[empire.productionQueue.size - 1]
                            )
                            building_expense += cost / time  # production_queue[production_queue.size -1].blocksize *
                            res = fo.issueRequeueProductionOrder(empire.productionQueue.size - 1, 0)  # move to front
                            debug("Requeueing %s to front of build queue, with result %d", building_name, res)
                    except:  # noqa: E722
                        debug("problem queueing BLD_SOL_ORB_GEN at planet %s of system", use_loc, use_sys)

    building_name = "BLD_ART_BLACK_HOLE"
    if (
        empire.buildingTypeAvailable(building_name)
        and aistate.character.may_build_building(building_name)
        and len(AIstate.empireStars.get(fo.starType.red, [])) > 0
    ):
        already_got_one = False
        for pid in get_all_empire_planets():
            planet = universe.getPlanet(pid)
            if planet and building_name in [
                bld.buildingTypeName for bld in map(universe.getBuilding, planet.buildingIDs)
            ]:
                already_got_one = True  # has been built, needs one turn to activate
        queued_building_locs = [
            element.locationID for element in (empire.productionQueue) if (element.name == building_name)
        ]  # TODO: check that queued locs or already built one are at red stars
        if not blackhole_pilots and len(queued_building_locs) == 0 and (red_pilots or not already_got_one):
            use_loc = None
            nominal_home = homeworld or universe.getPlanet(
                (red_pilots + get_owned_planets_in_system(AIstate.empireStars[fo.starType.red][0]))[0]
            )
            distance_map = {}
            for sys_id in AIstate.empireStars.get(fo.starType.red, []):
                if sys_id == INVALID_ID:
                    continue
                try:
                    distance_map[sys_id] = universe.jumpDistance(nominal_home.systemID, sys_id)
                except:  # noqa: E722
                    pass
            red_sys_list = sorted([(dist, sys_id) for sys_id, dist in distance_map.items()])
            for dist, sys_id in red_sys_list:
                for loc in get_owned_planets_in_system(sys_id):
                    planet = universe.getPlanet(loc)
                    if planet and planet.speciesName not in ["", None]:
                        species = fo.getSpecies(planet.speciesName)
                        if species and "PHOTOTROPHIC" in list(species.tags):
                            break
                else:
                    use_loc = list(
                        set(red_pilots).intersection(get_owned_planets_in_system(sys_id))
                        or get_owned_planets_in_system(sys_id)
                    )[0]
                if use_loc is not None:
                    break
            if use_loc is not None:
                planet_used = universe.getPlanet(use_loc)
                try:
                    res = fo.issueEnqueueBuildingProductionOrder(building_name, use_loc)
                    debug("Enqueueing %s at planet %s , with result %d", building_name, planet_used, res)
                    if res:
                        res = fo.issueRequeueProductionOrder(empire.productionQueue.size - 1, 0)  # move to front
                        debug("Requeueing %s to front of build queue, with result %d", building_name, res)
                except:  # noqa: E722
                    debug(f"problem queueing {building_name} at planet {planet_used}")

    building_name = "BLD_BLACK_HOLE_POW_GEN"
    if empire.buildingTypeAvailable(building_name) and aistate.character.may_build_building(building_name):
        already_got_one = False
        for pid in get_all_empire_planets():
            planet = universe.getPlanet(pid)
            if planet and building_name in [
                bld.buildingTypeName for bld in map(universe.getBuilding, planet.buildingIDs)
            ]:
                already_got_one = True
        queued_building_locs = [
            element.locationID for element in (empire.productionQueue) if (element.name == building_name)
        ]
        if (
            (len(AIstate.empireStars.get(fo.starType.blackHole, [])) > 0)
            and len(queued_building_locs) == 0
            and not already_got_one
        ):
            if not homeworld:
                use_sys = AIstate.empireStars.get(fo.starType.blackHole, [])[0]
            else:
                distance_map = {}
                for sys_id in AIstate.empireStars.get(fo.starType.blackHole, []):
                    if sys_id == INVALID_ID:
                        continue
                    try:
                        distance_map[sys_id] = universe.jumpDistance(homeworld.systemID, sys_id)
                    except:  # noqa: E722
                        pass
                use_sys = ([(-1, INVALID_ID)] + sorted([(dist, sys_id) for sys_id, dist in distance_map.items()]))[:2][
                    -1
                ][
                    -1
                ]  # kinda messy, but ensures a value
            if use_sys != INVALID_ID:
                try:
                    use_loc = get_owned_planets_in_system(use_sys)[0]
                    res = fo.issueEnqueueBuildingProductionOrder(building_name, use_loc)
                    debug(
                        "Enqueueing %s at planet %d (%s) , with result %d",
                        building_name,
                        use_loc,
                        universe.getPlanet(use_loc).name,
                        res,
                    )
                    if res:
                        res = fo.issueRequeueProductionOrder(empire.productionQueue.size - 1, 0)  # move to front
                        debug("Requeueing %s to front of build queue, with result %d" % (building_name, res))
                except:  # noqa: E722
                    warning("problem queueing BLD_BLACK_HOLE_POW_GEN at planet %s of system %s", use_loc, use_sys)

    building_name = "BLD_ENCLAVE_VOID"
    if empire.buildingTypeAvailable(building_name):
        already_got_one = False
        for pid in get_all_empire_planets():
            planet = universe.getPlanet(pid)
            if planet and building_name in [
                bld.buildingTypeName for bld in map(universe.getBuilding, planet.buildingIDs)
            ]:
                already_got_one = True
        queued_locs = [element.locationID for element in (empire.productionQueue) if (element.name == building_name)]
        if len(queued_locs) == 0 and homeworld and not already_got_one:
            try:
                res = fo.issueEnqueueBuildingProductionOrder(building_name, capital_id)
                debug(
                    "Enqueueing %s at planet %d (%s) , with result %d",
                    building_name,
                    capital_id,
                    universe.getPlanet(capital_id).name,
                    res,
                )
                if res:
                    res = fo.issueRequeueProductionOrder(empire.productionQueue.size - 1, 0)  # move to front
                    debug("Requeueing %s to front of build queue, with result %d", building_name, res)
            except:  # noqa: E722
                pass

    building_name = "BLD_GENOME_BANK"
    if empire.buildingTypeAvailable(building_name):
        already_got_one = False
        for pid in get_all_empire_planets():
            planet = universe.getPlanet(pid)
            if planet and building_name in [
                bld.buildingTypeName for bld in map(universe.getBuilding, planet.buildingIDs)
            ]:
                already_got_one = True
        queued_locs = [element.locationID for element in (empire.productionQueue) if (element.name == building_name)]
        if len(queued_locs) == 0 and homeworld and not already_got_one:
            try:
                res = fo.issueEnqueueBuildingProductionOrder(building_name, capital_id)
                debug(
                    "Enqueueing %s at planet %d (%s) , with result %d",
                    building_name,
                    capital_id,
                    universe.getPlanet(capital_id).name,
                    res,
                )
                if res:
                    res = fo.issueRequeueProductionOrder(empire.productionQueue.size - 1, 0)  # move to front
                    debug("Requeueing %s to front of build queue, with result %d", building_name, res)
            except:  # noqa: E722
                pass

    building_name = "BLD_NEUTRONIUM_EXTRACTOR"
    already_got_extractor = False
    if (
        empire.buildingTypeAvailable(building_name)
        and [element.locationID for element in (empire.productionQueue) if (element.name == building_name)] == []
        and AIstate.empireStars.get(fo.starType.neutron, [])
    ):
        # building_type = fo.getBuildingType(building_name)
        for pid in get_all_empire_planets():
            planet = universe.getPlanet(pid)
            if planet:
                building_names = [bld.buildingTypeName for bld in map(universe.getBuilding, planet.buildingIDs)]
                if (
                    planet.systemID in AIstate.empireStars.get(fo.starType.neutron, [])
                    and building_name in building_names
                ) or "BLD_NEUTRONIUM_SYNTH" in building_names:
                    already_got_extractor = True
        if not already_got_extractor:
            if not homeworld:
                use_sys = AIstate.empireStars.get(fo.starType.neutron, [])[0]
            else:
                distance_map = {}
                for sys_id in AIstate.empireStars.get(fo.starType.neutron, []):
                    if sys_id == INVALID_ID:
                        continue
                    try:
                        distance_map[sys_id] = universe.jumpDistance(homeworld.systemID, sys_id)
                    except Exception:
                        warning("Could not get jump distance from %d to %d", homeworld.systemID, sys_id, exc_info=True)
                debug([INVALID_ID] + sorted([(dist, sys_id) for sys_id, dist in distance_map.items()]))
                use_sys = ([(-1, INVALID_ID)] + sorted([(dist, sys_id) for sys_id, dist in distance_map.items()]))[:2][
                    -1
                ][
                    -1
                ]  # kinda messy, but ensures a value
            if use_sys != INVALID_ID:
                try:
                    use_loc = get_owned_planets_in_system(use_sys)[0]
                    res = fo.issueEnqueueBuildingProductionOrder(building_name, use_loc)
                    debug(
                        "Enqueueing %s at planet %d (%s) , with result %d",
                        building_name,
                        use_loc,
                        universe.getPlanet(use_loc).name,
                        res,
                    )
                    if res:
                        res = fo.issueRequeueProductionOrder(empire.productionQueue.size - 1, 0)  # move to front
                        debug("Requeueing %s to front of build queue, with result %d", building_name, res)
                except:  # noqa: E722
                    warning(f"problem queueing BLD_NEUTRONIUM_EXTRACTOR at planet {use_loc} of system {use_sys}")

    _build_ship_facilities(Shipyard.GEO)

    # with current stats the AI considers Titanic Hull superior to Scattered Asteroid, so don't bother building for now
    # TODO: uncomment once dynamic assessment of prospective designs is enabled & indicates building is worthwhile
    _build_ship_facilities(Shipyard.ASTEROID_REF)

    _build_ship_facilities(Shipyard.NEUTRONIUM_FORGE, get_priority_locations())

    colony_ship_map = {}
    for fid in FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.COLONISATION):
        fleet = universe.getFleet(fid)
        if not fleet:
            continue
        for shipID in fleet.shipIDs:
            ship = universe.getShip(shipID)
            if ship and (aistate.get_ship_role(ship.design.id) == ShipRoleType.CIVILIAN_COLONISATION):
                colony_ship_map.setdefault(ship.speciesName, []).append(1)

    building_name = "BLD_CONC_CAMP"
    building_type = fo.getBuildingType(building_name)
    for pid in get_inhabited_planets():
        planet = universe.getPlanet(pid)
        if not planet:
            continue
        can_build_camp = building_type.canBeProduced(empire.empireID, pid) and empire.buildingTypeAvailable(
            building_name
        )
        t_pop = planet.initialMeterValue(fo.meterType.targetPopulation)
        c_pop = planet.initialMeterValue(fo.meterType.population)
        t_ind = planet.currentMeterValue(fo.meterType.targetIndustry)
        c_ind = planet.currentMeterValue(fo.meterType.industry)
        pop_disqualified = (c_pop <= 32) or (c_pop < 0.9 * t_pop)
        this_spec = planet.speciesName
        safety_margin_met = (
            can_build_colony_for_species(this_spec)
            and (len(get_empire_planets_with_species(this_spec)) + len(colony_ship_map.get(this_spec, [])) >= 2)
        ) or (c_pop >= 50)
        if (
            pop_disqualified or not safety_margin_met
        ):  # check even if not aggressive, etc, just in case acquired planet with a ConcCamp on it
            for bldg in planet.buildingIDs:
                if universe.getBuilding(bldg).buildingTypeName == building_name:
                    res = fo.issueScrapOrder(bldg)
                    debug("Tried scrapping %s at planet %s, got result %d", building_name, planet.name, res)
        elif aistate.character.may_build_building(building_name) and can_build_camp and (t_pop >= 36):
            if (
                (planet.focus == FocusType.FOCUS_GROWTH)
                or (AIDependencies.COMPUTRONIUM_SPECIAL in planet.specials)
                or (pid == capital_id)
            ):
                continue
                # now that focus setting takes these into account, probably works ok to have conc camp, but let's not push it
            queued_building_locs = [
                element.locationID for element in (empire.productionQueue) if (element.name == building_name)
            ]
            if c_pop >= 0.95 * t_pop:
                if pid not in queued_building_locs:
                    if planet.focus in [FocusType.FOCUS_INDUSTRY]:
                        if c_ind >= t_ind + c_pop:
                            continue
                    else:
                        old_focus = planet.focus
                        fo.issueChangeFocusOrder(pid, FocusType.FOCUS_INDUSTRY)
                        universe.updateMeterEstimates([pid])
                        t_ind = planet.currentMeterValue(fo.meterType.targetIndustry)
                        if c_ind >= t_ind + c_pop:
                            fo.issueChangeFocusOrder(pid, old_focus)
                            universe.updateMeterEstimates([pid])
                            continue
                    res = fo.issueEnqueueBuildingProductionOrder(building_name, pid)
                    debug(
                        "Enqueueing %s at planet %d (%s) , with result %d",
                        building_name,
                        pid,
                        universe.getPlanet(pid).name,
                        res,
                    )
                    if res:
                        queued_building_locs.append(pid)
                        fo.issueRequeueProductionOrder(empire.productionQueue.size - 1, 0)  # move to front
                    else:
                        # TODO: enable location condition reporting a la mapwnd BuildDesignatorWnd
                        warning(
                            f"Enqueing Conc Camp at {planet} despite building_type.canBeProduced(empire.empireID, pid) reporting {can_build_camp}"
                        )
    building_expense += _build_scanning_facility()

    _build_orbital_drydock(top_pilot_systems)

    building_name = "BLD_XENORESURRECTION_LAB"
    queued_xeno_lab_locs = [element.locationID for element in (empire.productionQueue) if element.name == building_name]
    for pid in get_all_empire_planets():
        if pid in queued_xeno_lab_locs or not empire.canBuild(fo.buildType.BT_BUILDING, building_name, pid):
            continue
        res = fo.issueEnqueueBuildingProductionOrder(building_name, pid)
        debug("Enqueueing %s at planet %d (%s) , with result %d", building_name, pid, planet.name, res)
        if res:
            res = fo.issueRequeueProductionOrder(empire.productionQueue.size - 1, 2)  # move to near front
            debug("Requeueing %s to front of build queue, with result %d", building_name, res)
            break
        else:
            warning("Failed enqueueing %s at planet %s, got result %d", building_name, planet, res)

    # ignore acquired-under-construction colony buildings for which our empire lacks the species
    queued_clny_bld_locs = [
        element.locationID
        for element in (empire.productionQueue)
        if (element.name.startswith("BLD_COL_") and empire_has_colony_bld_species(element.name))
    ]
    colony_bldg_entries = [
        entry
        for entry in get_colonisable_planet_ids().items()
        if entry[1][0] > MINIMUM_COLONY_SCORE
        and entry[0] not in queued_clny_bld_locs
        and entry[0] in get_empire_outposts()
        and not already_has_completed_colony_building(entry[0])
    ]
    colony_bldg_entries = colony_bldg_entries[: PriorityAI.allottedColonyTargets + 2]
    for entry in colony_bldg_entries:
        pid = entry[0]
        building_name = "BLD_COL_" + entry[1][1][3:]
        planet = universe.getPlanet(pid)
        building_type = fo.getBuildingType(building_name)
        # We may have conquered a planet with a queued colony.
        # If we want to build another species, we have to remove the queued one.
        _remove_other_colonies(pid, building_name)
        if not (building_type and building_type.canBeEnqueued(empire.empireID, pid)):
            continue
        res = fo.issueEnqueueBuildingProductionOrder(building_name, pid)
        debug("Enqueueing %s at planet %d (%s) , with result %d", building_name, pid, planet.name, res)
        if res:
            res = fo.issueRequeueProductionOrder(empire.productionQueue.size - 1, 2)  # move to near front
            debug("Requeueing %s to front of build queue, with result %d", building_name, res)
            break
        else:
            warning("Failed enqueueing %s at planet %s, got result %d" % (building_name, planet, res))

    buildings_to_scrap = ("BLD_EVACUATION", "BLD_GATEWAY_VOID")
    for pid in get_inhabited_planets():
        planet = universe.getPlanet(pid)
        if not planet:
            continue
        for bldg in planet.buildingIDs:
            building_name = universe.getBuilding(bldg).buildingTypeName
            if building_name in buildings_to_scrap:
                res = fo.issueScrapOrder(bldg)
                debug("Tried scrapping %s at planet %s, got result %d", building_name, planet.name, res)

    total_pp_spent = fo.getEmpire().productionQueue.totalSpent
    debug("  Total Production Points Spent: %s", total_pp_spent)

    wasted_pp = max(0, empire.productionPoints - total_pp_spent)
    debug("  Wasted Production Points: %s", wasted_pp)  # TODO: add resource group analysis
    avail_pp = empire.productionPoints - total_pp_spent - 0.0001

    production_queue = empire.productionQueue
    queued_colony_ships = {}
    queued_outpost_ships = 0
    queued_troop_ships = 0

    # TODO: blocked items might not need dequeuing, but rather for supply lines to be un-blockaded
    dequeue_list = []
    fo.updateProductionQueue()
    can_prioritize_troops = False
    for queue_index in range(len(production_queue)):
        element = production_queue[queue_index]
        block_str = (
            "%d x " % element.blocksize
        )  # ["a single ", "in blocks of %d "%element.blocksize][element.blocksize>1]
        debug(
            "    %s%s  requiring %s  more turns; alloc: %.2f PP with cum. progress of %.1f  being built at %s",
            block_str,
            element.name,
            element.turnsLeft,
            element.allocation,
            element.progress,
            universe.getPlanet(element.locationID).name,
        )
        if element.turnsLeft == -1:
            if element.locationID not in get_all_empire_planets():
                # dequeue_list.append(queue_index) #TODO add assessment of recapture -- invasion target etc.
                debug(
                    "element %s will never be completed as stands and location %d no longer owned; could consider deleting from queue",
                    element.name,
                    element.locationID,
                )  # TODO:
            else:
                debug(
                    "element %s is projected to never be completed as currently stands, but will remain on queue ",
                    element.name,
                )
        elif element.buildType == EmpireProductionTypes.BT_SHIP:
            this_role = aistate.get_ship_role(element.designID)
            if this_role == ShipRoleType.CIVILIAN_COLONISATION:
                this_spec = universe.getPlanet(element.locationID).speciesName
                queued_colony_ships[this_spec] = (
                    queued_colony_ships.get(this_spec, 0) + element.remaining * element.blocksize
                )
            elif this_role == ShipRoleType.CIVILIAN_OUTPOST:
                queued_outpost_ships += element.remaining * element.blocksize
            elif this_role == ShipRoleType.BASE_OUTPOST:
                queued_outpost_ships += element.remaining * element.blocksize
            elif this_role == ShipRoleType.MILITARY_INVASION:
                queued_troop_ships += element.remaining * element.blocksize
            elif (this_role == ShipRoleType.CIVILIAN_EXPLORATION) and (queue_index <= 1):
                if len(AIstate.opponentPlanetIDs) > 0:
                    can_prioritize_troops = True
    if queued_colony_ships:
        debug("\nFound colony ships in build queue: %s", queued_colony_ships)
    if queued_outpost_ships:
        debug("\nFound outpost ships and bases in build queue: %s", queued_outpost_ships)

    for queue_index in dequeue_list[::-1]:
        fo.issueDequeueProductionOrder(queue_index)

    all_military_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.MILITARY)
    total_military_ships = sum([aistate.fleetStatus.get(fid, {}).get("nships", 0) for fid in all_military_fleet_ids])
    all_troop_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.INVASION)
    total_troop_ships = sum([aistate.fleetStatus.get(fid, {}).get("nships", 0) for fid in all_troop_fleet_ids])
    avail_troop_fleet_ids = list(FleetUtilsAI.extract_fleet_ids_without_mission_types(all_troop_fleet_ids))
    total_available_troops = sum([aistate.fleetStatus.get(fid, {}).get("nships", 0) for fid in avail_troop_fleet_ids])
    debug(
        "Trooper Status turn %d: %d total, with %d unassigned. %d queued, compared to %d total Military Attack Ships",
        current_turn,
        total_troop_ships,
        total_available_troops,
        queued_troop_ships,
        total_military_ships,
    )
    if (
        capital_id is not None
        and (current_turn >= 40 or can_prioritize_troops)
        and aistate.systemStatus.get(capital_system_id, {}).get("fleetThreat", 0) == 0
        and aistate.systemStatus.get(capital_system_id, {}).get("neighborThreat", 0) == 0
    ):
        best_design_id, best_design, build_choices = get_best_ship_info(PriorityType.PRODUCTION_INVASION)
        if build_choices is not None and len(build_choices) > 0:
            loc = random.choice(build_choices)
            prod_time = best_design.productionTime(empire.empireID, loc)
            prod_cost = best_design.productionCost(empire.empireID, loc)
            troopers_needed = max(
                0,
                int(
                    min(
                        0.99 + (current_turn / 20.0 - total_available_troops) / max(2, prod_time - 1),
                        total_military_ships // 3 - total_troop_ships,
                    )
                ),
            )
            ship_number = troopers_needed
            per_turn_cost = float(prod_cost) / prod_time
            if (
                troopers_needed > 0
                and empire.productionPoints > 3 * per_turn_cost * queued_troop_ships
                and aistate.character.may_produce_troops()
            ):
                retval = fo.issueEnqueueShipProductionOrder(best_design_id, loc)
                if retval != 0:
                    debug(
                        "forcing %d new ship(s) to production queue: %s; per turn production cost %.1f\n",
                        ship_number,
                        best_design.name,
                        ship_number * per_turn_cost,
                    )
                    if ship_number > 1:
                        fo.issueChangeProductionQuantityOrder(production_queue.size - 1, 1, ship_number)
                    avail_pp -= ship_number * per_turn_cost
                    fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                    fo.updateProductionQueue()
        debug("")

    debug("")
    # get the highest production priorities
    production_priorities = {}
    for priority_type in get_priority_production_types():
        production_priorities[priority_type] = int(max(0, (aistate.get_priority(priority_type)) ** 0.5))

    sorted_priorities = sorted(production_priorities.items(), key=itemgetter(1), reverse=True)

    top_score = -1

    num_colony_fleets = len(
        FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.COLONISATION)
    )  # counting existing colony fleets each as one ship
    total_colony_fleets = sum(queued_colony_ships.values()) + num_colony_fleets
    num_outpost_fleets = len(
        FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.OUTPOST)
    )  # counting existing outpost fleets each as one ship
    total_outpost_fleets = queued_outpost_ships + num_outpost_fleets

    max_colony_fleets = PriorityAI.allottedColonyTargets
    max_outpost_fleets = max_colony_fleets

    _, _, colony_build_choices = get_best_ship_info(PriorityType.PRODUCTION_COLONISATION)
    military_emergency = PriorityAI.unmetThreat > (2.0 * MilitaryAI.get_tot_mil_rating())

    debug("Production Queue Priorities:")
    filtered_priorities = {}
    for priority_id, score in sorted_priorities:
        if military_emergency:
            if priority_id == PriorityType.PRODUCTION_EXPLORATION:
                score /= 10.0
            elif priority_id != PriorityType.PRODUCTION_MILITARY:
                score /= 2.0
        if top_score < score:
            top_score = score  # don't really need top_score nor sorting with current handling
        debug(" Score: %4d -- %s ", score, priority_id)
        if priority_id != PriorityType.PRODUCTION_BUILDINGS:
            if (
                (priority_id == PriorityType.PRODUCTION_COLONISATION)
                and (total_colony_fleets < max_colony_fleets)
                and (colony_build_choices is not None)
                and len(colony_build_choices) > 0
            ):
                filtered_priorities[priority_id] = score
            elif (priority_id == PriorityType.PRODUCTION_OUTPOST) and (total_outpost_fleets < max_outpost_fleets):
                filtered_priorities[priority_id] = score
            elif priority_id not in [PriorityType.PRODUCTION_OUTPOST, PriorityType.PRODUCTION_COLONISATION]:
                filtered_priorities[priority_id] = score
    if filtered_priorities == {}:
        debug("No non-building-production priorities with nonzero score, setting to default: Military")
        filtered_priorities[PriorityType.PRODUCTION_MILITARY] = 1
    if top_score <= 100:
        scaling_power = 1.0
    else:
        scaling_power = math.log(100) / math.log(top_score)
    for pty in filtered_priorities:
        filtered_priorities[pty] **= scaling_power

    available_pp = {
        tuple(el.key()): el.data() for el in empire.planetsWithAvailablePP
    }  # keys are sets of ints; data is doubles
    allocated_pp = {
        tuple(el.key()): el.data() for el in empire.planetsWithAllocatedPP
    }  # keys are sets of ints; data is doubles
    planets_with_wasted_pp = {tuple(pidset) for pidset in empire.planetsWithWastedPP}
    debug("avail_pp ( <systems> : pp ):")
    for planet_set in available_pp:
        debug(
            "\t%s\t%.2f",
            PlanetUtilsAI.sys_name_ids(set(PlanetUtilsAI.get_systems(planet_set))),
            available_pp[planet_set],
        )
    debug("\nallocated_pp ( <systems> : pp ):")
    for planet_set in allocated_pp:
        debug(
            "\t%s\t%.2f",
            PlanetUtilsAI.sys_name_ids(set(PlanetUtilsAI.get_systems(planet_set))),
            allocated_pp[planet_set],
        )

    debug("\n\nBuilding Ships in system groups with remaining PP:")
    for planet_set in planets_with_wasted_pp:
        total_pp = available_pp.get(planet_set, 0)
        avail_pp = total_pp - allocated_pp.get(planet_set, 0)
        if avail_pp <= 0.01:
            continue
        debug(
            "%.2f PP remaining in system group: %s",
            avail_pp,
            PlanetUtilsAI.sys_name_ids(set(PlanetUtilsAI.get_systems(planet_set))),
        )
        debug("\t owned planets in this group are:")
        debug("\t %s", PlanetUtilsAI.planet_string(planet_set))
        best_design_id, best_design, build_choices = get_best_ship_info(
            PriorityType.PRODUCTION_COLONISATION, planet_set
        )
        species_map = {}
        for loc in build_choices or []:
            this_spec = universe.getPlanet(loc).speciesName
            species_map.setdefault(this_spec, []).append(loc)
        colony_build_choices = []
        for pid, (score, this_spec) in aistate.colonisablePlanetIDs.items():
            # add planets multiple times to emulate choice with weight
            weight = int(math.ceil(score))
            planets_for_colonization = [pid_ for pid_ in species_map.get(this_spec, []) if pid_ in planet_set]
            weighted_planets = weight * planets_for_colonization
            colony_build_choices.extend(weighted_planets)

        local_priorities = {}
        local_priorities.update(filtered_priorities)
        best_ships = {}
        mil_build_choices = get_best_ship_ratings(planet_set)
        for priority in list(local_priorities):
            if priority == PriorityType.PRODUCTION_MILITARY:
                if not mil_build_choices:
                    del local_priorities[priority]
                    continue
                _, pid, best_design_id, best_design = mil_build_choices[0]
                build_choices = [pid]
                # score = ColonisationAI.pilotRatings.get(pid, 0)
                # if bestScore < ColonisationAI.curMidPilotRating:
            else:
                best_design_id, best_design, build_choices = get_best_ship_info(priority, planet_set)
            if best_design is None:
                del local_priorities[priority]  # must be missing a shipyard -- TODO build a shipyard if necessary
                continue
            best_ships[priority] = [best_design_id, best_design, build_choices]
            debug("best_ships[%s] = %s \t locs are %s from %s", priority, best_design.name, build_choices, planet_set)

        if len(local_priorities) == 0:
            debug(
                "Alert!! need shipyards in systemSet %s",
                PlanetUtilsAI.sys_name_ids(set(PlanetUtilsAI.get_systems(planet_set))),
            )
        priority_choices = []
        for priority in local_priorities:
            priority_choices.extend(int(local_priorities[priority]) * [priority])

        loop_count = 0
        while (
            (avail_pp > 0) and (loop_count < max(100, current_turn)) and (priority_choices != [])
        ):  # make sure don't get stuck in some nonbreaking loop like if all shipyards captured
            loop_count += 1
            debug("Beginning build enqueue loop %d; %.1f PP available", loop_count, avail_pp)
            this_priority = random.choice(priority_choices)
            debug("selected priority: %s", this_priority)
            making_colony_ship = False
            making_outpost_ship = False
            if this_priority == PriorityType.PRODUCTION_COLONISATION:
                if total_colony_fleets >= max_colony_fleets:
                    debug("Already sufficient colony ships in queue, trying next priority choice\n")
                    for i in range(len(priority_choices) - 1, -1, -1):
                        if priority_choices[i] == PriorityType.PRODUCTION_COLONISATION:
                            del priority_choices[i]
                    continue
                elif colony_build_choices is None or len(colony_build_choices) == 0:
                    for i in range(len(priority_choices) - 1, -1, -1):
                        if priority_choices[i] == PriorityType.PRODUCTION_COLONISATION:
                            del priority_choices[i]
                    continue
                else:
                    making_colony_ship = True
            if this_priority == PriorityType.PRODUCTION_OUTPOST:
                if total_outpost_fleets >= max_outpost_fleets:
                    debug("Already sufficient outpost ships in queue, trying next priority choice\n")
                    for i in range(len(priority_choices) - 1, -1, -1):
                        if priority_choices[i] == PriorityType.PRODUCTION_OUTPOST:
                            del priority_choices[i]
                    continue
                else:
                    making_outpost_ship = True
            best_design_id, best_design, build_choices = best_ships[this_priority]
            if making_colony_ship:
                loc = random.choice(colony_build_choices)
                best_design_id, best_design, build_choices = get_best_ship_info(
                    PriorityType.PRODUCTION_COLONISATION, loc
                )
            elif this_priority == PriorityType.PRODUCTION_MILITARY:
                selector = random.random()
                choice = mil_build_choices[0]  # mil_build_choices can't be empty due to earlier check
                for choice in mil_build_choices:
                    if choice[0] >= selector:
                        break
                loc, best_design_id, best_design = choice[1:4]
                if best_design is None:
                    warning(
                        "problem with mil_build_choices;"
                        f" with selector ({selector}) chose loc ({loc}), "
                        f"best_design_id ({best_design_id}), best_design (None) "
                        f"from mil_build_choices: {mil_build_choices}"
                    )
                    continue
            else:
                loc = random.choice(build_choices)

            ship_number = 1
            per_turn_cost = float(best_design.productionCost(empire.empireID, loc)) / best_design.productionTime(
                empire.empireID, loc
            )
            if this_priority == PriorityType.PRODUCTION_MILITARY:
                this_rating = get_rating_for_planet(pid)
                rating_ratio = float(this_rating) / best_pilot_rating()
                if rating_ratio < 0.1:
                    loc_planet = universe.getPlanet(loc)
                    if loc_planet:
                        pname = loc_planet.name
                        this_rating = rate_planetary_piloting(loc)
                        rating_ratio = float(this_rating) / best_pilot_rating()
                        qualifier = "suboptimal " if rating_ratio < 1.0 else ""
                        debug(
                            "Building mil ship at loc %d (%s) with %spilot Rating: %.1f; ratio to empire best is %.1f",
                            loc,
                            pname,
                            qualifier,
                            this_rating,
                            rating_ratio,
                        )
                while total_pp > 40 * per_turn_cost:
                    ship_number *= 2
                    per_turn_cost *= 2
            retval = fo.issueEnqueueShipProductionOrder(best_design_id, loc)
            if retval != 0:
                prioritized = False
                debug(
                    "adding %d new ship(s) at location %s to production queue: %s; per turn production cost %.1f\n",
                    ship_number,
                    PlanetUtilsAI.planet_string(loc),
                    best_design.name,
                    per_turn_cost,
                )
                if ship_number > 1:
                    fo.issueChangeProductionQuantityOrder(production_queue.size - 1, 1, ship_number)
                avail_pp -= per_turn_cost
                if making_colony_ship:
                    total_colony_fleets += ship_number
                    if total_pp > 4 * per_turn_cost:
                        fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                    continue
                if making_outpost_ship:
                    total_outpost_fleets += ship_number
                    if total_pp > 4 * per_turn_cost:
                        fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                    continue
                if total_pp > 10 * per_turn_cost:
                    leading_block_pp = 0
                    for elem in [production_queue[elemi] for elemi in range(0, min(4, production_queue.size))]:
                        cost, time = empire.productionCostAndTime(elem)
                        leading_block_pp += elem.blocksize * cost / time
                    if leading_block_pp > 0.5 * total_pp or (
                        military_emergency and this_priority == PriorityType.PRODUCTION_MILITARY
                    ):
                        prioritized = True
                        fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                if this_priority == PriorityType.PRODUCTION_INVASION:
                    queued_troop_ships += ship_number
                    if not prioritized:
                        fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
        # The AI will normally only consider queuing additional ships (above) the current queue is not using all the
        # available PP; this can delay the AI from pursuing easy invasion targets.
        # Queue an extra troopship if the following conditions are met:
        #   i) currently dealing with our Capital ResourceGroup
        #   ii) the invasion priority for this group is nonzero and the max priority, and
        #   iii) there are minimal troopships already enqueued
        invasion_priority = local_priorities.get(PriorityType.PRODUCTION_INVASION, 0)
        if (
            capital_id in planet_set
            and invasion_priority
            and invasion_priority == max(local_priorities.values())
            and queued_troop_ships <= 2
        ):  # todo get max from character module or otherwise calculate
            best_design_id, best_design, build_choices = best_ships[PriorityType.PRODUCTION_INVASION]
            loc = random.choice(build_choices)
            retval = fo.issueEnqueueShipProductionOrder(best_design_id, loc)
            if retval != 0:
                per_turn_cost = float(best_design.productionCost(empire.empireID, loc)) / best_design.productionTime(
                    empire.empireID, loc
                )
                avail_pp -= per_turn_cost
                debug(
                    "adding extra trooper at location %s to production queue: %s; per turn production cost %.1f\n",
                    PlanetUtilsAI.planet_string(loc),
                    best_design.name,
                    per_turn_cost,
                )

        debug("")
    update_stockpile_use()
    fo.updateProductionQueue()
    print_production_queue(after_turn=True)


def _get_queued_buildings(pid: PlanetId) -> list[BuildingName]:
    debug("Buildings already in Production Queue:")
    capital_queued_buildings = _get_queued_buildings_for_planet(pid)
    for bldg in capital_queued_buildings:
        debug(f"    {bldg.name} turns: {bldg.turnsLeft} PP: {bldg.allocation}")
    if not capital_queued_buildings:
        debug("No capital queued buildings")
    queued_building_names = [bldg.name for bldg in capital_queued_buildings]
    return queued_building_names


def _is_queued_building_on_planet(e: "fo.productionQueueElement", pid: PlanetId) -> bool:
    return e.buildType == EmpireProductionTypes.BT_BUILDING and e.locationID == pid


def _get_queued_buildings_for_planet(pid: PlanetId) -> Sequence["fo.productionQueueElement"]:
    queue = fo.getEmpire().productionQueue
    return [e for e in queue if _is_queued_building_on_planet(e, pid)]


def update_stockpile_use():
    """Decide which elements in the production_queue will be enabled for drawing from the imperial stockpile.  This
    initial version simply ensures that every resource group with at least one item on the queue has its highest
    priority item be stockpile-enabled.

    :return: None
    """
    # TODO: Do a priority and risk evaluation to decide on enabling stockpile draws
    empire = fo.getEmpire()
    production_queue = empire.productionQueue
    resource_groups = {tuple(el.key()) for el in empire.planetsWithAvailablePP}
    planets_in_stockpile_enabled_group = set()
    for queue_index, element in enumerate(production_queue):
        if element.locationID in planets_in_stockpile_enabled_group:
            # TODO: evaluate possibly disabling stockpile for current element if was previously enabled, perhaps
            # only allowing multiple stockpile enabled items in empire-capital-resource-group, or considering some
            # priority analysis
            continue
        group = next((_g for _g in resource_groups if element.locationID in _g), None)
        if group is None:
            continue  # we don't appear to own the location any more
        if fo.issueAllowStockpileProductionOrder(queue_index, True):
            planets_in_stockpile_enabled_group.update(group)


def empire_has_colony_bld_species(building_name: str) -> bool:
    """
    Checks if this building is a colony building for which this empire has the required source species available.
    """
    if not building_name.startswith("BLD_COL_"):
        return False
    species_name = "SP_" + building_name.split("BLD_COL_")[1]
    return can_build_colony_for_species(species_name)


def already_has_completed_colony_building(planet_id) -> bool:
    """
    Checks if a planet has an already-completed (but not yet 'hatched') colony building.
    """
    universe = fo.getUniverse()
    planet = universe.getPlanet(planet_id)
    return any(universe.getBuilding(bldg).name.startswith("BLD_COL_") for bldg in planet.buildingIDs)


def _build_ship_facilities(building_type: Shipyard, top_pids: set[PlanetId] = frozenset()) -> None:
    # TODO: add total_pp checks below, so don't overload queue
    if not building_type.available():
        return
    universe = fo.getUniverse()
    total_pp = fo.getEmpire().productionPoints
    prerequisite_type = building_type.prerequisite()
    queued_bld_pids = building_type.queued_at()
    if building_type in Shipyard.get_system_ship_facilities():
        current_coverage = building_type.built_or_queued_at_sys()
        open_systems = {
            universe.getPlanet(pid).systemID for pid in get_best_pilot_facilities(Shipyard.BASE.value)
        }.difference(current_coverage)
        try_systems = open_systems & prerequisite_type.built_or_queued_at_sys() if prerequisite_type else open_systems
        try_pids = {pid for sys_id in try_systems for pid in get_owned_planets_in_system(sys_id)}
    else:
        current_pids = get_best_pilot_facilities(building_type.value)
        try_pids = get_best_pilot_facilities(prerequisite_type.value).difference(queued_bld_pids, current_pids)
    debug(
        "Considering constructing a %s, have %d already built and %d queued",
        building_type,
        len(building_type.built_at()),
        len(building_type.queued_at()),
    )
    if not try_pids:
        # Recursion cannot handle the base yard, but currently the AI builds it everywhere anyway.
        # Production of shipyards should be re-written, the AI builds too many of them.
        if prerequisite_type != Shipyard.BASE:
            debug(f"Cannot build {building_type} at top-pilot planets, try building {prerequisite_type} first.")
            _build_ship_facilities(prerequisite_type, top_pids)
        return
    # ship facilities all have location independent costs
    turn_cost = building_type.turn_cost(list(try_pids)[0])
    max_under_construction = max(1, int(total_pp) // (int(5 * turn_cost)))
    max_total = max(1, int(total_pp) // int(2 * turn_cost))
    debug("Allowances: max total: %d, max under construction: %d", max_total, max_under_construction)
    if len(building_type.built_at()) >= max_total:
        return
    try_top_pids = [pid for pid in try_pids & top_pids if building_type.can_be_produced(pid)]
    try_other_pids = [pid for pid in try_pids - top_pids if building_type.can_be_produced(pid)]
    valid_pids = try_top_pids + try_other_pids
    debug("Have %d potential locations: %s", len(valid_pids), [universe.getPlanet(x) for x in valid_pids])
    # TODO: rank by defense ability, etc.
    num_queued = len(queued_bld_pids)
    already_covered = []  # just those covered on this turn
    while valid_pids:
        if num_queued >= max_under_construction:
            break
        pid = valid_pids.pop()
        if pid in already_covered:
            continue
        res = building_type.enqueue(pid)
        debug("Enqueueing %s at planet %s , with result %d", building_type, universe.getPlanet(pid), res)
        if res:
            num_queued += 1
            already_covered.extend(get_owned_planets_in_system(universe.getPlanet(pid).systemID))


def find_automatic_historic_analyzer_candidates() -> list[int]:
    """
    Find possible locations for the BLD_AUTO_HISTORY_ANALYSER building and return a subset of chosen building locations.

    :return: Random possible locations up to max queueable amount. Empty if no location found or can't queue another one
    """
    empire = fo.getEmpire()
    universe = fo.getUniverse()
    total_pp = empire.productionPoints
    history_analyser = "BLD_AUTO_HISTORY_ANALYSER"
    culture_archives = "BLD_CULTURE_ARCHIVES"

    ARB_LARGE_NUMBER = 1e4
    conditions = {
        # aggression: (min_pp, min_turn, min_pp_to_queue_another_one)
        fo.aggression.beginner: (100, 100, ARB_LARGE_NUMBER),
        fo.aggression.turtle: (75, 75, ARB_LARGE_NUMBER),
        fo.aggression.cautious: (40, 40, ARB_LARGE_NUMBER),
        fo.aggression.typical: (20, 20, 50),
        fo.aggression.aggressive: (10, 10, 40),
        fo.aggression.maniacal: (8, 5, 30),
    }

    min_pp, turn_trigger, min_pp_per_additional = conditions.get(
        get_aistate().character.get_trait(Aggression).key, (ARB_LARGE_NUMBER, ARB_LARGE_NUMBER, ARB_LARGE_NUMBER)
    )
    max_enqueued = 1 if total_pp > min_pp or fo.currentTurn() > turn_trigger else 0
    max_enqueued += int(total_pp / min_pp_per_additional)

    if max_enqueued <= 0:
        return []

    # find possible locations
    possible_locations = set()
    for pid in get_all_empire_planets():
        planet = universe.getPlanet(pid)
        if not planet or planet.currentMeterValue(fo.meterType.targetPopulation) < 1:
            continue
        buildings_here = [bld.buildingTypeName for bld in map(universe.getBuilding, planet.buildingIDs)]
        if planet and culture_archives in buildings_here and history_analyser not in buildings_here:
            possible_locations.add(pid)

    # check existing queued buildings and remove from possible locations
    queued_locs = {
        e.locationID
        for e in empire.productionQueue
        if e.buildType == EmpireProductionTypes.BT_BUILDING and e.name == history_analyser
    }

    possible_locations -= queued_locs
    chosen_locations = []
    for i in range(min(max_enqueued, len(possible_locations))):
        chosen_locations.append(possible_locations.pop())
    return chosen_locations


def _location_rating(planet: fo.planet) -> float:
    """Get a rating how good this planet would be for a building"""
    # Simple value so far, should be enhanced, e.g. a scanner should go to the planet with species that
    # has the best basic visions
    return planet.currentMeterValue(fo.meterType.maxTroops)


def _try_enqueue(
    building_type: BuildingTypeBase,
    candidates: Union[PlanetId, Iterable[PlanetId]],
    *,
    at_front: bool = False,
    ignore_dislike: bool = False,
) -> float:
    """
    Enqueue building at one of the planets in candidates.
    If at_front, building is added at the front of the queue.
    Returns PP per turn spent on the new building or 0.0 if nothing was enqueued.
    """
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    opinion = building_type.get_opinions()
    locations = []
    preferred_locations = []
    if isinstance(candidates, int):  # isinstance(candidates, PlanetId) does not work, at least not in python-3.9
        candidates = [candidates]
    for pid in candidates:
        planet = universe.getPlanet(pid)
        if not planet:
            error(f"Got pid {pid} in candidate, which does not seem to be a planetID")
            continue
        if not ignore_dislike and pid in opinion.dislikes:
            continue
        if not building_type.can_be_produced(pid) or not building_type.can_be_enqueued(pid):
            continue
        if pid in opinion.likes:
            preferred_locations.append((_location_rating(planet), pid))
        else:
            locations.append((_location_rating(planet), pid))
    for _, pid in sorted(preferred_locations, reverse=True) + sorted(locations, reverse=True):
        planet = universe.getPlanet(pid)
        res = building_type.enqueue(pid)
        debug("Enqueueing %s at planet %d (%s) , with result %d", building_type, pid, planet.name, res)
        if res:
            if at_front:
                res = fo.issueRequeueProductionOrder(empire.productionQueue.size - 1, 0)  # move to front
                debug("Requeueing %s to front of build queue, with result %d", building_type, res)
            return building_type.turn_cost(pid)
    return 0.0


def _may_enqueue_for_stability(building_type: BuildingTypeBase, new_turn_cost: float) -> float:
    """
    Build building if it seems worth doing so to increase stability.
    Only builds of locations.planets_enqueued is empty and new_turn_cost is 0.0,
    i.e. there are currently no build queue entries for the given building.
    returns new_turn_cost or turn_cost of the building enqueued by this function.
    """
    if building_type.queued_at() or new_turn_cost:
        return new_turn_cost
    # this can be improved a lot, taking into account value of planets, actual stability and
    # what effects the change would have. For the moment, keep it simple.
    # Note that the strongest effect is always on the building's planet itself.
    opinion = building_type.get_opinions()
    universe = fo.getUniverse()
    if len(opinion.likes) >= len(opinion.dislikes) * PlanetUtilsAI.dislike_factor():
        like_candidates = opinion.likes - building_type.built_or_queued_at()
        # plans may change, so consider only actual colonies that like it
        candidates = [pid for pid in like_candidates if universe.getPlanet(pid).speciesName]
        return _try_enqueue(building_type, candidates)
    return 0.0


def _build_scanning_facility() -> float:
    """Consider building Scanning Facilities, return added turn costs."""
    building_type = BuildingType.SCANNING_FACILITY
    empire = fo.getEmpire()
    if not building_type.available():
        return 0.0

    universe = fo.getUniverse()
    turn_cost = 0.0
    opinion = building_type.get_opinions()
    # TBD use actual cost?
    max_scanner_builds = max(1, int(empire.productionPoints / 30)) - len(building_type.queued_at())
    scanner_systems = building_type.built_or_queued_at_sys()
    debug(
        "Considering building %s, found current and queued systems %s, planets that like it %s, #dislikes: %d",
        building_type,
        PlanetUtilsAI.sys_name_ids(scanner_systems),
        PlanetUtilsAI.sys_name_ids(opinion.likes),
        len(opinion.dislikes),
    )
    for sys_id in get_owned_planets():
        if max_scanner_builds <= 0:
            break
        if sys_id in scanner_systems:
            continue
        need_scanner = False
        for neighbor in get_neighbors(sys_id):
            if universe.getVisibility(neighbor, empire.empireID) < fo.visibility.partial:
                need_scanner = True
                break
        if not need_scanner:
            continue
        # TBD: chose based on detection range
        cost = _try_enqueue(building_type, get_owned_planets_in_system(sys_id), at_front=True)
        if cost:
            max_scanner_builds -= 1
        turn_cost += cost
    return _may_enqueue_for_stability(building_type, turn_cost)


def _build_gas_giant_generator() -> float:  # noqa: C901
    """Consider building Gas Giant Generators, return added turn costs."""
    building_type = BuildingType.GAS_GIANT_GEN
    if not building_type.available():
        return 0.0

    ggg_min_stability = get_named_real("BLD_GAS_GIANT_GEN_MIN_STABILITY")
    universe = fo.getUniverse()
    colonized_planets = get_colonized_planets()
    opinion = building_type.get_opinions()
    systems = []
    for sys in colonized_planets.keys():
        planets = [(pid, universe.getPlanet(pid)) for pid in get_owned_planets_in_system(sys)]
        if sys in building_type.built_or_queued_at_sys() or fo.planetSize.gasGiant not in [x[1].size for x in planets]:
            continue
        rating = 0
        gas_giant = None
        best_gg = -2
        debug(f"Gas Giant Generator rating for {universe.getSystem(sys).name} ...")
        for pid, planet in planets:
            likes = opinion.value(pid, 1, 0, -1 * PlanetUtilsAI.dislike_factor())
            debug(f"  {planet.name} likes {likes}")
            # TBD -4 if build here...
            stability = planet.currentMeterValue(fo.meterType.targetHappiness) + likes
            rating += 3 * likes
            debug(f"  rating now {rating} from likes {likes} ")
            if planet.size == fo.planetSize.gasGiant:
                val = likes
                if val > best_gg:
                    best_gg = val
                    gas_giant = pid
            if planet.focus == FocusType.FOCUS_INDUSTRY and stability >= ggg_min_stability:
                rating += 20 + min(5, stability - ggg_min_stability)
                debug(f"  rating now {rating} from industry planet stability {stability} ")
            elif FocusType.FOCUS_INDUSTRY in planet.availableFoci and stability >= ggg_min_stability:
                rating += 5 + min(5, stability - ggg_min_stability)
                debug(f"  rating now {rating} from pot. industry planet stability {stability} ")
        if gas_giant:
            # if the inhabitants do not like it, this will require two other planets that profit from it
            rating += 15 * best_gg
            debug(f"  from best_gg {best_gg}, final rating: {rating}")
            systems.append((rating, gas_giant))
    # sorting so that highest ratings come last, which means they end up at the front of the queue
    systems.sort()
    turn_cost = 0.0
    for rating, gas_giant in systems:
        # 20 = one industry planet with exactly ggg_min_stability
        if rating >= 20:
            turn_cost += _try_enqueue(building_type, gas_giant, at_front=True, ignore_dislike=True)
    return _may_enqueue_for_stability(building_type, turn_cost)


def _build_translator():
    """Consider building Near Universal Translators, return added turn costs."""
    building_type = BuildingType.TRANSLATOR
    if building_type.available() and translators_wanted() and candidate_for_translator:
        # starting one per turn should be enough
        have_one = bool(building_type.built_or_queued_at())
        return _try_enqueue(building_type, candidate_for_translator, at_front=not have_one)
    return 0.0
    # may_enqueue_for_stability? Building is rather expensive...


def _build_regional_administration() -> float:
    """Consider building Imperial Regional Administrations, return added turn costs."""
    building_type = BuildingType.REGIONAL_ADMIN
    current_admin_systems = building_type.built_or_queued_at_sys() | BuildingType.PALACE.built_or_queued_at_sys()
    # No administrations at all means no palace. If we cannot even find a place for the palace,
    # there is no point in building regional administrations.
    if not building_type.available() or not current_admin_systems:
        return 0.0
    universe = fo.getUniverse()
    jumps_to_admin = [
        (min(universe.jumpDistance(sys_id, admin) for admin in current_admin_systems), sys_id)
        for sys_id in get_owned_planets()
    ]
    jumps_to_admin.sort(reverse=True)
    # Currently, 6 is required, this is supposed to change. Note that the game allows to enqueue several ones
    # close to each other, but only one will get finished.
    if jumps_to_admin[0][0] < 6:
        return 0.0
    debug(f"current_admin_systems: {current_admin_systems}, jumps_to_admin = {jumps_to_admin}")
    # with minimum distance 6, systems 3 jumps away from current admins cannot get closer
    systems_that_may_profit = [value for value in jumps_to_admin if value[0] > 3]
    best_sys_id = None
    # without a threshold the AI would build at the first planet 6 steps away from others,
    # although it may not give much and is possible quite exposed.
    best_rating = 10.0
    for distance, candidate in jumps_to_admin:
        if distance < 6:
            break
        rating = _rate_system_for_admin(candidate, systems_that_may_profit)
        if rating > best_rating:
            best_sys_id = candidate
            best_rating = rating
    # To add more than one, we'd have to recalculate everything, but one per turn is good enough.
    # In practice more than one would hardly ever be possible anyway.
    debug(f"best_sys_id={best_sys_id}, best_rating={best_rating}, planets: {get_owned_planets_in_system(best_sys_id)}")
    if not best_sys_id:
        return 0.0
    return _try_enqueue(building_type, get_owned_planets_in_system(best_sys_id))


def _rate_system_for_admin(sys_id: SystemId, systems_that_may_profit: list[tuple[int, SystemId]]) -> float:
    opinion = BuildingType.REGIONAL_ADMIN.get_opinions()
    planets = set(get_owned_planets_in_system(sys_id))
    dislikes = planets & opinion.dislikes
    likes = planets & opinion.likes
    if dislikes == planets:
        return 0.0
    # First like gets a big bonus, but we can build it only on one.
    # Number of planets to prefer better defended systems.
    rating = 1.5 * (len(likes) - len(dislikes) * PlanetUtilsAI.dislike_factor()) + 3 * (likes != set()) + len(planets)

    universe = fo.getUniverse()
    for current_distance, other_sys_id in systems_that_may_profit:
        difference = current_distance - universe.jumpDistance(sys_id, other_sys_id)
        if difference > 0:
            for pid in get_colonized_planets_in_system(other_sys_id):
                planet = universe.getPlanet(pid)
                if Tags.INDEPENDENT not in fo.getSpecies(planet.speciesName).tags:
                    stability = planet.currentMeterValue(fo.meterType.targetHappiness)
                    rating += difference
                    # reaching 10 gives a lot of bonuses
                    if stability < 0 or stability < 10 <= stability + difference:
                        rating += 2
    debug(f"admin rating {universe.getSystem(sys_id)}={rating}")
    return rating


def _build_military_command() -> float:
    """
    Consider building a Military Command, return added turn costs.
    Since its major purpose is to provide policy slots, and we may need the production for more important
    things, do not build it too early. Won't build it at all, if all our planets dislike it.
    """
    building_type = BuildingType.MILITARY_COMMAND
    palace_planet = BuildingType.PALACE.built_at()
    # an empire can only build one, and if we do not have a palace, this is definitely more important
    if building_type.built_or_queued_at() or not palace_planet:
        return 0.0
    # cost is independent of the location, but we need a valid location
    if fo.getEmpire().productionPoints > building_type.turn_cost(list(palace_planet)[0]) * 1.5:
        # default selection should prefer the capital, unless its species dislikes it.
        return _try_enqueue(building_type, get_inhabited_planets())
    return 0.0


TopPilotSystems = NewType("TopPilotSystems", dict[SystemId, list[tuple[PlanetId, float]]])


class ShipYardInfo(NamedTuple):
    queued_shipyard_pids: list[PlanetId]
    colony_systems: dict[PlanetId, SystemId]
    top_pilot_systems: TopPilotSystems


def _build_basic_shipyards() -> ShipYardInfo:  # noqa: C901
    """
    Consider building basic ship yards and also determine some value needed for other shipyard buildings.
    """
    building_type = Shipyard.BASE
    universe = fo.getUniverse()
    queued_shipyard_pids = building_type.queued_at()
    system_colonies = {}
    colony_systems = {}
    empire_species = get_empire_planets_by_species()
    for spec_name in get_colony_builders():
        if not get_colony_builder_locations(spec_name) and (
            spec_name in empire_species
        ):  # not enough current shipyards for this species #TODO: also allow orbital incubators and/or asteroid ships
            for pid in get_empire_planets_with_species(
                spec_name
            ):  # SP_EXOBOT may not actually have a colony yet but be in empireColonizers
                if pid in queued_shipyard_pids:
                    break  # won't try building more than one shipyard at once, per colonizer
            else:
                # no queued shipyards: get planets with target pop >=3 and
                # queue a shipyard on the one with the biggest current population
                planets = (universe.getPlanet(x) for x in get_empire_planets_with_species(spec_name))
                pops = sorted(
                    (planet_.initialMeterValue(fo.meterType.population), planet_.id)
                    for planet_ in planets
                    if (planet_ and planet_.initialMeterValue(fo.meterType.targetPopulation) >= 3.0)
                )
                pids = [pid for pop, pid in pops if building_type.can_be_produced(pid)]
                if pids:
                    build_loc = pids[-1]
                    res = _try_enqueue(building_type, build_loc)  # do not ignore dislikes here
                    if res > 0:
                        queued_shipyard_pids.append(build_loc)
                        break  # only start at most one new shipyard per species per turn
        for pid in get_empire_planets_with_species(spec_name):
            planet = universe.getPlanet(pid)
            if planet:
                system_colonies.setdefault(planet.systemID, {}).setdefault("pids", []).append(pid)
                colony_systems[pid] = planet.systemID

    for pid in get_empire_planets_with_species("SP_ACIREMA"):
        if (pid in queued_shipyard_pids) or not building_type.can_be_produced(pid):
            continue  # but not 'break' because we want to build shipyards at *every* Acirema planet
        # currently Acirema do not dislike ship yards, but if that changes, do not build shipyards anymore
        res = _try_enqueue(building_type, pid, at_front=True)
        if res > 0:
            queued_shipyard_pids.append(pid)

    top_pilot_systems = TopPilotSystems({})
    for pid, rating in get_pilot_ratings().items():
        if (rating <= medium_pilot_rating()) and (rating < GREAT_PILOT_RATING):
            continue
        top_pilot_systems.setdefault(universe.getPlanet(pid).systemID, []).append((pid, rating))
        if (pid in queued_shipyard_pids) or not building_type.can_be_produced(pid):
            continue  # but not 'break' because we want to build shipyards all top pilot planets
        # so far we ignore dislikes here, but this may have to change for Mu Ursh
        res = _try_enqueue(building_type, pid, at_front=True, ignore_dislike=True)
        if res:
            queued_shipyard_pids.append(pid)
    return ShipYardInfo(queued_shipyard_pids, colony_systems, top_pilot_systems)  # TBD return added costs?


def _build_energy_shipyards(  # noqa: C901
    queued_shipyard_pids: list[PlanetId],
    colony_systems: dict[PlanetId, SystemId],
    building_ratio: float,
    building_expense: float,
) -> tuple[list[tuple[float, PlanetId]], list[tuple[float, PlanetId]], float]:
    """
    Consider building Energy Compressor and Solar Containment Unit.
    Also determines pilot rating for planets in system with red stars and black holes.
    Returns blackhole_pilots, red_pilots and new value of building_expense.
    """
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    pop_ctrs = list(get_inhabited_planets())
    red_population_centres = sorted(
        [
            (get_rating_for_planet(pid), pid)
            for pid in pop_ctrs
            if colony_systems.get(pid, INVALID_ID) in AIstate.empireStars.get(fo.starType.red, [])
        ],
        reverse=True,
    )
    red_pilots = [pid for _, pid in red_population_centres if _ == best_pilot_rating()]
    blue_population_centres = sorted(
        [
            (get_rating_for_planet(pid), pid)
            for pid in pop_ctrs
            if colony_systems.get(pid, INVALID_ID) in AIstate.empireStars.get(fo.starType.blue, [])
        ],
        reverse=True,
    )
    blue_pilots = [pid for _, pid in blue_population_centres if _ == best_pilot_rating()]
    blackhole_pilots = sorted(
        [
            (get_rating_for_planet(pid), pid)
            for pid in pop_ctrs
            if colony_systems.get(pid, INVALID_ID) in AIstate.empireStars.get(fo.starType.blackHole, [])
        ],
        reverse=True,
    )
    blackhole_pilots = [pid for _, pid in blackhole_pilots if _ == best_pilot_rating()]
    energy_shipyard_pids = {}
    building_type = Shipyard.ENRG_COMP
    if building_type.available():
        queued_building_pids = building_type.queued_at()
        for pid in blackhole_pilots + blue_pilots:
            if len(queued_building_pids) > 1:  # build a max of 2 at once
                break
            this_planet = universe.getPlanet(pid)
            if not (
                this_planet and can_build_ship_for_species(this_planet.speciesName)
            ):  # TODO: also check that not already one for this spec in this sys
                continue
            energy_shipyard_pids.setdefault(this_planet.systemID, []).append(pid)
            if pid not in queued_building_pids and building_type.can_be_produced(pid):
                building_expense += _try_enqueue(building_type, pid, at_front=True)

    building_type = Shipyard.ENRG_SOLAR
    if building_type.available() and not building_type.queued_at():
        # TODO: check that production is not frozen at a queued location
        for pid in blackhole_pilots:
            this_planet = universe.getPlanet(pid)
            if not (
                this_planet and can_build_ship_for_species(this_planet.speciesName)
            ):  # TODO: also check that not already one for this spec in this sys
                continue
            if building_type.can_be_produced(pid):
                building_expense += _try_enqueue(building_type, pid, at_front=True)

    total_pp = empire.productionPoints
    building_type = Shipyard.BASE
    if building_type.available() and (building_expense < building_ratio * total_pp) and (total_pp > 50):
        for sys_id in energy_shipyard_pids:  # Todo ensure only one or 2 per sys
            # only start one per turn (TBD why [:2]?)
            for pid in energy_shipyard_pids[sys_id][:2]:
                res = _try_enqueue(building_type, pid, at_front=True)
                if res > 0:
                    queued_shipyard_pids.append(pid)
                    break  # only start one per turn
    return blackhole_pilots, red_pilots, building_expense


def _build_asteroid_processor(  # noqa: C901
    top_pilot_systems: TopPilotSystems, queued_shipyard_pids: list[PlanetId]
) -> float:
    """Consider building asteroid processor, return added turn costs."""
    building_type = Shipyard.ASTEROID
    building_expense = 0.0
    if building_type.available():
        universe = fo.getUniverse()
        queued_building_pids = building_type.queued_at()
        if not queued_building_pids:
            asteroid_systems = {}
            asteroid_yards = {}
            builder_systems = {}
            for pid in get_all_empire_planets():
                planet = universe.getPlanet(pid)
                this_spec = planet.speciesName
                sys_id = planet.systemID
                if planet.size == fo.planetSize.asteroids and sys_id in get_colonized_planets():
                    asteroid_systems.setdefault(sys_id, []).append(pid)
                    if pid in building_type.built_or_queued_at():
                        asteroid_yards[sys_id] = pid  # shouldn't ever overwrite another, but ok if it did
                if can_build_ship_for_species(this_spec):
                    if pid not in get_ship_builder_locations(this_spec):
                        builder_systems.setdefault(sys_id, []).append((planet.speciesName, pid))
            # check if we need to build another asteroid processor:
            # check if local shipyard to go with the asteroid processor
            yard_systems = []
            need_yard = {}
            top_pilot_locations = []
            for sys_id in set(asteroid_systems.keys()).difference(asteroid_yards.keys()):
                if sys_id in top_pilot_systems:
                    for pid, rating in top_pilot_systems[sys_id]:
                        if pid not in queued_shipyard_pids:  # will catch it later if shipyard already present
                            top_pilot_locations.append((rating, pid, sys_id))
            top_pilot_locations.sort(reverse=True)
            for _, _, sys_id in top_pilot_locations:
                if sys_id not in yard_systems:
                    yard_systems.append(sys_id)  # prioritize asteroid yards for acirema and/or other top pilots
                    for pid, _ in top_pilot_systems[sys_id]:
                        if pid not in queued_shipyard_pids:  # will catch it later if shipyard already present
                            need_yard[sys_id] = pid
            if (not yard_systems) and len(asteroid_yards.values()) <= int(
                fo.currentTurn() // 50
            ):  # not yet building & not enough current locs, find a location to build one
                colonizer_loc_choices = []
                builder_loc_choices = []
                bld_systems = set(asteroid_systems.keys()).difference(asteroid_yards.keys())
                for sys_id in bld_systems.intersection(builder_systems.keys()):
                    for this_spec, pid in builder_systems[sys_id]:
                        if can_build_colony_for_species(this_spec):
                            if pid in (get_colony_builder_locations(this_spec) + queued_shipyard_pids):
                                colonizer_loc_choices.insert(0, sys_id)
                            else:
                                colonizer_loc_choices.append(sys_id)
                                need_yard[sys_id] = pid
                        else:
                            if pid in (get_ship_builder_locations(this_spec) + queued_shipyard_pids):
                                builder_loc_choices.insert(0, sys_id)
                            else:
                                builder_loc_choices.append(sys_id)
                                need_yard[sys_id] = pid
                yard_systems.extend(
                    (colonizer_loc_choices + builder_loc_choices)[:1]
                )  # add at most one of these non top pilot locs
            new_yard_count = len(queued_building_pids)
            for sys_id in yard_systems:  # build at most 2 new asteroid yards at a time
                if new_yard_count >= 2:
                    break
                pid = asteroid_systems[sys_id][0]
                if sys_id in need_yard:
                    pid2 = need_yard[sys_id]
                    res = _try_enqueue(Shipyard.BASE, pid2, at_front=True)
                    if res > 0:
                        queued_shipyard_pids.append(pid2)
                        building_expense += res
                if pid not in queued_building_pids and building_type.can_be_produced(pid):
                    res = _try_enqueue(building_type, pid, at_front=True)
                    if res > 0:
                        new_yard_count += 1
                        queued_building_pids.append(pid)
                        building_expense += res
    return building_expense


def _build_orbital_drydock(top_pilot_systems: TopPilotSystems) -> None:  # noqa: C901
    """Consider building orbital drydocks."""
    building_type = Shipyard.ORBITAL_DRYDOCK
    if building_type.available():
        empire = fo.getEmpire()
        universe = fo.getUniverse()
        queued_pids = building_type.queued_at()
        current_drydock_sys = building_type.built_or_queued_at_sys()
        covered_drydock_systems = set()
        for start_set, dest_set in [
            (current_drydock_sys, covered_drydock_systems),
            (covered_drydock_systems, covered_drydock_systems),
        ]:  # coverage of neighbors up to 2 jumps away from a drydock
            for dd_sys_id in start_set.copy():
                dest_set.add(dd_sys_id)
                dest_set.update(get_neighbors(dd_sys_id))

        max_dock_builds = int(0.8 + empire.productionPoints / 120.0)
        debug(
            "Considering building %s, found current and queued systems %s",
            building_type,
            PlanetUtilsAI.sys_name_ids(current_drydock_sys),
        )
        for sys_id, pids in get_colonized_planets().items():  # TODO: sort/prioritize in some fashion
            local_top_pilots = dict(top_pilot_systems.get(sys_id, []))
            local_drydocks = get_empire_drydocks().get(sys_id, [])
            if len(queued_pids) >= max_dock_builds:
                debug("Drydock enqueing halted with %d of max %d", len(queued_pids), max_dock_builds)
                break
            if (sys_id in covered_drydock_systems) and not local_top_pilots:
                continue
            else:
                pass
            for _, pid in sorted([(local_top_pilots.get(pid, 0), pid) for pid in pids], reverse=True):
                if has_shipyard(pid):
                    continue
                if pid in local_drydocks or pid in queued_pids:
                    break
                if not building_type.can_be_enqueued(pid):
                    continue
                res = _try_enqueue(building_type, pid, at_front=(max_dock_builds >= 2))
                if res > 0:
                    queued_pids.append(pid)
                    system_id = universe.getPlanet(pid).systemID
                    covered_drydock_systems.add(system_id)
                    covered_drydock_systems.update(get_neighbors(system_id))


def _remove_other_colonies(pid: PlanetId, building_name: str) -> None:
    """
    Removes enqueued colony buildings at the given planet.
    Since colonies cannot be queued in parallel, to allow enqueuing building_name, all others must be removed.
    If building_name is already enqueued, it's fine of course.
    """
    numbered_queue = list(enumerate(fo.getEmpire().productionQueue))
    # It should not be more than one, except possibly when loading an old safe file, just to be sure, remove all.
    # Start at the end to avoid changing the numbers of further elements when removing one.
    for num, entry in reversed(numbered_queue):
        if entry.locationID == pid and entry.name.startswith("BLD_COL_") and entry.name != building_name:
            fo.issueDequeueProductionOrder(num)
