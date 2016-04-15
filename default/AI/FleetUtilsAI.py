import freeOrionAIInterface as fo  # pylint: disable=import-error
import FreeOrionAI as foAI
from EnumsAI import MissionType, ShipRoleType, ExplorableSystemType
import CombatRatingsAI
import traceback
from universe_object import Planet
from ShipDesignAI import get_part_type

__designStats = {}


def rating_needed(target, current=0):
    if current >= target or target <= 0:
        return 0
    else:
        return target + current - 2*(target*current)**0.5


def stats_meet_reqs(stats, reqs):
    try:
        for key in reqs:
            if stats.get(key, 0) < reqs[key]:
                return False
        return True
    except:
        return False


def count_parts_fleetwide(fleet_id, parts_list):
    tally = 0
    universe = fo.getUniverse()
    fleet = universe.getFleet(fleet_id)
    if not fleet:
        return 0
    for ship_id in fleet.shipIDs:
        ship = universe.getShip(ship_id)
        if not ship: 
            continue
        design = ship.design
        if not design: 
            continue
        for part in design.parts:
            if part in parts_list:
                tally += 1
    return tally


def count_troops_in_fleet(fleet_id):
    """
    :param fleet_id:
    :return: total troopCapacity of the fleet
    """
    universe = fo.getUniverse()
    fleet = universe.getFleet(fleet_id)
    if not fleet:
        return 0
    fleet_troop_capacity = 0
    for ship_id in fleet.shipIDs:
        ship = universe.getShip(ship_id)
        if ship:
            fleet_troop_capacity += ship.troopCapacity
    return fleet_troop_capacity


def get_targeted_planet_ids(planet_ids, mission_type):
    """return subset list of planet ids that are targets of the specified mission type
    :rtype : list of planet ids
    """
    selected_fleet_missions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([mission_type])
    targeted_planets = []
    for planet_id in planet_ids:
        # add planets that are target of a mission
        for fleet_mission in selected_fleet_missions:
            ai_target = Planet(planet_id)
            if fleet_mission.has_target(mission_type, ai_target):
                targeted_planets.append(planet_id)
    return targeted_planets


