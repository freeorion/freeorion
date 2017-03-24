import freeOrionAIInterface as fo  # interface used to interact with FreeOrion AI client # pylint: disable=import-error
import FreeOrionAI as foAI
import FleetUtilsAI
from EnumsAI import MissionType
import universe_object
import MoveUtilsAI
import PlanetUtilsAI
from freeorion_tools import dict_from_map, print_error
from AIDependencies import INVALID_ID


TARGET_POP = 'targetPop'
TROOPS = 'troops'

graphFlags = {}
interiorExploredSystemIDs = {}  # explored systems whose neighbors are also all
borderExploredSystemIDs = {}
borderUnexploredSystemIDs = {}


def get_current_exploration_info():
    """Returns ([current target list], [available scout list])."""
    fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.EXPLORATION)
    available_scouts = []
    already_covered = set()
    for fleet_id in fleet_ids:
        fleet_mission = foAI.foAIstate.get_fleet_mission(fleet_id)
        if not fleet_mission.type:
            available_scouts.append(fleet_id)
        else:
            if fleet_mission.type == MissionType.EXPLORATION:
                already_covered.add(fleet_mission.target)
                if not fleet_mission.target:
                    print "problem determining existing exploration target systems"
                else:
                    print "found existing exploration target: %s" % fleet_mission.target
    return list(already_covered), available_scouts


def assign_scouts_to_explore_systems():
    # TODO: use Graph Theory to explore closest systems
    universe = fo.getUniverse()
    capital_sys_id = PlanetUtilsAI.get_capital_sys_id()
    # order fleets to explore
    explorable_system_ids = list(borderUnexploredSystemIDs)
    if not explorable_system_ids or (capital_sys_id == INVALID_ID):
        return
    exp_systems_by_dist = sorted((universe.linearDistance(capital_sys_id, x), x) for x in explorable_system_ids)
    print "Exploration system considering following system-distance pairs:\n  %s" % ("\n  ".join("%3d: %5.1f" % info for info in exp_systems_by_dist))
    explore_list = [sys_id for dist, sys_id in exp_systems_by_dist]

    already_covered, available_scouts = get_current_exploration_info()

    print "Explorable system IDs: %s" % explore_list
    print "Already targeted: %s" % already_covered
    needs_vis = foAI.foAIstate.misc.setdefault('needs_vis', [])
    check_list = foAI.foAIstate.needsEmergencyExploration + needs_vis + explore_list
    if INVALID_ID in check_list:  # shouldn't normally happen, unless due to bug elsewhere
        for sys_list, name in [(foAI.foAIstate.needsEmergencyExploration, "foAI.foAIstate.needsEmergencyExploration"), (needs_vis, "needs_vis"), (explore_list, "explore_list")]:
            if INVALID_ID in sys_list:
                print_error("INVALID_ID found in " + name)
    needs_coverage = [sys_id for sys_id in check_list if sys_id not in already_covered and sys_id != INVALID_ID]  # emergency coverage can be due to invasion detection trouble, etc.
    print "Needs coverage: %s" % needs_coverage

    print "Available scouts & AIstate locs: %s" % [(x, foAI.foAIstate.fleetStatus.get(x, {}).get('sysID', INVALID_ID)) for x in available_scouts]
    print "Available scouts & universe locs: %s" % [(x, universe.getFleet(x).systemID) for x in available_scouts]
    if not needs_coverage or not available_scouts:
        return

    available_scouts = set(available_scouts)
    sent_list = []
    while available_scouts and needs_coverage:
        this_sys_id = needs_coverage.pop(0)
        sys_status = foAI.foAIstate.systemStatus.setdefault(this_sys_id, {})
        if this_sys_id not in explore_list:  # doesn't necessarily need direct visit
            if universe.getVisibility(this_sys_id, fo.empireID()) >= fo.visibility.partial:
                # already got visibility; remove from visit lists and skip
                if this_sys_id in needs_vis:
                    del needs_vis[needs_vis.index(this_sys_id)]
                if this_sys_id in foAI.foAIstate.needsEmergencyExploration:
                    del foAI.foAIstate.needsEmergencyExploration[
                        foAI.foAIstate.needsEmergencyExploration.index(this_sys_id)]
                print "system id %d already currently visible; skipping exploration" % this_sys_id
                continue
        # TODO: if blocked byu monster, try to find nearby system from which to see this system
        if not foAI.foAIstate.character.may_explore_system(sys_status.setdefault('monsterThreat', 0)) or (fo.currentTurn() < 20 and foAI.foAIstate.systemStatus[this_sys_id]['monsterThreat'] > 200):
            print "Skipping exploration of system %d due to Big Monster, threat %d" % (this_sys_id, foAI.foAIstate.systemStatus[this_sys_id]['monsterThreat'])
            continue
        this_fleet_list = FleetUtilsAI.get_fleets_for_mission(target_stats={}, min_stats={}, cur_stats={},
                                                              starting_system=this_sys_id, fleet_pool_set=available_scouts,
                                                              fleet_list=[])
        if not this_fleet_list:
            print "Seem to have run out of scouts while trying to cover sys_id %d" % this_sys_id
            break  # must have ran out of scouts
        fleet_id = this_fleet_list[0]
        fleet_mission = foAI.foAIstate.get_fleet_mission(fleet_id)
        target = universe_object.System(this_sys_id)
        if len(MoveUtilsAI.can_travel_to_system_and_return_to_resupply(fleet_id, fleet_mission.get_location_target(), target)) > 0:
            fleet_mission.set_target(MissionType.EXPLORATION, target)
            sent_list.append(this_sys_id)
        else:  # system too far out, skip it, but can add scout back to available pool
            print "sys_id %d too far out for fleet ( ID %d ) to reach" % (this_sys_id, fleet_id)
            available_scouts.update(this_fleet_list)
    print "Sent scouting fleets to sysIDs : %s" % sent_list
    return
    # pylint: disable=pointless-string-statement
    """
    #TODO: consider matching system to closest scout, also consider rejecting scouts that would travel a blockaded path
    sent_list=[]
    sysList= list(explorable_system_ids)
    shuffle( sysList ) #so that a monster defended system wont always be selected early
    fleetList = list(available_scouts)
    isys= -1
    jfleet= -1
    while ( jfleet < len(fleetList) -1) :
        jfleet += 1
        fleet_id = fleetList[ jfleet ]
        while ( isys < len(sysList) -1) :
            isys += 1
            sys_id = sysList[ isys]
            fleet_mission = foAI.foAIstate.get_fleet_mission( fleet_id )
            target = AITarget.AITarget(AITargetType.TARGET_SYSTEM, sys_id )
            # add exploration mission to fleet with target unexplored system and this system is in range
            #print "try to assign scout to system %d"%systemID
            if len(MoveUtilsAI.can_travel_to_system_and_return_to_resupply(fleet_id, fleet_mission.get_location_target(), target, fo.empireID())) > 0:
                fleet_mission.addAITarget(MissionType.EXPLORATION, target)
                sent_list.append(sys_id)
                break
    print "sent scouting fleets to sysIDs : %s"%sent_list
    """


