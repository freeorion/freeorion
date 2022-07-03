from typing import Dict, List, Mapping

from common.fo_typing import PlanetId, SpecialName
from empire.survey_lock import survey_universe_lock
from freeorion_tools.caching import cache_for_current_turn


def set_growth_special(special: SpecialName, pid: PlanetId):
    _get_growth_specials().setdefault(special, []).append(pid)


@survey_universe_lock
def get_growth_specials() -> Mapping[SpecialName, List[PlanetId]]:
    """
    Return map from a growth special to list of the planets where we have it.
    """
    return _get_growth_specials()


@cache_for_current_turn
def _get_growth_specials() -> Dict[SpecialName, List[PlanetId]]:
    """
    Return mutable state.
    """
    return {}
