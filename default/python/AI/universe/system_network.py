import freeOrionAIInterface as fo
from typing import Sequence

from AIDependencies import INVALID_ID
from common.fo_typing import SystemId
from freeorion_tools.caching import cache_for_current_turn


def _min_max(a, b) -> Sequence:
    """
    Return args in sorted order.

    >>> _min_max(1, 2)
    (1, 2)

    >>> _min_max(2, 1)
    (1, 2)
    """
    return (a, b) if a <= b else (b, a)


def systems_connected(system1: SystemId, system2: SystemId) -> bool:
    """
    Return True if systems connected.

    Implementation notes: This is expensive operation on server.
    So we are caching it as much as possible.
    Sorting planets by ids could increase cache hits.
    """
    return _systems_connected(*_min_max(system1, system2))


@cache_for_current_turn
def _systems_connected(system1: SystemId, system2: SystemId) -> bool:
    if system1 == INVALID_ID:
        return False

    return fo.getUniverse().systemsConnected(system1, system2, fo.empireID())