def get_fleets_for_mission(nships, target_stats, min_stats, cur_stats, species, systems_to_check, systems_checked,
                           fleet_pool_set, fleet_list, take_any=False, extend_search=True, tried_fleets=None,
                           verbose=False, depth=0):
    """
    Implements breadth-first search through systems
    mutates cur_stats with running status, systems_to_check and systems_checked as systems are checked,
    fleet_pool_set as fleets are checked also mutates fleet_list as running list of selected fleets;
    this list will be returned as the function return value if the target statsare met or if upon exhausting
    systems_to_check both take_any is true and the min stats are met.
    Otherwise, an empty list is returned by the function, in which case the caller can make an evaluation of
    an emergency use of the found fleets in fleet_list; if not to be used they should be added back to the main pool.
    """
    if verbose:
        print ("get_fleets_for_mission: (nships:%1d, targetStats:%s, minStats:%s, "
               "curStats:%s, species:%6s, systemsToCheck:%8s, systemsChecked:%8s, "
               "fleetPoolSet:%8s, fleetList:%8s) ") % (nships, target_stats, min_stats, cur_stats, species,
                                                       systems_to_check, systems_checked, fleet_pool_set, fleet_list)
    universe = fo.getUniverse()
    if not (systems_to_check and fleet_pool_set):
        if verbose:
            print "no more systems or fleets to check"
        if take_any:
            return fleet_list
        elif (stats_meet_reqs(cur_stats, min_stats)
              and sum(len(universe.getFleet(fid).shipIDs) for fid in fleet_list) >= nships):
            return fleet_list
        else:
            return []
    this_system_id = systems_to_check.pop(0)  # take the head of the line
    systems_checked.append(this_system_id)

    accessible_fleets = foAI.foAIstate.systemStatus.get(this_system_id, {}).get('myFleetsAccessible', [])
    fleets_here = [fid for fid in accessible_fleets if fid in fleet_pool_set]

    if verbose:
        print "found fleetPool Fleets %s" % fleets_here
    while fleets_here:
        fleet_id = fleets_here.pop(0)
        fleet = universe.getFleet(fleet_id)
        if not fleet:
            print "in get_fleets_for_mission, fleet_id %d appears invalid; cannot retrieve" % fleet_id
            fleet_pool_set.remove(fleet_id)
            continue
        if len(list(fleet.shipIDs)) > 1:
            new_fleets = split_fleet(fleet_id)  # try splitting fleet
            fleet_pool_set.update(new_fleets)
            fleets_here.extend(new_fleets)
        meets_species_req = False
        needs_species = False
        if species != "":
            needs_species = True
        has_species = ""

        colonization_roles = (ShipRoleType.CIVILIAN_COLONISATION,
                              ShipRoleType.BASE_COLONISATION)
        for shipID in fleet.shipIDs:
            ship = universe.getShip(shipID)
            if foAI.foAIstate.get_ship_role(ship.design.id) in colonization_roles:
                has_species = ship.speciesName
                if has_species == species:
                    meets_species_req = True
                    break
        troop_capacity = 0
        needs_troops = 'troopCapacity' in target_stats
        if needs_troops:
            troop_capacity = count_troops_in_fleet(fleet_id)

        use_fleet = ((not needs_species and not has_species) or meets_species_req)\
                    and ((needs_troops and troop_capacity > 0) or (not needs_troops and not troop_capacity))
        if use_fleet:
            fleet_list.append(fleet_id)
            fleet_pool_set.remove(fleet_id)
            this_rating = foAI.foAIstate.get_rating(fleet_id)  # Done!
            cur_stats['rating'] = CombatRatingsAI.combine_ratings(cur_stats.get('rating', 0), this_rating)
            if 'troopCapacity' in target_stats:
                # ToDo: Check if replaceable by troop_capacity
                cur_stats['troopCapacity'] = cur_stats.get('troopCapacity', 0) + count_troops_in_fleet(fleet_id)
            if (sum([len(universe.getFleet(fid).shipIDs) for fid in fleet_list]) >= nships)\
                    and stats_meet_reqs(cur_stats, target_stats):
                if verbose:
                    print "returning fleetlist: %s" % fleet_list
                return fleet_list
    # finished loop without meeting reqs
    if extend_search:
        for neighbor_id in [el.key() for el in universe.getSystemNeighborsMap(this_system_id, foAI.foAIstate.empireID)]:
            if (neighbor_id not in systems_checked and neighbor_id not in systems_to_check
                and neighbor_id in foAI.foAIstate.exploredSystemIDs):
                systems_to_check.append(neighbor_id)
    try:
        return get_fleets_for_mission(nships, target_stats, min_stats, cur_stats, species, systems_to_check,
                                      systems_checked, fleet_pool_set, fleet_list, take_any=take_any,
                                      extend_search=extend_search, verbose=verbose, depth=depth + 1)
    except:
        s1 = len(systems_to_check)
        s2 = len(systems_checked)
        s3 = len(set(systems_to_check + systems_checked))
        print "Error: exception triggered in 'getFleetsForMissions' and caught at depth %d w/s1/s2/s3 (%d/%d/%d): " % (
            depth + 2, s1, s2, s3)
        print traceback.format_exc()
        return []