def follow_vis_system_connections(start_system_id, home_system_id):
    universe = fo.getUniverse()
    empire_id = fo.empireID()
    exploration_list = [start_system_id]
    while exploration_list:
        cur_system_id = exploration_list.pop()
        if cur_system_id in graphFlags:
            continue
        graphFlags[cur_system_id] = 1
        system = universe.getSystem(cur_system_id)
        if cur_system_id in foAI.foAIstate.visBorderSystemIDs:
            pre_vis = "a border system"
        elif cur_system_id in foAI.foAIstate.visInteriorSystemIDs:
            pre_vis = "an interior system"
        else:
            pre_vis = "an unknown system"
        system_header = "*** system %s;" % system
        if fo.currentTurn() < 50:
            visibility_turn_list = sorted(universe.getVisibilityTurnsMap(cur_system_id, empire_id).items(),
                                          key=lambda x: x[0].numerator)
            visibility_info = ', '.join('%s: %s' % (vis.name, turn) for vis, turn in visibility_turn_list)
            print "%s previously %s. Visibility per turn: %s " % (system_header, pre_vis, visibility_info)
            status_info = []
        else:
            status_info = [system_header]

        has_been_visible = universe.getVisibilityTurnsMap(cur_system_id, empire_id).get(fo.visibility.partial, 0) > 0
        is_connected = universe.systemsConnected(cur_system_id, home_system_id, -1)  # self.empire_id)
        status_info.append("    -- is%s partially visible" % ([" not", ""][has_been_visible]))
        status_info.append("    -- is%s visibly connected to homesystem" % ([" not", ""][is_connected]))
        if has_been_visible:
            sys_status = foAI.foAIstate.systemStatus.setdefault(cur_system_id, {})
            foAI.foAIstate.visInteriorSystemIDs.add(cur_system_id)
            foAI.foAIstate.visBorderSystemIDs.discard(cur_system_id)
            neighbors = set(dict_from_map(universe.getSystemNeighborsMap(cur_system_id, empire_id)).keys())
            sys_status.setdefault('neighbors', set()).update(neighbors)
            sys_planets = sys_status.setdefault('planets', {})
            if fo.currentTurn() < 50:
                print "    previously known planets: %s" % sys_planets.keys()
            if system:
                for planet_id in system.planetIDs:
                    sys_planets.setdefault(planet_id, {}).setdefault(TARGET_POP, 0)
                    sys_planets[planet_id].setdefault(TROOPS, 0)
                    planet = universe.getPlanet(planet_id)

                    if planet:
                        new_pop = planet.currentMeterValue(fo.meterType.targetPopulation)
                        if new_pop != sys_planets[planet_id][TARGET_POP]:
                            if fo.currentTurn() < 50:
                                print "  * updating targetPop of planet %s to %.2f from %.2f" % (planet, new_pop, sys_planets[planet_id][TARGET_POP])
                        troops = planet.currentMeterValue(fo.meterType.troops)
                        if troops != sys_planets[planet_id].get(TROOPS, 0):
                            if fo.currentTurn() < 50:
                                print "  * updating troops of planet %s to %.2f from %.2f" % (planet, troops, sys_planets[planet_id][
                                    TROOPS])
                        sys_planets[planet_id][TARGET_POP] = new_pop
                        sys_planets[planet_id][TROOPS] = troops
            if fo.currentTurn() < 50:
                print "    known planets %s" % sys_planets.keys()
            if neighbors:
                status_info.append(" -- has neighbors %s" % sorted(neighbors))
                for sys_id in neighbors:
                    if sys_id not in foAI.foAIstate.exploredSystemIDs:
                        foAI.foAIstate.unexploredSystemIDs.add(sys_id)
                    if (sys_id not in graphFlags) and (sys_id not in foAI.foAIstate.visInteriorSystemIDs):
                        foAI.foAIstate.visBorderSystemIDs.add(sys_id)
                        exploration_list.append(sys_id)
        if fo.currentTurn() < 50:
            print '\n'.join(status_info)
            print "----------------------------------------------------------"


