import freeOrionAIInterface as fo
from logging import debug, error
from typing import Optional

import AIDependencies
import AIstate
import EspionageAI
import InvasionAI
import MilitaryAI
import PlanetUtilsAI
import PriorityAI
from AIDependencies import INVALID_ID, Tags
from aistate_interface import get_aistate
from colonization import get_nest_rating, update_planet_supply
from colonization.calculate_population import calc_max_pop
from colonization.claimed_stars import (
    count_claimed_stars,
    has_claimed_star,
    is_system_star_claimed,
)
from colonization.colony_score import MINIMUM_COLONY_SCORE
from colonization.rate_pilots import rate_piloting_tag
from common.fo_typing import PlanetId, SpeciesName
from common.print_utils import Sequence
from empire.pilot_rating import best_pilot_rating
from EnumsAI import FocusType, MissionType
from freeorion_tools import (
    get_partial_visibility_turn,
    get_species_tag_grade,
    tech_is_complete,
)
from freeorion_tools.caching import cache_by_turn_persistent, cache_for_session
from turn_state import (
    get_inhabited_planets,
    get_owned_planets,
    get_owned_planets_in_system,
    get_system_supply,
    have_computronium,
    population_with_industry_focus,
    population_with_research_focus,
)

empire_metabolisms = {}


def calculate_planet_colonization_rating(
    *,
    planet_id: PlanetId,
    mission_type: MissionType,
    spec_name: Optional[SpeciesName],
    detail: Optional[list],
    empire_research_list: Optional[Sequence],
) -> float:
    """Returns the colonisation value of a planet."""
    if detail is None:
        detail = []

    if spec_name is None:
        spec_name = ""

    if empire_research_list is None:
        empire = fo.getEmpire()
        empire_research_list = tuple(element.tech for element in empire.researchQueue)

    return _calculate_planet_colonization_rating(planet_id, mission_type, spec_name, detail, empire_research_list)


