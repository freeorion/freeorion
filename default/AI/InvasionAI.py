import freeOrionAIInterface as fo  # pylint: disable=import-error
import FreeOrionAI as foAI
import AIstate
import AIDependencies
import EnumsAI
import FleetUtilsAI
import PlanetUtilsAI
import AITarget
import math
import ProductionAI
import ColonisationAI
import MilitaryAI
from freeorion_tools import tech_is_complete
from freeorion_debug import Timer

invasion_timer = Timer('get_invasion_fleets()', write_log=False)


def get_invasion_fleets():
    invasion_timer.start("gathering initial info")

    all_invasion_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION)
    AIstate.invasionFleetIDs = FleetUtilsAI.extract_fleet_ids_without_mission_types(all_invasion_fleet_ids)

    # get suppliable planets
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empire_id = empire.empireID
    capital_id = PlanetUtilsAI.get_capital()
    homeworld=None
    if capital_id:
        homeworld = universe.getPlanet(capital_id)
    if homeworld:
        home_system_id = homeworld.systemID
    else:
        home_system_id = -1

    fleet_suppliable_system_ids = empire.fleetSupplyableSystemIDs
    fleet_suppliable_planet_ids = PlanetUtilsAI.get_planets_in__systems_ids(fleet_suppliable_system_ids)

    prime_invadable_system_ids = set(ColonisationAI.annexable_system_ids)
    prime_invadable_planet_ids = PlanetUtilsAI.get_planets_in__systems_ids(prime_invadable_system_ids)

    visible_system_ids = foAI.foAIstate.visInteriorSystemIDs.keys() + foAI.foAIstate. visBorderSystemIDs.keys()

    if home_system_id != -1:
        accessible_system_ids = [sys_id for sys_id in visible_system_ids if (sys_id != -1) and universe.systemsConnected(sys_id, home_system_id, empire_id)]
    else:
        print "Invasion Warning: this empire has no identifiable homeworld, will therefor treat all visible planets as accessible."
        accessible_system_ids = visible_system_ids  # TODO: check if any troop ships still owned, use their system as home system
    acessible_planet_ids = PlanetUtilsAI.get_planets_in__systems_ids(accessible_system_ids)
    print "Accessible Systems: ", ", ".join(PlanetUtilsAI.sys_name_ids(accessible_system_ids))
    print

    all_owned_planet_ids = PlanetUtilsAI.get_all_owned_planet_ids(acessible_planet_ids)  # need these for unpopulated outposts
    all_populated_planets = PlanetUtilsAI.get_populated_planet_ids(acessible_planet_ids)  # need this for natives
    print "All Visible and accessible Populated PlanetIDs (including this empire's): ", ", ".join(PlanetUtilsAI.planet_name_ids(all_populated_planets))
    print
    print "Prime Invadable Target Systems: ", ", ".join(PlanetUtilsAI.sys_name_ids(prime_invadable_system_ids))
    print

    empire_owned_planet_ids = PlanetUtilsAI.get_owned_planets_by_empire(universe.planetIDs)

    invadable_planet_ids = set(prime_invadable_planet_ids).intersection(set(all_owned_planet_ids).union(all_populated_planets) - set(empire_owned_planet_ids))
    print "Prime Invadable PlanetIDs: ", ", ".join(PlanetUtilsAI.planet_name_ids(invadable_planet_ids))

    print
    print "Current Invasion Targeted SystemIDs: ", ", ".join(PlanetUtilsAI.sys_name_ids(AIstate.invasionTargetedSystemIDs))
    invasion_targeted_planet_ids = get_invasion_targeted_planet_ids(universe.planetIDs, EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION)
    invasion_targeted_planet_ids.extend(get_invasion_targeted_planet_ids(universe.planetIDs, EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION))
    all_invasion_targeted_system_ids = set(PlanetUtilsAI.get_systems(invasion_targeted_planet_ids))

    print "Current Invasion Targeted PlanetIDs: ", ", ".join(PlanetUtilsAI.planet_name_ids(invasion_targeted_planet_ids))

    invasion_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION)
    if not invasion_fleet_ids:
        print "Available Invasion Fleets: 0"
    else:
        print "Invasion FleetIDs: %s" % FleetUtilsAI.get_empire_fleet_ids_by_role(EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION)

    num_invasion_fleets = len(FleetUtilsAI.extract_fleet_ids_without_mission_types(invasion_fleet_ids))
    print "Invasion Fleets Without Missions:    %s" % num_invasion_fleets
    
    invasion_timer.start("planning troop base production")
    # only do base invasions if aggression is typical or above
    reserved_troop_base_targets = []
    if foAI.foAIstate.aggression > fo.aggression.typical:
        available_pp = {}
        for el in empire.planetsWithAvailablePP:  # keys are sets of ints; data is doubles
            avail_pp = el.data()
            for pid in el.key():
                available_pp[pid] = avail_pp
        for pid in invadable_planet_ids:  # TODO: reorganize
            planet = universe.getPlanet(pid)
            if not planet: 
                continue
            sys_id = planet.systemID
            sys_partial_vis_turn = universe.getVisibilityTurnsMap(planet.systemID, empire_id).get(fo.visibility.partial, -9999)
            planet_partial_vis_turn = universe.getVisibilityTurnsMap(pid, empire_id).get(fo.visibility.partial, -9999)
            if planet_partial_vis_turn < sys_partial_vis_turn:
                continue
            for pid2 in ColonisationAI.empire_species_systems.get(sys_id,  {}).get('pids', []):
                if available_pp.get(pid2,  0) < 2:  # TODO: improve troop base PP sufficiency determination
                    break
                planet2 = universe.getPlanet(pid2)
                if not planet2: 
                    continue
                if pid not in foAI.foAIstate.qualifyingTroopBaseTargets and planet2.speciesName in ColonisationAI.empire_ship_builders:
                    foAI.foAIstate.qualifyingTroopBaseTargets.setdefault(pid,  [pid2,  -1])
                    break

        for pid in list(foAI.foAIstate.qualifyingTroopBaseTargets):
            planet = universe.getPlanet(pid)  # TODO: also check that still have a colony in this system that can make troops
            if planet and planet.owner == empire_id:
                del foAI.foAIstate.qualifyingTroopBaseTargets[pid]

        secure_ai_fleet_missions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([EnumsAI.AIFleetMissionType.FLEET_MISSION_SECURE])
        for pid in (set(foAI.foAIstate.qualifyingTroopBaseTargets.keys()) - set(invasion_targeted_planet_ids)):  # TODO: consider overriding standard invasion mission
            planet = universe.getPlanet(pid)
            if foAI.foAIstate.qualifyingTroopBaseTargets[pid][1] != -1:
                reserved_troop_base_targets.append(pid)
                if planet:
                    all_invasion_targeted_system_ids.add(planet.systemID)
                continue  # already building for here
            sys_id = planet.systemID
            this_sys_status = foAI.foAIstate.systemStatus.get(sys_id,  {})
            if (planet.currentMeterValue(fo.meterType.shield) > 0 and
                    this_sys_status.get('myFleetRating', 0) < 0.8 * this_sys_status.get('totalThreat', 0)):
                continue
            loc = foAI.foAIstate.qualifyingTroopBaseTargets[pid][0]
            this_score,  p_troops = evaluate_invasion_planet(pid, empire, secure_ai_fleet_missions, False)
            best_ship, col_design, build_choices = ProductionAI.getBestShipInfo(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_ORBITAL_INVASION, loc)
            if not best_ship:
                continue
            n_bases = math.ceil((p_troops+1) / 2)  # TODO: reconsider this +1 safety factor
            retval = fo.issueEnqueueShipProductionOrder(best_ship, loc)
            print "Enqueueing %d Troop Bases at %s for %s" % (n_bases, PlanetUtilsAI.planet_name_ids([loc]), PlanetUtilsAI.planet_name_ids([pid]))
            if retval != 0:
                all_invasion_targeted_system_ids.add(planet.systemID)
                reserved_troop_base_targets.append(pid)
                foAI.foAIstate.qualifyingTroopBaseTargets[pid][1] = loc
                fo.issueChangeProductionQuantityOrder(empire.productionQueue.size - 1, 1, int(n_bases))
                fo.issueRequeueProductionOrder(empire.productionQueue.size - 1, 0)

    invasion_timer.start("evaluating target planets")
    # TODO: check if any invasion_targeted_planet_ids need more troops assigned
    evaluated_planet_ids = list(set(invadable_planet_ids) - set(invasion_targeted_planet_ids) - set(reserved_troop_base_targets))
    print "Evaluating potential invasions, PlanetIDs: %s" % evaluated_planet_ids

    evaluated_planets = assign_invasion_values(evaluated_planet_ids, EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION, fleet_suppliable_planet_ids, empire)

    sorted_planets = [(pid, pscore % 10000, ptroops) for pid, (pscore, ptroops) in evaluated_planets.items()]
    sorted_planets.sort(key=lambda x: x[1], reverse=True)
    sorted_planets = [(pid, pscore % 10000, ptroops) for pid, pscore, ptroops in sorted_planets]

    print
    if sorted_planets:
        print "Invadable planets:\n%-6s | %-6s | %-16s | %-16s | Troops" % ('ID', 'Score', 'Name', 'Race')
        for pid, pscore, ptroops in sorted_planets:
            planet = universe.getPlanet(pid)
            if planet:
                print "%6d | %6d | %16s | %16s | %d" % (pid, pscore, planet.name, planet.speciesName, ptroops)
            else:
                print "%6d | %6d | Error: invalid planet ID" % (pid, pscore)
    else:
        print "No Invadable planets identified"

    sorted_planets = filter(lambda x: x[1] > 0, sorted_planets)
    # export opponent planets for other AI modules
    AIstate.opponentPlanetIDs = [pid for pid, _, _ in sorted_planets]
    AIstate.invasionTargets = sorted_planets

    # export invasion targeted systems for other AI modules
    AIstate.invasionTargetedSystemIDs = list(all_invasion_targeted_system_ids)
    invasion_timer.stop(section_name="evaluating %d target planets" % (len(evaluated_planet_ids)))
    invasion_timer.end()


