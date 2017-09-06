import sys

import freeOrionAIInterface as fo  # pylint: disable=import-error
import FreeOrionAI as foAI
import AIstate
import universe_object
import fleet_orders
import ColonisationAI
import FleetUtilsAI
import PlanetUtilsAI
from freeorion_tools import ppstring
from AIDependencies import INVALID_ID, DRYDOCK_HAPPINESS_THRESHOLD

from common.configure_logging import convenience_function_references_for_logger
(debug, info, warn, error, fatal) = convenience_function_references_for_logger(__name__)

def create_move_orders_to_system(fleet, target):
    """
    Create a list of move orders from the fleet's current system to the target system.

    :param fleet: Fleet to be moved
    :type fleet: universe_object.Fleet
    :param target: target system
    :type target: universe_object.System
    :return: list of move orders
    :rtype: list[fleet_orders.OrdersMove]
    """
    # TODO: use Graph Theory to construct move orders
    # TODO: add priority
    starting_system = fleet.get_system()  # current fleet location or current target system if on starlane
    # if the mission does not end at the targeted system, make sure we can actually return to supply after moving.
    ensure_return = target.id not in set(AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs
                                         + AIstate.invasionTargetedSystemIDs + AIstate.blockadeTargetedSystemIDs)
    system_targets = can_travel_to_system(fleet.id, starting_system, target, ensure_return=ensure_return)
    result = [fleet_orders.OrderMove(fleet, system) for system in system_targets]
    if not result and starting_system.id != target.id:
        print >> sys.stderr, "fleet %s can't travel to system %s" % (fleet.id, target)
    return result


def can_travel_to_system(fleet_id, from_system_target, to_system_target, ensure_return=False):
    """
    Return list systems to be visited.

    :param fleet_id:
    :type fleet_id: int
    :param from_system_target:
    :type from_system_target: universe_object.System
    :param to_system_target:
    :type to_system_target:  universe_object.System
    :param ensure_return:
    :type ensure_return: bool
    :return:
    :rtype: list
    """
    empire = fo.getEmpire()
    empire_id = empire.empireID
    fleet_supplyable_system_ids = set(empire.fleetSupplyableSystemIDs)
    # get current fuel and max fuel
    universe = fo.getUniverse()
    fuel = int(FleetUtilsAI.get_fuel(fleet_id))  # round down to get actually number of jumps
    if fuel < 1.0 or from_system_target.id == to_system_target.id:
        return []
    if True:  # TODO: sort out if shortestPath leaves off some intermediate destinations
        path_func = universe.leastJumpsPath
    else:
        path_func = universe.shortestPath
    start_sys_id = from_system_target.id
    target_sys_id = to_system_target.id
    if start_sys_id != INVALID_ID and target_sys_id != INVALID_ID:
        short_path = list(path_func(start_sys_id, target_sys_id, empire_id))
    else:
        short_path = []
    legs = zip(short_path[:-1], short_path[1:])
    # suppliedStops = [ sid for sid in short_path if sid in fleet_supplyable_system_ids ]
    # unsupplied_stops = [sid for sid in short_path if sid not in suppliedStops ]
    unsupplied_stops = [sys_b for sys_a, sys_b in legs if ((sys_a not in fleet_supplyable_system_ids) and (sys_b not in fleet_supplyable_system_ids))]
    # print "getting path from %s to %s "%(ppstring(PlanetUtilsAI.sys_name_ids([ start_sys_id ])), ppstring(PlanetUtilsAI.sys_name_ids([ target_sys_id ])) ),
    # print " ::: found initial path %s having suppliedStops %s and unsupplied_stops %s ; tot fuel available is %.1f"%( ppstring(PlanetUtilsAI.sys_name_ids( short_path[:])), suppliedStops, unsupplied_stops, fuel)
    if False:
        if target_sys_id in fleet_supplyable_system_ids:
            print "target has FleetSupply"
        elif target_sys_id in ColonisationAI.annexable_ring1:
            print "target in Ring 1"
        elif target_sys_id in ColonisationAI.annexable_ring2 and foAI.foAIstate.character.may_travel_beyond_supply(2):
            print "target in Ring 2, has enough aggression"
        elif target_sys_id in ColonisationAI.annexable_ring3 and foAI.foAIstate.character.may_travel_beyond_supply(3):
            print "target in Ring 2, has enough aggression"

    def has_enough_fuel(distance_from_supply):
        # Let f be the ships fuel
        # Let n be the number of unsupplied stops
        # Let d be the distance from target system to supply
        #
        # After reaching our target, we have f_new = f_current - n
        # We then need another d fuel to resupply,
        # i.e. f_new >= d or f_current >= (n + d)
        return fuel > (len(unsupplied_stops) + distance_from_supply)

    def movement_allowed(x):
        return foAI.foAIstate.character.may_travel_beyond_supply(x) and has_enough_fuel(x)

    # TODO: use any-iterator to simplify conditions
    if (not unsupplied_stops or not ensure_return
            or target_sys_id in fleet_supplyable_system_ids and movement_allowed(0)
            or target_sys_id in ColonisationAI.annexable_ring1 and movement_allowed(1)
            or target_sys_id in ColonisationAI.annexable_ring2 and movement_allowed(2)
            or target_sys_id in ColonisationAI.annexable_ring3 and movement_allowed(3)):
        return [universe_object.System(sid) for sid in short_path]
    else:
        # print " getting path from 'can_travel_to_system_and_return_to_resupply' ",
        return can_travel_to_system_and_return_to_resupply(fleet_id, from_system_target, to_system_target)


