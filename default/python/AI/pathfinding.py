from heapq import heappush, heappop
from collections import namedtuple

import freeOrionAIInterface as fo
from AIDependencies import INVALID_ID
from turn_state import state

from common.configure_logging import convenience_function_references_for_logger
(debug, info, warn, error, fatal) = convenience_function_references_for_logger(__name__)


_ACCEPTABLE_DETOUR_LENGTH = 2000


# TODO Add a list of systems as parameter that are forbidden for search (e.g. blocked by monster)
# TODO Allow short idling in system to use stationary refuel mechanics in unsupplied systems if we
#      are very close to the target system but can't reach it due to fuel constraints
# TODO Consider additional optimizations:
#    - Check if universe.shortestPath complies with fuel mechanics before running
#      pathfinder (seemingly not worth it - we have exact heuristic in that case)
#    - Cut off branches that can't reach target due to supply distance
#    - For large graphs, check existence of a path on a simplified graph (use single node to represent supply clusters)
#    - For large graphs, check if there are any must-visit nodes (e.g. only possible resupplying system),
#      then try to find the shortest path between those and start/target.
def find_path_with_resupply(start, target, fleet_id, minimum_fuel_at_target=0):
    """Find the shortest possible path between two systems that complies with FreeOrion fuel mechanics.

     If the fleet can travel the shortest possible path between start and target system, then return that path.
     Otherwise, find the shortest possible detour including refueling.

     The core algorithm is a modified A* with the universe.shortestPathDistance as heuristic.
     While searching for a path, keep track of the fleet's fuel. Compared to standard A*/dijkstra,
     nodes are locked only for a certain minimum level of fuel - if a longer path yields a higher fuel
     level at a given system, then that path is considered as possible detour for refueling and added to the queue.

    :param start: start system id
    :type start: int
    :param target:  target system id
    :type target: int
    :param fleet_id: fleet to find the path for
    :type fleet_id: int
    :param minimum_fuel_at_target: optional - if specified, only accept paths that leave the
                                   fleet with at least this much fuel left at the target system
    :type minimum_fuel_at_target: int
    :return: shortest possible path including resupply-detours in the form of system ids
             including both start and target system
    :rtype: tuple[int]
    """
    universe = fo.getUniverse()
    empire_id = fo.empireID()

    if start == INVALID_ID or target == INVALID_ID:
        warn("Requested path between invalid systems.")
        return None

    # make sure the minimum fuel at target is realistic
    if minimum_fuel_at_target < 0:
        error("Requested negative fuel at target.")
        return None

    # make sure the target is connected to the start system
    shortest_possible_path_distance = universe.shortestPathDistance(start, target)
    if shortest_possible_path_distance == -1:
        warn("Requested path between disconnected systems, doing nothing.")
        return None

    # make sure the minimum fuel at target is realistic
    fleet = universe.getFleet(fleet_id)
    if fleet.maxFuel < minimum_fuel_at_target:
        return None

    supplied_systems = set(fo.getEmpire().fleetSupplyableSystemIDs)
    start_fuel = fleet.maxFuel if start in supplied_systems else fleet.fuel

    # We have 1 free jump from supplied system into unsupplied systems.
    # Thus, the target system must be at most maxFuel + 1 jumps away
    # in order to reach the system under standard conditions.
    # In some edge cases, we may have more supply here than what the
    # supply graph suggests. For example, we could have recently refueled
    # in a system that is now blockaded by an enemy or we have found
    # the refuel special.
    target_distance_from_supply = -min(state.get_system_supply(target), 0)
    if (fleet.maxFuel + 1 < (target_distance_from_supply + minimum_fuel_at_target) and
            universe.jumpDistance(start, target) > (start_fuel - minimum_fuel_at_target)):
        # can't possibly reach this system with the required fuel
        return None

    # initialize data structures
    path_cache = {}
    queue = []

    # add starting system to queue
    path_information = namedtuple('path_information', ['distance', 'fuel', 'path'])
    heappush(queue, (shortest_possible_path_distance, path_information(distance=0, fuel=start_fuel, path=(start,))))

    while queue:
        # get next system with path information
        (_, path_info) = heappop(queue)
        current = path_info.path[-1]

        # did we reach the target?
        if current == target:
            # do we satisfy fuel constraints?
            if path_info.fuel < minimum_fuel_at_target:
                continue

            return path_info

        # add information about how we reached here to the cache
        path_cache.setdefault(current, []).append(path_info)

        # check if we have enough fuel to travel to neighbors
        if path_info.fuel < 1:
            continue

        # add neighboring systems to the queue if the resulting path
        # is either shorter or offers more fuel than the other paths
        # which we already found to those systems
        for neighbor in universe.getImmediateNeighbors(current, empire_id):
            new_dist = path_info.distance + universe.linearDistance(current, neighbor)
            new_fuel = (fleet.maxFuel if (neighbor in supplied_systems or current in supplied_systems) else
                        path_info.fuel - 1)

            # check if the node is already closed, i.e. a path was already found which both is shorter and offers
            # more fuel. Priority queueing should ensure that all previously found paths here are shorter but check
            # it anyway to be sure...
            if any((dist <= new_dist and fuel >= new_fuel) for dist, fuel, _ in path_cache.get(neighbor, [])):
                continue

            # calculate the new distance prediction, i.e. the A* heuristic.
            predicted_distance = new_dist + universe.shortestPathDistance(neighbor, target)

            # Ignore paths that are much longer than the shortest possible path
            if predicted_distance > max(2*shortest_possible_path_distance,
                                        shortest_possible_path_distance + _ACCEPTABLE_DETOUR_LENGTH):
                continue

            # All checks passed, consider this path for further pathfinding
            heappush(queue, (predicted_distance, path_information(new_dist, new_fuel, path_info.path + (neighbor,))))

    # no path exists, not even if we refuel on the way
    return None
