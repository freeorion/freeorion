import freeOrionAIInterface as fo
from logging import error, warning
from typing import Dict, Mapping, Tuple

from AIDependencies import INVALID_ID
from common.fo_typing import SystemId
from freeorion_tools.caching import cache_for_current_turn


@cache_for_current_turn
def _get_system_supply_map() -> Dict[SystemId, int]:
    return fo.getEmpire().supplyProjections()  # map from system_id to supply


def get_system_supply(sys_id: SystemId) -> int:
    """
    Get the supply level of a system.

    Negative values indicate jumps away from supply. Return -99 if system is not connected.
    """
    far_away_from_supply = -99

    retval = _get_system_supply_map().get(sys_id, None)
    if retval is None:
        # This is only expected to happen if a system has no path to any supplied system.
        # As the current code should not allow such queries, this is logged as warning.
        # If future code breaks this assumption, feel free to adjust logging.
        warning("Queried supply value of a system not mapped in empire.supplyProjections(): %d" % sys_id)
        return far_away_from_supply
    return retval


@cache_for_current_turn
def _get_systems_map_by_jumps_to_supply() -> Mapping[int, Tuple[SystemId]]:
    systems_by_jumps_to_supply = {}

    for sys_id, supply_val in _get_system_supply_map().items():
        systems_by_jumps_to_supply.setdefault(min(0, supply_val), []).append(sys_id)
    return {key: tuple(value) for key, value in systems_by_jumps_to_supply.items()}


def get_systems_by_supply_tier(supply_tier: int) -> Tuple[SystemId]:
    """
    Get systems with supply tier.

    The current implementation does not distinguish between positive supply levels and caps at 0.
    Negative values indicate jumps away from supply.
    """
    if supply_tier > 0:
        warning(
            "The current implementation does not distinguish between positive supply levels. "
            "Interpreting the query as supply_tier=0 (indicating system in supply)."
        )
        supply_tier = 0
    return _get_systems_map_by_jumps_to_supply().get(supply_tier, tuple())


@cache_for_current_turn
def _get_enemy_supply_distance_map() -> Mapping[int, int]:
    enemies = [fo.getEmpire(_id) for _id in fo.allEmpireIDs() if _id != fo.empireID()]
    distance_to_enemy_supply = {}

    for enemy in enemies:
        if enemy is None:
            error("Got None for enemy empire!")
            continue

        for sys_id, supply_val in enemy.supplyProjections().items():
            distance_to_enemy_supply[sys_id] = min(distance_to_enemy_supply.get(sys_id, 999), -supply_val)

    return distance_to_enemy_supply


def get_distance_to_enemy_supply(sys_id: int) -> int:
    return _get_enemy_supply_distance_map().get(sys_id, 999)


@cache_for_current_turn
def _system_to_supply_group() -> Mapping[SystemId, int]:
    """
    Create a mapping of SystemIds to supply_group_id.
    Used buy supply_connected, get_supply_group_id and get_supply_group.
    """
    result = {}
    for num, group in enumerate(fo.getEmpire().resourceSupplyGroups, start=1):
        result.update((sys_id, num) for sys_id in group)
    return result


def supply_connected(system_id1: SystemId, system_id2: SystemId) -> bool:
    """Return whether the two systems are connected by this empire's supply chains."""
    mapping = _system_to_supply_group()
    # Different defaults to get False when neither system is part of any group
    return mapping.get(system_id1, -1) == mapping.get(system_id2, -2)


def get_supply_group_id(system_id: SystemId) -> int:
    """
    Return a number identifying a supply group, INVALID_ID if none.
    These number have no meaning beyond the current turn, groups may be completely different next turn.
    The number can be used to check if a set of system belongs to the same or multiple groups or as input to
    get_supply_group
    """
    return _system_to_supply_group().get(system_id, INVALID_ID)


def get_supply_group(group_id: int) -> fo.IntSet:
    """
    Returns a Set of systems in the given group. Note that group_ids are only valid for the current turn,
    so the argument must have been returned by get_supply_group_id in the same turn.
    """
    # There is no way to directly get the nth element, but it should iterate in the same order every time,
    # see _system_to_supply_group.
    for num, group in enumerate(fo.getEmpire().resourceSupplyGroups, start=1):
        if num == group_id:
            return group
    raise (ValueError(f"invalid supply group_id: {group_id}"))