def update_explored_systems():
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    obs_lanes = empire.obstructedStarlanes()
    obs_lanes_list = [el for el in obs_lanes]  # should result in list of tuples (sys_id1, sys_id2)
    if obs_lanes_list:
        print "Obstructed starlanes are: %s" % ', '.join('%s-%s' % item for item in obs_lanes_list)
    else:
        print "No obstructed Starlanes"
    empire_id = fo.empireID()
    newly_explored = []
    still_unexplored = []
    for sys_id in list(foAI.foAIstate.unexploredSystemIDs):
        if empire.hasExploredSystem(sys_id):  # consider making determination according to visibility rather than actual visit, which I think is what empire.hasExploredSystem covers
            foAI.foAIstate.unexploredSystemIDs.discard(sys_id)
            foAI.foAIstate.exploredSystemIDs.add(sys_id)
            system = universe.getSystem(sys_id)
            print "Moved system %s from unexplored list to explored list" % system
            if sys_id in borderUnexploredSystemIDs:
                del borderUnexploredSystemIDs[sys_id]
            newly_explored.append(sys_id)
        else:
            still_unexplored.append(sys_id)

    neighbor_list = []
    dummy = []
    for id_list, next_list in [(newly_explored, neighbor_list), (neighbor_list, dummy)]:
        for sys_id in id_list:
            neighbors = list(universe.getImmediateNeighbors(sys_id, empire_id))
            all_explored = True
            for neighbor_id in neighbors:
                if neighbor_id in foAI.foAIstate.unexploredSystemIDs:  # when it matters, unexplored will be smaller than explored
                    all_explored = False
                else:
                    next_list.append(neighbor_id)
            if all_explored:
                interiorExploredSystemIDs[sys_id] = 1
                if sys_id in borderExploredSystemIDs:
                    del borderExploredSystemIDs[sys_id]
            else:
                borderExploredSystemIDs[sys_id] = 1

    for sys_id in still_unexplored:
        neighbors = list(universe.getImmediateNeighbors(sys_id, empire_id))
        any_explored = False
        for neighbor_id in neighbors:
            if neighbor_id in foAI.foAIstate.exploredSystemIDs:  # consider changing to unexplored test -- when it matters, unexplored will be smaller than explored, but need to not get previously untreated neighbors
                any_explored = True
        if any_explored:
            borderUnexploredSystemIDs[sys_id] = 1
    return newly_explored
