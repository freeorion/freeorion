from logging import error, warning
from typing import Dict, Mapping, Tuple

import freeOrionAIInterface as fo
from freeorion_tools import cache_for_current_turn


@cache_for_current_turn
def _get_system_supply_map() -> Dict[int, int]:
    return fo.getEmpire().supplyProjections()  # map from system_id to supply


def get_system_supply(sys_id: int) -> int:
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
def _get_systems_map_by_jumps_to_supply() -> Mapping[int, Tuple[int]]:
    systems_by_jumps_to_supply = {}

    for sys_id, supply_val in _get_system_supply_map().items():
        systems_by_jumps_to_supply.setdefault(min(0, supply_val), []).append(sys_id)
    return {key: tuple(value) for key, value in systems_by_jumps_to_supply.items()}


def get_systems_by_supply_tier(supply_tier: int) -> Tuple[int]:
    """
    Get systems with supply tier.

    The current implementation does not distinguish between positive supply levels and caps at 0.
    Negative values indicate jumps away from supply.
    """
    if supply_tier > 0:
        warning("The current implementation does not distinguish between positive supply levels. "
                "Interpreting the query as supply_tier=0 (indicating system in supply).")
        supply_tier = 0
    return _get_systems_map_by_jumps_to_supply().get(supply_tier, tuple())


@cache_for_current_turn
def _get_enemy_supply_distance_map() -> Mapping[int, int]:
    enemies = [fo.getEmpire(_id) for _id in fo.allEmpireIDs() if _id != fo.empireID()]
    distance_to_enemy_supply = {}

    for enemy in enemies:
        if enemy is None:
            error('Got None for enemy empire!')
            continue

        for sys_id, supply_val in enemy.supplyProjections().items():
            distance_to_enemy_supply[sys_id] = min(
                distance_to_enemy_supply.get(sys_id, 999), -supply_val)

    return distance_to_enemy_supply


def get_distance_to_enemy_supply(sys_id: int) -> int:
    return _get_enemy_supply_distance_map().get(sys_id, 999)
