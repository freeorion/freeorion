from logging import debug, error, info

import freeOrionAIInterface as fo  # interface used to interact with FreeOrion AI client # pylint: disable=import-error
import FleetUtilsAI
from EnumsAI import MissionType
import MoveUtilsAI
import PlanetUtilsAI
from AIDependencies import INVALID_ID
from freeorion_tools import get_partial_visibility_turn
from aistate_interface import get_aistate
from target import TargetSystem

graph_flags = set()
border_unexplored_system_ids = set()


def get_current_exploration_info():
    """Returns ([current target list], [available scout list])."""
    fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.EXPLORATION)
    available_scouts = []
    already_covered = set()
    aistate = get_aistate()
    for fleet_id in fleet_ids:
        fleet_mission = aistate.get_fleet_mission(fleet_id)
        if not fleet_mission.type:
            available_scouts.append(fleet_id)
        else:
            if fleet_mission.type == MissionType.EXPLORATION:
                already_covered.add(fleet_mission.target.id)
                if not fleet_mission.target:
                    debug("problem determining existing exploration target systems")
                else:
                    debug("found existing exploration target: %s" % fleet_mission.target)
    debug("Current exploration targets: %s" % already_covered)
    debug("Available scout fleets: %s" % available_scouts)
    return list(already_covered), available_scouts


def assign_scouts_to_explore_systems():
    # TODO: use Graph Theory to explore closest systems
    universe = fo.getUniverse()
    capital_sys_id = PlanetUtilsAI.get_capital_sys_id()
    # order fleets to explore
    if not border_unexplored_system_ids or (capital_sys_id == INVALID_ID):
        return
    exp_systems_by_dist = sorted((universe.linearDistance(capital_sys_id, x), x) for x in border_unexplored_system_ids)
    debug("Exploration system considering following system-distance pairs:\n  %s" % (
        "\n  ".join("%3d: %5.1f" % (sys_id, dist) for (dist, sys_id) in exp_systems_by_dist)))
    explore_list = [sys_id for dist, sys_id in exp_systems_by_dist]

    already_covered, available_scouts = get_current_exploration_info()

    debug("Explorable system IDs: %s" % explore_list)
    debug("Already targeted: %s" % already_covered)
    aistate = get_aistate()
    needs_vis = aistate.misc.setdefault('needs_vis', [])
    check_list = aistate.needsEmergencyExploration + needs_vis + explore_list
    if INVALID_ID in check_list:  # shouldn't normally happen, unless due to bug elsewhere
        for sys_list, name in [(aistate.needsEmergencyExploration, "aistate.needsEmergencyExploration"),
                               (needs_vis, "needs_vis"), (explore_list, "explore_list")]:
            if INVALID_ID in sys_list:
                error("INVALID_ID found in " + name, exc_info=True)
    # emergency coverage can be due to invasion detection trouble, etc.
    debug("Check list: %s" % check_list)
    needs_coverage = [sys_id for sys_id in check_list if sys_id not in already_covered and sys_id != INVALID_ID]
    debug("Needs coverage: %s" % needs_coverage)

    debug("Available scouts & AIstate locs: %s" % [(x, aistate.fleetStatus.get(x, {}).get('sysID', INVALID_ID))
                                                   for x in available_scouts])
    debug("Available scouts & universe locs: %s" % [(x, universe.getFleet(x).systemID) for x in available_scouts])
    if not needs_coverage or not available_scouts:
        return

    # clean up targets which can not or don't need to be scouted
    for sys_id in list(needs_coverage):
        if sys_id not in explore_list:  # doesn't necessarily need direct visit
            if universe.getVisibility(sys_id, fo.empireID()) >= fo.visibility.partial:
                # already got visibility; remove from visit lists and skip
                if sys_id in needs_vis:
                    del needs_vis[needs_vis.index(sys_id)]
                if sys_id in aistate.needsEmergencyExploration:
                    del aistate.needsEmergencyExploration[
                        aistate.needsEmergencyExploration.index(sys_id)]
                debug("system id %d already currently visible; skipping exploration" % sys_id)
                needs_coverage.remove(sys_id)
                continue

        # skip systems threatened by monsters
        sys_status = aistate.systemStatus.setdefault(sys_id, {})
        if (not aistate.character.may_explore_system(sys_status.setdefault('monsterThreat', 0)) or (
                fo.currentTurn() < 20 and aistate.systemStatus[sys_id]['monsterThreat'] > 0)):
            debug("Skipping exploration of system %d due to Big Monster, threat %d" % (
                sys_id, aistate.systemStatus[sys_id]['monsterThreat']))
            needs_coverage.remove(sys_id)
            continue

    # find the jump distance for all possible scout-system pairings
    options = []
    available_scouts = set(available_scouts)
    for fleet_id in available_scouts:
        fleet_mission = aistate.get_fleet_mission(fleet_id)
        start = fleet_mission.get_location_target()
        for sys_id in needs_coverage:
            target = TargetSystem(sys_id)
            path = MoveUtilsAI.can_travel_to_system(fleet_id, start, target, ensure_return=True)
            if not path:
                continue
            num_jumps = len(path) - 1  # -1 as path contains the original system
            options.append((num_jumps, fleet_id, sys_id))

    # Apply a simple, greedy heuristic to match scouts to nearby systems:
    # Always choose the shortest possible path from the remaining scout-system pairing.
    # This is clearly not optimal in the general case but it works well enough for now.
    # TODO: Consider using a more sophisticated assignment algorithm
    options.sort()
    while options:
        debug("Remaining options: %s" % options)
        _, fleet_id, sys_id = options[0]
        fleet_mission = aistate.get_fleet_mission(fleet_id)
        target = TargetSystem(sys_id)
        info("Sending fleet %d to explore %s" % (fleet_id, target))
        fleet_mission.set_target(MissionType.EXPLORATION, target)
        options = [option for option in options if option[1] != fleet_id and option[2] != sys_id]
        available_scouts.remove(fleet_id)
        needs_coverage.remove(sys_id)

    debug("Exploration assignment finished.")
    debug("Unassigned scouts: %s" % available_scouts)
    debug("Unassigned exploration targets: %s" % needs_coverage)