def get_invasion_targeted_planet_ids(planet_ids, mission_type):
    invasion_feet_missions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([mission_type])
    targeted_planets = []
    for pid in planet_ids:
        # add planets that are target of a mission
        for mission in invasion_feet_missions:
            target = AITarget.AITarget(EnumsAI.TargetType.TARGET_PLANET, pid)
            if mission.has_target(mission_type, target):
                targeted_planets.append(pid)
    return targeted_planets


def retaliation_risk_factor(empire_id):
    """A multiplicative adjustment to planet scores to account for risk of retaliation from planet owner."""
    # TODO implement (in militaryAI) actual military risk assessment of other empires
    if empire_id == -1:  # unowned
        return 1.5  # since no risk of retaliation, increase score
    else:
        return 1.0


def assign_invasion_values(planet_ids, mission_type, fleet_suppliable_planet_ids, empire):
    """Creates a dictionary that takes planet_ids as key and their invasion score as value."""
    planet_values = {}
    neighbor_values = {}
    neighbor_val_ratio = .95
    universe = fo.getUniverse()
    secure_missions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([EnumsAI.AIFleetMissionType.FLEET_MISSION_SECURE])
    for pid in planet_ids:
        planet_values[pid] = neighbor_values.setdefault(pid, evaluate_invasion_planet(pid, empire, secure_missions))
        print "planet %d, values %s" % (pid, planet_values[pid])
        planet = universe.getPlanet(pid)
        species_name = (planet and planet.speciesName) or ""
        species = fo.getSpecies(species_name)
        if species and species.canProduceShips:
            system = universe.getSystem(planet.systemID)
            if not system:
                continue
            planet_industries = {}
            for pid2 in system.planetIDs:
                planet2 = universe.getPlanet(pid2)
                species_name2 = (planet2 and planet2.speciesName) or ""
                species2 = fo.getSpecies(species_name2)
                if species2 and species2.canProduceShips:
                    planet_industries[pid2] = planet2.currentMeterValue(fo.meterType.industry) + 0.1  # to prevent divide-by-zero
            industry_ratio = planet_industries[pid] / max(planet_industries.values())
            for pid2 in system.planetIDs:
                if pid2 == pid:
                    continue
                planet2 = universe.getPlanet(pid2)
                if planet2 and (planet2.owner != empire.empireID) and ((planet2.owner != -1) or (planet.currentMeterValue(fo.meterType.population) > 0)):  # TODO check for allies
                    planet_values[pid][0] += industry_ratio * neighbor_val_ratio * (neighbor_values.setdefault(pid2, evaluate_invasion_planet(pid2, empire, secure_missions))[0])
    return planet_values


