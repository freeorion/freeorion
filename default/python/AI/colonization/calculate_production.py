import freeOrionAIInterface as fo
from typing import List

import AIDependencies
from buildings import BuildingType
from colonization.colony_score import debug_rating
from freeorion_tools import get_named_real, get_species_industry, tech_soon_available
from freeorion_tools.bonus_calculation import Bonus
from freeorion_tools.caching import cache_for_current_turn
from turn_state import have_honeycomb


def calculate_production(planet: fo.planet, species: fo.species, max_population: float, stability: float) -> float:
    """
    Calculate how much PP the planet's population could generate with industry focus.
    This only considers values that actually rely on industry focus, those that do not are handled by
    calculate_planet_colonization_rating._rate_focus_independent.
    """
    if stability <= 0.0:
        return 0.0

    bonus_modified = _get_production_bonus_modified(planet, stability)
    skill_multiplier = get_species_industry(species.name)
    bonus_by_policy = _get_production_bonus_mod_by_policy(stability)
    policy_multiplier = _get_policy_multiplier(stability)
    bonus_unmodified = _get_production_bonus_unmodified(planet, stability)
    bonus_flat = _get_production_flat(planet, stability)
    per_population = (
        (AIDependencies.INDUSTRY_PER_POP + bonus_modified) * skill_multiplier + bonus_by_policy
    ) * policy_multiplier + bonus_unmodified
    result = max_population * per_population + bonus_flat
    debug_rating(
        f"calculate_production pop={max_population:.2f}, st={stability:.2f}, b1={bonus_modified:.2f}, "
        f"m1={skill_multiplier:.2f}, b2={bonus_by_policy:.2f}, m2={policy_multiplier:.2f}, "
        f"b3={bonus_unmodified:.2f}, flat={bonus_flat:.2f} -> {result:.2f}"
    )
    return result


@cache_for_current_turn
def _get_modified_industry_bonuses() -> List[Bonus]:
    """
    Get list of per-population bonuses which are added before multiplication with the species skill value.
    """
    # TBD check connection!?
    have_center = bool(BuildingType.INDUSTRY_CENTER.built_or_queued_at())
    centre_bonus1 = get_named_real("BLD_INDUSTRY_CENTER_1_TARGET_INDUSTRY_PERPOP")
    centre_bonus2 = get_named_real("BLD_INDUSTRY_CENTER_2_TARGET_INDUSTRY_PERPOP")
    centre_bonus3 = get_named_real("BLD_INDUSTRY_CENTER_3_TARGET_INDUSTRY_PERPOP")
    return [
        Bonus(
            tech_soon_available("PRO_FUSION_GEN", 3),
            get_named_real("PRO_FUSION_GEN_MIN_STABILITY"),
            get_named_real("PRO_FUSION_GEN_TARGET_INDUSTRY_PERPOP"),
        ),
        Bonus(
            tech_soon_available("GRO_ENERGY_META", 3),
            get_named_real("GRO_ENERGY_META_MIN_STABILITY"),
            get_named_real("GRO_ENERGY_META_TARGET_INDUSTRY_PERPOP"),
        ),
        # special case: these are not cumulative
        Bonus(
            have_center and tech_soon_available("PRO_INDUSTRY_CENTER_III", 1),  # expensive research
            get_named_real("BLD_INDUSTRY_CENTER_3_MIN_STABILITY"),
            centre_bonus3 - centre_bonus2,
        ),
        Bonus(
            have_center and tech_soon_available("PRO_INDUSTRY_CENTER_II", 2),
            get_named_real("BLD_INDUSTRY_CENTER_2_MIN_STABILITY"),
            centre_bonus2 - centre_bonus1,
        ),
        Bonus(
            # normally we have the tech when we have the building, but we could have conquered one
            have_center and tech_soon_available("PRO_INDUSTRY_CENTER_II", 3),
            get_named_real("BLD_INDUSTRY_CENTER_1_MIN_STABILITY"),
            centre_bonus1,
        ),
    ]


def _get_production_bonus_modified(planet: fo.planet, stability: float) -> float:
    """
    Calculate bonus production per population which would be added before multiplication with the species skill value.
    """
    specials_bonus = sum(
        AIDependencies.INDUSTRY_PER_POP for s in planet.specials if s in AIDependencies.industry_boost_specials_modified
    )
    return specials_bonus + sum(bonus.get_bonus(stability) for bonus in _get_modified_industry_bonuses())


def _get_policy_multiplier(stability) -> float:
    if fo.getEmpire().policyAdopted("INDUSTRIALISM") and stability >= get_named_real("PLC_INDUSTRIALISM_MIN_STABILITY"):
        return 1.0 + get_named_real("PLC_INDUSTRIALISM_TARGET_INDUSTRY_PERCENT")
    return 1.0


