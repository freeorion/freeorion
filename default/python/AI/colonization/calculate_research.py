import freeOrionAIInterface as fo
from typing import List

import AIDependencies
from buildings import BuildingType
from colonization.colony_score import debug_rating
from common.fo_typing import SystemId
from EnumsAI import FocusType
from freeorion_tools import (
    get_named_int,
    get_named_real,
    get_species_research,
    tech_soon_available,
)
from freeorion_tools.bonus_calculation import Bonus
from freeorion_tools.caching import cache_for_current_turn
from PlanetUtilsAI import get_empire_populated_planets
from turn_state import get_empire_planets_by_species, have_computronium


def calculate_research(planet: fo.planet, species: fo.species, max_population: float, stability: float) -> float:
    """
    Calculate how much PP the planet's population could generate with industry focus.
    This only considers values that actually rely on industry focus, those that do not are handled by
    calculate_planet_colonization_rating._rate_focus_independent.
    """
    if stability <= 0.0:
        return 0.0

    bonus_modified = _get_research_bonus_modified(planet, stability)
    skill_multiplier = get_species_research(species.name)
    bonus_by_policy = _get_research_bonus_modified_by_policy(stability)
    flat_by_policy = _get_research_flat_modified_by_policy(planet, stability)
    policy_multiplier = _get_policy_multiplier(stability)
    bonus_unmodified = _get_research_bonus_unmodified(planet, stability)

    result = max_population * (AIDependencies.INDUSTRY_PER_POP + bonus_modified) * skill_multiplier
    result = (result + max_population * bonus_by_policy + flat_by_policy) * policy_multiplier
    result += max_population * bonus_unmodified
    debug_rating(
        f"calculate_research pop={max_population:.2f}, st={stability:.2f}, b1={bonus_modified:.2f}, "
        f"m1={skill_multiplier:.2f}, b2={bonus_by_policy:.2f}, f2={flat_by_policy:.2f}, "
        f"m2={policy_multiplier:.2f}, b3={bonus_unmodified:.2f} -> {result:.2f}"
    )
    return result


@cache_for_current_turn
def _get_modified_research_bonuses(planet: fo.planet) -> List[Bonus]:
    """
    Get list of per-population bonuses which are added before multiplication with the species skill value.
    """
    # use fo.getEmpire().researchQueue directly here to simplify caching this function
    specials = planet.specials
    return [
        Bonus(
            AIDependencies.ANCIENT_RUINS_SPECIAL in specials or AIDependencies.ANCIENT_RUINS_SPECIAL2 in specials,
            get_named_real("ANCIENT_RUINS_MIN_STABILITY"),
            get_named_real("ANCIENT_RUINS_TARGET_RESEARCH_PERPOP"),
        ),
        Bonus(
            fo.getEmpire().policyAdopted("PLC_DIVERSITY"),
            get_named_real("PLC_DIVERSITY_MIN_STABILITY"),
            (len(get_empire_planets_by_species()) - get_named_int("PLC_DIVERSITY_THRESHOLD"))
            * get_named_real("PLC_DIVERSITY_SCALING"),
        ),
        Bonus(
            tech_soon_available("GRO_ENERGY_META", 3),
            get_named_real("GRO_ENERGY_META_MIN_STABILITY"),
            get_named_real("GRO_ENERGY_META_TARGET_RESEARCH_PERPOP"),
        ),
        Bonus(
            bool(BuildingType.ENCLAVE_VOID.built_or_queued_at()),
            get_named_real("BLD_ENCLAVE_VOID_MIN_STABILITY"),
            get_named_real("BLD_ENCLAVE_VOID_TARGET_RESEARCH_PERPOP"),
        ),
    ]


def _get_research_bonus_modified(planet: fo.planet, stability: float) -> float:
    """
    Calculate bonus research per population which would be added before multiplication with the species skill value.
    """
    return sum(bonus.get_bonus(stability) for bonus in _get_modified_research_bonuses(planet))


