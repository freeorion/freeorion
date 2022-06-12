from unittest.mock import Mock

from common.statistic_interface import StatKey
from freeorion_tools.statistics import Stats


def test_stat_key():
    writer = Mock()
    stats = Stats(writer=writer)
    stats.write(StatKey.SHIP_CONT, 11)
    writer.assert_called_once_with("##ShipCount:11")
