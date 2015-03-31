import freeOrionAIInterface as fo  # interface used to interact with FreeOrion AI client # pylint: disable=import-error
import FreeOrionAI as foAI
import FleetUtilsAI
from EnumsAI import AIFleetMissionType
import AITarget
import MoveUtilsAI
import PlanetUtilsAI
from freeorion_tools import dict_from_map


TARGET_POP = 'targetPop'
TROOPS = 'troops'

graphFlags = {}
interiorExploredSystemIDs = {}  # explored systems whose neighbors are also all
borderExploredSystemIDs = {}
borderUnexploredSystemIDs = {}

def get_current_exploration_info(verbose=True):
    """returns ( [current target list] , [available scout list] ) """
    fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(AIFleetMissionType.FLEET_MISSION_EXPLORATION)
    available_scouts = []
    already_covered = set()
    for fleet_id in fleet_ids:
        fleet_mission = foAI.foAIstate.get_fleet_mission(fleet_id)
        if len(fleet_mission.get_mission_types()) == 0:
            available_scouts.append(fleet_id)
        else:
            targets = [targ.target_id for targ in fleet_mission.get_targets(AIFleetMissionType.FLEET_MISSION_EXPLORATION)]
            if verbose:
                if len(targets) == 0:
                    print "problem determining existing exploration target systems from targets:\n%s" % (fleet_mission.get_targets(AIFleetMissionType.FLEET_MISSION_EXPLORATION))
                else:
                    print "found existing exploration targets: %s" % targets
            already_covered.update(targets)
    return list(already_covered), available_scouts


def assign_scouts_to_explore_systems():
    # TODO: use Graph Theory to explore closest systems
    universe = fo.getUniverse()
    capital_sys_id = PlanetUtilsAI.get_capital_sys_id()
    # order fleets to explore
    #explorable_system_ids = foAI.foAIstate.get_explorable_systems(AIExplorableSystemType.EXPLORABLE_SYSTEM_UNEXPLORED)
    explorable_system_ids = list(borderUnexploredSystemIDs)
    if not explorable_system_ids or (capital_sys_id == -1):
        return
    exp_systems_by_dist = sorted(map(lambda x: (universe.linearDistance(capital_sys_id, x), x), explorable_system_ids))
    print "Exploration system considering following system-distance pairs:\n %s" % ("[ " + ", ".join(["%3d : %5.1f" % (sys, dist) for dist, sys in exp_systems_by_dist]) + " ]")
    explore_list = [sys_id for dist, sys_id in exp_systems_by_dist]

    already_covered, available_scouts = get_current_exploration_info()

    print "explorable sys IDs: %s" % explore_list
    print "already targeted: %s" % already_covered
    if 'needsEmergencyExploration' not in dir(foAI.foAIstate):
        foAI.foAIstate.needsEmergencyExploration = []
    needs_coverage = foAI.foAIstate.needsEmergencyExploration + [sys_id for sys_id in explore_list if sys_id not in already_covered]  # emergency coverage cane be due to invasion detection trouble, etc.
    print "needs coverage: %s" % needs_coverage

    print "available scouts & AIstate locs: %s" % (map(lambda x: (x, foAI.foAIstate.fleetStatus.get(x, {}).get('sysID', -1)), available_scouts))
    print "available scouts & universe locs: %s" % (map(lambda x: (x, universe.getFleet(x).systemID), available_scouts))
    if not needs_coverage or not available_scouts:
        return

    available_scouts = set(available_scouts)
    sent_list = []
    while (len(available_scouts) > 0) and (len(needs_coverage) > 0):
        this_sys_id = needs_coverage.pop(0)
        if (foAI.foAIstate.systemStatus.setdefault(this_sys_id, {}).setdefault('monsterThreat', 0) > 2000 * foAI.foAIstate.aggression) or (fo.currentTurn() < 20 and foAI.foAIstate.systemStatus[this_sys_id]['monsterThreat'] > 200):
            print "Skipping exploration of system %d due to Big Monster, threat %d" % (this_sys_id, foAI.foAIstate.systemStatus[this_sys_id]['monsterThreat'])
            continue
        found_fleets = []
        this_fleet_list = FleetUtilsAI.get_fleets_for_mission(nships=1, target_stats={}, min_stats={}, cur_stats={}, species="", systems_to_check=[this_sys_id], systems_checked=[],
                                                     fleet_pool_set=available_scouts, fleet_list=found_fleets, verbose=False)
        if not this_fleet_list:
            print "seem to have run out of scouts while trying to cover sys_id %d" % this_sys_id
            break  # must have ran out of scouts
        fleet_id = this_fleet_list[0]
        fleet_mission = foAI.foAIstate.get_fleet_mission(fleet_id)
        target = AITarget.TargetSystem(this_sys_id)
        if len(MoveUtilsAI.can_travel_to_system_and_return_to_resupply(fleet_id, fleet_mission.get_location_target(), target)) > 0:
            fleet_mission.add_target(AIFleetMissionType.FLEET_MISSION_EXPLORATION, target)
            sent_list.append(this_sys_id)
        else:  # system too far out, skip it, but can add scout back to available pool
            print "sys_id %d too far out for fleet ( ID %d ) to reach" % (this_sys_id, fleet_id)
            available_scouts.update(this_fleet_list)
    print "sent scouting fleets to sysIDs : %s" % sent_list
    return
    # pylint: disable=pointless-string-statement
    """
    #TODO: consider matching sys to closest scout, also consider rejecting scouts that would travel a blockaded path
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
                fleet_mission.addAITarget(AIFleetMissionType.FLEET_MISSION_EXPLORATION, target)
                sent_list.append(sys_id)
                break
    print "sent scouting fleets to sysIDs : %s"%sent_list
    """


