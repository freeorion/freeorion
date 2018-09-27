import math
from logging import debug, info, warn

import freeOrionAIInterface as fo

from aistate_interface import get_aistate
import AIDependencies
import AIstate
import ColonisationAI
import CombatRatingsAI
import EspionageAI
import FleetUtilsAI
import MilitaryAI
import PlanetUtilsAI
import ProductionAI
from AIDependencies import INVALID_ID
from EnumsAI import MissionType, PriorityType
from common.print_utils import Table, Text, Float
from freeorion_tools import tech_is_complete, AITimer, get_partial_visibility_turn
from target import TargetPlanet, TargetSystem
from turn_state import state

MAX_BASE_TROOPERS_GOOD_INVADERS = 20
MAX_BASE_TROOPERS_POOR_INVADERS = 10
_TROOPS_SAFETY_MARGIN = 1  # try to send this amount of additional troops to account for uncertainties in calculation
MIN_INVASION_SCORE = 20

invasion_timer = AITimer('get_invasion_fleets()', write_log=False)


def get_invasion_fleets():
    invasion_timer.start("gathering initial info")
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empire_id = fo.empireID()

    home_system_id = PlanetUtilsAI.get_capital_sys_id()
    aistate = get_aistate()
    visible_system_ids = list(aistate.visInteriorSystemIDs) + list(aistate.visBorderSystemIDs)

    if home_system_id != INVALID_ID:
        accessible_system_ids = [sys_id for sys_id in visible_system_ids if
                                 (sys_id != INVALID_ID) and universe.systemsConnected(sys_id, home_system_id,
                                                                                      empire_id)]
    else:
        debug("Empire has no identifiable homeworld; will treat all visible planets as accessible.")
        # TODO: check if any troop ships owned, use their system as home system
        accessible_system_ids = visible_system_ids

    acessible_planet_ids = PlanetUtilsAI.get_planets_in__systems_ids(accessible_system_ids)
    all_owned_planet_ids = PlanetUtilsAI.get_all_owned_planet_ids(acessible_planet_ids)  # includes unpopulated outposts
    all_populated_planets = PlanetUtilsAI.get_populated_planet_ids(acessible_planet_ids)  # includes unowned natives
    empire_owned_planet_ids = PlanetUtilsAI.get_owned_planets_by_empire(universe.planetIDs)
    invadable_planet_ids = set(all_owned_planet_ids).union(all_populated_planets) - set(empire_owned_planet_ids)

    invasion_targeted_planet_ids = get_invasion_targeted_planet_ids(universe.planetIDs, MissionType.INVASION)
    invasion_targeted_planet_ids.extend(
        get_invasion_targeted_planet_ids(universe.planetIDs, MissionType.ORBITAL_INVASION))
    all_invasion_targeted_system_ids = set(PlanetUtilsAI.get_systems(invasion_targeted_planet_ids))

    invasion_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.INVASION)
    num_invasion_fleets = len(FleetUtilsAI.extract_fleet_ids_without_mission_types(invasion_fleet_ids))

    debug("Current Invasion Targeted SystemIDs: %s" % PlanetUtilsAI.sys_name_ids(AIstate.invasionTargetedSystemIDs))
    debug("Current Invasion Targeted PlanetIDs: %s" % PlanetUtilsAI.planet_string(invasion_targeted_planet_ids))
    debug(invasion_fleet_ids and "Invasion Fleet IDs: %s" % invasion_fleet_ids or "Available Invasion Fleets: 0")
    debug("Invasion Fleets Without Missions: %s" % num_invasion_fleets)

    invasion_timer.start("planning troop base production")
    reserved_troop_base_targets = []
    if aistate.character.may_invade_with_bases():
        available_pp = {}
        for el in empire.planetsWithAvailablePP:  # keys are sets of ints; data is doubles
            avail_pp = el.data()
            for pid in el.key():
                available_pp[pid] = avail_pp
        # For planning base trooper invasion targets we have a two-pass system.  (1) In the first pass we consider all
        # the invasion targets and figure out which ones appear to be suitable for using base troopers against (i.e., we
        # already have a populated planet in the same system that could build base troopers) and we have at least a
        # minimal amount of PP available, and (2) in the second pass we go through the reserved base trooper target list
        # and check to make sure that there does not appear to be too much military action still needed before the
        # target is ready to be invaded, we double check that not too many base troopers would be needed, and if things
        # look clear then we queue up the base troopers on the Production Queue and keep track of where we are building
        # them, and how many; we may also disqualify and remove previously qualified targets (in case, for example,
        # we lost our base trooper source planet since it was first added to list).
        #
        # For planning and tracking base troopers under construction, we use a dictionary store in
        # get_aistate().qualifyingTroopBaseTargets, keyed by the invasion target planet ID.  We only store values
        # for invasion targets that appear likely to be suitable for base trooper use, and store a 2-item list.
        # The first item in this list is the ID of the planet where we expect to build the base troopers, and the second
        # entry initially is set to INVALID_ID (-1).  The presence of this entry in qualifyingTroopBaseTargets
        # flags this target as being reserved as a base-trooper invasion target.
        # In the second pass, if/when we actually start construction, then we modify the record, replacing that second
        # value with the ID of the planet where the troopers are actually being built.  (Right now that is always the
        # same as the source planet originally identified, but we could consider reevaluating that, or use that second
        # value to instead record how many base troopers have been queued, so that on later turns we can assess if the
        # process got delayed & perhaps more troopers need to be queued).
        secure_ai_fleet_missions = aistate.get_fleet_missions_with_any_mission_types([MissionType.SECURE,
                                                                                      MissionType.MILITARY])

        # Pass 1: identify qualifying base troop invasion targets
        for pid in invadable_planet_ids:  # TODO: reorganize
            if pid in aistate.qualifyingTroopBaseTargets:
                continue
            planet = universe.getPlanet(pid)
            if not planet:
                continue
            sys_id = planet.systemID
            sys_partial_vis_turn = get_partial_visibility_turn(sys_id)
            planet_partial_vis_turn = get_partial_visibility_turn(pid)
            if planet_partial_vis_turn < sys_partial_vis_turn:
                continue
            best_base_planet = INVALID_ID
            best_trooper_count = 0
            for pid2 in state.get_empire_planets_by_system(sys_id, include_outposts=False):
                if available_pp.get(pid2, 0) < 2:  # TODO: improve troop base PP sufficiency determination
                    break
                planet2 = universe.getPlanet(pid2)
                if not planet2 or planet2.speciesName not in ColonisationAI.empire_ship_builders:
                    continue
                best_base_trooper_here = \
                    ProductionAI.get_best_ship_info(PriorityType.PRODUCTION_ORBITAL_INVASION, pid2)[1]
                if not best_base_trooper_here:
                    continue
                troops_per_ship = best_base_trooper_here.troopCapacity
                if not troops_per_ship:
                    continue
                species_troop_grade = CombatRatingsAI.get_species_troops_grade(planet2.speciesName)
                troops_per_ship = CombatRatingsAI.weight_attack_troops(troops_per_ship, species_troop_grade)
                if troops_per_ship > best_trooper_count:
                    best_base_planet = pid2
                    best_trooper_count = troops_per_ship
            if best_base_planet != INVALID_ID:
                aistate.qualifyingTroopBaseTargets.setdefault(pid, [best_base_planet, INVALID_ID])

        # Pass 2: for each target previously identified for base troopers, check that still qualifies and
        # check how many base troopers would be needed; if reasonable then queue up the troops and record this in
        # get_aistate().qualifyingTroopBaseTargets
        for pid in aistate.qualifyingTroopBaseTargets.keys():
            planet = universe.getPlanet(pid)
            if planet and planet.owner == empire_id:
                del aistate.qualifyingTroopBaseTargets[pid]
                continue
            if pid in invasion_targeted_planet_ids:  # TODO: consider overriding standard invasion mission
                continue
            if aistate.qualifyingTroopBaseTargets[pid][1] != -1:
                reserved_troop_base_targets.append(pid)
                if planet:
                    all_invasion_targeted_system_ids.add(planet.systemID)
                # TODO: evaluate changes to situation, any more troops needed, etc.
                continue  # already building for here
            _, planet_troops = evaluate_invasion_planet(pid, secure_ai_fleet_missions, True)
            sys_id = planet.systemID
            this_sys_status = aistate.systemStatus.get(sys_id, {})
            troop_tally = 0
            for _fid in this_sys_status.get('myfleets', []):
                troop_tally += FleetUtilsAI.count_troops_in_fleet(_fid)
            if troop_tally > planet_troops:  # base troopers appear unneeded
                del aistate.qualifyingTroopBaseTargets[pid]
                continue
            if (planet.currentMeterValue(fo.meterType.shield) > 0 and
                    (this_sys_status.get('myFleetRating', 0) < 0.8 * this_sys_status.get('totalThreat', 0) or
                     this_sys_status.get('myFleetRatingVsPlanets', 0) < this_sys_status.get('planetThreat', 0))):
                # this system not secured, so ruling out invasion base troops for now
                # don't immediately delete from qualifyingTroopBaseTargets or it will be opened up for regular troops
                continue
            loc = aistate.qualifyingTroopBaseTargets[pid][0]
            best_base_trooper_here = ProductionAI.get_best_ship_info(PriorityType.PRODUCTION_ORBITAL_INVASION, loc)[1]
            loc_planet = universe.getPlanet(loc)
            if best_base_trooper_here is None:  # shouldn't be possible at this point, but just to be safe
                warn("Could not find a suitable orbital invasion design at %s" % loc_planet)
                continue
            # TODO: have TroopShipDesigner give the expected number of troops including species effects directly
            troops_per_ship = best_base_trooper_here.troopCapacity
            species_troop_grade = CombatRatingsAI.get_species_troops_grade(loc_planet.speciesName)
            troops_per_ship = CombatRatingsAI.weight_attack_troops(troops_per_ship, species_troop_grade)
            if not troops_per_ship:
                warn("The best orbital invasion design at %s seems not to have any troop capacity." % loc_planet)
                continue
            _, col_design, build_choices = ProductionAI.get_best_ship_info(PriorityType.PRODUCTION_ORBITAL_INVASION,
                                                                           loc)
            if not col_design:
                continue
            if loc not in build_choices:
                warn('Best troop design %s can not be produced at planet with id: %s\d' % (col_design, build_choices))
                continue
            n_bases = math.ceil((planet_troops + 1) / troops_per_ship)  # TODO: reconsider this +1 safety factor
            # TODO: evaluate cost and time-to-build of best base trooper here versus cost and time-to-build-and-travel
            # for best regular trooper elsewhere
            # For now, we assume what building base troopers is best so long as either (1) we would need no more than
            # MAX_BASE_TROOPERS_POOR_INVADERS base troop ships, or (2) our base troopers have more than 1 trooper per
            # ship and we would need no more than MAX_BASE_TROOPERS_GOOD_INVADERS base troop ships
            if (n_bases > MAX_BASE_TROOPERS_POOR_INVADERS or
                    (troops_per_ship > 1 and n_bases > MAX_BASE_TROOPERS_GOOD_INVADERS)):
                debug("ruling out base invasion troopers for %s due to high number (%d) required." % (planet, n_bases))
                del aistate.qualifyingTroopBaseTargets[pid]
                continue
            debug("Invasion base planning, need %d troops at %d per ship, will build %d ships." % (
                (planet_troops + 1), troops_per_ship, n_bases))
            retval = fo.issueEnqueueShipProductionOrder(col_design.id, loc)
            debug("Enqueueing %d Troop Bases at %s for %s" % (n_bases, PlanetUtilsAI.planet_string(loc),
                                                              PlanetUtilsAI.planet_string(pid)))
            if retval != 0:
                all_invasion_targeted_system_ids.add(planet.systemID)
                reserved_troop_base_targets.append(pid)
                aistate.qualifyingTroopBaseTargets[pid][1] = loc
                fo.issueChangeProductionQuantityOrder(empire.productionQueue.size - 1, 1, int(n_bases))
                fo.issueRequeueProductionOrder(empire.productionQueue.size - 1, 0)

    invasion_timer.start("evaluating target planets")
    # TODO: check if any invasion_targeted_planet_ids need more troops assigned
    evaluated_planet_ids = list(
        set(invadable_planet_ids) - set(invasion_targeted_planet_ids) - set(reserved_troop_base_targets))
    evaluated_planets = assign_invasion_values(evaluated_planet_ids)

    sorted_planets = [(pid, pscore % 10000, ptroops) for pid, (pscore, ptroops) in evaluated_planets.items()]
    sorted_planets.sort(key=lambda x: x[1], reverse=True)
    sorted_planets = [(pid, pscore % 10000, ptroops) for pid, pscore, ptroops in sorted_planets]

    invasion_table = Table([Text('Planet'), Float('Score'), Text('Species'), Float('Troops')],
                           table_name="Potential Targets for Invasion Turn %d" % fo.currentTurn())

    for pid, pscore, ptroops in sorted_planets:
        planet = universe.getPlanet(pid)
        invasion_table.add_row([
            planet,
            pscore,
            planet and planet.speciesName or "unknown",
            ptroops
        ])
    info(invasion_table)

    sorted_planets = filter(lambda x: x[1] > 0, sorted_planets)
    # export opponent planets for other AI modules
    AIstate.opponentPlanetIDs = [pid for pid, __, __ in sorted_planets]
    AIstate.invasionTargets = sorted_planets

    # export invasion targeted systems for other AI modules
    AIstate.invasionTargetedSystemIDs = list(all_invasion_targeted_system_ids)
    invasion_timer.stop(section_name="evaluating %d target planets" % (len(evaluated_planet_ids)))
    invasion_timer.stop_print_and_clear()


