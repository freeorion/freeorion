from typing import Dict

_planet_supply_map: Dict[int, int] = {}


def get_planet_supply(pid: int, default: int) -> int:
    """
    Return planet supply.

    Note:
    This method has temporal coupling with the `update_planet_supply`.
    Supply map is persistent during session, so if you try to access it
    before planet was updated you will get old/default value.
    """
    return _planet_supply_map.get(pid, default)


def update_planet_supply(pid: int, value: int) -> None:
    """
    Update the planet supply.
    """
    _planet_supply_map[pid] = value