def evaluate_invasion_planet(planet_id, empire, secure_fleet_missions, verbose=True):
    """Return the invasion value (score, troops) of a planet."""
    detail = []
    building_values = {"BLD_IMPERIAL_PALACE": 1000,
                       "BLD_CULTURE_ARCHIVES": 1000,
                       "BLD_SHIPYARD_BASE": 100,
                       "BLD_SHIPYARD_ORG_ORB_INC": 200,
                       "BLD_SHIPYARD_ORG_XENO_FAC": 200,
                       "BLD_SHIPYARD_ORG_CELL_GRO_CHAMB": 200,
                       "BLD_SHIPYARD_CON_NANOROBO": 300,
                       "BLD_SHIPYARD_CON_GEOINT": 400,
                       "BLD_SHIPYARD_CON_ADV_ENGINE": 1000,
                       "BLD_SHIPYARD_AST": 300,
                       "BLD_SHIPYARD_AST_REF": 1000,
                       "BLD_SHIPYARD_ENRG_SOLAR": 1500,
                       "BLD_INDUSTRY_CENTER": 500,
                       "BLD_GAS_GIANT_GEN": 200,
                       "BLD_SOL_ORB_GEN": 800,
                       "BLD_BLACK_HOLE_POW_GEN": 2000,
                       "BLD_ENCLAVE_VOID": 500,
                       "BLD_NEUTRONIUM_EXTRACTOR": 2000,
                       "BLD_NEUTRONIUM_SYNTH": 2000,
                       "BLD_NEUTRONIUM_FORGE": 1000,
                       "BLD_CONC_CAMP": 100,
                       "BLD_BIOTERROR_PROJECTOR": 1000,
                       "BLD_SHIPYARD_ENRG_COMP": 3000,
                       }
    # TODO: add more factors, as used for colonization
    universe = fo.getUniverse()
    empire_id = empire.empireID
    max_jumps = 8
    planet = universe.getPlanet(planet_id)
    if planet is None:  # TODO: exclude planets with stealth higher than empireDetection
        print "invasion AI couldn't access any info for planet id %d" % planet_id
        return [0, 0]

    sys_partial_vis_turn = universe.getVisibilityTurnsMap(planet.systemID, empire_id).get(fo.visibility.partial, -9999)
    planet_partial_vis_turn = universe.getVisibilityTurnsMap(planet_id, empire_id).get(fo.visibility.partial, -9999)

    if planet_partial_vis_turn < sys_partial_vis_turn:
        print "invasion AI couldn't get current info on planet id %d (was stealthed at last sighting)" % planet_id
        # TODO: track detection strength, order new scouting when it goes up
        return [0, 0]  # last time we had partial vis of the system, the planet was stealthed to us

    species_name = planet.speciesName
    species = fo.getSpecies(species_name)
    if not species:  # this call iterates over this Empire's available species with which it could colonize after an invasion
        planet_eval = ColonisationAI.assign_colonisation_values([planet_id], EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION, None, empire, detail)
        pop_val = max(0.75*planet_eval.get(planet_id, [0])[0], ColonisationAI.evaluate_planet(planet_id, EnumsAI.AIFleetMissionType.FLEET_MISSION_OUTPOST, None, empire, detail))
    else:
        pop_val = ColonisationAI.evaluate_planet(planet_id, EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION, species_name, empire, detail)

    bld_tally = 0
    for bldType in [universe.getObject(bldg).buildingTypeName for bldg in planet.buildingIDs]:
        bval = building_values.get(bldType, 50)
        bld_tally += bval
        detail.append("%s: %d" % (bldType, bval))

    p_sys_id = planet.systemID
    capitol_id = PlanetUtilsAI.get_capital()
    least_jumps_path = []
    clear_path = True
    if capitol_id:
        homeworld = universe.getPlanet(capitol_id)
        if homeworld:
            home_system_id = homeworld.systemID
            eval_system_id = planet.systemID
            if (home_system_id != -1) and (eval_system_id != -1):
                least_jumps_path = list(universe.leastJumpsPath(home_system_id, eval_system_id, empire_id))
                max_jumps = len(least_jumps_path)
    system_status = foAI.foAIstate.systemStatus.get(p_sys_id, {})
    system_fleet_treat = system_status.get('fleetThreat', 1000)
    system_monster_threat = system_status.get('monsterThreat', 0)
    sys_total_threat = system_fleet_treat + system_monster_threat + system_status.get('planetThreat', 0)
    max_path_threat = system_fleet_treat
    mil_ship_rating = ProductionAI.cur_best_mil_ship_rating()
    for path_sys_id in least_jumps_path:
        path_leg_status = foAI.foAIstate.systemStatus.get(path_sys_id, {})
        path_leg_threat = path_leg_status.get('fleetThreat', 1000) + path_leg_status.get('monsterThreat', 0)
        if path_leg_threat > 0.5 * mil_ship_rating:
            clear_path = False
            if path_leg_threat > max_path_threat:
                max_path_threat = path_leg_threat

    troops = planet.currentMeterValue(fo.meterType.troops)
    max_troops = planet.currentMeterValue(fo.meterType.maxTroops)

    this_system = universe.getSystem(p_sys_id)
    secure_targets = [p_sys_id] + list(this_system.planetIDs)
    system_secured = False
    for mission in secure_fleet_missions:
        if system_secured:
            break
        secure_fleet_id = mission.target_id
        s_fleet = universe.getFleet(secure_fleet_id)
        if not s_fleet or s_fleet.systemID != p_sys_id:
            continue
        for ai_target in mission.get_targets(EnumsAI.AIFleetMissionType.FLEET_MISSION_SECURE):
            target_obj = ai_target.target_obj
            if (target_obj is not None) and target_obj.id in secure_targets:
                system_secured = True
                break

    if verbose:
        print "invasion eval of %s %d --- maxShields %.1f -- sysFleetThreat %.1f -- sysMonsterThreat %.1f" % (
            planet.name, planet_id, planet.currentMeterValue(fo.meterType.maxShield), system_fleet_treat,
            system_monster_threat)
    supply_val = 0
    enemy_val = 0
    if planet.owner != -1:  # value in taking this away from an enemy
        enemy_val = 20 * (planet.currentMeterValue(fo.meterType.targetIndustry) + 2*planet.currentMeterValue(fo.meterType.targetResearch))
    if p_sys_id in ColonisationAI.annexable_system_ids:  # TODO: extend to rings
        supply_val = 100
    elif p_sys_id in ColonisationAI.annexable_ring1:
        supply_val = 200
    elif p_sys_id in ColonisationAI.annexable_ring2:
        supply_val = 300
    elif p_sys_id in ColonisationAI.annexable_ring3:
        supply_val = 400
    if max_path_threat > 0.5 * mil_ship_rating:
        if max_path_threat < 3 * mil_ship_rating:
            supply_val *= 0.5
        else:
            supply_val *= 0.2
        
    threat_factor = min(1, 0.2*MilitaryAI.totMilRating/(sys_total_threat+0.001))**2  # devalue invasions that would require too much military force
    build_time = 4

    planned_troops = troops if system_secured else min(troops+max_jumps+build_time, max_troops)
    if not tech_is_complete("SHP_ORG_HULL"):
        troop_cost = math.ceil(planned_troops/6.0) * (40*(1+foAI.foAIstate.shipCount * AIDependencies.SHIP_UPKEEP))
    else:
        troop_cost = math.ceil(planned_troops/6.0) * (20*(1+foAI.foAIstate.shipCount * AIDependencies.SHIP_UPKEEP))
    planet_score = retaliation_risk_factor(planet.owner) * threat_factor * max(0, pop_val+supply_val+bld_tally+enemy_val-0.8*troop_cost)
    if clear_path:
        planet_score *= 1.5
    invasion_score = [planet_score, planned_troops]
    print invasion_score, "projected Troop Cost:", troop_cost, ", threatFactor: ", threat_factor, ", planet detail ", detail, "popval, supplyval, bldval, enemyval", pop_val, supply_val, bld_tally, enemy_val
    return invasion_score