def follow_vis_system_connections(start_system_id, home_system_id):
    universe = fo.getUniverse()
    empire_id = fo.empireID()
    exploration_list = [start_system_id]
    aistate = get_aistate()
    while exploration_list:
        cur_system_id = exploration_list.pop()
        if cur_system_id in graph_flags:
            continue
        graph_flags.add(cur_system_id)
        system = universe.getSystem(cur_system_id)
        if cur_system_id in aistate.visBorderSystemIDs:
            pre_vis = "a border system"
        elif cur_system_id in aistate.visInteriorSystemIDs:
            pre_vis = "an interior system"
        else:
            pre_vis = "an unknown system"
        system_header = "*** system %s;" % system
        if fo.currentTurn() < 50:
            visibility_turn_list = sorted(universe.getVisibilityTurnsMap(cur_system_id, empire_id).items(),
                                          key=lambda x: x[0].numerator)
            visibility_info = ', '.join('%s: %s' % (vis.name, turn) for vis, turn in visibility_turn_list)
            debug("%s previously %s. Visibility per turn: %s " % (system_header, pre_vis, visibility_info))
            status_info = []
        else:
            status_info = [system_header]

        has_been_visible = get_partial_visibility_turn(cur_system_id) > 0
        is_connected = universe.systemsConnected(cur_system_id, home_system_id, -1)  # self.empire_id)
        status_info.append("    -- is%s partially visible" % ("" if has_been_visible else " not"))
        status_info.append("    -- is%s visibly connected to homesystem" % ("" if is_connected else " not"))
        if has_been_visible:
            sys_status = aistate.systemStatus.setdefault(cur_system_id, {})
            aistate.visInteriorSystemIDs.add(cur_system_id)
            aistate.visBorderSystemIDs.discard(cur_system_id)
            neighbors = set(universe.getImmediateNeighbors(cur_system_id, empire_id))
            sys_status.setdefault('neighbors', set()).update(neighbors)
            if neighbors:
                status_info.append(" -- has neighbors %s" % sorted(neighbors))
                for sys_id in neighbors:
                    if sys_id not in aistate.exploredSystemIDs:
                        aistate.unexploredSystemIDs.add(sys_id)
                    if (sys_id not in graph_flags) and (sys_id not in aistate.visInteriorSystemIDs):
                        aistate.visBorderSystemIDs.add(sys_id)
                        exploration_list.append(sys_id)
        if fo.currentTurn() < 50:
            debug('\n'.join(status_info))
            debug("----------------------------------------------------------")


def update_explored_systems():
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    obs_lanes = empire.obstructedStarlanes()
    obs_lanes_list = [el for el in obs_lanes]  # should result in list of tuples (sys_id1, sys_id2)
    if obs_lanes_list:
        debug("Obstructed starlanes are: %s" % ', '.join('%s-%s' % item for item in obs_lanes_list))
    else:
        debug("No obstructed Starlanes")
    empire_id = fo.empireID()
    newly_explored = []
    still_unexplored = []
    aistate = get_aistate()
    for sys_id in list(aistate.unexploredSystemIDs):
        # consider making determination according to visibility rather than actual visit,
        # which I think is what empire.hasExploredSystem covers (Dilvish-fo)
        if empire.hasExploredSystem(sys_id):
            aistate.unexploredSystemIDs.discard(sys_id)
            aistate.exploredSystemIDs.add(sys_id)
            system = universe.getSystem(sys_id)
            debug("Moved system %s from unexplored list to explored list" % system)
            border_unexplored_system_ids.discard(sys_id)
            newly_explored.append(sys_id)
        else:
            still_unexplored.append(sys_id)

    neighbor_list = []
    dummy = []
    for id_list, next_list in [(newly_explored, neighbor_list), (neighbor_list, dummy)]:
        for sys_id in id_list:
            neighbors = list(universe.getImmediateNeighbors(sys_id, empire_id))
            for neighbor_id in neighbors:
                # when it matters, unexplored will be smaller than explored
                if neighbor_id not in aistate.unexploredSystemIDs:
                    next_list.append(neighbor_id)

    for sys_id in still_unexplored:
        neighbors = list(universe.getImmediateNeighbors(sys_id, empire_id))
        if any(nid in aistate.exploredSystemIDs for nid in neighbors):
            border_unexplored_system_ids.add(sys_id)
    return newly_explored