def _calculate_planet_colonization_rating(
    planet_id: PlanetId,
    mission_type: MissionType,
    species_name: SpeciesName,
    detail: list,
    empire_research_list: Sequence,
) -> float:
    empire = fo.getEmpire()
    retval = 0
    character = get_aistate().character
    discount_multiplier = character.preferred_discount_multiplier([30.0, 40.0])
    species = fo.getSpecies(species_name)
    species_foci = [] and species and list(species.foci)
    tag_list = list(species.tags) if species else []
    pilot_val = pilot_rating = 0
    if species and species.canProduceShips:
        pilot_val = pilot_rating = rate_piloting_tag(species_name)
        if pilot_val > best_pilot_rating():
            pilot_val *= 2
        if pilot_val > 2:
            retval += discount_multiplier * 5 * pilot_val
            detail.append("Pilot Val %.1f" % (discount_multiplier * 5 * pilot_val))

    if empire.productionPoints < 100:
        backup_factor = 0.0
    else:
        backup_factor = min(1.0, (empire.productionPoints / 200.0) ** 2)

    universe = fo.getUniverse()
    capital_id = PlanetUtilsAI.get_capital()
    homeworld = universe.getPlanet(capital_id)
    planet = universe.getPlanet(planet_id)
    prospective_invasion_targets = [
        pid
        for pid, pscore, trp in AIstate.invasionTargets[: PriorityAI.allotted_invasion_targets()]
        if pscore > InvasionAI.MIN_INVASION_SCORE
    ]

    if species_name != planet.speciesName and planet.speciesName and mission_type != MissionType.INVASION:
        return 0

    this_sysid = planet.systemID
    if homeworld:
        home_system_id = homeworld.systemID
        eval_system_id = this_sysid
        if home_system_id != INVALID_ID and eval_system_id != INVALID_ID:
            least_jumps = universe.jumpDistance(home_system_id, eval_system_id)
            if least_jumps == -1:  # indicates no known path
                return 0.0

    if planet is None:
        vis_map = universe.getVisibilityTurnsMap(planet_id, empire.empireID)
        debug("Planet %d object not available; visMap: %s" % (planet_id, vis_map))
        return 0
    # only count existing presence if not target planet
    # TODO: consider neighboring sytems for smaller contribution, and bigger contributions for
    # local colonies versus local outposts
    locally_owned_planets = [lpid for lpid in get_owned_planets_in_system(this_sysid) if lpid != planet_id]
    planets_with_species = get_inhabited_planets()
    locally_owned_pop_ctrs = [lpid for lpid in locally_owned_planets if lpid in planets_with_species]
    # triple count pop_ctrs
    existing_presence = len(locally_owned_planets) + 2 * len(locally_owned_pop_ctrs)
    system = universe.getSystem(this_sysid)

    sys_supply = get_system_supply(this_sysid)
    planet_supply = AIDependencies.supply_by_size.get(planet.size, 0)
    bld_types = set(universe.getBuilding(bldg).buildingTypeName for bldg in planet.buildingIDs).intersection(
        AIDependencies.building_supply
    )
    planet_supply += sum(
        AIDependencies.building_supply[bld_type].get(int(psize), 0)
        for psize in [-1, planet.size]
        for bld_type in bld_types
    )

    supply_specials = set(planet.specials).union(system.specials).intersection(AIDependencies.SUPPLY_MOD_SPECIALS)
    planet_supply += sum(
        AIDependencies.SUPPLY_MOD_SPECIALS[_special].get(int(psize), 0)
        for _special in supply_specials
        for psize in [-1, planet.size]
    )

    ind_tag_mod = AIDependencies.SPECIES_INDUSTRY_MODIFIER.get(get_species_tag_grade(species_name, Tags.INDUSTRY), 1.0)
    res_tag_mod = AIDependencies.SPECIES_RESEARCH_MODIFIER.get(get_species_tag_grade(species_name, Tags.RESEARCH), 1.0)
    if species:
        supply_tag_mod = AIDependencies.SPECIES_SUPPLY_MODIFIER.get(get_species_tag_grade(species_name, Tags.SUPPLY), 1)
    else:
        supply_tag_mod = 0

    # determine the potential supply provided by owning this planet, and if the planet is currently populated by
    # the evaluated species, then save this supply value in a cache.
    # The system supply value can be negative (indicates the respective number of starlane jumps to the closest supplied
    # system).  So if this total planet supply value is non-negative, then the planet would be supply connected (to at
    # least a portion of the existing empire) if the planet becomes owned by the AI.

    planet_supply += supply_tag_mod
    planet_supply = max(planet_supply, 0)  # planets can't have negative supply
    if planet.speciesName == species_name:
        update_planet_supply(planet_id, planet_supply + sys_supply)

    threat_factor = _determine_colony_threat_factor(planet_id, species_name, existing_presence)

    sys_partial_vis_turn = get_partial_visibility_turn(this_sysid)
    planet_partial_vis_turn = get_partial_visibility_turn(planet_id)

    if planet_partial_vis_turn < sys_partial_vis_turn:
        # last time we had partial vis of the system, the planet was stealthed to us
        # print "Colonization AI couldn't get current info on planet id %d (was stealthed at last sighting)" % planet_id
        return 0  # TODO: track detection strength, order new scouting when it goes up

    star_bonus = 0
    colony_star_bonus = 0
    research_bonus = 0
    growth_val = 0
    fixed_ind = 0
    fixed_res = 0
    if system:
        already_got_this_one = this_sysid in get_owned_planets()
        # TODO: Should probably consider pilot rating also for Phototropic species
        if "PHOTOTROPHIC" not in tag_list and pilot_rating >= best_pilot_rating():
            if system.starType == fo.starType.red and tech_is_complete("LRN_STELLAR_TOMOGRAPHY"):
                star_bonus += 40 * discount_multiplier  # can be used for artif'l black hole and solar hull
                detail.append("Red Star for Art Black Hole for solar hull %.1f" % (40 * discount_multiplier))
            elif system.starType == fo.starType.blackHole and tech_is_complete("SHP_FRC_ENRG_COMP"):
                star_bonus += 100 * discount_multiplier  # can be used for solar hull
                detail.append("Black Hole for solar hull %.1f" % (100 * discount_multiplier))

        if tech_is_complete("PRO_SOL_ORB_GEN") or "PRO_SOL_ORB_GEN" in empire_research_list[:5]:
            if system.starType in [fo.starType.blue, fo.starType.white]:
                if has_claimed_star(fo.starType.blue, fo.starType.white):
                    star_bonus += 20 * discount_multiplier
                    detail.append("PRO_SOL_ORB_GEN BW %.1f" % (20 * discount_multiplier))
                elif not already_got_this_one:
                    # still has extra value as an alternate location for solar generators
                    star_bonus += 10 * discount_multiplier * backup_factor
                    detail.append(
                        "PRO_SOL_ORB_GEN BW Backup Location %.1f" % (10 * discount_multiplier * backup_factor)
                    )
                elif fo.currentTurn() > 100:  # lock up this whole system
                    pass
                    # starBonus += 5  # TODO: how much?
                    # detail.append("PRO_SOL_ORB_GEN BW LockingDownSystem %.1f"%5)
            if system.starType in [fo.starType.yellow, fo.starType.orange]:
                if has_claimed_star(fo.starType.blue, fo.starType.white, fo.starType.yellow, fo.starType.orange):
                    star_bonus += 10 * discount_multiplier
                    detail.append("PRO_SOL_ORB_GEN YO %.1f" % (10 * discount_multiplier))
                else:
                    pass
                    # starBonus +=2  # still has extra value as an alternate location for solar generators
                    # detail.append("PRO_SOL_ORB_GEN YO Backup %.1f" % 2)
        if system.starType in [fo.starType.blackHole] and fo.currentTurn() > 100:
            if not already_got_this_one:
                # whether have tech yet or not, assign some base value
                star_bonus += 10 * discount_multiplier * backup_factor
                detail.append("Black Hole %.1f" % (10 * discount_multiplier * backup_factor))
            else:
                star_bonus += 5 * discount_multiplier * backup_factor
                detail.append("Black Hole Backup %.1f" % (5 * discount_multiplier * backup_factor))
        if tech_is_complete(AIDependencies.PRO_SOL_ORB_GEN):  # start valuing as soon as PRO_SOL_ORB_GEN done
            if system.starType == fo.starType.blackHole:
                # pretty rare planets, good for generator
                this_val = 0.5 * max(population_with_industry_focus(), 20) * discount_multiplier
                if has_claimed_star(fo.starType.blackHole):
                    star_bonus += this_val
                    detail.append("PRO_SINGULAR_GEN %.1f" % this_val)
                elif not is_system_star_claimed(system):
                    # still has extra value as an alternate location for generators & for blocking enemies generators
                    star_bonus += this_val * backup_factor
                    detail.append("PRO_SINGULAR_GEN Backup %.1f" % (this_val * backup_factor))
            elif system.starType == fo.starType.red and has_claimed_star(fo.starType.blackHole):
                rfactor = (1.0 + count_claimed_stars(fo.starType.red)) ** -2
                star_bonus += 40 * discount_multiplier * backup_factor * rfactor  # can be used for artif'l black hole
                detail.append("Red Star for Art Black Hole %.1f" % (40 * discount_multiplier * backup_factor * rfactor))
        if tech_is_complete("PRO_NEUTRONIUM_EXTRACTION") or "PRO_NEUTRONIUM_EXTRACTION" in empire_research_list[:8]:
            if system.starType in [fo.starType.neutron]:
                if has_claimed_star(fo.starType.neutron):
                    star_bonus += 80 * discount_multiplier  # pretty rare planets, good for armor
                    detail.append("PRO_NEUTRONIUM_EXTRACTION %.1f" % (80 * discount_multiplier))
                else:
                    # still has extra value as an alternate location for generators & for bnlocking enemies generators
                    star_bonus += 20 * discount_multiplier * backup_factor
                    detail.append("PRO_NEUTRONIUM_EXTRACTION Backup %.1f" % (20 * discount_multiplier * backup_factor))
        if tech_is_complete("SHP_ENRG_BOUND_MAN") or "SHP_ENRG_BOUND_MAN" in empire_research_list[:6]:
            # TODO: base this on pilot val, and also consider red stars
            if system.starType in [fo.starType.blackHole, fo.starType.blue]:
                init_val = 100 * discount_multiplier * (pilot_val or 1)
                if has_claimed_star(fo.starType.blackHole, fo.starType.blue):
                    colony_star_bonus += init_val  # pretty rare planets, good for energy shipyards
                    detail.append("SHP_ENRG_BOUND_MAN %.1f" % init_val)
                elif not is_system_star_claimed(system):
                    # still has extra value as an alternate location for energy shipyard
                    colony_star_bonus += 0.5 * init_val * backup_factor
                    detail.append("SHP_ENRG_BOUND_MAN Backup %.1f" % (0.5 * init_val * backup_factor))
    retval += star_bonus

    planet_specials = list(planet.specials)
    if "ECCENTRIC_ORBIT_SPECIAL" in planet.specials:
        fixed_res += discount_multiplier * 6
        detail.append("ECCENTRIC_ORBIT_SPECIAL %.1f" % (discount_multiplier * 6))

    if mission_type == MissionType.OUTPOST or (mission_type == MissionType.INVASION and not species_name):

        if "ANCIENT_RUINS_SPECIAL" in planet.specials:  # TODO: add value for depleted ancient ruins
            retval += discount_multiplier * 30
            detail.append("Undepleted Ruins %.1f" % discount_multiplier * 30)

        for special in planet_specials:
            if "_NEST_" in special:
                nest_val = get_nest_rating(special, 5.0) * discount_multiplier  # get an outpost on the nest quick
                retval += nest_val
                detail.append("%s %.1f" % (special, nest_val))
            elif special == "FORTRESS_SPECIAL":
                fort_val = 10 * discount_multiplier
                retval += fort_val
                detail.append("%s %.1f" % (special, fort_val))
            elif special == "HONEYCOMB_SPECIAL":
                honey_val = 0.3 * (
                    AIDependencies.HONEYCOMB_IND_MULTIPLIER
                    * AIDependencies.INDUSTRY_PER_POP
                    * population_with_industry_focus()
                    * discount_multiplier
                )
                retval += honey_val
                detail.append("%s %.1f" % (special, honey_val))
        if planet.size == fo.planetSize.asteroids:
            ast_val = 0
            if system:
                for pid in system.planetIDs:
                    other_planet = universe.getPlanet(pid)
                    if other_planet.size == fo.planetSize.asteroids:
                        if pid == planet_id:
                            continue
                        elif pid < planet_id and planet.unowned:
                            ast_val = 0
                            break
                    elif other_planet.speciesName:
                        if other_planet.owner == empire.empireID:
                            ownership_factor = 1.0
                        elif pid in prospective_invasion_targets:
                            ownership_factor = character.secondary_valuation_factor_for_invasion_targets()
                        else:
                            ownership_factor = 0.0
                        ast_val += ownership_factor * _base_asteroid_mining_val() * discount_multiplier
                retval += ast_val
                if ast_val > 0:
                    detail.append("AsteroidMining %.1f" % ast_val)
            ast_val = 0
            if tech_is_complete("SHP_ASTEROID_HULLS"):
                per_ast = 20
            elif tech_is_complete("CON_ORBITAL_CON"):
                per_ast = 5
            else:
                per_ast = 0.1
            if system:
                for pid in system.planetIDs:
                    other_planet = universe.getPlanet(pid)
                    if other_planet.size == fo.planetSize.asteroids:
                        if pid == planet_id:
                            continue
                        elif pid < planet_id and planet.unowned:
                            ast_val = 0
                            break
                    elif other_planet.speciesName:
                        other_species = fo.getSpecies(other_planet.speciesName)
                        if other_species and other_species.canProduceShips:
                            if other_planet.owner == empire.empireID:
                                ownership_factor = 1.0
                            elif pid in prospective_invasion_targets:
                                ownership_factor = character.secondary_valuation_factor_for_invasion_targets()
                            else:
                                ownership_factor = 0.0
                            ast_val += ownership_factor * per_ast * discount_multiplier
                retval += ast_val
                if ast_val > 0:
                    detail.append("AsteroidShipBuilding %.1f" % ast_val)
        # We will assume that if any GG in the system is populated, they all will wind up populated; cannot then hope
        # to build a GGG on a non-populated GG
        populated_gg_factor = 1.0
        per_gg = 0
        if planet.size == fo.planetSize.gasGiant:
            # TODO: Given current industry calc approach, consider bringing this max val down to actual max val of 10
            if tech_is_complete("PRO_ORBITAL_GEN"):
                per_gg = 20
            elif tech_is_complete("CON_ORBITAL_CON"):
                per_gg = 10
            if species_name:
                populated_gg_factor = 0.5
        else:
            per_gg = 5
        if system:
            gg_list = []
            orb_gen_val = 0
            gg_detail = []
            for pid in system.planetIDs:
                other_planet = universe.getPlanet(pid)
                if other_planet.size == fo.planetSize.gasGiant:
                    gg_list.append(pid)
                    if other_planet.speciesName:
                        populated_gg_factor = 0.5
                if (
                    pid != planet_id
                    and other_planet.owner == empire.empireID
                    and FocusType.FOCUS_INDUSTRY in list(other_planet.availableFoci) + [other_planet.focus]
                ):
                    orb_gen_val += per_gg * discount_multiplier
                    # Note, this reported value may not take into account a later adjustment from a populated gg
                    gg_detail.append(
                        "GGG for %s %.1f" % (other_planet.name, discount_multiplier * per_gg * populated_gg_factor)
                    )
            if planet_id in sorted(gg_list)[:1]:
                retval += orb_gen_val * populated_gg_factor
                detail.extend(gg_detail)
            else:
                detail.append("Won't GGG")
        if existing_presence:
            detail.append("preexisting system colony")
            retval = (retval + existing_presence * _get_defense_value(species_name)) * 1.5

        # Fixme - sys_supply is always <= 0 leading to incorrect supply bonus score
        supply_val = 0
        if sys_supply < 0:
            if sys_supply + planet_supply >= 0:
                supply_val += 30 * (planet_supply - max(-3, sys_supply))
            else:
                retval += 30 * (planet_supply + sys_supply)  # a penalty
        elif planet_supply > sys_supply and (sys_supply < 2):  # TODO: check min neighbor supply
            supply_val += 25 * (planet_supply - sys_supply)
        detail.append("sys_supply: %d, planet_supply: %d, supply_val: %.0f" % (sys_supply, planet_supply, supply_val))
        retval += supply_val

        if threat_factor < 1.0:
            threat_factor = _revise_threat_factor(threat_factor, retval, this_sysid, MINIMUM_COLONY_SCORE)
            retval *= threat_factor
            detail.append("threat reducing value by %3d %%" % (100 * (1 - threat_factor)))
        return int(retval)
    else:  # colonization mission
        if not species:
            return 0
        supply_val = 0
        if "ANCIENT_RUINS_SPECIAL" in planet.specials:
            retval += discount_multiplier * 50
            detail.append("Undepleted Ruins %.1f" % discount_multiplier * 50)
        if "HONEYCOMB_SPECIAL" in planet.specials:
            honey_val = (
                AIDependencies.HONEYCOMB_IND_MULTIPLIER
                * AIDependencies.INDUSTRY_PER_POP
                * population_with_industry_focus()
                * discount_multiplier
            )
            if FocusType.FOCUS_INDUSTRY not in species_foci:
                honey_val *= -0.3  # discourage settlement by colonizers not able to use Industry Focus
            retval += honey_val
            detail.append("%s %.1f" % ("HONEYCOMB_SPECIAL", honey_val))

        # Fixme - sys_supply is always <= 0 leading to incorrect supply bonus score
        if sys_supply <= 0:
            if sys_supply + planet_supply >= 0:
                supply_val = 40 * (planet_supply - max(-3, sys_supply))
            else:
                supply_val = 200 * (planet_supply + sys_supply)  # a penalty
                if species_name == "SP_SLY":
                    # Sly are essentially stuck with lousy supply, so don't penalize for that
                    supply_val = 0
        elif planet_supply > sys_supply == 1:  # TODO: check min neighbor supply
            supply_val = 20 * (planet_supply - sys_supply)
        detail.append("sys_supply: %d, planet_supply: %d, supply_val: %.0f" % (sys_supply, planet_supply, supply_val))

        # if AITags != "":
        # print "Species %s has AITags %s"%(specName, AITags)

        retval += fixed_res
        retval += colony_star_bonus
        asteroid_bonus = 0
        gas_giant_bonus = 0
        flat_industry = 0
        mining_bonus = 0
        per_ggg = 10

        asteroid_factor = 0.0
        gg_factor = 0.0
        ast_shipyard_name = ""
        if system and FocusType.FOCUS_INDUSTRY in species.foci:
            for pid in system.planetIDs:
                if pid == planet_id:
                    continue
                p2 = universe.getPlanet(pid)
                if p2:
                    if p2.size == fo.planetSize.asteroids:
                        this_factor = 0.0
                        if p2.owner == empire.empireID:
                            this_factor = 1.0
                        elif p2.unowned:
                            this_factor = 0.5
                        elif pid in prospective_invasion_targets:
                            this_factor = character.secondary_valuation_factor_for_invasion_targets()
                        if this_factor > asteroid_factor:
                            asteroid_factor = this_factor
                            ast_shipyard_name = p2.name
                    if p2.size == fo.planetSize.gasGiant:
                        if p2.owner == empire.empireID:
                            gg_factor = max(gg_factor, 1.0)
                        elif p2.unowned:
                            gg_factor = max(gg_factor, 0.5)
                        elif pid in prospective_invasion_targets:
                            gg_factor = max(gg_factor, character.secondary_valuation_factor_for_invasion_targets())
        if asteroid_factor > 0.0:
            if tech_is_complete("PRO_MICROGRAV_MAN") or "PRO_MICROGRAV_MAN" in empire_research_list[:10]:
                flat_industry += 2 * asteroid_factor  # will go into detailed industry projection
                detail.append("Asteroid mining ~ %.1f" % (5 * asteroid_factor * discount_multiplier))
            if tech_is_complete("SHP_ASTEROID_HULLS") or "SHP_ASTEROID_HULLS" in empire_research_list[:11]:
                if species and species.canProduceShips:
                    asteroid_bonus = 30 * discount_multiplier * pilot_val
                    detail.append(
                        "Asteroid ShipBuilding from %s %.1f" % (ast_shipyard_name, discount_multiplier * 30 * pilot_val)
                    )
        if gg_factor > 0.0:
            if tech_is_complete("PRO_ORBITAL_GEN") or "PRO_ORBITAL_GEN" in empire_research_list[:5]:
                flat_industry += per_ggg * gg_factor  # will go into detailed industry projection
                detail.append("GGG ~ %.1f" % (per_ggg * gg_factor * discount_multiplier))

        # calculate the maximum population of the species on that planet.
        if planet.speciesName not in AIDependencies.SPECIES_FIXED_POPULATION:
            max_pop_size = calc_max_pop(planet, species, detail)
        else:
            max_pop_size = AIDependencies.SPECIES_FIXED_POPULATION[planet.speciesName]
            detail.append("Fixed max population of %.2f" % max_pop_size)

        if max_pop_size <= 0:
            detail.append(
                "Non-positive population projection for species '%s', so no colonization value"
                % (species and species.name)
            )
            return 0

        for special in ["MINERALS_SPECIAL", "CRYSTALS_SPECIAL", "ELERIUM_SPECIAL"]:
            if special in planet_specials:
                mining_bonus += 1

        has_blackhole = has_claimed_star(fo.starType.blackHole)
        ind_tech_map_flat = AIDependencies.INDUSTRY_EFFECTS_FLAT_NOT_MODIFIED_BY_SPECIES
        ind_tech_map_before_species_mod = AIDependencies.INDUSTRY_EFFECTS_PER_POP_MODIFIED_BY_SPECIES
        ind_tech_map_after_species_mod = AIDependencies.INDUSTRY_EFFECTS_PER_POP_NOT_MODIFIED_BY_SPECIES

        ind_mult = 1
        for tech in ind_tech_map_before_species_mod:
            if tech_is_complete(tech) and (tech != AIDependencies.PRO_SINGULAR_GEN or has_blackhole):
                ind_mult += ind_tech_map_before_species_mod[tech]

        ind_mult = ind_mult * max(
            ind_tag_mod, 0.5 * (ind_tag_mod + res_tag_mod)
        )  # TODO: report an actual calc for research value

        for tech in ind_tech_map_after_species_mod:
            if tech_is_complete(tech) and (tech != AIDependencies.PRO_SINGULAR_GEN or has_blackhole):
                ind_mult += ind_tech_map_after_species_mod[tech]

        max_ind_factor = 0
        for tech in ind_tech_map_flat:
            if tech_is_complete(tech):
                fixed_ind += discount_multiplier * ind_tech_map_flat[tech]

        if FocusType.FOCUS_INDUSTRY in species.foci:
            if "TIDAL_LOCK_SPECIAL" in planet.specials:
                ind_mult += 1
            max_ind_factor += AIDependencies.INDUSTRY_PER_POP * mining_bonus
            max_ind_factor += AIDependencies.INDUSTRY_PER_POP * ind_mult
        cur_pop = 1.0  # assume an initial colonization value
        if planet.speciesName != "":
            cur_pop = planet.currentMeterValue(fo.meterType.population)
        elif tech_is_complete("GRO_LIFECYCLE_MAN"):
            cur_pop = 3.0
        cur_industry = planet.currentMeterValue(fo.meterType.industry)
        ind_val = _project_ind_val(
            cur_pop, max_pop_size, cur_industry, max_ind_factor, flat_industry, discount_multiplier
        )
        detail.append("ind_val %.1f" % ind_val)
        # used to give preference to closest worlds

        for special in [spec for spec in planet_specials if spec in AIDependencies.metabolismBoosts]:
            # TODO: also consider potential future benefit re currently unpopulated planets
            gbonus = (
                discount_multiplier
                * AIDependencies.INDUSTRY_PER_POP
                * ind_mult
                * empire_metabolisms.get(AIDependencies.metabolismBoosts[special], 0)
            )  # due to growth applicability to other planets
            growth_val += gbonus
            detail.append("Bonus for %s: %.1f" % (special, gbonus))

        if FocusType.FOCUS_RESEARCH in species.foci:
            research_bonus += discount_multiplier * 2 * AIDependencies.RESEARCH_PER_POP * max_pop_size
            if "ANCIENT_RUINS_SPECIAL" in planet.specials or "ANCIENT_RUINS_DEPLETED_SPECIAL" in planet.specials:
                research_bonus += discount_multiplier * 2 * AIDependencies.RESEARCH_PER_POP * max_pop_size * 5
                detail.append("Ruins Research")
            if "TEMPORAL_ANOMALY_SPECIAL" in planet.specials:
                research_bonus += discount_multiplier * 2 * AIDependencies.RESEARCH_PER_POP * max_pop_size * 25
                detail.append("Temporal Anomaly Research")
            if AIDependencies.COMPUTRONIUM_SPECIAL in planet.specials:
                comp_bonus = (
                    0.5
                    * AIDependencies.TECH_COST_MULTIPLIER
                    * AIDependencies.RESEARCH_PER_POP
                    * AIDependencies.COMPUTRONIUM_RES_MULTIPLIER
                    * population_with_research_focus()
                    * discount_multiplier
                )
                if have_computronium():
                    comp_bonus *= backup_factor
                research_bonus += comp_bonus
                detail.append(AIDependencies.COMPUTRONIUM_SPECIAL)

        retval += (
            max(ind_val + asteroid_bonus + gas_giant_bonus, research_bonus, growth_val)
            + fixed_ind
            + fixed_res
            + supply_val
        )
        if existing_presence:
            detail.append("preexisting system colony")
            retval = (retval + existing_presence * _get_defense_value(species_name)) * 2
        if threat_factor < 1.0:
            threat_factor = _revise_threat_factor(threat_factor, retval, this_sysid, MINIMUM_COLONY_SCORE)
            retval *= threat_factor
            detail.append("threat reducing value by %3d %%" % (100 * (1 - threat_factor)))
    return retval


