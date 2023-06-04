from collections.abc import Mapping
from typing import Any

from common.print_utils._fields import Text
from common.print_utils._table import Table


def dict_to_table(data: Mapping[str, Any], name=None) -> Table:
    t = Table(Text("Key"), Text("Value", align=">"), hide_header=True, table_name=name)

    for key, val in data.items():
        t.add_row(key, str(val))
    return t
