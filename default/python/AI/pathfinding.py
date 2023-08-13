import freeOrionAIInterface as fo
from collections import namedtuple
from heapq import heappop, heappush
from logging import error, warning
from typing import Callable, Optional

import PlanetUtilsAI
from AIDependencies import INVALID_ID
from aistate_interface import get_aistate
from common.fo_typing import SystemId
from EnumsAI import MissionType
from freeorion_tools import chat_human, get_partial_visibility_turn
from freeorion_tools.caching import cache_for_current_turn
from turn_state import get_system_supply
from universe.system_network import get_neighbors, get_shortest_distance

_DEBUG_CHAT = False
_ACCEPTABLE_DETOUR_LENGTH = 2000
PathInformation = namedtuple("PathInformation", ["distance", "fuel", "path"])


# cache this so that boost python does not need to make a new copy every time this info is needed in a turn.
@cache_for_current_turn
def _get_unobstructed_systems():
    return fo.getEmpire().supplyUnobstructedSystems


def _info_string(path_info):
    return f"dist {path_info.distance:.1f}, path {PlanetUtilsAI.sys_name_ids(path_info.path)}"


# Note that this can cover departure from uncontested systems as well as from contested systems where our forces
# arrived first (and those forces may or may not be the forces for whom the pathfinding is being done)
def _more_careful_travel_starlane_func(c, d):
    return c in _get_unobstructed_systems()


# In comparison to  _more_careful_travel_starlane_func, this can also allow passage through contested
# systems where our forces already present there (which may or may not be the forces for whome the pathfinding
# is being done) arrived second but are currently preserving travel along the starlane of interest.
def _somewhat_careful_travel_starlane_func(c, d):
    return any((c in _get_unobstructed_systems(), fo.getEmpire().preservedLaneTravel(c, d)))


# For some activities like scouting, we may want to allow an extra bit of risk from routing through a system which
# has all its exits necessarily marked as restricted simply because it has never been partially visible
def _risky_travel_starlane_func(c, d):
    return any((_somewhat_careful_travel_starlane_func(c, d), get_partial_visibility_turn(c) <= 0))


def _may_travel_anywhere(*args, **kwargs):
    return True


_STARLANE_TRAVEL_FUNC_MAP = {
    MissionType.EXPLORATION: _risky_travel_starlane_func,
    MissionType.OUTPOST: _more_careful_travel_starlane_func,
    MissionType.COLONISATION: _more_careful_travel_starlane_func,
    MissionType.INVASION: _somewhat_careful_travel_starlane_func,
    MissionType.MILITARY: _may_travel_anywhere,
    MissionType.SECURE: _may_travel_anywhere,
}