def split_fleet(fleet_id):
    """Splits a fleet into its ships."""
    universe = fo.getUniverse()
    empire_id = fo.empireID()
    fleet = universe.getFleet(fleet_id)
    newfleets = []

    if fleet is None:
        return []
    if not fleet.ownedBy(empire_id):
        return []

    if len(list(fleet.shipIDs)) <= 1:  # fleet with only one ship cannot be split
        return []
    ship_ids = list(fleet.shipIDs)
    for ship_id in ship_ids[1:]:
        new_fleet_id = fo.issueNewFleetOrder("Fleet %4d" % ship_id, ship_id)
        if new_fleet_id:
            new_fleet = universe.getFleet(new_fleet_id)
            if not new_fleet:
                print "Error: newly split fleet %d not available from universe" % new_fleet_id
            fo.issueRenameOrder(new_fleet_id, "Fleet %4d" % new_fleet_id)  # to ease review of debugging logs
            fo.issueAggressionOrder(new_fleet_id, True)
            foAI.foAIstate.update_fleet_rating(new_fleet_id)
            newfleets.append(new_fleet_id)
            foAI.foAIstate.newlySplitFleets[new_fleet_id] = True
        else:
            if fleet.systemID == -1:
                print "Error - tried to split ship id (%d) from fleet %d when fleet is in starlane" % (ship_id, fleet_id)
            else:
                print "Error - got no fleet ID back after trying to split ship id (%d) from fleet %d" % (ship_id, fleet_id)
    foAI.foAIstate.get_fleet_role(fleet_id, force_new=True)
    foAI.foAIstate.update_fleet_rating(fleet_id)
    if newfleets:
        foAI.foAIstate.ensure_have_fleet_missions(newfleets)
    return newfleets


def merge_fleet_a_into_b(fleet_a_id, fleet_b_id, leave_rating=0, need_rating=0, context=""):
    universe = fo.getUniverse()
    fleet_a = universe.getFleet(fleet_a_id)
    fleet_b = universe.getFleet(fleet_b_id)
    if not fleet_a or not fleet_b:
        return 0
    remaining_rating = CombatRatingsAI.get_fleet_rating(fleet_a_id)
    transferred_rating = 0
    b_has_monster = False
    for ship_id in fleet_b.shipIDs:
        this_ship = universe.getShip(ship_id)
        if not this_ship:
            continue
        if this_ship.isMonster:
            b_has_monster = True
            break
    for ship_id in fleet_a.shipIDs:
        this_ship = universe.getShip(ship_id)
        if not this_ship or this_ship.isMonster != b_has_monster:  # TODO Is there any reason for the monster check?
            continue
        this_rating = CombatRatingsAI.ShipCombatStats(ship_id).get_rating()
        remaining_rating = rating_needed(remaining_rating, this_rating)
        if remaining_rating < leave_rating:  # merging this would leave old fleet under minimum rating, try other ships.
            continue
        transferred = fo.issueFleetTransferOrder(ship_id, fleet_b_id)
        if transferred:
            transferred_rating = CombatRatingsAI.combine_ratings(transferred_rating, this_rating)
        else:
            print "  *** attempted transfer of ship %4d, formerly of fleet %4d, into fleet %4d with result %d; %s" % (
                ship_id, fleet_a_id, fleet_b_id, transferred, [" context is %s" % context, ""][context == ""])
        if need_rating != 0 and need_rating <= transferred_rating:
            break
    fleet_a = universe.getFleet(fleet_a_id)
    if not fleet_a or fleet_a.empty or fleet_a_id in universe.destroyedObjectIDs(fo.empireID()):
        foAI.foAIstate.delete_fleet_info(fleet_a_id)
    foAI.foAIstate.update_fleet_rating(fleet_b_id)


def fleet_has_ship_with_role(fleet_id, ship_role):
    """Returns True if a ship with shipRole is in the fleet."""
    universe = fo.getUniverse()
    fleet = universe.getFleet(fleet_id)

    if fleet is None:
        return False
    for ship_id in fleet.shipIDs:
        ship = universe.getShip(ship_id)
        if foAI.foAIstate.get_ship_role(ship.design.id) == ship_role:
            return True
    return False


def get_ship_id_with_role(fleet_id, ship_role, verbose=True):
    """Returns a ship with the specified role in the fleet."""

    if not fleet_has_ship_with_role(fleet_id, ship_role):
        if verbose:
            print "No ship with role %s found." % ship_role
        return None

    universe = fo.getUniverse()
    fleet = universe.getFleet(fleet_id)

    for ship_id in fleet.shipIDs:
        ship = universe.getShip(ship_id)
        if foAI.foAIstate.get_ship_role(ship.design.id) == ship_role:
            return ship_id