def _get_production_bonus_mod_by_policy(stability: float) -> float:
    """
    Calculate bonus production per population which we would get independent of the species production skill,
    but still affected by industrialism.
    """
    # TBD: check connections?
    bonuses = [
        Bonus(
            bool(BuildingType.BLACK_HOLE_POW_GEN.built_or_queued_at()),
            get_named_real("BLD_BLACK_HOLE_POW_GEN_MIN_STABILITY"),
            get_named_real("BLD_BLACK_HOLE_POW_GEN_TARGET_INDUSTRY_PERPOP"),
        ),
    ]
    return sum(bonus.get_bonus(stability) for bonus in bonuses)


def _get_production_bonus_unmodified(planet: fo.planet, stability: float) -> float:
    """
    Calculate bonus production per population which we would get independent of the species production skill.
    """
    specials_bonus = sum(
        # growth.macros: STANDARD_INDUSTRY_BOOST
        AIDependencies.INDUSTRY_PER_POP
        for s in planet.specials
        if s in AIDependencies.industry_boost_specials_unmodified
    )

    # TBD: check connections?
    bonuses = [
        Bonus(
            bool(BuildingType.SOL_ORB_GEN.built_or_queued_at()),  # TBD: check star type
            get_named_real("BLD_SOL_ORB_GEN_MIN_STABILITY"),
            get_named_real("BLD_SOL_ORB_GEN_BRIGHT_TARGET_INDUSTRY_PERPOP"),
        ),
        Bonus(
            have_honeycomb(),
            0.0,
            get_named_real("HONEYCOMB_TARGET_INDUSTRY_PERPOP"),
        ),
        # TBD: Collective Thought Network? Ignored for now, AI doesn't know how to handle it.
        # It shouldn't make a big difference for selecting which planets to colonise anyway.
    ]
    return specials_bonus + sum(bonus.get_bonus(stability) for bonus in bonuses)


def _get_production_flat(planet: fo.planet, stability: float) -> float:
    """
    Calculate population independent production bonus.
    """
    value = _get_asteroid_and_ggg_value(planet, stability)
    # this is a rather expensive one, so consider it only if it is on top of the list
    if tech_soon_available(AIDependencies.PRO_AUTO_2, 1):
        value += get_named_real("PRO_SENTIENT_AUTO_TARGET_INDUSTRY_FLAT")
    return value


def _get_asteroid_and_ggg_value(planet: fo.planet, stability: float) -> float:
    """
    Calculate an estimate of the bonus we may get from asteroids (microgravity) and gas giant generators.
    """
    universe = fo.getUniverse()
    system = universe.getSystem(planet.systemID)
    ast_min_stability = get_named_real("PRO_MICROGRAV_MAN_MIN_STABILITY")
    count_asteroids = ast_min_stability <= stability and tech_soon_available(AIDependencies.PRO_MICROGRAV_MAN, 5)
    ggg_min_stability = get_named_real("BLD_GAS_GIANT_GEN_MIN_STABILITY")
    count_ggg = ggg_min_stability <= stability and tech_soon_available(AIDependencies.PRO_ORBITAL_GEN, 5)
    asteroid_value = 0.0
    ggg_value = 0.0
    for pid in system.planetIDs:
        p2 = universe.getPlanet(pid)
        if p2:
            if count_asteroids and p2.size == fo.planetSize.asteroids:
                full_bonus = get_named_real("PRO_MICROGRAV_MAN_TARGET_INDUSTRY_FLAT")
                if p2.owner == fo.empireID or pid == planet.id:
                    asteroid_value = full_bonus
                # If we cannot see it, we cannot put an outpost on it, although in case of asteroids it likely
                # only hidden temporarily.
                elif universe.getVisibility(pid, fo.empireID()) and p2.unowned:
                    asteroid_value = max(asteroid_value, 0.5 * full_bonus)
                # TODO prospective_invasion_targets?
            if count_ggg:
                ggg_value = max(ggg_value, _ggg_value(planet, p2))
    debug_rating(
        f"_get_asteroid_and_ggg_value amin={ast_min_stability:.1f}->{count_asteroids:.1f}, "
        f"gmin={ggg_min_stability:.1f}->{count_ggg:.1f}, aval={asteroid_value:.1f}, gval={ggg_value:.1f}"
    )
    return asteroid_value + ggg_value


def _ggg_value(planet: fo.planet, p2: fo.planet) -> float:
    """
    Check if p2 not planet and is a gas giant. If determines a value for the potential GGG on p2.
    """
    universe = fo.getUniverse()
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
                return 0.4 * ggg_colony_flat  # we could build one, but inhabitants won't like it
        # If we cannot see it, we cannot put an outpost on it, most likely it is not really unowned
        if universe.getVisibility(p2.id, fo.empireID()) and p2.unowned:
            return 0.5 * ggg_colony_flat  # unowned means we could probably get it easily
        # TODO prospective_invasion_targets?
        #  So far we do not consider owned gas giant for planets here
    return 0.0
