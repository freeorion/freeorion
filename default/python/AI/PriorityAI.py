import freeOrionAIInterface as fo
import math
from logging import debug
from operator import itemgetter

import AIDependencies
import AIstate
import ColonisationAI
import ExplorationAI
import FleetUtilsAI
import InvasionAI
import MilitaryAI
import PlanetUtilsAI
import ResearchAI
from AIDependencies import INVALID_ID
from aistate_interface import get_aistate
from empire.colony_builders import (
    can_build_colony_for_species,
    can_build_only_sly_colonies,
)
from empire.colony_status import colonies_is_under_attack, colonies_is_under_treat
from EnumsAI import (
    EmpireProductionTypes,
    MissionType,
    PriorityType,
    ShipRoleType,
    get_priority_production_types,
)
from expansion_plans import get_colonisable_outpost_ids, get_colonisable_planet_ids
from freeorion_tools import tech_is_complete
from freeorion_tools.timers import AITimer
from turn_state import (
    get_number_of_colonies,
    get_owned_planets,
    have_asteroids,
    have_gas_giant,
)
from turn_state.design import cur_best_military_design_rating, get_best_ship_info

prioritiees_timer = AITimer("calculate_priorities()")

allottedColonyTargets = 0
allotted_outpost_targets = 0
unmetThreat = 0


def calculate_priorities():
    """Calculates the priorities of the AI player."""
    debug("\n=== Preparing to Calculate Priorities ===")
    prioritiees_timer.start("setting Production Priority")
    aistate = get_aistate()
    # let this one stay fixed & just adjust Research and Influence
    aistate.set_priority(PriorityType.RESOURCE_PRODUCTION, 50)
    # RESOURCE_INFLUENCE priority is calculated in PolicyAI

    debug("\n*** Calculating Research Priority ***\n")
    prioritiees_timer.start("setting Research Priority")
    aistate.set_priority(
        PriorityType.RESOURCE_RESEARCH, _calculate_research_priority()
    )  # TODO: do univ _survey before this

    debug("\n*** Updating Colonization Status ***\n")
    prioritiees_timer.start("Evaluating Colonization Status")
    ColonisationAI.get_colony_fleets()  # TODO sets aistate.colonisablePlanetIDs and many other values used by other modules

    debug("\n*** Updating Invasion Status ***\n")
    prioritiees_timer.start("Evaluating Invasion Status")
    InvasionAI.get_invasion_fleets()  # sets AIstate.opponentPlanetIDs, and AIstate.invasionTargetedPlanetIDs

    debug("\n*** Updating Military Status ***\n")
    prioritiees_timer.start("Evaluating Military Status")
    MilitaryAI.get_military_fleets()

    debug("\n** Calculating Production Priorities ***\n")
    prioritiees_timer.start("reporting Production Priority")
    _calculate_industry_priority()  # purely for reporting purposes
    prioritiees_timer.start("setting Exploration Priority")

    aistate.set_priority(PriorityType.PRODUCTION_EXPLORATION, _calculate_exploration_priority())
    prioritiees_timer.start("setting Colony Priority")
    aistate.set_priority(PriorityType.PRODUCTION_COLONISATION, _calculate_colonisation_priority())
    prioritiees_timer.start("setting Outpost Priority")
    aistate.set_priority(PriorityType.PRODUCTION_OUTPOST, _calculate_outpost_priority())
    prioritiees_timer.start("setting Invasion Priority")
    aistate.set_priority(PriorityType.PRODUCTION_INVASION, _calculate_invasion_priority())
    prioritiees_timer.start("setting Military Priority")
    aistate.set_priority(PriorityType.PRODUCTION_MILITARY, _calculate_military_priority())
    prioritiees_timer.start("setting other priorities")
    aistate.set_priority(PriorityType.PRODUCTION_BUILDINGS, 25)

    prioritiees_timer.stop_print_and_clear()


def _calculate_industry_priority():  # currently only used to print status
    """calculates the demand for industry"""
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    # get current industry production & Target
    industry_production = empire.resourceProduction(fo.resourceType.industry)
    owned_planet_ids = PlanetUtilsAI.get_owned_planets_by_empire()
    planets = (universe.getPlanet(x) for x in owned_planet_ids)
    target_pp = sum(x.currentMeterValue(fo.meterType.targetIndustry) for x in planets)

    # currently, previously set to 50 in calculatePriorities(), this is just for reporting
    industry_priority = get_aistate().get_priority(PriorityType.RESOURCE_PRODUCTION)

    debug("")
    debug(
        f"Industry Production (current/target) : ( {industry_production:.1f} / {target_pp:.1f} ) at turn {fo.currentTurn()}"
    )
    debug("Priority for Industry: %s" % industry_priority)
    return industry_priority


