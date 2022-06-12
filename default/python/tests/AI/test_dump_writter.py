from unittest.mock import Mock

from freeorion_tools.log_dumper import DumpKey
from freeorion_tools.log_dumper.dumper import Dumper


def test_dump_key():
    writer = Mock()
    d = Dumper(writer=writer)
    d.dump(DumpKey.SHIP_CONT, 11)
    writer.assert_called_once_with("##ShipCount:11")