def can_travel_to_system_and_return_to_resupply(fleet_id, from_system_target, to_system_target):
    """
    Filter systems where fleet can travel from starting system. # TODO rename function

    :param fleet_id:
    :type fleet_id: int
    :param from_system_target:
    :type from_system_target: universe_object.System
    :param to_system_target:
    :type to_system_target: universe_object.System
    :return:
    :rtype: list
    """
    system_targets = []
    if not from_system_target.id == to_system_target.id:
        fleet_supplyable_system_ids = fo.getEmpire().fleetSupplyableSystemIDs
        fuel = int(FleetUtilsAI.get_fuel(fleet_id))  # int to get actual number of jumps
        max_fuel = int(FleetUtilsAI.get_max_fuel(fleet_id))
        # try to find path without going resupply first
        supply_system_target = get_nearest_supplied_system(to_system_target.id)
        system_targets = __find_path_with_fuel_to_system_with_possible_return(from_system_target, to_system_target, system_targets, fleet_supplyable_system_ids, max_fuel, fuel, supply_system_target)
        # resupply in system first is required to find path
        if from_system_target.id not in fleet_supplyable_system_ids and not system_targets:
            # add supply system to visit
            from_system_target = get_nearest_supplied_system(from_system_target.id)
            system_targets.append(from_system_target)
            # find path from supplied system to wanted system
            system_targets = __find_path_with_fuel_to_system_with_possible_return(from_system_target, to_system_target, system_targets, fleet_supplyable_system_ids, max_fuel, max_fuel, supply_system_target)
    return system_targets


def get_nearest_supplied_system(start_system_id):
    """ Return systemAITarget of nearest supplied system from starting system startSystemID."""
    empire = fo.getEmpire()
    fleet_supplyable_system_ids = empire.fleetSupplyableSystemIDs
    universe = fo.getUniverse()

    if start_system_id in fleet_supplyable_system_ids:
        return universe_object.System(start_system_id)
    else:
        min_jumps = 9999  # infinity
        supply_system_id = INVALID_ID
        for system_id in fleet_supplyable_system_ids:
            if start_system_id != INVALID_ID and system_id != INVALID_ID:
                least_jumps_len = universe.jumpDistance(start_system_id, system_id)
                if least_jumps_len < min_jumps:
                    min_jumps = least_jumps_len
                    supply_system_id = system_id
        return universe_object.System(supply_system_id)


