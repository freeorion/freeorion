import freeOrionAIInterface as fo
from typing import FrozenSet, Sequence, Set

from AIDependencies import INVALID_ID
from common.fo_typing import SystemId
from freeorion_tools.caching import cache_for_current_turn
from PlanetUtilsAI import get_capital_sys_id


def _min_max(a, b) -> Sequence:
    """
    Return args in sorted order.

    This function is designed to improve caches performaces,
    when order of arguments doesn't matter we can reduce number of cached calls by
    setting them in the sorted order.

    >>> _min_max(1, 2)
    (1, 2)

    >>> _min_max(2, 1)
    (1, 2)
    """
    return (a, b) if a <= b else (b, a)


def systems_connected(system1: SystemId, system2: SystemId) -> bool:
    """
    Return True if systems connected.
    """
    return _systems_connected(*_min_max(system1, system2))


@cache_for_current_turn
def _systems_connected(system1: SystemId, system2: SystemId) -> bool:
    if system1 == INVALID_ID:
        return False

    # optimization: We cache a set of systems connected to the capital
    # system in a single DFS and check if the systems are both in that set.
    # This allows us to avoid a BFS for each pair of systems.
    # Only in case neither of the system connects to the capital, we need to
    # run the BFS.
    connected_system_set = systems_connected_to_system(get_capital_sys_id())
    sys1_connected_to_capital = system1 in connected_system_set
    sys2_connected_to_capital = system2 in connected_system_set
    if sys1_connected_to_capital and sys2_connected_to_capital:
        # both connected to the capital - so must be connected to each other
        return True

    if sys1_connected_to_capital != sys2_connected_to_capital:
        # Only one connected to the capital - can't be connected to each other
        return False

    # both are not connected to the home system - they may or may not be connected to each other
    return fo.getUniverse().systemsConnected(system1, system2, fo.empireID())


def get_shortest_distance(system_1: SystemId, system_2: SystemId) -> float:
    """
    Return the distance between the systems where objects are located.
    """
    return _get_shortest_distance(*_min_max(system_1, system_2))


@cache_for_current_turn
def _get_shortest_distance(system_1: SystemId, system_2: SystemId) -> float:
    return fo.getUniverse().shortestPathDistance(system_1, system_2)


@cache_for_current_turn
def get_neighbors(sid: SystemId) -> Set[SystemId]:
    return set(fo.getUniverse().getImmediateNeighbors(sid, fo.empireID()))


@cache_for_current_turn
def systems_connected_to_system(system_id: SystemId) -> Set[SystemId]:
    """
    Use depth-first-search to find connected systems to system_id
    """
    connected_systems = set()
    if system_id == INVALID_ID:
        return connected_systems

    to_visit = [system_id]
    while to_visit:
        current_system = to_visit.pop()
        if current_system in connected_systems:
            continue

        connected_systems.add(current_system)
        to_visit.extend(get_neighbors(current_system))

    return connected_systems


@cache_for_current_turn
def within_n_jumps(system_id: SystemId, n: int) -> FrozenSet[SystemId]:
    """
    Returns a frozenset of all systems within n jumps from the given system.
    """
    if n < 1:
        return frozenset({system_id})
    elif n == 1:
        return frozenset({system_id} | get_neighbors(system_id))
    tier_minus_2 = within_n_jumps(system_id, n - 2)
    tier_minus_1 = within_n_jumps(system_id, n - 1)
    result = set(tier_minus_1)
    for sys_id in tier_minus_1 - tier_minus_2:
        result.update(get_neighbors(sys_id))
    return frozenset(result)