def follow_vis_system_connections(start_system_id, home_system_id):
    universe = fo.getUniverse()
    empire_id = foAI.foAIstate.empireID
    exploration_list = [start_system_id]
    while exploration_list:
        cur_system_id = exploration_list.pop()
        if cur_system_id in graphFlags:
            continue
        graphFlags[cur_system_id] = 1
        system = universe.getSystem(cur_system_id)
        if not system:
            sys_name = foAI.foAIstate.systemStatus.get(cur_system_id, {}).get('name', "name unknown")
        else:
            sys_name = system.name or foAI.foAIstate.systemStatus.get(cur_system_id, {}).get('name', "name unknown")
        if cur_system_id in foAI.foAIstate.visBorderSystemIDs:
            pre_vis = "a border system"
        elif cur_system_id in foAI.foAIstate.visInteriorSystemIDs:
            pre_vis = "an interior system"
        else:
            pre_vis = "an unknown system"
        if fo.currentTurn() < 50:
            visibility_turn_list = sorted(universe.getVisibilityTurnsMap(cur_system_id, empire_id).items(),
                                          key=lambda x: x[0].numerator)
            visibility_info = ['%s: %s' % (vis.name, turn) for vis, turn in visibility_turn_list]
            print "*** system ID %d ( %s ) ; previously %s, new visibility turns info: %s " % (cur_system_id, sys_name, pre_vis, visibility_info)
        status_str = "*** system ID %d ( %s ) ; " % (cur_system_id, sys_name)
        has_been_visible = universe.getVisibilityTurnsMap(cur_system_id, empire_id).get(fo.visibility.partial, 0) > 0
        is_connected = universe.systemsConnected(cur_system_id, home_system_id, -1)  # self.empire_id)
        status_str += " -- is %s partially visible " % (["not", ""][has_been_visible])
        status_str += " -- is %s visibly connected to homesystem " % (["not", ""][is_connected])
        if has_been_visible:
            sys_status = foAI.foAIstate.systemStatus.setdefault(cur_system_id, {})
            foAI.foAIstate.visInteriorSystemIDs[cur_system_id] = 1
            if cur_system_id in foAI.foAIstate.visBorderSystemIDs:
                del foAI.foAIstate.visBorderSystemIDs[cur_system_id]
            #neighbors= dict( [(el.key(), el.data()) for el in universe.getSystemNeighborsMap(cur_system_id, empire_id)] )  #
            neighbors = set(dict_from_map(universe.getSystemNeighborsMap(cur_system_id, empire_id)).keys())
            sys_status.setdefault('neighbors', set()).update(neighbors)
            sys_planets = sys_status.setdefault('planets', {})
            if fo.currentTurn() < 50:
                print "    previously knew of system %d planets %s" % (cur_system_id, sys_planets.keys())
            if system:
                for planet_id in system.planetIDs:
                    sys_planets.setdefault(planet_id, {}).setdefault(TARGET_POP, 0)
                    sys_planets[planet_id].setdefault(TROOPS, 0)
                    planet = universe.getPlanet(planet_id)

                    if planet:
                        new_pop = planet.currentMeterValue(fo.meterType.targetPopulation)
                        if new_pop != sys_planets[planet_id][TARGET_POP]:
                            if fo.currentTurn() < 50:
                                print "  * updating targetPop of planet %d ( %s ) to %.2f from %.2f" % (planet_id, planet.name, new_pop, sys_planets[planet_id][TARGET_POP])
                        troops = planet.currentMeterValue(fo.meterType.troops)
                        if troops != sys_planets[planet_id].get(TROOPS, 0):
                            if fo.currentTurn() < 50:
                                print "  * updating troops of planet %d ( %s ) to %.2f from %.2f" % (planet_id, planet.name, troops, sys_planets[planet_id][
                                    TROOPS])
                        sys_planets[planet_id][TARGET_POP] = new_pop
                        sys_planets[planet_id][TROOPS] = troops
            if fo.currentTurn() < 50:
                print "    now know of system %d planets %s" % (cur_system_id, sys_planets.keys())
            #neighbors = list( universe.getImmediateNeighbors(cur_system_id, empire_id) )  #imNeighbors
            #if set(neighbors) != set(neighbors2):
            # print "Error with neighbors: imn giving %s ; giN giving %s"%(neighbors2, neighbors)
            if neighbors:
                status_str += " -- has neighbors %s " % neighbors
                for sys_id in neighbors:
                    if sys_id not in foAI.foAIstate.exploredSystemIDs:
                        foAI.foAIstate.unexploredSystemIDs[sys_id] = 1
                    if (sys_id not in graphFlags) and (sys_id not in foAI.foAIstate.visInteriorSystemIDs):
                        foAI.foAIstate.visBorderSystemIDs[sys_id] = 1
                        exploration_list.append(sys_id)
        if fo.currentTurn() < 50:
            print status_str
            print "----------------------------------------------------------"


def update_explored_systems():
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    obs_lanes = empire.obstructedStarlanes()
    #print "object is: %s"%(obs_lanes, ) #IntPairVec
    obs_lanes_list = [el for el in obs_lanes]  # should result in list of tuples (sys_id1, sys_id2)
    if obs_lanes_list:
        print "obstructed starlanes are: %s" % obs_lanes_list
    else:
        print "No obstructed Starlanes"
    empire_id = foAI.foAIstate.empireID
    newly_explored = []
    still_unexplored = []
    for sys_id in list(foAI.foAIstate.unexploredSystemIDs):
        if empire.hasExploredSystem(sys_id):  # consider making determination according to visibility rather than actual visit, which I think is what empire.hasExploredSystem covers
            del foAI.foAIstate.unexploredSystemIDs[sys_id]
            foAI.foAIstate.exploredSystemIDs[sys_id] = 1
            sys = universe.getSystem(sys_id)
            print "Moved system %d ( %s ) from unexplored list to explored list" % (sys_id, (sys and sys.name) or "name unknown")
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