def send_invasion_fleets(fleet_ids, evaluated_planets, mission_type):
    """sends a list of invasion fleets to a list of planet_value_pairs"""
    universe = fo.getUniverse()
    invasion_fleet_pool = set(fleet_ids)
    if not invasion_fleet_pool:
        return

    for planet_id, pscore, ptroops in evaluated_planets:
        planet = universe.getPlanet(planet_id)
        if not planet:
            continue
        sys_id = planet.systemID
        found_fleets = []
        found_stats = {}
        min_stats = {'rating': 0, 'troopCapacity': ptroops}
        target_stats = {'rating': 10, 'troopCapacity': ptroops+1}
        these_fleets = FleetUtilsAI.get_fleets_for_mission(1, target_stats, min_stats, found_stats, "",
                                                           systems_to_check=[sys_id], systems_checked=[],
                                                           fleet_pool_set=invasion_fleet_pool, fleet_list=found_fleets,
                                                           verbose=False)
        if not these_fleets:
            if not FleetUtilsAI.stats_meet_reqs(found_stats, min_stats):
                print "Insufficient invasion troop allocation for system %d ( %s ) -- requested %s , found %s" % (
                    sys_id, universe.getSystem(sys_id).name, min_stats, found_stats)
                invasion_fleet_pool.update(found_fleets)
                continue
            else:
                these_fleets = found_fleets
        target = AITarget.AITarget(EnumsAI.TargetType.TARGET_PLANET, planet_id)
        print "assigning invasion fleets %s to target %s" % (these_fleets, target)
        for fleetID in these_fleets:
            fleet_mission = foAI.foAIstate.get_fleet_mission(fleetID)
            fleet_mission.clear_fleet_orders()
            fleet_mission.clear_targets((fleet_mission.get_mission_types() + [-1])[0])
            fleet_mission.add_target(mission_type, target)