def _determine_colony_threat_factor(planet_id, spec_name, existing_presence):
    universe = fo.getUniverse()
    planet = universe.getPlanet(planet_id)
    if not planet:  # should have been checked previously, but belt-and-suspenders
        error("Can't retrieve planet ID %d" % planet_id)
        return 0
    sys_status = get_aistate().systemStatus.get(planet.systemID, {})
    cur_best_mil_ship_rating = max(MilitaryAI.cur_best_mil_ship_rating(), 0.001)
    local_defenses = sys_status.get("all_local_defenses", 0)
    local_threat = sys_status.get("fleetThreat", 0) + sys_status.get("monsterThreat", 0)
    neighbor_threat = sys_status.get("neighborThreat", 0)
    jump2_threat = 0.6 * max(0, sys_status.get("jump2_threat", 0) - sys_status.get("my_neighbor_rating", 0))
    area_threat = neighbor_threat + jump2_threat
    area_threat *= 2.0 / (existing_presence + 2)  # once we have a foothold be less scared off by area threats
    # TODO: evaluate detectability by specific source of area threat, also consider if the subject AI already has
    # researched techs that would grant a stealth bonus
    local_enemies = sys_status.get("enemies_nearly_supplied", [])
    # could more conservatively base detection on get_aistate().misc.get("observed_empires")
    if not EspionageAI.colony_detectable_by_empire(planet_id, spec_name, empire=local_enemies, default_result=True):
        area_threat *= 0.05
    net_threat = max(0, local_threat + area_threat - local_defenses)
    # even if our military has lost all warships, rate planets as if we have at least one
    reference_rating = max(
        cur_best_mil_ship_rating,
        MilitaryAI.get_preferred_max_military_portion_for_single_battle()
        * MilitaryAI.get_concentrated_tot_mil_rating(),
    )
    threat_factor = min(1.0, reference_rating / (net_threat + 0.001)) ** 2
    if threat_factor < 0.5:
        mil_ref_string = "Military rating reference: %.1f" % reference_rating
        debug(
            "Significant threat discounting %2d%% at %s, local defense: %.1f, local threat %.1f, area threat %.1f"
            % (100 * (1 - threat_factor), planet.name, local_defenses, local_threat, area_threat)
            + mil_ref_string
        )
    return threat_factor