def get_best_drydock_system_id(start_system_id, fleet_id):
    """

    Get system_id of best drydock capable of repair, where best is nearest drydock
    that has a current and target happiness greater than the HAPPINESS _THRESHOLD
    with a path that is not blockaded or that the fleet can fight through to with
    acceptable losses.

    :param start_system_id: current location of fleet - used to find closest target
    :type start_system_id: int
    :param fleet_id: fleet that need path to drydock
    :type: int
    :return: closest system_id capable of repairing
    :rtype: int
    """
    if start_system_id == INVALID_ID:
        print >> sys.stderr, "get_best_drydock_system_id passed bad system id."
        return None

    if fleet_id == INVALID_ID:
        print >> sys.stderr, "get_best_drydock_system_id passed bad fleet id."
        return None

    universe = fo.getUniverse()
    start_sys = universe.getSystem(start_system_id)
    drydock_system_ids = set()
    for sys_id, pids in ColonisationAI.empire_dry_docks.iteritems():
        if sys_id == INVALID_ID:
            print >> sys.stderr, "get_best_drydock_system_id passed bad dry dock sys_id."
            continue
        for pid in pids:
            planet = universe.getPlanet(pid)
            if (planet
                  and planet.currentMeterValue(fo.meterType.happiness) >= DRYDOCK_HAPPINESS_THRESHOLD
                  and planet.currentMeterValue(fo.meterType.targetHappiness) >= DRYDOCK_HAPPINESS_THRESHOLD):
                drydock_system_ids.add(sys_id)
                break

    sys_distances = sorted([(universe.jumpDistance(start_system_id, sys_id), sys_id)
                            for sys_id in drydock_system_ids])

    fleet_rating = foAI.foAIstate.get_rating(fleet_id)
    for dock_sys in [universe.getSystem(sys_id) for (_, sys_id) in sys_distances]:
        path = can_travel_to_system(fleet_id, start_sys, dock_sys)

        path_rating = sum([foAI.foAIstate.systemStatus[path_sys.id]['totalThreat']
                           for path_sys in path])

        SAFETY_MARGIN = 10
        if SAFETY_MARGIN * path_rating <= fleet_rating:
            print ("Drydock recommendation %s(%d) from %s(%d) for fleet %s(%d) with fleet rating %2f and path rating %2f."
                   % (dock_sys.name, dock_sys.id,
                      start_sys.name, start_sys.id,
                      universe.getFleet(fleet_id).name, fleet_id,
                      fleet_rating, path_rating))
            return dock_sys.id

    print ("No safe drydock recommendation from %s(%d) for fleet %s(%d) with fleet rating %2f."
           % (start_sys.name, start_sys.id,
              universe.getFleet(fleet_id).name, fleet_id,
              fleet_rating))
    return None


def get_safe_path_leg_to_dest(fleet_id, start_id, dest_id):
    start_targ = universe_object.System(start_id)
    dest_targ = universe_object.System(dest_id)
    # TODO actually get a safe path
    this_path = can_travel_to_system(fleet_id, start_targ, dest_targ, ensure_return=False)
    path_ids = [targ.id for targ in this_path if targ.id != start_id] + [start_id]
    start_info = PlanetUtilsAI.sys_name_ids([start_id])
    dest_info = PlanetUtilsAI.sys_name_ids([dest_id])
    path_info = [PlanetUtilsAI.sys_name_ids([sys_id]) for sys_id in path_ids]
    print "Fleet %d requested safe path leg from %s to %s, found path %s" % (fleet_id, ppstring(start_info), ppstring(dest_info), ppstring(path_info))
    return path_ids[0]