def assign_invasion_fleets_to_invade():
    """Assign fleet targets to invadable planets."""
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    fleet_suppliable_system_ids = empire.fleetSupplyableSystemIDs
    fleet_suppliable_planet_ids = PlanetUtilsAI.get_planets_in__systems_ids(fleet_suppliable_system_ids)

    all_troopbase_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(EnumsAI.AIFleetMissionType.
                                                                        FLEET_MISSION_ORBITAL_INVASION)
    available_troopbase_fleet_ids = set(FleetUtilsAI.extract_fleet_ids_without_mission_types(all_troopbase_fleet_ids))
    for fid in list(available_troopbase_fleet_ids):
        if fid not in available_troopbase_fleet_ids:  # TODO: I do not see how this check makes sense, maybe remove?
            continue
        fleet = universe.getFleet(fid)
        if not fleet: 
            continue
        sys_id = fleet.systemID
        system = universe.getSystem(sys_id)
        available_planets = set(system.planetIDs).intersection(set(foAI.foAIstate.qualifyingTroopBaseTargets.keys()))
        print "Considering Base Troopers in %s, found planets %s and registered targets %s with status %s" % (
            system.name, list(system.planetIDs), available_planets,
            [(pid, foAI.foAIstate.qualifyingTroopBaseTargets[pid]) for pid in available_planets])
        targets = [pid for pid in available_planets if foAI.foAIstate.qualifyingTroopBaseTargets[pid][1] != -1]
        if not targets:
            print "Error: found no valid target for troop base in system %s (%d)" % (system.name, sys_id)
            continue
        status = foAI.foAIstate.systemStatus.get(sys_id, {})
        local_base_troops = set(status.get('myfleets', [])).intersection(available_troopbase_fleet_ids)
        troop_capacity_tally = 0
        for fid2 in local_base_troops:
            troop_capacity_tally += FleetUtilsAI.count_troops_in_fleet(fid2)

        target_id = -1
        best_score = -1
        target_troops = 0
        for pid, rating in assign_invasion_values(targets, EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION,
                                                  fleet_suppliable_planet_ids, empire).items():
            p_score, p_troops = rating
            if p_score > best_score:
                best_score = p_score
                target_id = pid
                target_troops = p_troops
        if target_id != -1:
            local_base_troops.discard(fid)
            found_fleets = []
            troops_needed = max(0, target_troops - FleetUtilsAI.count_troops_in_fleet(fid))
            found_stats = {}
            min_stats = {'rating': 0, 'troopCapacity': troops_needed}
            target_stats = {'rating': 10, 'troopCapacity': troops_needed}

            FleetUtilsAI.get_fleets_for_mission(1, target_stats, min_stats, found_stats, "",
                                                systems_to_check=[sys_id], systems_checked=[],
                                                fleet_pool_set=local_base_troops, fleet_list=found_fleets,
                                                verbose=False)
            for fid2 in found_fleets:
                FleetUtilsAI.merge_fleet_a_into_b(fid2, fid)
                available_troopbase_fleet_ids.discard(fid2)
            available_troopbase_fleet_ids.discard(fid)
            foAI.foAIstate.qualifyingTroopBaseTargets[target_id][1] = -1  # TODO: should probably delete
            target = AITarget.AITarget(EnumsAI.TargetType.TARGET_PLANET, target_id)
            fleet_mission = foAI.foAIstate.get_fleet_mission(fid)
            fleet_mission.add_target(EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION, target)
    
    invasion_fleet_ids = AIstate.invasionFleetIDs

    send_invasion_fleets(invasion_fleet_ids, AIstate.invasionTargets, EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION)
    all_invasion_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION)
    for fid in FleetUtilsAI.extract_fleet_ids_without_mission_types(all_invasion_fleet_ids):
        this_mission = foAI.foAIstate.get_fleet_mission(fid)
        this_mission.check_mergers(context="Post-send consolidation of unassigned troops")