def _base_asteroid_mining_val():
    """returns an estimation for the industry value of an asteroid belt for a colony in the system"""
    return 2 if tech_is_complete("PRO_MICROGRAV_MAN") else 1


def _get_defense_value(species_name: str) -> float:
    """
    :param species_name:
    :return: planet defenses contribution towards planet evaluations
    """
    # TODO: assess species defense characteristics
    if species_name:
        return _get_base_colony_defense_value()
    else:
        return _get_base_outpost_defense_value()


@cache_by_turn_persistent
def _get_base_colony_defense_value():
    """
    :return:planet defenses contribution towards planet evaluations
    :rtype float
    """
    # TODO: assess current AI defense technology, compare the resulting planetary rating to the current
    # best military ship design, derive a planet defenses value related to the cost of such a ship

    net_count = sum(tech_is_complete(tech_name) for tech_name in AIDependencies.DEFENSE_DEFENSE_NET_TECHS)
    regen_count = sum(tech_is_complete(tech_name) for tech_name in AIDependencies.DEFENSE_REGEN_TECHS)
    garrison_count = sum(tech_is_complete(tech_name) for tech_name in AIDependencies.DEFENSE_GARRISON_TECHS)
    shield_count = sum(tech_is_complete(tech_name) for tech_name in AIDependencies.DEFENSE_SHIELDS_TECHS)
    # not counting mine techs because their effects are per-system, not per-planet

    # for now, just combing these for rough composite factor
    result = 4 * (0.1 + net_count) * (1 + regen_count / 2.0) * (1 + garrison_count / 4.0) * (1 + shield_count / 2.0)
    return round(result, 2)