def get_empire_fleet_ids():
    """Returns all fleetIDs for current empire."""
    empire_id = foAI.foAIstate.empireID
    universe = fo.getUniverse()
    empire_fleet_ids = []
    destroyed_object_ids = universe.destroyedObjectIDs(empire_id)
    for fleet_id in set(list(universe.fleetIDs) + list(foAI.foAIstate.newlySplitFleets)):
        fleet = universe.getFleet(fleet_id)
        if fleet is None:
            continue
        if fleet.ownedBy(empire_id) and fleet_id not in destroyed_object_ids and not fleet.empty and fleet.shipIDs:
            empire_fleet_ids.append(fleet_id)
    return empire_fleet_ids


def get_empire_fleet_ids_by_role(fleet_role):
    """Returns a list with fleet_ids that have the specified role."""
    fleet_ids = get_empire_fleet_ids()
    fleet_ids_with_role = []
    for fleet_id in fleet_ids:
        if foAI.foAIstate.get_fleet_role(fleet_id) != fleet_role:
            continue
        fleet_ids_with_role.append(fleet_id)
    return fleet_ids_with_role


def extract_fleet_ids_without_mission_types(fleets_ids):
    """Extracts a list with fleetIDs that have no mission."""
    return [fleet_id for fleet_id in fleets_ids if not foAI.foAIstate.get_fleet_mission(fleet_id).type]


def assess_fleet_role(fleet_id):
    """
    Assesses ShipRoles represented in a fleet and
    returns a corresponding overall fleetRole (of type MissionType).
    """
    universe = fo.getUniverse()
    ship_roles = {}
    fleet = universe.getFleet(fleet_id)
    if not fleet:
        print "couldn't get fleet with id " + str(fleet_id)
        return ShipRoleType.INVALID

    # count ship_roles
    for ship_id in fleet.shipIDs:
        ship = universe.getShip(ship_id)
        if ship.design:
            role = foAI.foAIstate.get_ship_role(ship.design.id)
        else:
            role = ShipRoleType.INVALID

        if role != ShipRoleType.INVALID:
            ship_roles[role] = ship_roles.get(role, 0) + 1
    # determine most common ship_role
    favourite_role = ShipRoleType.INVALID
    for ship_role in ship_roles:
        if ship_roles[ship_role] == max(ship_roles.values()):
            favourite_role = ship_role

    # assign fleet role
    if ShipRoleType.CIVILIAN_COLONISATION in ship_roles:
        selected_role = MissionType.COLONISATION
    elif ShipRoleType.BASE_COLONISATION in ship_roles:
        selected_role = MissionType.COLONISATION
    elif ShipRoleType.CIVILIAN_OUTPOST in ship_roles:
        selected_role = MissionType.OUTPOST
    elif ShipRoleType.BASE_OUTPOST in ship_roles:
        selected_role = MissionType.ORBITAL_OUTPOST
    elif ShipRoleType.BASE_INVASION in ship_roles:
        selected_role = MissionType.ORBITAL_INVASION
    elif ShipRoleType.BASE_DEFENSE in ship_roles:
        selected_role = MissionType.ORBITAL_DEFENSE
    elif ShipRoleType.MILITARY_INVASION in ship_roles:
        selected_role = MissionType.INVASION
    ####
    elif favourite_role == ShipRoleType.CIVILIAN_EXPLORATION:
        selected_role = MissionType.EXPLORATION
    elif favourite_role == ShipRoleType.MILITARY_ATTACK:
        selected_role = MissionType.MILITARY
    elif favourite_role == ShipRoleType.MILITARY:
        selected_role = MissionType.MILITARY
    else:
        selected_role = ShipRoleType.INVALID
    return selected_role


