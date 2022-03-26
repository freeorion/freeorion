from empire.survey_lock import survey_universe_lock
from freeorion_tools.caching import cache_for_current_turn


class _ColonyStatus:
    def __init__(self):
        self.colonies_under_attack = False
        self.colonies_under_threat = False


@survey_universe_lock
def colonies_is_under_attack() -> bool:
    return bool(_get_colony_status().colonies_under_attack)


@survey_universe_lock
def colonies_is_under_treat() -> bool:
    return bool(_get_colony_status().colonies_under_threat)


@cache_for_current_turn
def _get_colony_status() -> _ColonyStatus:
    return _ColonyStatus()


def set_colonies_is_under_treat():
    _get_colony_status().colonies_under_threat = True


def set_colonies_is_under_attack():
    _get_colony_status().colonies_under_attack = True
