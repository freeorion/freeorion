from freeorion_tools import get_named_int, get_named_real


def get_policy_diversity_threshold() -> int:
    return get_named_int("PLC_DIVERSITY_THRESHOLD")


def get_policy_diversity_scaling() -> float:
    return get_named_real("PLC_DIVERSITY_SCALING")


def get_policy_diversity_min_stability() -> float:
    return get_named_real("PLC_DIVERSITY_MIN_STABILITY")


def get_ancient_ruins_min_stability() -> float:
    return get_named_real("ANCIENT_RUINS_MIN_STABILITY")


def get_ancient_ruins_target_research_perpop() -> float:
    return get_named_real("ANCIENT_RUINS_TARGET_RESEARCH_PERPOP")