def get_invasion_targeted_planet_ids(planet_ids, mission_type):
    invasion_feet_missions = get_aistate().get_fleet_missions_with_any_mission_types([mission_type])
    targeted_planets = []
    for pid in planet_ids:
        # add planets that are target of a mission
        for mission in invasion_feet_missions:
            target = TargetPlanet(pid)
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


def assign_invasion_values(planet_ids):
    """Creates a dictionary that takes planet_ids as key and their invasion score as value."""
    empire_id = fo.empireID()
    planet_values = {}
    neighbor_values = {}
    neighbor_val_ratio = .95
    universe = fo.getUniverse()
    secure_missions = get_aistate().get_fleet_missions_with_any_mission_types([MissionType.SECURE,
                                                                               MissionType.MILITARY])
    for pid in planet_ids:
        planet_values[pid] = neighbor_values.setdefault(pid, evaluate_invasion_planet(pid, secure_missions))
        debug("planet %d, values %s", pid, planet_values[pid])
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
                    # to prevent divide-by-zero
                    planet_industries[pid2] = planet2.initialMeterValue(fo.meterType.industry) + 0.1
            industry_ratio = planet_industries[pid] / max(planet_industries.values())
            for pid2 in system.planetIDs:
                if pid2 == pid:
                    continue
                planet2 = universe.getPlanet(pid2)
                # TODO check for allies
                if (planet2 and (planet2.owner != empire_id) and
                        ((planet2.owner != -1) or (planet2.initialMeterValue(fo.meterType.population) > 0))):
                    planet_values[pid][0] += (
                        industry_ratio *
                        neighbor_val_ratio *
                        (neighbor_values.setdefault(pid2, evaluate_invasion_planet(pid2, secure_missions))[0])
                    )
    return planet_values