def _revise_threat_factor(
    threat_factor: float,
    planet_value: float,
    system_id: int,
    min_planet_value: float = MINIMUM_COLONY_SCORE,
) -> float:
    """
    Check if the threat_factor should be made less severe.

    If the AI does have enough total military to secure this system, and the target has more than minimal value,
    don't let the threat_factor discount the adjusted value below min_planet_value +1, so that if there are no
    other targets the AI could still pursue this one.  Otherwise, scoring pressure from
    MilitaryAI.get_preferred_max_military_portion_for_single_battle might prevent the AI from pursuing a heavily
    defended but still obtainable target even if it has no softer locations available.

    :param threat_factor: the base threat factor
    :param planet_value: the planet score
    :param system_id: the system ID of subject planet
    :param min_planet_value: a floor planet value if the AI has enough military to secure the system
    :return: the (potentially) revised threat_factor
    """

    # the default value below for fleetThreat shouldn't come in to play, but is just to be absolutely sure we don't
    # send colony ships into some system for which we have not evaluated fleetThreat
    system_status = get_aistate().systemStatus.get(system_id, {})
    system_fleet_treat = system_status.get("fleetThreat", 1000)
    # TODO: consider taking area threat into account here.  Arguments can be made both ways, see discussion in PR2069
    sys_total_threat = system_fleet_treat + system_status.get("monsterThreat", 0) + system_status.get("planetThreat", 0)
    if (MilitaryAI.get_concentrated_tot_mil_rating() > sys_total_threat) and (planet_value > 2 * min_planet_value):
        threat_factor = max(threat_factor, (min_planet_value + 1) / planet_value)
    return threat_factor


