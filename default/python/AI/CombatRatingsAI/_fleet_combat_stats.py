import freeOrionAIInterface as fo
from typing import List

from CombatRatingsAI._ship_combat_stats import ShipCombatStats, get_ship_combat_stats
from common.fo_typing import FleetId
from freeorion_tools import combine_ratings
from freeorion_tools.caching import cache_for_current_turn


class FleetCombatStats:
    """Stores combat related stats of the fleet."""

    def __init__(self, fleet_id: FleetId, max_stats=False):
        self._fleet_id = fleet_id
        self._max_stats = max_stats
        self._ship_stats = self._get_stats_from_fleet()

    def get_ship_combat_stats(self) -> List[ShipCombatStats]:
        """Returns list of ShipCombatStats of the fleet."""
        return list(self._ship_stats)

    def get_rating(self, enemy_stats: ShipCombatStats = None) -> float:
        """Calculates the rating of the fleet by combining all its ships ratings.

        :param enemy_stats: enemy to be rated against
        :return: Rating of the fleet
        """
        return combine_ratings(x.get_rating(enemy_stats) for x in self._ship_stats)

    def get_rating_vs_planets(self) -> float:
        return combine_ratings(x.get_rating_vs_planets() for x in self._ship_stats)

    def _get_stats_from_fleet(self):
        """Calculate fleet combat stats (i.e. the stats of all its ships)."""
        universe = fo.getUniverse()
        fleet = universe.getFleet(self._fleet_id)
        if not fleet:
            return []
        return [get_ship_combat_stats(ship_id=ship_id, max_stats=self._max_stats) for ship_id in fleet.shipIDs]


@cache_for_current_turn
def get_fleet_combat_stats(fleet_id, max_stats=False):
    return FleetCombatStats(fleet_id, max_stats=max_stats)


def get_ships_stats_for_fleet(fleet_id, max_stats=False) -> List[ShipCombatStats]:
    return get_fleet_combat_stats(fleet_id, max_stats=max_stats).get_ship_combat_stats()