@cache_for_current_turn
def _get_modified_by_policy_research_bonuses(stability: float) -> List[Bonus]:
    return [
        Bonus(
            tech_soon_available("LRN_QUANT_NET", 3),
            get_named_real("LRN_QUANT_NET_MIN_STABILITY"),
            get_named_real("LRN_QUANT_NET_TARGET_RESEARCH_PERPOP"),
        ),
        # TBD check connection, _empire_resources should collect a list of systems or planets
        Bonus(
            have_computronium(),
            get_named_real("COMPUTRONIUM_MIN_STABILITY"),
            get_named_real("COMPUTRONIUM_TARGET_RESEARCH_PERPOP"),
        ),
        Bonus(
            fo.getEmpire().policyAdopted("PLC_LIBERTY"),
            get_named_real("PLC_LIBERTY_MIN_STABILITY"),
            (min(stability, get_named_real("PLC_LIBERTY_MAX_STABILITY")) - get_named_real("PLC_LIBERTY_MIN_STABILITY"))
            * get_named_real("PLC_LIBERTY_RESEARCH_BONUS_SCALING"),
        ),
    ]


def _get_research_bonus_modified_by_policy(stability: float) -> float:
    """
    Calculate bonus research per population which we would get independent of the species research skill,
    but still affected by industrialism.
    """
    return sum(bonus.get_bonus(stability) for bonus in _get_modified_by_policy_research_bonuses(stability))


@cache_for_current_turn
def _get_stellar_tomography_bonus(system_id: SystemId) -> float:
    universe = fo.getUniverse()
    system = universe.getSystem(system_id)
    if system.starType == fo.starType.blackHole:
        factor = get_named_real("LRN_STELLAR_TOMO_BLACK_TARGET_RESEARCH_PERPLANET")
    elif system.starType == fo.starType.neutron:
        factor = get_named_real("LRN_STELLAR_TOMO_NEUTRON_TARGET_RESEARCH_PERPLANET")
    else:
        factor = get_named_real("LRN_STELLAR_TOMO_NORMAL_STAR_TARGET_RESEARCH_PERPLANET")

    def is_our_researcher(pid):
        planet = universe.getPlanet(pid)
        return planet.focus == FocusType.FOCUS_RESEARCH and planet.owner == fo.empireID()

    num_researcher = sum(1 for pid in system.planetIDs if is_our_researcher(pid))
    # if we add another planet with research focus, all will profit. (n+1)^2 - n^2 = 2n + 1
    return (num_researcher * 2 + 1) * factor


def _get_research_flat_modified_by_policy(planet: fo.planet, stability: float) -> float:
    bonuses = [
        Bonus(
            tech_soon_available("LRN_STELLAR_TOMOGRAPHY", 3),
            0.0,
            _get_stellar_tomography_bonus(planet.systemID),
        ),
        # TODO Species InterDesign Academy
    ]
    return sum(bonus.get_bonus(stability) for bonus in bonuses)


def _get_policy_multiplier(stability) -> float:
    # if fo.getEmpire().policyAdopted("TECHNOCRACY") and stability >= get_named_real("PLC_TECHNOCRACY_MIN_STABILITY"):
    #     return 1.0 + get_named_real("PLC_TECHNOCRACY_TARGET_RESEARCH_PERCENT")
    if fo.getEmpire().policyAdopted("TECHNOCRACY") and stability >= get_named_real("PLC_TECHNOCRACY_MIN_STABILITY"):
        return 1.0 + get_named_real("PLC_TECHNOCRACY_TARGET_RESEARCH_PERCENT")
    return 1.0


@cache_for_current_turn
def _get_distributed_thought_bonus(system_id: SystemId) -> float:
    universe = fo.getUniverse()
    max_distance = max(
        (
            universe.linearDistance(system_id, p.systemID)
            for p in get_empire_populated_planets()
            if p.focus == FocusType.FOCUS_RESEARCH
        ),
        default=0.0,
    )
    # TBD? This could also affect other planets...
    return get_named_real("DISTRIB_THOUGH_RESEARCH_SCALING") * max_distance**0.5


def _get_research_bonus_unmodified(planet: fo.planet, stability: float):
    """
    Calculate bonus research per population which we would get independent of the species research skill.
    """
    bonuses = [
        Bonus(
            fo.getEmpire().policyAdopted("PLC_ALGORITHMIC_RESEARCH"),
            get_named_real("LRN_ALGO_RESEARCH_MIN_STABILITY"),
            get_named_real("LRN_ALGO_RESEARCH_TARGET_RESEARCH_PERCONSTRUCTION"),
        ),
        Bonus(
            tech_soon_available("LRN_DISTRIB_THOUGHT", 3),
            get_named_real("LRN_DISTRIB_THOUGHT_MIN_STABILITY"),
            _get_distributed_thought_bonus(planet.systemID),
        ),
        # TODO: distributed thought and collective thought
    ]
    return sum(bonus.get_bonus(stability) for bonus in bonuses)
