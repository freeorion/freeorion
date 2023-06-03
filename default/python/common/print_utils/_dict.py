from typing import Any

from common.print_utils import Table, Text


def dict_to_table(data: dict[str, Any], name=None) -> str:
    t = Table(Text("Key"), Text("Value", align=">"), table_name=name)

    for key, val in data.items():
        t.add_row(key, str(val))

    return t.get_table()
