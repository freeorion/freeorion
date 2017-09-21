import freeOrionAIInterface as fo  # pylint: disable=import-error
import FreeOrionAI as foAI
import AIstate
import universe_object
import fleet_orders
import ColonisationAI
import FleetUtilsAI
import PlanetUtilsAI
import pathfinding
from freeorion_tools import ppstring
from AIDependencies import INVALID_ID, DRYDOCK_HAPPINESS_THRESHOLD
from turn_state import state

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
    :type start: universe_object.System
    :param target:
    :type target:  universe_object.System
    :param ensure_return:
    :type ensure_return: bool
    :return:
    :rtype: list
    """
    if start == target:
        return [universe_object.System(start.id)]

    debug("Requesting path from %s to %s" % (start, target))
    target_distance_from_supply = -min(state.get_system_supply(target.id), 0)

    # low-aggression AIs may not travel far from supply
    if not foAI.foAIstate.character.may_travel_beyond_supply(target_distance_from_supply):
        debug("May not move %d out of supply" % target_distance_from_supply)
        return []

    min_fuel_at_target = target_distance_from_supply if ensure_return else 0
    path_info = pathfinding.find_path_with_resupply(start.id, target.id, fleet_id,
                                                    minimum_fuel_at_target=min_fuel_at_target)
    if path_info is None:
        debug("Found no valid path.")
        return []

    debug("Found valid path: %s" % str(path_info))
    return [universe_object.System(sys_id) for sys_id in path_info.path]


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
        warn("get_best_drydock_system_id passed bad system id.")
        return None

    if fleet_id == INVALID_ID:
        warn("get_best_drydock_system_id passed bad fleet id.")
        return None

    universe = fo.getUniverse()
    start_sys = universe.getSystem(start_system_id)
    drydock_system_ids = set()
    for sys_id, pids in ColonisationAI.empire_dry_docks.iteritems():
        if sys_id == INVALID_ID:
            warn("get_best_drydock_system_id passed bad dry dock sys_id.")
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
