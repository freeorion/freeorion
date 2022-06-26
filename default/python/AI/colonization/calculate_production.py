import freeOrionAIInterface as fo
from typing import Iterable, NamedTuple

import AIDependencies
from buildings import BuildingType
from freeorion_tools import get_named_real, get_species_tag_value, tech_is_complete
from turn_state import have_honeycomb


def calculate_production(
    planet: fo.planet, species: fo.species, max_population: float, stability: float, research_list: Iterable
) -> float:
    """
    Calculate how much PP the planet's population could generate with industry focus.
    This only considers values that actually rely on industry focus, those that do not are handled by
    calculate_planet_colonization_rating._rate_focus_independent.
    """
    if stability <= 0.0:
        return 0.0

    bonus_modified = _get_production_bonus_modified(planet, stability, research_list)
    skill_multiplier = get_species_tag_value(species.name, AIDependencies.Tags.INDUSTRY)
    bonus_by_policy = _get_production_bonus_mod_by_policy(stability)
    policy_multiplier = _get_policy_multiplier(stability)
    bonus_unmodified = _get_production_bonus_unmodified(planet, stability)
    bonus_flat = _get_production_flat(planet, stability, research_list)
    per_population = (
        (AIDependencies.INDUSTRY_PER_POP + bonus_modified) * skill_multiplier + bonus_by_policy
    ) * policy_multiplier + bonus_unmodified
    return max_population * per_population + bonus_flat


class _ProductionBonus(NamedTuple):
    available: bool
    min_stability: float
    value: float


def _get_production_bonus_modified(planet: fo.planet, stability: float, research_list: Iterable):
    """
    Calculate bonus production per population which is added before multiplication with the species skill value.
    """
    result = sum(
        AIDependencies.INDUSTRY_PER_POP for s in planet.specials if s in AIDependencies.industry_boost_specials_modified
    )

    # TBD check connection!?
    # TBD: cache bonuses?
    have_center = bool(BuildingType.INDUSTRY_CENTER.built_or_queued_at())
    centre_bonus1 = get_named_real("BLD_INDUSTRY_CENTER_1_TARGET_INDUSTRY_PERPOP")
    centre_bonus2 = get_named_real("BLD_INDUSTRY_CENTER_2_TARGET_INDUSTRY_PERPOP")
    centre_bonus3 = get_named_real("BLD_INDUSTRY_CENTER_3_TARGET_INDUSTRY_PERPOP")
    bonuses = [
        _ProductionBonus(
            tech_is_complete("PRO_FUSION_GEN") or "PRO_FUSION_GEN" in research_list[:3],
            get_named_real("PRO_FUSION_GEN_MIN_STABILITY"),
            get_named_real("PRO_FUSION_GEN_TARGET_INDUSTRY_PERPOP"),
        ),
        _ProductionBonus(
            tech_is_complete("GRO_ENERGY_META") or "GRO_ENERGY_META" in research_list[:3],
            get_named_real("GRO_ENERGY_META_MIN_STABILITY"),
            get_named_real("GRO_ENERGY_META_TARGET_INDUSTRY_PERPOP"),
        ),
        # special case: these are not cumulative
        _ProductionBonus(
            have_center and tech_is_complete("PRO_INDUSTRY_CENTER_III"),
            get_named_real("BLD_INDUSTRY_CENTER_3_MIN_STABILITY"),
            centre_bonus3 - centre_bonus2,
        ),
        _ProductionBonus(
            have_center and tech_is_complete("PRO_INDUSTRY_CENTER_II"),
            get_named_real("BLD_INDUSTRY_CENTER_2_MIN_STABILITY"),
            centre_bonus2 - centre_bonus1,
        ),
        _ProductionBonus(
            have_center and tech_is_complete("PRO_INDUSTRY_CENTER_I"),
            get_named_real("BLD_INDUSTRY_CENTER_1_MIN_STABILITY"),
            centre_bonus1,
        ),
    ]
    for bonus in bonuses:
        if bonus.available and stability >= bonus.min_stability:
            result += bonus.value
    return result


def _get_policy_multiplier(stability) -> float:
    if fo.getEmpire().policyAdopted("INDUSTRIALISM") and stability >= get_named_real("PLC_INDUSTRIALISM_MIN_STABILITY"):
        return 1.0 + get_named_real("PLC_INDUSTRIALISM_TARGET_INDUSTRY_PERCENT")
    return 1.0


def _get_production_bonus_mod_by_policy(stability: float):
    """
    Calculate bonus production per population which is independent of the species production skill, but affected
    by industrialism.
    """
    # TBD: check connections?
    result = 0.0
    bonuses = [
        _ProductionBonus(
            bool(BuildingType.BLACK_HOLE_POW_GEN.built_or_queued_at()),
            get_named_real("BLD_BLACK_HOLE_POW_GEN_MIN_STABILITY"),
            get_named_real("BLD_BLACK_HOLE_POW_GEN_TARGET_INDUSTRY_PERPOP"),
        ),
    ]
    for bonus in bonuses:
        if bonus.available and stability >= bonus.min_stability:
            result += bonus.value
    return result