def _calculate_research_priority():  # noqa: C901
    """Calculates the AI empire's demand for research."""
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    current_turn = fo.currentTurn()
    aistate = get_aistate()
    enemies_sighted = aistate.misc.get("enemies_sighted", {})
    recent_enemies = [x for x in enemies_sighted if x > current_turn - 8]

    industry_priority = aistate.get_priority(PriorityType.RESOURCE_PRODUCTION)

    got_algo = tech_is_complete(AIDependencies.LRN_ALGO_ELEGANCE)
    got_quant = tech_is_complete(AIDependencies.LRN_QUANT_NET)
    research_queue_list = ResearchAI.get_research_queue_techs()
    orb_gen_tech = AIDependencies.PRO_ORBITAL_GEN
    got_orb_gen = tech_is_complete(orb_gen_tech)
    mgrav_prod_tech = AIDependencies.PRO_MICROGRAV_MAN
    got_mgrav_prod = tech_is_complete(mgrav_prod_tech)
    # got_solar_gen = tech_is_complete(AIDependencies.PRO_SOL_ORB_GEN)

    milestone_techs = [
        "PRO_ADAPTIVE_AUTOMATION",
        "LRN_DISTRIB_THOUGHT",
        "LRN_QUANT_NET",
        "SHP_WEAPON_2_4",
        "SHP_WEAPON_3_2",
        "SHP_WEAPON_4_2",
    ]
    milestones_done = [mstone for mstone in milestone_techs if tech_is_complete(mstone)]
    debug("Research Milestones accomplished at turn %d: %s" % (current_turn, milestones_done))

    total_pp = empire.productionPoints
    total_rp = empire.resourceProduction(fo.resourceType.research)
    industry_surge = (
        aistate.character.may_surge_industry(total_pp, total_rp)
        and (
            # TODO: having a gas giant does not necessarily mean we want to build a GGG, especially for Sly
            ((orb_gen_tech in research_queue_list[:2] or got_orb_gen) and have_gas_giant())
            or ((mgrav_prod_tech in research_queue_list[:2] or got_mgrav_prod) and have_asteroids())
        )
        and (get_number_of_colonies() < 12)
    )
    # get current industry production & Target
    owned_planet_ids = PlanetUtilsAI.get_owned_planets_by_empire()
    planets = (universe.getPlanet(x) for x in owned_planet_ids)
    target_rp = sum(map(lambda _x: _x.currentMeterValue(fo.meterType.targetResearch), planets))
    galaxy_is_sparse = ColonisationAI.galaxy_is_sparse()
    enemies_sighted = aistate.misc.get("enemies_sighted", {})

    style_index = aistate.character.preferred_research_cutoff([0, 1])
    if aistate.character.may_maximize_research():
        style_index += 1

    cutoff_sets = [[25, 45, 70, 110], [30, 45, 70, 150], [25, 40, 80, 160]]
    cutoffs = cutoff_sets[style_index]
    settings = [[1.3, 0.7, 0.5, 0.4, 0.35], [1.4, 0.8, 0.6, 0.5, 0.35], [1.4, 0.8, 0.6, 0.5, 0.4]][style_index]

    if (
        (current_turn < cutoffs[0])
        or (not got_algo)
        or ((style_index == 0) and not got_orb_gen and (current_turn < cutoffs[1]))
    ):
        research_priority = (
            settings[0] * industry_priority
        )  # high research at beginning of game to get easy gro tech and to get research booster Algotrithmic Elegance
    elif (not got_orb_gen) or (current_turn < cutoffs[1]):
        research_priority = settings[1] * industry_priority  # med-high research
    elif current_turn < cutoffs[2]:
        research_priority = settings[2] * industry_priority  # med-high industry
    elif current_turn < cutoffs[3]:
        research_priority = settings[3] * industry_priority  # med-high industry
    else:
        research_queue = list(empire.researchQueue)
        research_priority = settings[4] * industry_priority  # high industry , low research
        if len(research_queue) == 0:
            research_priority = 0  # done with research
        elif len(research_queue) < 5 and research_queue[-1].allocation > 0:
            research_priority = len(research_queue) * 0.01 * industry_priority  # barely not done with research
        elif len(research_queue) < 10 and research_queue[-1].allocation > 0:
            research_priority = (4 + 2 * len(research_queue)) * 0.01 * industry_priority  # almost done with research
        elif len(research_queue) < 20 and research_queue[int(len(research_queue) // 2)].allocation > 0:
            research_priority *= 0.7  # closing in on end of research
    if industry_surge:
        if galaxy_is_sparse and not any(enemies_sighted):
            research_priority *= 0.5
        else:
            research_priority *= 0.8

    if (tech_is_complete("SHP_WEAPON_2_4") or tech_is_complete("SHP_WEAPON_4_1")) and tech_is_complete(
        AIDependencies.PRO_AUTO_1
    ):
        # industry_factor = [ [0.25, 0.2], [0.3, 0.25], [0.3, 0.25] ][style_index ]
        # researchPriority = min(researchPriority, industry_factor[got_solar_gen]*industryPriority)
        research_priority *= 0.9
    if got_quant:
        research_priority = min(research_priority + 0.1 * industry_priority, research_priority * 1.3)
    research_priority = int(research_priority)
    debug(f"Research Production (current/target) : ( {total_rp:.1f} / {target_rp:.1f} )")
    debug(
        "Priority for Research: %d (new target ~ %d RP)"
        % (research_priority, total_pp * research_priority / industry_priority)
    )

    if len(enemies_sighted) < (2 + current_turn / 20.0):  # TODO: adjust for colonisation priority
        research_priority *= 1.2
    if (current_turn > 20) and (len(recent_enemies) > 3):
        research_priority *= 0.8
    return research_priority


def _calculate_exploration_priority():
    """Calculates the demand for scouts by unexplored systems."""
    empire = fo.getEmpire()
    num_unexplored_systems = len(ExplorationAI.border_unexplored_system_ids)
    aistate = get_aistate()
    num_scouts = sum(
        [
            aistate.fleetStatus.get(fid, {}).get("nships", 0)
            for fid in FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.EXPLORATION)
        ]
    )
    production_queue = empire.productionQueue
    queued_scout_ships = 0
    for queue_index in range(0, len(production_queue)):
        element = production_queue[queue_index]
        if element.buildType == EmpireProductionTypes.BT_SHIP:
            if aistate.get_ship_role(element.designID) == ShipRoleType.CIVILIAN_EXPLORATION:
                queued_scout_ships += element.remaining * element.blocksize

    mil_ships = MilitaryAI.get_num_military_ships()
    # intent of the following calc is essentially
    # new_scouts_needed = min(need_cap_A, need_cap_B, base_need) - already_got_or_queued
    # where need_cap_A is to help prevent scouting needs from swamping military needs, and
    # need_cap_B is to help regulate investment into scouting while the empire is small.
    # These caps could perhaps instead be tied more directly to military priority and
    # total empire production.
    desired_number_of_scouts = int(
        min(4 + mil_ships / 5, 4 + fo.currentTurn() / 50.0, 2 + num_unexplored_systems**0.5)
    )
    scouts_needed = max(0, desired_number_of_scouts - (num_scouts + queued_scout_ships))
    exploration_priority = int(40 * scouts_needed)

    debug("")
    debug("Number of Scouts: %s" % num_scouts)
    debug("Number of Unexplored systems: %s" % num_unexplored_systems)
    debug("Military size: %s" % mil_ships)
    debug("Priority for scouts: %s" % exploration_priority)

    return exploration_priority


def _calculate_colonisation_priority():
    """Calculates the demand for colony ships by colonisable planets."""
    global allottedColonyTargets
    aistate = get_aistate()
    enemies_sighted = aistate.misc.get("enemies_sighted", {})
    galaxy_is_sparse = ColonisationAI.galaxy_is_sparse()
    total_pp = fo.getEmpire().productionPoints
    num_colonies = get_number_of_colonies()
    colony_growth_barrier = aistate.character.max_number_colonies()
    if num_colonies > colony_growth_barrier:
        return 0.0
    colony_cost, turns_to_build = ColonisationAI.colony_pod_cost_turns()

    mil_prio = aistate.get_priority(PriorityType.PRODUCTION_MILITARY)
    allotted_portion = [[[0.6, 0.8], [0.3, 0.4]], [[0.8, 0.9], [0.4, 0.6]]][galaxy_is_sparse][any(enemies_sighted)]
    allotted_portion = aistate.character.preferred_colonization_portion(allotted_portion)
    # if ( get_aistate().get_priority(AIPriorityType.PRIORITY_PRODUCTION_COLONISATION)
    # > 2 * get_aistate().get_priority(AIPriorityType.PRIORITY_PRODUCTION_MILITARY)):
    # allotted_portion *= 1.5
    if mil_prio < 100:
        allotted_portion *= 2
    elif mil_prio < 200:
        allotted_portion *= 1.5
    elif fo.currentTurn() > 100:
        allotted_portion *= 0.75 ** (num_colonies / 10.0)
    # allottedColonyTargets = 1+ int(fo.currentTurn()/50)
    allottedColonyTargets = 1 + int(total_pp * turns_to_build * allotted_portion / colony_cost)
    outpost_prio = aistate.get_priority(PriorityType.PRODUCTION_OUTPOST)

    # if have no SP_SLY, and have any outposts to build, don't build colony ships TODO: make more complex assessment
    if not can_build_colony_for_species("SP_SLY") and outpost_prio > 0:
        return 0.0
    min_score = ColonisationAI.MINIMUM_COLONY_SCORE
    minimal_top = min_score + 2  # one more than the conditional floor set by ColonisationAI.revise_threat_factor()
    minimal_opportunities = [
        species_name
        for (_, (score, species_name)) in get_colonisable_planet_ids().items()
        if min_score < score <= minimal_top
    ]
    decent_opportunities = [
        species_name for (_, (score, species_name)) in get_colonisable_planet_ids().items() if score > minimal_top
    ]
    minimal_planet_factor = 0.2  # count them for something, but not much
    num_colonisable_planet_ids = len(decent_opportunities) + minimal_planet_factor * len(minimal_opportunities)
    if num_colonisable_planet_ids == 0:
        return 1

    colony_ship_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.COLONISATION)
    num_colony_ships = len(FleetUtilsAI.extract_fleet_ids_without_mission_types(colony_ship_ids))
    colonisation_priority = (
        60 * (1.0 + num_colonisable_planet_ids - num_colony_ships) / (num_colonisable_planet_ids + 1)
    )

    if can_build_only_sly_colonies():
        colonisation_priority *= 2
    elif can_build_colony_for_species("SP_SLY"):
        colony_opportunities = minimal_opportunities + decent_opportunities
        colonisation_priority *= (1.0 + colony_opportunities.count("SP_SLY")) / len(colony_opportunities)

    if colonisation_priority < 1:
        return 0
    return colonisation_priority


