from collections import Counter

import AIDependencies
from CombatRatingsAI._fleet_combat_stats import get_fleet_combat_stats
from CombatRatingsAI._ship_combat_stats import ShipCombatStats, get_ship_combat_stats
from common.fo_typing import FleetId, ShipId
from EnumsAI import MissionType
from freeorion_tools.caching import cache_for_current_turn


@cache_for_current_turn
def get_empire_standard_military_ship_stats() -> ShipCombatStats:
    """Get the current empire standard military ship stats, i.e. the most common ship type within the empire.

    :return: Stats of most common military ship in the empire
    """

    # avoid circular imports
    from FleetUtilsAI import get_empire_fleet_ids_by_role

    stats_dict = Counter()
    for fleet_id in get_empire_fleet_ids_by_role(MissionType.MILITARY):
        ship_stats = get_fleet_combat_stats(fleet_id, max_stats=True).get_ship_combat_stats()
        stats_dict.update(ship_stats)

    most_commons = stats_dict.most_common(1)
    if most_commons:
        return most_commons[0][0]
    else:
        return default_ship_stats()


def default_ship_stats() -> ShipCombatStats:
    """Return some ship stats to assume if no other intel is available.

    :return: Some weak standard ship
    """
    return ShipCombatStats(
        attacks=((6.0, 1),),
        structure=15,
        shields=0,
        fighter_capacity=0,
        fighter_launch_rate=0,
        fighter_damage=0,
        flak_shots=0,
        has_interceptors=False,
        damage_vs_planets=0,
        has_bomber=False,
    )


def weight_shields(shields: float, grade: str) -> float:
    """Re-weights shields based on species defense bonus."""
    offset = {"NO": 0, "BAD": 0, "": 0, "GOOD": 1.0, "GREAT": 0, "ULTIMATE": 0}.get(grade, 0)
    return shields + offset * AIDependencies.SHIP_SHIELD_FACTOR


def rating_needed(target: float, current: float = 0) -> float:
    """Estimate the needed rating to achieve target rating.

    :param target: Target rating to be reached
    :param current: Already existing rating
    :return: Estimated missing rating to reach target
    """
    if current >= target or target <= 0:
        return 0
    else:
        return target + current - 2 * (target * current) ** 0.5


def rating_difference(first_rating: float, second_rating: float) -> float:
    """Return the absolute nonlinear difference between ratings.

    :param first_rating: rating of a first force
    :param second_rating: rating of a second force
    :return: Estimated rating by which the greater force (nonlinearly) exceeds the lesser
    """

    return rating_needed(max(first_rating, second_rating), min(first_rating, second_rating))


def get_fleet_rating(fleet_id: FleetId, enemy_stats: ShipCombatStats = None) -> float:
    """Get rating for the fleet against specified enemy.

    :param fleet_id: fleet to be rated
    :param enemy_stats: enemy to be rated against
    """
    return get_fleet_combat_stats(fleet_id, max_stats=False).get_rating(enemy_stats)


def get_fleet_rating_against_planets(fleet_id: FleetId):
    return get_fleet_combat_stats(fleet_id, max_stats=False).get_rating_vs_planets()


def get_ship_rating(ship_id: ShipId, enemy_stats=None):
    return get_ship_combat_stats(ship_id=ship_id, max_stats=False).get_rating(enemy_stats)