def assess_ship_design_role(design):
    parts = [get_part_type(partname) for partname in design.parts if partname and get_part_type(partname)]

    if any(p.partClass == fo.shipPartClass.colony and p.capacity == 0 for p in parts):
        if design.speed > 0:
            return ShipRoleType.CIVILIAN_OUTPOST
        else:
            return ShipRoleType.BASE_OUTPOST

    if any(p.partClass == fo.shipPartClass.colony and p.capacity > 0 for p in parts):
        if design.speed > 0:
            return ShipRoleType.CIVILIAN_COLONISATION
        else:
            return ShipRoleType.BASE_COLONISATION

    if any(p.partClass == fo.shipPartClass.troops for p in parts):
        if design.speed > 0:
            return ShipRoleType.MILITARY_INVASION
        else:
            return ShipRoleType.BASE_INVASION

    if design.speed == 0:
        if not parts or parts[0].partClass == fo.shipPartClass.shields:  # ToDo: Update logic for new ship designs
            return ShipRoleType.BASE_DEFENSE
        else:
            return ShipRoleType.INVALID

    if design.isArmed:
        return ShipRoleType.MILITARY
    if any(p.partClass == fo.shipPartClass.detection for p in parts):
        return ShipRoleType.CIVILIAN_EXPLORATION
    else:   # if no suitable role found, use as (bad) scout as it still has inherent detection
        return ShipRoleType.CIVILIAN_EXPLORATION


def generate_fleet_orders_for_fleet_missions():
    """Generates fleet orders from targets."""
    print("Generating fleet orders")

    # The following fleet lists are based on *Roles* -- Secure type missions are done by fleets with Military Roles
    print "Fleets by Role\n"
    print "Exploration Fleets: %s" % get_empire_fleet_ids_by_role(MissionType.EXPLORATION)
    print "Colonization Fleets: %s" % get_empire_fleet_ids_by_role(MissionType.COLONISATION)
    print "Outpost Fleets: %s" % get_empire_fleet_ids_by_role(MissionType.OUTPOST)
    print "Invasion Fleets: %s" % get_empire_fleet_ids_by_role(MissionType.INVASION)
    print "Military Fleets: %s" % get_empire_fleet_ids_by_role(MissionType.MILITARY)
    print "Orbital Defense Fleets: %s" % get_empire_fleet_ids_by_role(MissionType.ORBITAL_DEFENSE)
    print "Outpost Base Fleets: %s" % get_empire_fleet_ids_by_role(MissionType.ORBITAL_OUTPOST)
    print "Invasion Base Fleets: %s" % get_empire_fleet_ids_by_role(MissionType.ORBITAL_INVASION)
    print "Securing Fleets: %s  (currently FLEET_MISSION_MILITARY should be used instead of this Role)" % get_empire_fleet_ids_by_role(MissionType.SECURE)

    if fo.currentTurn() < 50:
        print
        print "Explored systems:"
        print_systems(foAI.foAIstate.get_explorable_systems(ExplorableSystemType.EXPLORED))
        print "Unexplored systems:"
        print_systems(foAI.foAIstate.get_explorable_systems(ExplorableSystemType.UNEXPLORED))
        print

    exploration_fleet_missions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([MissionType.EXPLORATION])
    if exploration_fleet_missions:
        print "Exploration targets:"
        for explorationAIFleetMission in exploration_fleet_missions:
            print " - %s" % explorationAIFleetMission
    else:
        print "Exploration targets: None"

    colonisation_fleet_missions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([MissionType.COLONISATION])
    if colonisation_fleet_missions:
        print "Colonization targets: "
    else:
        print "Colonization targets: None"
    for colonisation_fleet_mission in colonisation_fleet_missions:
        print "    %s" % colonisation_fleet_mission

    outpost_fleet_missions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([MissionType.OUTPOST])
    if outpost_fleet_missions:
        print "Outpost targets: "
    else:
        print "Outpost targets: None"
    for outpost_fleet_mission in outpost_fleet_missions:
        print "    %s" % outpost_fleet_mission

    outpost_base_fleet_missions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([MissionType.ORBITAL_OUTPOST])
    if outpost_base_fleet_missions:
        print "Outpost Base targets (must have been interrupted by combat): "
    else:
        print "Outpost targets: None (as expected, due to expected timing of order submission and execution)"
    for outpost_fleet_mission in outpost_base_fleet_missions:
        print "    %s" % outpost_fleet_mission

    invasion_fleet_missions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([MissionType.INVASION])
    if invasion_fleet_missions:
        print "Invasion targets: "
    else:
        print "Invasion targets: None"
    for invasion_fleet_mission in invasion_fleet_missions:
        print "    %s" % invasion_fleet_mission

    troop_base_fleet_missions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([MissionType.ORBITAL_INVASION])
    if troop_base_fleet_missions:
        print "Invasion Base targets (must have been interrupted by combat): "
    else:
        print "Invasion Base targets: None (as expected, due to expected timing of order submission and execution)"
    for invasion_fleet_mission in troop_base_fleet_missions:
        print "    %s" % invasion_fleet_mission

    military_fleet_missions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([MissionType.MILITARY])
    if military_fleet_missions:
        print "General Military targets: "
    else:
        print "General Military targets: None"
    for military_fleet_mission in military_fleet_missions:
        print "    %s" % military_fleet_mission

    secure_fleet_missions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([MissionType.SECURE])
    if secure_fleet_missions:
        print "Secure targets: "
    else:
        print "Secure targets: None"
    for secure_fleet_mission in secure_fleet_missions:
        print "    %s" % secure_fleet_mission

    orb_defense_fleet_missions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([MissionType.ORBITAL_DEFENSE])
    if orb_defense_fleet_missions:
        print "Orbital Defense targets: "
    else:
        print "Orbital Defense targets: None"
    for orb_defence_fleet_mission in orb_defense_fleet_missions:
        print "    %s" % orb_defence_fleet_mission

    fleet_missions = foAI.foAIstate.get_all_fleet_missions()

    for mission in fleet_missions:
        mission.generate_fleet_orders()


