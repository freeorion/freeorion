from typing import Any

from common.print_utils import Table, Text


def dict_to_table(data: dict[str, Any], name=None):
    t = Table(Text("Key"), Text("Value", align=">"), table_name=name)

    for key, val in data.items():
        t.add_row(key, str(val))

    return t.get_table()


sample = {
    "structure": 128.0,
    "shields": 0.0,
    "attacks": {18.0: 1.0},
    "fighter_capacity": 0,
    "fighter_launch_rate": 0,
    "fighter_damage": 0,
    "flak_shots": 0,
    "has_interceptors": False,
    "damage_vs_planets": 18.0,
    "has_bomber": False,
}


print(dict_to_table(sample, name="Fooo"))