def _get_production_bonus_unmodified(planet: fo.planet, stability: float):
    """
    Calculate bonus production per population which is independent of the species production skill.
    """
    result = sum(
        # growth.macros: STANDARD_INDUSTRY_BOOST
        AIDependencies.INDUSTRY_PER_POP
        for s in planet.specials
        if s in AIDependencies.industry_boost_specials_unmodified
    )

    # TBD: check connections?
    bonuses = [
        _ProductionBonus(
            bool(BuildingType.SOL_ORB_GEN.built_or_queued_at()),  # TBD: check star type
            get_named_real("BLD_SOL_ORB_GEN_MIN_STABILITY"),
            get_named_real("BLD_SOL_ORB_GEN_BRIGHT_TARGET_INDUSTRY_PERPOP"),
        ),
        _ProductionBonus(
            have_honeycomb(),
            0.0,
            get_named_real("HONEYCOMB_TARGET_INDUSTRY_PERPOP"),
        ),
    ]
    for bonus in bonuses:
        if bonus.available and stability >= bonus.min_stability:
            result += bonus.value
    return result


def _get_production_flat(planet: fo.planet, stability: float, research_list: Iterable) -> float:
    """
    Calculate population independent production bonus.
    """
    value = _get_asteroid_and_ggg_value(planet, stability, research_list)
    tech = AIDependencies.PRO_AUTO_1
    # this is a rather expensive one, so consider it only if it is on top of the list
    if tech_is_complete(tech) or tech == research_list[0]:
        value += get_named_real("PRO_SENTIENT_AUTO_TARGET_INDUSTRY_FLAT")
    return value


def _get_asteroid_and_ggg_value(planet: fo.planet, stability: float, research_list: Iterable) -> float:
    """
    Calculate an estimate of the bonus we may get from asteroids (micro-gravity) and gas giant generators.
    """
    universe = fo.getUniverse()
    system = universe.getSystem(planet.systemID)
    ast_tech = AIDependencies.PRO_MICROGRAV_MAN
    ast_min_stability = get_named_real("PRO_MICROGRAV_MAN_MIN_STABILITY")
    count_asteroids = ast_min_stability <= stability and (tech_is_complete(ast_tech) or ast_tech in research_list[:5])
    ggg_tech = AIDependencies.PRO_ORBITAL_GEN
    ggg_min_stability = get_named_real("BLD_GAS_GIANT_GEN_MIN_STABILITY")
    count_ggg = ggg_min_stability <= stability and (tech_is_complete(ggg_tech) or ggg_tech in research_list[:5])
    asteroid_value = 0.0
    ggg_value = 0.0
    for pid in system.planetIDs:
        p2 = universe.getPlanet(pid)
        if p2:
            if count_asteroids and p2.size == fo.planetSize.asteroids:
                full_bonus = get_named_real("PRO_MICROGRAV_MAN_TARGET_INDUSTRY_FLAT")
                if p2.owner == fo.empireID or pid == planet.id:
                    asteroid_value = full_bonus
                elif p2.unowned:
                    asteroid_value = max(asteroid_value, 0.5 * full_bonus)
                # TODO prospective_invasion_targets?
            if count_ggg:
                ggg_value = max(ggg_value, _ggg_value(planet, p2))
    return asteroid_value + ggg_value


def _ggg_value(planet: fo.planet, p2: fo.planet) -> float:
    """
    Check if p2 not planet and is a gas giant. If determines a value for the potential GGG on p2.
    """
    if p2.id != planet.id and p2.size == fo.planetSize.gasGiant:
        ggg_colony_flat = get_named_real("BLD_GAS_GIANT_GEN_COLONY_TARGET_INDUSTRY_FLAT")
        ggg_outpost_flat = get_named_real("BLD_GAS_GIANT_GEN_OUTPOST_TARGET_INDUSTRY_FLAT")
        # species living on a GG do not like a GGG, so we ignore the planet itself here
        if p2.owner == fo.empireID():
            if not p2.speciesName:
                return ggg_outpost_flat
            if p2.id in BuildingType.GAS_GIANT_GEN.built_or_queued_at():
                return ggg_colony_flat
            else:
                return 0.4 * ggg_colony_flat  # we could built one, but inhabitants won't like it
        if p2.unowned:
            return 0.5 * ggg_colony_flat  # unowned means we could probably get it easily
        # TODO prospective_invasion_targets?
        #  So far we do not consider owned gas giant for planets here
    return 0.0