@cache_by_turn_persistent
def _get_base_outpost_defense_value() -> float:
    """Return planet defenses contribution towards planet evaluations."""
    # TODO: assess current AI defense technology, compare the resulting planetary rating to the current
    # best military ship design, derive a planet defenses value related to the cost of such a ship

    net_count = sum(tech_is_complete(tech_name) for tech_name in AIDependencies.DEFENSE_DEFENSE_NET_TECHS)
    regen_count = sum(tech_is_complete(tech_name) for tech_name in AIDependencies.DEFENSE_REGEN_TECHS)
    garrison_count = sum(tech_is_complete(tech_name) for tech_name in AIDependencies.DEFENSE_GARRISON_TECHS)
    shield_count = sum(tech_is_complete(tech_name) for tech_name in AIDependencies.DEFENSE_SHIELDS_TECHS)
    # not counting mine techs because their effects are per-system, not per-planet

    # for now, just combing these for rough composite factor
    # since outposts have no infrastructure (until late game at least), many of these have less weight
    # than for colonies
    result = 3 * (0.1 + net_count) * (1 + regen_count / 3.0) * (1 + garrison_count / 6.0) * (1 + shield_count / 3.0)

    return round(result, 2)


@cache_for_session
def _project_ind_val(init_pop, max_pop_size, init_industry, max_ind_factor, flat_industry, discount_multiplier):
    """Return a discouted value for a projected industry stream over time with changing population."""
    discount_factor = 0.95
    if discount_multiplier > 1.0:
        discount_factor = 1.0 - 1.0 / discount_multiplier
    cur_pop = float(init_pop)
    cur_ind = float(init_industry)
    ind_val = 0
    val_factor = 1.0
    for turn in range(50):
        cur_pop += _next_turn_pop_change(cur_pop, max_pop_size)
        cur_ind = min(cur_ind + 1, max(0, cur_ind - 1, flat_industry + cur_pop * max_ind_factor))
        val_factor *= discount_factor
        ind_val += val_factor * cur_ind
    return ind_val


def _next_turn_pop_change(cur_pop: float, target_pop: float):
    """
    Population change calc taken from PopCenter.cpp.
    """
    if target_pop > cur_pop:
        pop_change = cur_pop * (target_pop + 1 - cur_pop) / 100
        return min(pop_change, target_pop - cur_pop)
    else:
        pop_change = -(cur_pop - target_pop) / 10
        return max(pop_change, target_pop - cur_pop)
