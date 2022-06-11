from unittest.mock import Mock

from freeorion_tools.log_dumper.dump_interface import DumpKey
from freeorion_tools.log_dumper.dumper.dumper import Dumper


def test_dump_key():
    writer = Mock()
    d = Dumper(writer=writer)
    d.dump(DumpKey.AI_NAME, "hello")
    writer.assert_called_once_with("**ai_name:str:hello")