def __find_path_with_fuel_to_system_with_possible_return(from_system_target, to_system_target, result_system_targets, fleet_supplyable_system_ids, max_fuel, fuel, supply_system_target):
    """
    Return systems required to visit with fuel to nearest supplied system.

    :param from_system_target:
    :type from_system_target: universe_object.System
    :param to_system_target:
    :type to_system_target: universe_object.System
    :param result_system_targets:
    :type result_system_targets: list
    :param fleet_supplyable_system_ids:
    :type fleet_supplyable_system_ids: list
    :param max_fuel:
    :type max_fuel: int
    :param fuel:
    :type fuel: int
    :param supply_system_target:
    :type supply_system_target: universe_object.System
    :return:
    :rtype list:
    """
    empire_id = fo.empireID()
    result = True
    # try to find if there is possible path to wanted system from system
    new_targets = result_system_targets[:]
    if from_system_target and to_system_target and supply_system_target:
        universe = fo.getUniverse()
        if from_system_target.id != INVALID_ID and to_system_target.id != INVALID_ID:
            least_jumps_path = universe.leastJumpsPath(from_system_target.id, to_system_target.id, empire_id)
        else:
            least_jumps_path = []
            result = False
        from_system_id = from_system_target.id
        for system_id in least_jumps_path:
            if from_system_id != system_id:
                if from_system_id in fleet_supplyable_system_ids:
                    # from supplied system fleet can travel without fuel consumption and also in this system refuels
                    fuel = max_fuel
                else:
                    fuel -= 1

                # leastJumpPath can differ from shortestPath
                # TODO: use Graph Theory to optimize
                if True or (system_id != to_system_target.id and system_id in fleet_supplyable_system_ids):  # TODO: restructure
                    new_targets.append(universe_object.System(system_id))
                if fuel < 0:
                    result = False
            from_system_id = system_id
    else:
        result = False

    # if there is path to wanted system, then also if there is path back to supplyable system
    if result:
        # jump from A to B means least_jumps_path=[A,B], but min_jumps=1
        min_jumps = len(universe.leastJumpsPath(to_system_target.id, supply_system_target.id, empire_id)) - 1

        if min_jumps > fuel:
            # print "fleetID:" + str(fleetID) + " fuel:" + str(fuel) + " required: " + str(min_jumps)
            result = False
        # else:
        #     resultSystemAITargets.append(toSystemAITarget)

    if not result:
        return []
    return new_targets


def get_resupply_fleet_order(fleet_target, current_system_target):
    """
    Return fleet_orders.OrderResupply to nearest supplied system.

    :param fleet_target: fleet that need to be resupplied
    :type fleet_target: universe_object.Fleet
    # TODO check if we can remove this id, because fleet already have it.
    :param current_system_target: current system of fleet
    :type current_system_target: universe_object.System
    :return: order to resupply
    :rtype fleet_orders.OrderResupply
    """
    # find nearest supplied system
    supplied_system_target = get_nearest_supplied_system(current_system_target.id)
    # create resupply AIFleetOrder
    return fleet_orders.OrderResupply(fleet_target, supplied_system_target)


def get_repair_fleet_order(fleet_target, current_sys_id):
    """
    Return fleet_orders.OrderRepair for fleet to proceed system with drydock.

    :param fleet_target: fleet that need to be repaired
    :type fleet_target: universe_object.Fleet
    # TODO check if we can remove this id, because fleet already have it.
    :param current_sys_id: current system id
    :type current_sys_id: int
    :return: order to repair
    :rtype fleet_orders.OrderRepair
    """
    # TODO Cover new mechanics where happiness increases repair rate - don't always use nearest system!
    # find nearest supplied system
    drydock_sys_id = get_best_drydock_system_id(current_sys_id, fleet_target.id)
    if drydock_sys_id is None:
        return None
    print "ordering fleet %d to %s for repair" % (fleet_target.id, ppstring(PlanetUtilsAI.sys_name_ids([drydock_sys_id])))
    # create resupply AIFleetOrder
    return fleet_orders.OrderRepair(fleet_target, universe_object.System(drydock_sys_id))
