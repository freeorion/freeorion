from typing import Dict, List

from common.fo_typing import PlanetId, SpecialName
from freeorion_tools.caching import cache_for_current_turn


def set_growth_special(special: SpecialName, pid: PlanetId):
    get_growth_specials().setdefault(special, []).append(pid)


@cache_for_current_turn
def get_growth_specials() -> Dict[SpecialName, List[PlanetId]]:
    """
    Return map from the species to list of the planet where you could build a ship with it.
    """
    return {}