def evaluate_invasion_planet(planet_id, secure_fleet_missions, verbose=True):
    """Return the invasion value (score, troops) of a planet."""
    universe = fo.getUniverse()
    empire_id = fo.empireID()
    detail = []

    planet = universe.getPlanet(planet_id)
    if planet is None:
        debug("Invasion AI couldn't access any info for planet id %d" % planet_id)
        return [0, 0]

    system_id = planet.systemID

    # by using the following instead of simply relying on stealth meter reading, can (sometimes) plan ahead even if
    # planet is temporarily shrouded by an ion storm
    predicted_detectable = EspionageAI.colony_detectable_by_empire(planet_id, empire=fo.empireID(),
                                                                   default_result=False)
    if not predicted_detectable:
        if get_partial_visibility_turn(planet_id) < fo.currentTurn():
            debug("InvasionAI predicts planet id %d to be stealthed" % planet_id)
            return [0, 0]
        else:
            debug("InvasionAI predicts planet id %d to be stealthed" % planet_id +
                  ", but somehow have current visibity anyway, will still consider as target")

    # Check if the target planet was extra-stealthed somehow its system was last viewed
    # this test below may augment the tests above, but can be thrown off by temporary combat-related sighting
    system_last_seen = get_partial_visibility_turn(planet_id)
    planet_last_seen = get_partial_visibility_turn(system_id)
    if planet_last_seen < system_last_seen:
        # TODO: track detection strength, order new scouting when it goes up
        debug("Invasion AI considering planet id %d (stealthed at last view), still proceeding." % planet_id)

    # get a baseline evaluation of the planet as determined by ColonisationAI
    species_name = planet.speciesName
    species = fo.getSpecies(species_name)
    if not species or AIDependencies.TAG_DESTROYED_ON_CONQUEST in species.tags:
        # this call iterates over this Empire's available species with which it could colonize after an invasion
        planet_eval = ColonisationAI.assign_colonisation_values([planet_id], MissionType.INVASION, None, detail)
        colony_base_value = max(0.75 * planet_eval.get(planet_id, [0])[0],
                                ColonisationAI.evaluate_planet(planet_id, MissionType.OUTPOST, None, detail))
    else:
        colony_base_value = ColonisationAI.evaluate_planet(planet_id, MissionType.INVASION, species_name, detail)

    # Add extra score for all buildings on the planet
    building_values = {"BLD_IMPERIAL_PALACE": 1000,
                       "BLD_CULTURE_ARCHIVES": 1000,
                       "BLD_AUTO_HISTORY_ANALYSER": 100,
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
    bld_tally = 0
    for bldType in [universe.getBuilding(bldg).buildingTypeName for bldg in planet.buildingIDs]:
        bval = building_values.get(bldType, 50)
        bld_tally += bval
        detail.append("%s: %d" % (bldType, bval))

    # Add extra score for unlocked techs when we conquer the species
    tech_tally = 0
    value_per_pp = 4
    for unlocked_tech in AIDependencies.SPECIES_TECH_UNLOCKS.get(species_name, []):
        if not tech_is_complete(unlocked_tech):
            rp_cost = fo.getTech(unlocked_tech).researchCost(empire_id)
            tech_value = value_per_pp * rp_cost
            tech_tally += tech_value
            detail.append("%s: %d" % (unlocked_tech, tech_value))

    max_jumps = 8
    capitol_id = PlanetUtilsAI.get_capital()
    least_jumps_path = []
    clear_path = True
    if capitol_id:
        homeworld = universe.getPlanet(capitol_id)
        if homeworld and homeworld.systemID != INVALID_ID and system_id != INVALID_ID:
            least_jumps_path = list(universe.leastJumpsPath(homeworld.systemID, system_id, empire_id))
            max_jumps = len(least_jumps_path)
    aistate = get_aistate()
    system_status = aistate.systemStatus.get(system_id, {})
    system_fleet_treat = system_status.get('fleetThreat', 1000)
    system_monster_threat = system_status.get('monsterThreat', 0)
    sys_total_threat = system_fleet_treat + system_monster_threat + system_status.get('planetThreat', 0)
    max_path_threat = system_fleet_treat
    mil_ship_rating = MilitaryAI.cur_best_mil_ship_rating()
    for path_sys_id in least_jumps_path:
        path_leg_status = aistate.systemStatus.get(path_sys_id, {})
        path_leg_threat = path_leg_status.get('fleetThreat', 1000) + path_leg_status.get('monsterThreat', 0)
        if path_leg_threat > 0.5 * mil_ship_rating:
            clear_path = False
            if path_leg_threat > max_path_threat:
                max_path_threat = path_leg_threat

    pop = planet.currentMeterValue(fo.meterType.population)
    target_pop = planet.currentMeterValue(fo.meterType.targetPopulation)
    troops = planet.currentMeterValue(fo.meterType.troops)
    troop_regen = planet.currentMeterValue(fo.meterType.troops) - planet.initialMeterValue(fo.meterType.troops)
    max_troops = planet.currentMeterValue(fo.meterType.maxTroops)
    # TODO: refactor troop determination into function for use in mid-mission updates and also consider defender techs
    max_troops += AIDependencies.TROOPS_PER_POP * (target_pop - pop)

    this_system = universe.getSystem(system_id)
    secure_targets = [system_id] + list(this_system.planetIDs)
    system_secured = False
    for mission in secure_fleet_missions:
        if system_secured:
            break
        secure_fleet_id = mission.fleet.id
        s_fleet = universe.getFleet(secure_fleet_id)
        if not s_fleet or s_fleet.systemID != system_id:
            continue
        if mission.type in [MissionType.SECURE, MissionType.MILITARY]:
            target_obj = mission.target.get_object()
            if target_obj is not None and target_obj.id in secure_targets:
                system_secured = True
                break
    system_secured = system_secured and system_status.get('myFleetRating', 0)

    if verbose:
        debug("Invasion eval of %s\n"
              " - maxShields: %.1f\n"
              " - sysFleetThreat: %.1f\n"
              " - sysMonsterThreat: %.1f",
              planet, planet.currentMeterValue(fo.meterType.maxShield), system_fleet_treat, system_monster_threat)
    enemy_val = 0
    if planet.owner != -1:  # value in taking this away from an enemy
        enemy_val = 20 * (planet.currentMeterValue(fo.meterType.targetIndustry) +
                          2*planet.currentMeterValue(fo.meterType.targetResearch))

    # devalue invasions that would require too much military force
    preferred_max_portion = MilitaryAI.get_preferred_max_military_portion_for_single_battle()
    total_max_mil_rating = MilitaryAI.get_concentrated_tot_mil_rating()
    threat_exponent = 2  # TODO: make this a character trait; higher aggression with a lower exponent
    threat_factor = min(1, preferred_max_portion * total_max_mil_rating/(sys_total_threat+0.001))**threat_exponent

    design_id, _, locs = ProductionAI.get_best_ship_info(PriorityType.PRODUCTION_INVASION)
    if not locs or not universe.getPlanet(locs[0]):
        # We are in trouble anyway, so just calculate whatever approximation...
        build_time = 4
        planned_troops = troops if system_secured else min(troops + troop_regen*(max_jumps + build_time), max_troops)
        planned_troops += .01  # we must attack with more troops than there are defenders
        troop_cost = math.ceil((planned_troops+_TROOPS_SAFETY_MARGIN) / 6.0) * 20 * FleetUtilsAI.get_fleet_upkeep()
    else:
        loc = locs[0]
        species_here = universe.getPlanet(loc).speciesName
        design = fo.getShipDesign(design_id)
        cost_per_ship = design.productionCost(empire_id, loc)
        build_time = design.productionTime(empire_id, loc)
        troops_per_ship = CombatRatingsAI.weight_attack_troops(design.troopCapacity,
                                                               CombatRatingsAI.get_species_troops_grade(species_here))
        planned_troops = troops if system_secured else min(troops + troop_regen*(max_jumps + build_time), max_troops)
        planned_troops += .01  # we must attack with more troops than there are defenders
        ships_needed = math.ceil((planned_troops+_TROOPS_SAFETY_MARGIN) / float(troops_per_ship))
        troop_cost = ships_needed * cost_per_ship  # fleet upkeep is already included in query from server

    # apply some bias to expensive operations
    normalized_cost = float(troop_cost) / max(fo.getEmpire().productionPoints, 1)
    normalized_cost = max(1., normalized_cost)
    cost_score = (normalized_cost**2 / 50.0) * troop_cost

    base_score = colony_base_value + bld_tally + tech_tally + enemy_val - cost_score
    # If the AI does have enough total miltary to attack this target, and the target is more than minimally valuable,
    # don't let the threat_factor discount the adjusted value below MIN_INVASION_SCORE +1, so that if there are no
    # other targets the AI could still pursue this one.  Otherwise, scoring pressure from
    # MilitaryAI.get_preferred_max_military_portion_for_single_battle might prevent the AI from attacking heavily
    # defended but still defeatable targets even if it has no softer targets available.
    if total_max_mil_rating > sys_total_threat and base_score > 2 * MIN_INVASION_SCORE:
        threat_factor = max(threat_factor, (MIN_INVASION_SCORE + 1)/base_score)
    planet_score = retaliation_risk_factor(planet.owner) * threat_factor * max(0, base_score)
    if clear_path:
        planet_score *= 1.5
    if verbose:
        debug(' - planet score: %.2f\n'
              ' - planned troops: %.2f\n'
              ' - projected troop cost: %.1f\n'
              ' - threat factor: %s\n'
              ' - planet detail: %s\n'
              ' - popval: %.1f\n'
              ' - bldval: %s\n'
              ' - enemyval: %s',
              planet_score, planned_troops, troop_cost, threat_factor, detail, colony_base_value, bld_tally, enemy_val)
        debug(' - system secured: %s' % system_secured)
    return [planet_score, planned_troops]


def send_invasion_fleets(fleet_ids, evaluated_planets, mission_type):
    """sends a list of invasion fleets to a list of planet_value_pairs"""
    if not fleet_ids:
        return

    universe = fo.getUniverse()
    invasion_fleet_pool = set(fleet_ids)

    for planet_id, pscore, ptroops in evaluated_planets:
        if pscore < MIN_INVASION_SCORE:
            continue
        planet = universe.getPlanet(planet_id)
        if not planet:
            continue
        sys_id = planet.systemID
        found_fleets = []
        found_stats = {}
        min_stats = {'rating': 0, 'troopCapacity': ptroops}
        target_stats = {'rating': 10,
                        'troopCapacity': ptroops + _TROOPS_SAFETY_MARGIN,
                        'target_system': TargetSystem(sys_id)}
        these_fleets = FleetUtilsAI.get_fleets_for_mission(target_stats, min_stats, found_stats,
                                                           starting_system=sys_id, fleet_pool_set=invasion_fleet_pool,
                                                           fleet_list=found_fleets)
        if not these_fleets:
            if not FleetUtilsAI.stats_meet_reqs(found_stats, min_stats):
                debug("Insufficient invasion troop allocation for system %d ( %s ) -- requested %s , found %s" % (
                    sys_id, universe.getSystem(sys_id).name, min_stats, found_stats))
                invasion_fleet_pool.update(found_fleets)
                continue
            else:
                these_fleets = found_fleets
        target = TargetPlanet(planet_id)
        debug("assigning invasion fleets %s to target %s" % (these_fleets, target))
        aistate = get_aistate()
        for fleetID in these_fleets:
            fleet_mission = aistate.get_fleet_mission(fleetID)
            fleet_mission.clear_fleet_orders()
            fleet_mission.clear_target()
            fleet_mission.set_target(mission_type, target)


def assign_invasion_bases():
    """Assign our troop bases to invasion targets."""
    universe = fo.getUniverse()
    all_troopbase_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.ORBITAL_INVASION)
    available_troopbase_fleet_ids = set(FleetUtilsAI.extract_fleet_ids_without_mission_types(all_troopbase_fleet_ids))

    aistate = get_aistate()
    for fid in list(available_troopbase_fleet_ids):
        if fid not in available_troopbase_fleet_ids:  # entry may have been discarded in previous loop iterations
            continue
        fleet = universe.getFleet(fid)
        if not fleet:
            continue
        sys_id = fleet.systemID
        system = universe.getSystem(sys_id)
        available_planets = set(system.planetIDs).intersection(set(aistate.qualifyingTroopBaseTargets.keys()))
        debug("Considering Base Troopers in %s, found planets %s and registered targets %s with status %s" % (
            system.name, list(system.planetIDs), available_planets,
            [(pid, aistate.qualifyingTroopBaseTargets[pid]) for pid in available_planets]))
        targets = [pid for pid in available_planets if aistate.qualifyingTroopBaseTargets[pid][1] != -1]
        if not targets:
            debug("Failure: found no valid target for troop base in system %s" % system)
            continue
        status = aistate.systemStatus.get(sys_id, {})
        local_base_troops = set(status.get('myfleets', [])).intersection(available_troopbase_fleet_ids)

        target_id = INVALID_ID
        best_score = -1
        target_troops = 0
        for pid, (p_score, p_troops) in assign_invasion_values(targets).items():
            if p_score > best_score:
                best_score = p_score
                target_id = pid
                target_troops = p_troops
        if target_id == INVALID_ID:
            continue
        local_base_troops.discard(fid)
        found_fleets = []
        troops_needed = max(0, target_troops - FleetUtilsAI.count_troops_in_fleet(fid))
        found_stats = {}
        min_stats = {'rating': 0, 'troopCapacity': troops_needed}
        target_stats = {'rating': 10, 'troopCapacity': troops_needed + _TROOPS_SAFETY_MARGIN}

        FleetUtilsAI.get_fleets_for_mission(target_stats, min_stats, found_stats,
                                            starting_system=sys_id, fleet_pool_set=local_base_troops,
                                            fleet_list=found_fleets)
        for fid2 in found_fleets:
            FleetUtilsAI.merge_fleet_a_into_b(fid2, fid)
            available_troopbase_fleet_ids.discard(fid2)
        available_troopbase_fleet_ids.discard(fid)
        aistate.qualifyingTroopBaseTargets[target_id][1] = -1  # TODO: should probably delete
        target = TargetPlanet(target_id)
        fleet_mission = aistate.get_fleet_mission(fid)
        fleet_mission.set_target(MissionType.ORBITAL_INVASION, target)


def assign_invasion_fleets_to_invade():
    """Assign fleet targets to invadable planets."""
    aistate = get_aistate()

    assign_invasion_bases()

    all_invasion_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.INVASION)
    invasion_fleet_ids = FleetUtilsAI.extract_fleet_ids_without_mission_types(all_invasion_fleet_ids)
    send_invasion_fleets(invasion_fleet_ids, AIstate.invasionTargets, MissionType.INVASION)
    all_invasion_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.INVASION)
    for fid in FleetUtilsAI.extract_fleet_ids_without_mission_types(all_invasion_fleet_ids):
        this_mission = aistate.get_fleet_mission(fid)
        this_mission.check_mergers(context="Post-send consolidation of unassigned troops")