def _calculate_outpost_priority():
    """Calculates the demand for outpost ships by colonisable planets."""
    global allotted_outpost_targets
    base_outpost_cost = AIDependencies.OUTPOST_POD_COST

    aistate = get_aistate()
    enemies_sighted = aistate.misc.get("enemies_sighted", {})
    galaxy_is_sparse = ColonisationAI.galaxy_is_sparse()
    total_pp = fo.getEmpire().productionPoints
    colony_growth_barrier = aistate.character.max_number_colonies()
    if get_number_of_colonies() > colony_growth_barrier:
        return 0.0
    mil_prio = aistate.get_priority(PriorityType.PRODUCTION_MILITARY)

    not_sparse, enemy_unseen = 0, 0
    is_sparse, enemy_seen = 1, 1
    allotted_portion = {
        (not_sparse, enemy_unseen): (0.6, 0.8),
        (not_sparse, enemy_seen): (0.3, 0.4),
        (is_sparse, enemy_unseen): (0.8, 0.9),
        (is_sparse, enemy_seen): (0.3, 0.4),
    }[(galaxy_is_sparse, any(enemies_sighted))]
    allotted_portion = aistate.character.preferred_outpost_portion(allotted_portion)
    if mil_prio < 100:
        allotted_portion *= 2
    elif mil_prio < 200:
        allotted_portion *= 1.5
    allotted_outpost_targets = 1 + int(total_pp * 3 * allotted_portion / base_outpost_cost)

    num_outpost_targets = len(
        [
            pid
            for (pid, (score, specName)) in get_colonisable_outpost_ids().items()
            if score > max(1.0 * base_outpost_cost / 3.0, ColonisationAI.MINIMUM_COLONY_SCORE)
        ][:allotted_outpost_targets]
    )
    if num_outpost_targets == 0 or not tech_is_complete(AIDependencies.OUTPOSTING_TECH):
        return 0

    outpost_ship_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.OUTPOST)
    num_outpost_ships = len(FleetUtilsAI.extract_fleet_ids_without_mission_types(outpost_ship_ids))
    outpost_priority = (50.0 * (num_outpost_targets - num_outpost_ships)) / num_outpost_targets

    # If the only possible race for colonization is SP_SLY (early stage of game before conquers and exobots)
    # discourage early outposting due to supply and stealth considerations they are best off
    # using colony ships until they have other colonizers (and more established military)
    if can_build_only_sly_colonies():
        outpost_priority /= 3.0

    # print
    # print "Number of Outpost Ships : " + str(num_outpost_ships)
    # print "Number of Colonisable outposts: " + str(num_outpost_planet_ids)
    debug("Priority for outpost ships: %s" % outpost_priority)

    if outpost_priority < 1:
        return 0
    return outpost_priority