def issue_fleet_orders_for_fleet_missions():
    """Issues fleet orders."""
    print
    universe = fo.getUniverse()
    fleet_missions = foAI.foAIstate.get_all_fleet_missions()
    thisround = 0
    while thisround < 3:
        thisround += 1
        print "Issuing fleet orders round %d:" % thisround
        for mission in fleet_missions:
            fleet_id = mission.fleet.id
            fleet = mission.fleet.get_object()
            # check that fleet was merged into another previously during this turn
            if not fleet or not fleet.shipIDs or fleet_id in universe.destroyedObjectIDs(fo.empireID()):
                continue
            mission.issue_fleet_orders()
        fleet_missions = foAI.foAIstate.misc.get('ReassignedFleetMissions', [])
        foAI.foAIstate.misc['ReassignedFleetMissions'] = []
    print


def print_systems(system_ids):
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    fleet_supplyable_system_ids = empire.fleetSupplyableSystemIDs
    for system_id in system_ids:
        system = universe.getSystem(system_id)
        if system:
            print "  %s" % system,
        else:
            print "  S_%s<>" % system_id,
        print 'supplied' if system_id in fleet_supplyable_system_ids else ''


def get_fighter_capacity_of_fleet(fleet_id):
    """Return current and max fighter capacity

    :param fleet_id:
    :type fleet_id: int
    :return: current and max fighter capacity
    """
    universe = fo.getUniverse()
    fleet = universe.getFleet(fleet_id)
    cur_capacity = 0
    max_capacity = 0
    ships = (universe.getShip(ship_id) for ship_id in (fleet.shipIDs if fleet else []))
    for ship in ships:
        design = ship and ship.design
        design_parts = design.parts if design and design.isArmed else []
        for partname in design_parts:
            part = get_part_type(partname)
            if part and part.partClass == fo.shipPartClass.fighterHangar:
                cur_capacity += ship.currentPartMeterValue(fo.meterType.capacity, partname)
                max_capacity += ship.currentPartMeterValue(fo.meterType.maxCapacity, partname)
    return cur_capacity, max_capacity
