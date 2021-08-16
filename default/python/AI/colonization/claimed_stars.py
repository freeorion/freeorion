import freeOrionAIInterface as fo
from collections import defaultdict
from typing import Mapping, Set

import AIstate
from common.fo_typing import SystemId
from freeorion_tools.caching import cache_for_current_turn


def has_claimed_star(*stars: "fo.starType") -> bool:
    """
    Return `True` if at least one star of that type is claimed.
    """
    return bool(set(_get_claimed_stars()).intersection(stars))


def is_system_star_claimed(system: "fo.system"):
    return system.systemID in _get_claimed_stars()[system.starType]


def count_claimed_stars(star_type: "fo.starType") -> int:
    """
    Count claimed starts of specified type.
    """
    return len(_get_claimed_stars()[star_type])


@cache_for_current_turn
def _get_claimed_stars() -> Mapping["fo.starType", Set[SystemId]]:
    """
    Return dictionary of star type: list of colonised and planned to be colonized systems.
    Start type converted to int because `cache_by_turn` store its value in savegame
    and boost objects are not serializable.
    """
    claimed_stars = defaultdict(set)

    claimed_stars.update({int(s_type): set(AIstate.empireStars[s_type]) for s_type in AIstate.empireStars})

    universe = fo.getUniverse()
    for sys_id in set(AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs):
        t_sys = universe.getSystem(sys_id)
        if not t_sys:
            continue
        claimed_stars[t_sys.starType].add(sys_id)
    return claimed_stars