def _calculate_invasion_priority():  # noqa: C901
    """Calculates the demand for troop ships by opponent planets."""

    aistate = get_aistate()
    if not aistate.character.may_invade():
        return 0

    empire = fo.getEmpire()
    enemies_sighted = aistate.misc.get("enemies_sighted", {})
    multiplier = 1
    colony_growth_barrier = aistate.character.max_number_colonies()
    if get_number_of_colonies() > colony_growth_barrier:
        return 0.0

    if len(aistate.colonisablePlanetIDs) > 0:
        best_colony_score = max(2, next(iter(aistate.colonisablePlanetIDs.items()))[1][0])
    else:
        best_colony_score = 2

    total_val = 0
    troops_needed = 0
    for pid, pscore, trp in AIstate.invasionTargets[: allotted_invasion_targets()]:
        if pscore > best_colony_score:
            multiplier += 1
            total_val += 2 * pscore
        else:
            total_val += pscore
        troops_needed += trp + 4  # ToDo: This seems like it could be improved by some dynamic calculation of buffer

    if total_val == 0:
        return 0

    production_queue = empire.productionQueue
    queued_troop_capacity = 0
    for queue_index in range(0, len(production_queue)):
        element = production_queue[queue_index]
        if element.buildType == EmpireProductionTypes.BT_SHIP:
            if aistate.get_ship_role(element.designID) in [ShipRoleType.MILITARY_INVASION, ShipRoleType.BASE_INVASION]:
                design = fo.getShipDesign(element.designID)
                queued_troop_capacity += element.remaining * element.blocksize * design.troopCapacity
    _, best_design, _ = get_best_ship_info(PriorityType.PRODUCTION_INVASION)
    if best_design:
        troops_per_best_ship = best_design.troopCapacity
    else:
        return 1e-6  # if we can not build troop ships, we don't want to build (non-existing) invasion ships

    # don't count troop bases here as these cannot be redeployed after misplaning
    # troopFleetIDs = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.INVASION)\
    #                 + FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.ORBITAL_INVASION)
    troop_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.INVASION)
    total_troop_capacity = sum([FleetUtilsAI.count_troops_in_fleet(fid) for fid in troop_fleet_ids])
    troop_ships_needed = math.ceil(
        (troops_needed - (total_troop_capacity + queued_troop_capacity)) / troops_per_best_ship
    )

    # invasion_priority = max( 10+ 200*max(0, troop_ships_needed ) , int(0.1* total_val) )
    invasion_priority = multiplier * (30 + 150 * max(0, troop_ships_needed))
    if not colonies_is_under_attack():
        if not colonies_is_under_treat():
            invasion_priority *= 2.0
        else:
            invasion_priority *= 1.5
    if not enemies_sighted:
        invasion_priority *= 1.5

    if invasion_priority < 0:
        return 0

    return invasion_priority * aistate.character.invasion_priority_scaling()


