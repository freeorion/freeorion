from unittest.mock import Mock

from freeorion_tools.statistics import StatKey
from freeorion_tools.statistics.statistic import Stats


def test_dump_key():
    writer = Mock()
    d = Stats(writer=writer)
    d.write(StatKey.SHIP_CONT, 11)
    writer.assert_called_once_with("##ShipCount:11")
