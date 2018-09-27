from logging import warn, debug

import freeOrionAIInterface as fo  # pylint: disable=import-error
from aistate_interface import get_aistate
import AIstate
import fleet_orders
import PlanetUtilsAI
import pathfinding
from AIDependencies import INVALID_ID, DRYDOCK_HAPPINESS_THRESHOLD
from target import TargetSystem
from turn_state import state


def create_move_orders_to_system(fleet, target):
    """
    Create a list of move orders from the fleet's current system to the target system.

    :param fleet: Fleet to be moved
    :type fleet: target.TargetFleet
    :param target: target system
    :type target: target.TargetSystem
    :return: list of move orders
    :rtype: list[fleet_orders.OrdersMove]
    """
    # TODO: use Graph Theory to construct move orders
    # TODO: add priority
    starting_system = fleet.get_system()  # current fleet location or current target system if on starlane
    if starting_system == target:
        # nothing to do here
        return []
    # if the mission does not end at the targeted system, make sure we can actually return to supply after moving.
    ensure_return = target.id not in set(AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs
                                         + AIstate.invasionTargetedSystemIDs)
    system_targets = can_travel_to_system(fleet.id, starting_system, target, ensure_return=ensure_return)
    result = [fleet_orders.OrderMove(fleet, system) for system in system_targets]
    if not result and starting_system.id != target.id:
        warn("fleet %s can't travel to system %s" % (fleet.id, target))
    return result


def can_travel_to_system(fleet_id, start, target, ensure_return=False):
    """
    Return list systems to be visited.

    :param fleet_id:
    :type fleet_id: int
    :param start:
    :type start: target.TargetSystem
    :param target:
    :type target:  target.TargetSystem
    :param ensure_return:
    :type ensure_return: bool
    :return:
    :rtype: list
    """
    if start == target:
        return [TargetSystem(start.id)]

    debug("Requesting path for fleet %s from %s to %s" % (fo.getUniverse().getFleet(fleet_id), start, target))
    target_distance_from_supply = -min(state.get_system_supply(target.id), 0)

    # low-aggression AIs may not travel far from supply
    if not get_aistate().character.may_travel_beyond_supply(target_distance_from_supply):
        debug("May not move %d out of supply" % target_distance_from_supply)
        return []

    min_fuel_at_target = target_distance_from_supply if ensure_return else 0
    path_info = pathfinding.find_path_with_resupply(start.id, target.id, fleet_id,
                                                    minimum_fuel_at_target=min_fuel_at_target)
    if path_info is None:
        debug("Found no valid path.")
        return []

    debug("Found valid path: %s" % str(path_info))
    return [TargetSystem(sys_id) for sys_id in path_info.path]


def get_nearest_supplied_system(start_system_id):
    """ Return systemAITarget of nearest supplied system from starting system startSystemID."""
    empire = fo.getEmpire()
    fleet_supplyable_system_ids = empire.fleetSupplyableSystemIDs
    universe = fo.getUniverse()

    if start_system_id in fleet_supplyable_system_ids:
        return TargetSystem(start_system_id)
    else:
        min_jumps = 9999  # infinity
        supply_system_id = INVALID_ID
        for system_id in fleet_supplyable_system_ids:
            if start_system_id != INVALID_ID and system_id != INVALID_ID:
                least_jumps_len = universe.jumpDistance(start_system_id, system_id)
                if least_jumps_len < min_jumps:
                    min_jumps = least_jumps_len
                    supply_system_id = system_id
        return TargetSystem(supply_system_id)