def allotted_invasion_targets():
    return min(1 + int(fo.currentTurn() // 50), 3)


def _calculate_military_priority():  # noqa: C901
    """Calculates the demand for military ships by military targeted systems."""
    global unmetThreat

    universe = fo.getUniverse()
    capital_id = PlanetUtilsAI.get_capital()
    if capital_id is None or capital_id == INVALID_ID:
        return 0  # no capitol (not even a capitol-in-the-making), means can't produce any ships

    have_l1_weaps = (
        tech_is_complete("SHP_WEAPON_1_4")
        or (tech_is_complete("SHP_WEAPON_1_3") and tech_is_complete("SHP_MIL_ROBO_CONT"))
        or tech_is_complete("SHP_WEAPON_2_1")
        or tech_is_complete("SHP_WEAPON_4_1")
    )
    have_l2_weaps = tech_is_complete("SHP_WEAPON_2_3") or tech_is_complete("SHP_WEAPON_4_1")
    aistate = get_aistate()
    enemies_sighted = aistate.misc.get("enemies_sighted", {})

    target_planet_ids = (
        [pid for pid, pscore, trp in AIstate.invasionTargets[: allotted_invasion_targets()]]
        + [pid for pid, pscore in list(get_colonisable_planet_ids(True).items())[:allottedColonyTargets]]
        + [pid for pid, pscore in list(get_colonisable_outpost_ids(True).items())[:allottedColonyTargets]]
    )

    my_systems = set(get_owned_planets())
    target_systems = set(PlanetUtilsAI.get_systems(target_planet_ids))

    cur_ship_rating = cur_best_military_design_rating()
    current_turn = fo.currentTurn()
    ships_needed = 0
    defense_ships_needed = 0
    ships_needed_allocation = []
    for sys_id in my_systems.union(target_systems):
        status = aistate.systemStatus.get(sys_id, {})
        my_rating = status.get("myFleetRating", 0)
        my_defenses = status.get("mydefenses", {}).get("overall", 0)
        base_monster_threat = status.get("monsterThreat", 0)
        # scale monster threat so that in early - mid game big monsters don't over-drive military production
        monster_threat = base_monster_threat
        if current_turn > 200:
            pass
        elif current_turn > 100:
            if base_monster_threat >= 2000:
                monster_threat = 2000 + (current_turn / 100.0 - 1) * (base_monster_threat - 2000)
        elif current_turn > 30:
            if base_monster_threat >= 2000:
                monster_threat = 0
        else:
            if base_monster_threat > 200:
                monster_threat = 0
        if sys_id in my_systems:
            threat_root = (
                status.get("fleetThreat", 0) ** 0.5
                + 0.8 * status.get("max_neighbor_threat", 0) ** 0.5
                + 0.2 * status.get("neighborThreat", 0) ** 0.5
                + monster_threat**0.5
                + status.get("planetThreat", 0) ** 0.5
            )
        else:
            threat_root = (
                status.get("fleetThreat", 0) ** 0.5 + monster_threat**0.5 + status.get("planetThreat", 0) ** 0.5
            )
        ships_needed_here = math.ceil(
            (max(0, (threat_root - (my_rating**0.5 + my_defenses**0.5))) ** 2) / cur_ship_rating
        )
        ships_needed += ships_needed_here
        ships_needed_allocation.append((universe.getSystem(sys_id), ships_needed_here))
        if sys_id in my_systems:
            defense_ships_needed += ships_needed_here

    part1 = min(1 * fo.currentTurn(), 40)
    part2 = max(0, int(75 * ships_needed))
    military_priority = part1 + part2
    if not have_l1_weaps:
        military_priority /= 2.0
    elif not (have_l2_weaps and enemies_sighted):
        military_priority /= 1.5
    fmt_string = "Calculating Military Priority:  min(t,40) + max(0,75 * ships_needed) \n\t  Priority: %d  \t ships_needed: %d \t defense_ships_needed: %d \t curShipRating: %.0f \t l1_weaps: %s \t enemies_sighted: %s"
    debug(
        fmt_string
        % (military_priority, ships_needed, defense_ships_needed, cur_ship_rating, have_l1_weaps, enemies_sighted)
    )
    debug("Source of milship demand: %s" % ships_needed_allocation)

    military_priority *= aistate.character.military_priority_scaling()
    return max(military_priority, 0)


def _calculate_top_production_queue_priority():
    """Calculates the top production queue priority."""
    production_queue_priorities = {}
    aistate = get_aistate()
    for priorityType in get_priority_production_types():
        production_queue_priorities[priorityType] = aistate.get_priority(priorityType)

    sorted_priorities = sorted(production_queue_priorities.items(), key=itemgetter(1), reverse=True)
    top_production_queue_priority = -1
    for evaluationPair in sorted_priorities:
        if top_production_queue_priority < 0:
            top_production_queue_priority = evaluationPair[0]

    return top_production_queue_priority
