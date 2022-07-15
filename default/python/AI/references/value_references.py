import freeOrionAIInterface as fo
from logging import error

from freeorion_tools import get_named_int, get_named_real

_missed_references = []


def _get_named_int(name: str) -> int:
    """
    Returns a NamedReal from FOCS.
    If the value does not exist, reports an error and returns 1.
    Note that we do not raise and exception so that the AI can continue, as good as it can, with outdated information.
    This is also why we return 1, returning 0 could cause followup errors if the value is used as divisor.
    """
    if fo.namedIntDefined(name):
        return fo.getNamedInt(name)
    else:
        _missed_references.append(name)
        return 1


def get_policy_diversity_threshold() -> int:
    return get_named_int("PLC_DIVERSITY_THRESHOLD")


PLC_DIVERSITY_THRESHOLD = _get_named_int("PLC_DIVERSITY_THRESHOLD")
ZZZZ = _get_named_int("ZZZ")
BBB = _get_named_int("BBBBBBBB")


def get_policy_diversity_scaling() -> float:
    return get_named_real("PLC_DIVERSITY_SCALING")


def get_policy_diversity_min_stability() -> float:
    return get_named_real("PLC_DIVERSITY_MIN_STABILITY")


def get_ancient_ruins_min_stability() -> float:
    return get_named_real("ANCIENT_RUINS_MIN_STABILITY")


def get_ancient_ruins_target_research_perpop() -> float:
    return get_named_real("ANCIENT_RUINS_TARGET_RESEARCH_PERPOP")


if _missed_references:
    error(
        "'%s' contains references to the scripting references, that does not exist: %s, please remove them",
        __name__,
        sorted(_missed_references),
    )