def get_best_drydock_system_id(start_system_id, fleet_id):
    """

    Get system_id of best drydock capable of repair, where best is nearest drydock
    that has a current and target happiness greater than the HAPPINESS_THRESHOLD
    with a path that is not blockaded or that the fleet can fight through to with
    acceptable losses.

    :param start_system_id: current location of fleet - used to find closest target
    :type start_system_id: int
    :param fleet_id: fleet that needs path to drydock
    :type: int
    :return: most suitable system id where the fleet should be repaired.
    :rtype: int
    """
    if start_system_id == INVALID_ID:
        warn("get_best_drydock_system_id passed bad system id.")
        return None

    if fleet_id == INVALID_ID:
        warn("get_best_drydock_system_id passed bad fleet id.")
        return None

    universe = fo.getUniverse()
    start_system = TargetSystem(start_system_id)
    drydock_system_ids = set()
    for sys_id, pids in state.get_empire_drydocks().iteritems():
        if sys_id == INVALID_ID:
            warn("get_best_drydock_system_id passed bad drydock sys_id.")
            continue
        for pid in pids:
            planet = universe.getPlanet(pid)
            if (planet and
                    planet.currentMeterValue(fo.meterType.happiness) >= DRYDOCK_HAPPINESS_THRESHOLD and
                    planet.currentMeterValue(fo.meterType.targetHappiness) >= DRYDOCK_HAPPINESS_THRESHOLD):
                drydock_system_ids.add(sys_id)
                break

    sys_distances = sorted([(universe.jumpDistance(start_system_id, sys_id), sys_id)
                            for sys_id in drydock_system_ids])

    aistate = get_aistate()
    fleet_rating = aistate.get_rating(fleet_id)
    for _, dock_sys_id in sys_distances:
        dock_system = TargetSystem(dock_sys_id)
        path = can_travel_to_system(fleet_id, start_system, dock_system)

        path_rating = sum([aistate.systemStatus[path_sys.id]['totalThreat']
                           for path_sys in path])

        SAFETY_MARGIN = 10
        if SAFETY_MARGIN * path_rating <= fleet_rating:
            debug("Drydock recommendation %s from %s for fleet %s with fleet rating %.1f and path rating %.1f."
                  % (dock_system, start_system, universe.getFleet(fleet_id), fleet_rating, path_rating))
            return dock_system.id

    debug("No safe drydock recommendation from %s for fleet %s with fleet rating %.1f."
          % (start_system, universe.getFleet(fleet_id), fleet_rating))
    return None


def get_safe_path_leg_to_dest(fleet_id, start_id, dest_id):
    start_targ = TargetSystem(start_id)
    dest_targ = TargetSystem(dest_id)
    # TODO actually get a safe path
    this_path = can_travel_to_system(fleet_id, start_targ, dest_targ, ensure_return=False)
    path_ids = [targ.id for targ in this_path if targ.id != start_id] + [start_id]
    universe = fo.getUniverse()
    debug("Fleet %d requested safe path leg from %s to %s, found path %s" % (
        fleet_id, universe.getSystem(start_id), universe.getSystem(dest_id), PlanetUtilsAI.sys_name_ids(path_ids)))
    return path_ids[0]


def get_resupply_fleet_order(fleet_target, current_system_target):
    """Return fleet_orders.OrderResupply to nearest supplied system.

    :param fleet_target: fleet that needs to be resupplied
    :type fleet_target: target.TargetFleet
    # TODO check if we can remove this id, because fleet already have it.
    :param current_system_target: current system of fleet
    :type current_system_target: target.TargetSystem
    :return: order to resupply
    :rtype fleet_orders.OrderResupply
    """
    # find nearest supplied system
    supplied_system_target = get_nearest_supplied_system(current_system_target.id)
    # create resupply AIFleetOrder
    return fleet_orders.OrderResupply(fleet_target, supplied_system_target)


def get_repair_fleet_order(fleet, current_system_id):
    """Return fleet_orders.OrderRepair for fleet to proceed to system with drydock.

    :param fleet: fleet that need to be repaired
    :type fleet: target.TargetFleet
    # TODO check if we can remove this id, because fleet already have it.
    :param current_system_id: current location of the fleet, next system if currently on starlane.
    :type current_system_id: int
    :return: order to repair
    :rtype fleet_orders.OrderRepair
    """
    # TODO Cover new mechanics where happiness increases repair rate - don't always use nearest system!
    # find nearest drydock system
    drydock_sys_id = get_best_drydock_system_id(current_system_id, fleet.id)
    if drydock_sys_id is None:
        return None

    debug("Ordering fleet %s to %s for repair" % (fleet, fo.getUniverse().getSystem(drydock_sys_id)))
    return fleet_orders.OrderRepair(fleet, TargetSystem(drydock_sys_id))