def find_path_with_resupply(
    start: int,
    target: int,
    fleet_id: int,
    minimum_fuel_at_target: int = 0,
    mission_type_override: Optional[MissionType] = None,
) -> PathInformation:
    """
    :param start: start system id
    :param target:  target system id
    :param fleet_id: fleet to find the path for
    :param minimum_fuel_at_target: optional - if specified, only accept paths that leave the
                                   fleet with at least this much fuel left at the target system
    :param mission_type_override: optional - use the specified mission type, rather than the fleet's
                                  current mission type, for pathfinding routing choices
    :return: shortest possible path including resupply-detours in the form of system ids
             including both start and target system
    """

    universe = fo.getUniverse()
    fleet = universe.getFleet(fleet_id)
    if not fleet:
        return None
    empire = fo.getEmpire()
    supplied_systems = set(empire.fleetSupplyableSystemIDs)
    start_fuel = fleet.maxFuel if start in supplied_systems else fleet.fuel

    # We have 1 free jump from supplied system into unsupplied systems.
    # Thus, the target system must be at most maxFuel + 1 jumps away
    # in order to reach the system under standard conditions.
    # In some edge cases, we may have more supply here than what the
    # supply graph suggests. For example, we could have recently refueled
    # in a system that is now blockaded by an enemy or we have found
    # the refuel special.
    target_distance_from_supply = -min(get_system_supply(target), 0)
    if fleet.maxFuel + 1 < (target_distance_from_supply + minimum_fuel_at_target) and universe.jumpDistance(
        start, target
    ) > (start_fuel - minimum_fuel_at_target):
        # can't possibly reach this system with the required fuel
        return None

    mission_type = (
        mission_type_override if mission_type_override is not None else get_aistate().get_fleet_mission(fleet_id)
    )
    may_travel_starlane_func = _STARLANE_TRAVEL_FUNC_MAP.get(mission_type, _more_careful_travel_starlane_func)

    path_info = find_path_with_resupply_generic(
        start,
        target,
        start_fuel,
        fleet.maxFuel,
        lambda s: s in supplied_systems,
        minimum_fuel_at_target,
        may_travel_starlane_func=may_travel_starlane_func,
    )

    if not _DEBUG_CHAT:
        return path_info

    if may_travel_starlane_func != _may_travel_anywhere:
        risky_path = find_path_with_resupply_generic(
            start, target, start_fuel, fleet.maxFuel, lambda s: s in supplied_systems, minimum_fuel_at_target
        )
        if path_info and may_travel_starlane_func == _risky_travel_starlane_func:
            safest_path = find_path_with_resupply_generic(
                start,
                target,
                start_fuel,
                fleet.maxFuel,
                lambda s: s in supplied_systems,
                minimum_fuel_at_target,
                may_travel_starlane_func=_more_careful_travel_starlane_func,
            )
            if safest_path and path_info.distance < safest_path.distance:
                message = "(Scout?) Fleet %d chose somewhat risky path %s instead of safe path %s" % (
                    fleet_id,
                    _info_string(path_info),
                    _info_string(risky_path),
                )
                chat_human(message)

        if path_info and risky_path and risky_path.distance < path_info.distance:
            message = "Fleet %d chose safer path %s instead of risky path %s" % (
                fleet_id,
                _info_string(path_info),
                _info_string(risky_path),
            )
            chat_human(message)

    return path_info


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
def find_path_with_resupply_generic(  # noqa: C901
    start: SystemId,
    target: SystemId,
    start_fuel: float,
    max_fuel: float,
    system_suppliable_func: Callable[[int], bool],
    minimum_fuel_at_target=0,
    may_travel_system_func: Optional[Callable[[int], bool]] = None,
    may_travel_starlane_func: Optional[Callable[[int, int], bool]] = None,
) -> Optional[PathInformation]:
    """Find the shortest possible path between two systems that complies with FreeOrion fuel mechanics.

     If the fleet can travel the shortest possible path between start and target system, then return that path.
     Otherwise, find the shortest possible detour including refueling.

     The core algorithm is a modified A* with the universe.shortestPathDistance as heuristic.
     While searching for a path, keep track of the fleet's fuel. Compared to standard A*/dijkstra,
     nodes are locked only for a certain minimum level of fuel - if a longer path yields a higher fuel
     level at a given system, then that path is considered as possible detour for refueling and added to the queue.

    :param start_fuel: starting fuel of the fleet
    :param max_fuel: max fuel of the fleet
    :param system_suppliable_func: boolean function with one int param s, specifying if a system s provides fleet supply
    :param minimum_fuel_at_target: optional - if specified, only accept paths that leave the
                                   fleet with at least this much fuel left at the target system
    :param may_travel_system_func: optional - boolean function with one int param, s, specifying if
                                   a system s is OK to travel through
    :param may_travel_starlane_func: optional - boolean function with 2 int params c, d, specifying if
                                     a starlane from c to d is OK to travel through
    :return: shortest possible path including resupply-detours in the form of system ids
             including both start and target system
    """

    universe = fo.getUniverse()

    if start == INVALID_ID or target == INVALID_ID:
        warning("Requested path between invalid systems.")
        return None

    # make sure the minimum fuel at target is realistic
    if minimum_fuel_at_target < 0:
        error("Requested negative fuel at target.")
        return None

    # make sure the minimum fuel at target is realistic
    if max_fuel < minimum_fuel_at_target:
        return None

    # make sure the target is connected to the start system
    shortest_possible_path_distance = get_shortest_distance(start, target)
    if shortest_possible_path_distance == -1:
        warning("Requested path between disconnected systems, doing nothing.")
        return None

    if may_travel_system_func is None:
        may_travel_system_func = _may_travel_anywhere
    if may_travel_starlane_func is None:
        may_travel_starlane_func = _may_travel_anywhere

    # initialize data structures
    path_cache = {}
    queue = []

    # add starting system to queue
    heappush(queue, (shortest_possible_path_distance, PathInformation(distance=0, fuel=start_fuel, path=(start,))))

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
        for neighbor in get_neighbors(current):
            # A system we have never had partial vis for will count as fully blockaded for us, but perhaps if
            # we are scouting we might want to be able to route a path through it anyway.
            if any(
                (
                    not may_travel_starlane_func(current, neighbor),
                    neighbor != target and not may_travel_system_func(neighbor),
                )
            ):
                continue
            new_dist = path_info.distance + universe.linearDistance(current, neighbor)
            new_fuel = (
                max_fuel
                if (system_suppliable_func(neighbor) or system_suppliable_func(current))
                else path_info.fuel - 1
            )

            # check if the node is already closed, i.e. a path was already found which both is shorter and offers
            # more fuel. Priority queueing should ensure that all previously found paths here are shorter but check
            # it anyway to be sure...
            if any((dist <= new_dist and fuel >= new_fuel) for dist, fuel, _ in path_cache.get(neighbor, [])):
                continue

            # calculate the new distance prediction, i.e. the A* heuristic.
            predicted_distance = new_dist + get_shortest_distance(neighbor, target)

            # Ignore paths that are much longer than the shortest possible path
            if predicted_distance > max(
                2 * shortest_possible_path_distance, shortest_possible_path_distance + _ACCEPTABLE_DETOUR_LENGTH
            ):
                continue

            # All checks passed, consider this path for further pathfinding
            heappush(queue, (predicted_distance, PathInformation(new_dist, new_fuel, path_info.path + (neighbor,))))

    # no path exists, not even if we refuel on the way
    return None
