import freeOrionAIInterface as fo # pylint: disable=import-error
import FreeOrionAI as foAI
from EnumsAI import AIFleetMissionType, AIShipRoleType, AIExplorableSystemType, AIShipDesignTypes
import traceback

__designStats={}
__AIShipRoleTypeNames = AIShipRoleType()
__AIFleetMissionTypeNames = AIFleetMissionType()


def stats_meet_reqs(stats, reqs):
    try:
        for key in reqs:
            #print " -- FleetUtilsAI.statsMeetsReqs checking stat %s for value >= %f :  found val % f "%(key, reqs[key], stats.get(key, 0) )
            if stats.get(key, 0) < reqs[key]:
                return False
        return True
    except:
        return False


def count_parts_fleetwide(fleet_id, parts_list):
    tally=0
    universe = fo.getUniverse()
    fleet = universe.getFleet(fleet_id)
    if not fleet:
        return 0
    for shipID in fleet.shipIDs:
        ship = universe.getShip(shipID)
        if not ship: 
            continue
        design = ship.design
        if not design: 
            continue
        for part in design.parts:
            if part in parts_list:
                tally += 1
    return tally

def get_fleets_for_mission(nships, target_stats, min_stats, cur_stats, species, systems_to_check, systems_checked, fleet_pool_set, fleet_list,
                                                            take_any=False, extend_search=True, tried_fleets=None, verbose=False, depth=0):
    """implements breadth-first search through systems
        mutates cur_stats with running status, systems_to_check and systems_checked as systems are checked, fleet_pool_set as fleets are checked
        also mutates fleet_list as running list of selected fleets; this list will be returned as the function return value if the target stats
        are met or if upon exhausting systems_to_check both take_any is true and the min stats are met. Otherwise, an empty list is returned by the function,
        in which case the caller can make an evaluation of an emergency use of the found fleets in fleet_list; if not to be used they should be added back to the main pool"""

    if tried_fleets is None:
        tried_fleets = set()
    if verbose:
        print "get_fleets_for_mission: (nships:%1d, targetStats:%s, minStats:%s, curStats:%s, species:%6s, systemsToCheck:%8s, systemsChecked:%8s, fleetPoolSet:%8s, fleetList:%8s) "%(
                                                                                                                                        nships, target_stats, min_stats, cur_stats, species, systems_to_check, systems_checked, fleet_pool_set, fleet_list)
    universe = fo.getUniverse()
    if not (systems_to_check and fleet_pool_set):
        if verbose:
            print "no more systems or fleets to check"
        if take_any or ( stats_meet_reqs(cur_stats, min_stats) and ( sum( [len(universe.getFleet(fID).shipIDs) for fID in fleet_list] ) >= nships)):
            return fleet_list
        else:
            return []
    this_system_id = systems_to_check.pop(0)  # take the head of the line
    systems_checked.append(this_system_id)
    fleets_here = [ fid for fid in foAI.foAIstate.systemStatus.get(this_system_id, {}).get('myFleetsAccessible', []) if fid in fleet_pool_set]
    if verbose:
        print "found fleetPool Fleets %s"%fleets_here
    while fleets_here:
        fleet_id=fleets_here.pop(0)
        fleet = universe.getFleet(fleet_id)
        if not fleet:
            print "in get_fleets_for_mission, fleet_id %d appears invalid; cannot retrieve"%fleet_id
            fleet_pool_set.remove( fleet_id)
            continue
        if len (list(fleet.shipIDs)) > 1:
            newFleets = split_fleet(fleet_id) # try splitting fleet
            fleet_pool_set.update(newFleets)
            fleets_here.extend(newFleets)
        meets_species_req=False
        needs_species=False
        if species != "":
            needs_species=True
        has_species=""
        for shipID in fleet.shipIDs:
            ship = universe.getShip(shipID)
            if foAI.foAIstate.get_ship_role(ship.design.id) in [ AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION, AIShipRoleType.SHIP_ROLE_BASE_COLONISATION]:
                has_species = ship.speciesName
                if has_species==species:
                    meets_species_req=True
                    break
        has_pods=0
        needs_troops = 'troopPods' in target_stats
        if needs_troops:
            has_pods = count_parts_fleetwide(fleet_id, ["GT_TROOP_POD"])

        use_fleet = ( ( not needs_species and not has_species) or meets_species_req ) and ( (  needs_troops and has_pods >0) or ( not needs_troops and has_pods==0 )  )
        if use_fleet:
            fleet_list.append(fleet_id)
            fleet_pool_set.remove( fleet_id)
            this_rating=foAI.foAIstate.get_rating(fleet_id)
            cur_stats['attack'] = cur_stats.get('attack', 0) + this_rating['attack']
            cur_stats['health'] = cur_stats.get('health', 0) + this_rating['health']
            cur_stats['rating'] = cur_stats['attack'] * cur_stats['health']
            if 'troopPods' in target_stats:
                cur_stats['troopPods'] = cur_stats.get('troopPods', 0) + count_parts_fleetwide(fleet_id, ["GT_TROOP_POD"])
            if ( sum( [len(universe.getFleet(fID).shipIDs) for fID in fleet_list] ) >= nships ) and stats_meet_reqs(cur_stats, target_stats) :
                if verbose:
                    print "returning fleetlist: %s"%fleet_list
                return fleet_list
    # finished loop without meeting reqs
    if extend_search:
        for neighborID in [el.key() for el in universe.getSystemNeighborsMap(this_system_id, foAI.foAIstate.empireID) ]:
            if neighborID not in systems_checked and neighborID not in systems_to_check and neighborID in foAI.foAIstate.exploredSystemIDs:
                systems_to_check.append(neighborID)
    try:
        return get_fleets_for_mission(nships, target_stats, min_stats, cur_stats, species, systems_to_check, systems_checked, fleet_pool_set, fleet_list, take_any, extend_search, verbose, depth=depth+1)
    except:
        s1=len(systems_to_check)
        s2=len(systems_checked)
        s3=len(set( systems_to_check + systems_checked ) )
        print "Error: exception triggered in 'getFleetsForMissions' and caught at depth %d w/s1/s2/s3 (%d/%d/%d): "%(depth+2, s1, s2, s3), traceback.format_exc()
        #print ("Error: call parameters were targetStats: %s, curStats: %s, species: '%s', systemsToCheck: %s, systemsChecked: %s, fleetPoolSet: %s, fleetList: %s"%(
        # targetStats, curStats, species, systemsToCheck, systemsChecked, fleetPoolSet, fleetList))
        return []


def split_fleet(fleetID):
    """splits a fleet into its ships"""

    universe = fo.getUniverse()
    empireID = fo.empireID()

    fleet = universe.getFleet(fleetID)
    newfleets = []

    if fleet is None: return []
    if not fleet.ownedBy(empireID): return []

    if len(list(fleet.shipIDs)) <= 1:  # fleet with only one ship cannot be split
        return []
    shipIDs = list( fleet.shipIDs )
    for shipID in shipIDs[1:]:
        newFleetID = fo.issueNewFleetOrder("Fleet %d"% shipID, shipID)
        if newFleetID:
            newFleet=universe.getFleet(newFleetID)
            if not newFleet:
                print "Error: newly split fleet %d not available from universe"%newFleetID
            fo.issueRenameOrder(newFleetID, "Fleet %5d"%newFleetID) #to ease review of debugging logs
            fo.issueAggressionOrder(newFleetID, True)
            role = foAI.foAIstate.get_fleet_role(newFleetID) #and mission?
            foAI.foAIstate.get_rating(newFleetID) #
            newfleets.append(newFleetID)
            foAI.foAIstate.newlySplitFleets[newFleetID]=True
        else:
            if fleet.systemID==-1:
                print "Error - tried to split ship id (%d) from fleet %d when fleet is in starlane"%(shipID, fleetID)
            else:
                print "Error - got no fleet ID back after trying to split ship id (%d) from fleet %d"%(shipID, fleetID)
    foAI.foAIstate.get_fleet_role(fleetID, forceNew=True) #
    foAI.foAIstate.update_fleet_rating(fleetID) #
    if newfleets:
        foAI.foAIstate.ensure_have_fleet_missions(newfleets)
    return newfleets


def merge_fleet_a_into_b(fleetA_ID, fleetB_ID, leaveRating=0, needRating=0, context=""):
    universe = fo.getUniverse()
    fleetA = universe.getFleet(fleetA_ID)
    sysID=fleetA.systemID
    fleetB = universe.getFleet(fleetB_ID)
    if not fleetA or not fleetB:
        return 0
    success = True
    initRating = foAI.foAIstate.get_rating(fleetA_ID)
    remainingRating = initRating.copy()
    transferredRating = 0
    transferredAttack=0
    transferredHealth=0
    BHasMonster=False
    for shipID in fleetB.shipIDs:
        thisShip=universe.getShip(shipID)
        if not thisShip:
            continue
        if thisShip.isMonster:
            BHasMonster = True
            break
    for shipID in fleetA.shipIDs:
        thisShip=universe.getShip(shipID)
        if (not thisShip) or ( thisShip.isMonster != BHasMonster ) :
            continue
        stats = foAI.foAIstate.get_design_id_stats(thisShip.designID)
        thisRating = stats['attack'] * ( stats['structure'] + stats['shields'] )
        if (remainingRating['attack'] -stats['attack'])*(remainingRating['health'] -( stats['structure'] + stats['shields'] )) < leaveRating:
            continue
        #remainingRating -= thisRating
        remainingRating['attack'] -= stats['attack']
        remainingRating['health'] -= ( stats['structure'] + stats['shields'] )
        thisSuccess = ( fo.issueFleetTransferOrder(shipID, fleetB_ID) )#returns a zero if transfer failure
        if thisSuccess:
            transferredRating += thisRating
            transferredAttack += stats['attack']
            transferredHealth += ( stats['structure'] + stats['shields'] )
        else:
            print "\t\t\t\t *** attempted transfer of ship %4d, formerly of fleet %4d, into fleet %4d with result %d; %s"%(shipID, fleetA_ID, fleetB_ID, thisSuccess, [" context is %s"%context, ""][context==""])
        success = success and thisSuccess
        if needRating !=0 and needRating <= transferredAttack*transferredHealth:  #transferredRating:
            break
    fleetA = universe.getFleet(fleetA_ID)
    if (not fleetA) or fleetA.empty or fleetA_ID in universe.destroyedObjectIDs(fo.empireID()):
        #print "\t\t\t\t\tdeleting fleet info for old fleet %d after transfers into fleet %d"%(fleetA_ID, fleetB_ID)
        foAI.foAIstate.delete_fleet_info(fleetA_ID)
    else:
        newARating = foAI.foAIstate.update_fleet_rating(fleetA_ID)
        if success : #and ( newARating==remainingRating) :
            #print "\t\t\t\t\t\t\%d rating from fleet %d successfully transferred to fleet %d, leaving %d"%(transferredAttack*transferredHealth, fleetA_ID, fleetB_ID, newARating['overall'])
            pass
        else:
            #print "\t\t\t\t\t\t transfer of %d rating from fleet %d to fleet %d was attempted but appears to have had problems, leaving %d"%(transferredAttack*transferredHealth, fleetA_ID, fleetB_ID, newARating['overall'])
            pass
    foAI.foAIstate.update_fleet_rating(fleetB_ID)
    return transferredAttack*transferredHealth, transferredAttack, transferredHealth


def fleet_has_ship_with_role(fleetID, shipRole):
    """returns True if a ship with shipRole is in the fleet"""

    universe = fo.getUniverse()
    fleet = universe.getFleet(fleetID)

    if fleet is None: return False
    for shipID in fleet.shipIDs:
        ship = universe.getShip(shipID)
        if foAI.foAIstate.get_ship_role(ship.design.id) == shipRole:
            return True
    return False


def get_ship_id_with_role(fleetID, shipRole, verbose = True):
    """returns a ship with the specified role in the fleet"""

    if not fleet_has_ship_with_role(fleetID, shipRole):
        if verbose:
            print "No ship with role " + __AIShipRoleTypeNames.name(shipRole) + " found."
        return None

    universe = fo.getUniverse()
    fleet = universe.getFleet(fleetID)

    for shipID in fleet.shipIDs:
        ship = universe.getShip(shipID)
        if foAI.foAIstate.get_ship_role(ship.design.id) == shipRole:
            return shipID


def get_empire_fleet_ids( empireID=None):
    """returns all fleetIDs of specified empire, defauls to current empire"""
    if empireID is None:
        empireID = foAI.foAIstate.empireID
    universe = fo.getUniverse()
    empireFleetIDs = []
    destroyedObjectIDs = universe.destroyedObjectIDs(empireID)
    for fleetID in set(list(universe.fleetIDs) + list(foAI.foAIstate.newlySplitFleets)):
        fleet = universe.getFleet(fleetID)
        if fleet is None: continue
        if ( fleet.ownedBy(empireID)) and (fleetID not in destroyedObjectIDs) and (not fleet.empty )and (not (len(fleet.shipIDs)==0) ):
            empireFleetIDs.append( fleetID )
    return empireFleetIDs


def get_empire_fleet_ids_by_role(fleetRole):
    """returns a list with fleetIDs that have the specified role"""
    fleetIDs = get_empire_fleet_ids()
    fleetIDsWithRole = []
    for fleetID in fleetIDs:
        if not (foAI.foAIstate.get_fleet_role(fleetID) == fleetRole): continue
        fleetIDsWithRole.append(fleetID)
    return fleetIDsWithRole


def extract_fleet_ids_without_mission_types(fleets_ids):
    """extracts a list with fleetIDs that have no mission"""
    return [fleet_id for fleet_id in fleets_ids if not foAI.foAIstate.get_fleet_mission(fleet_id).get_mission_types()]


def assess_fleet_role(fleetID):
    """assesses ShipRoles represented in a fleet and returns a corresponding overall fleetRole"""
    universe = fo.getUniverse()
    shipRoles = {}
    fleet = universe.getFleet(fleetID)
    if not fleet:
        print "couldn't get fleet with id " + str(fleetID)
        return AIShipRoleType.SHIP_ROLE_INVALID

    # count shipRoles
    for shipID in fleet.shipIDs:
        ship = universe.getShip(shipID)
        if ship.design:
            role = foAI.foAIstate.get_ship_role(ship.design.id)
        else:
            role = AIShipRoleType.SHIP_ROLE_INVALID

        if role != AIShipRoleType.SHIP_ROLE_INVALID:
            shipRoles[role] = shipRoles.get(role, 0) + 1
    # determine most common shipRole
    favouriteRole = AIShipRoleType.SHIP_ROLE_INVALID
    for shipRole in shipRoles:
        if shipRoles[shipRole] == max(shipRoles.values()):
            favouriteRole = shipRole

    # assign fleet role
    if AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION in shipRoles:
        selectedRole= AIFleetMissionType.FLEET_MISSION_COLONISATION
    elif AIShipRoleType.SHIP_ROLE_BASE_COLONISATION in shipRoles:
        selectedRole= AIFleetMissionType.FLEET_MISSION_ORBITAL_COLONISATION
    elif AIShipRoleType.SHIP_ROLE_CIVILIAN_OUTPOST in shipRoles:
        selectedRole= AIFleetMissionType.FLEET_MISSION_OUTPOST
    elif AIShipRoleType.SHIP_ROLE_BASE_OUTPOST in shipRoles:
        selectedRole= AIFleetMissionType.FLEET_MISSION_ORBITAL_OUTPOST
    elif AIShipRoleType.SHIP_ROLE_BASE_INVASION in shipRoles:
        selectedRole= AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION
    elif AIShipRoleType.SHIP_ROLE_BASE_DEFENSE in shipRoles:
        selectedRole= AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE
    elif AIShipRoleType.SHIP_ROLE_MILITARY_INVASION in shipRoles:
        selectedRole= AIFleetMissionType.FLEET_MISSION_INVASION
    ####
    elif favouriteRole == AIShipRoleType.SHIP_ROLE_CIVILIAN_EXPLORATION:
        selectedRole= AIFleetMissionType.FLEET_MISSION_EXPLORATION
    elif favouriteRole == AIShipRoleType.SHIP_ROLE_MILITARY_ATTACK:
        selectedRole= AIFleetMissionType.FLEET_MISSION_ATTACK
    elif favouriteRole == AIShipRoleType.SHIP_ROLE_MILITARY:
        selectedRole= AIFleetMissionType.FLEET_MISSION_MILITARY
    else:
        selectedRole= AIShipRoleType.SHIP_ROLE_INVALID
    #print "fleetID %d : primary fleet mission type %d: '%s' ; found ship roles %s : %s ; rating %d"%(fleetID,selectedRole, __AIFleetMissionTypeNames.name(selectedRole),
    # shipRoles, [ "%s: %d "%(__AIShipRoleTypeNames.name(rtype), rnum) for rtype, rnum in shipRoles.items()] , foAI.foAIstate.get_rating(fleetID).get('overall', 0))
    return selectedRole


def assess_ship_design_role(design):
    if "CO_OUTPOST_POD" in design.parts:
        if design.starlaneSpeed > 0:
            return AIShipRoleType.SHIP_ROLE_CIVILIAN_OUTPOST
        else:
            return AIShipRoleType.SHIP_ROLE_BASE_OUTPOST

    if "CO_COLONY_POD" in design.parts or "CO_SUSPEND_ANIM_POD" in design.parts:
        if design.starlaneSpeed > 0:
            return AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION
        else:
            return AIShipRoleType.SHIP_ROLE_BASE_COLONISATION

    if "GT_TROOP_POD" in design.parts:
        if design.starlaneSpeed > 0:
            return AIShipRoleType.SHIP_ROLE_MILITARY_INVASION
        else:
            return AIShipRoleType.SHIP_ROLE_BASE_INVASION

    if design.starlaneSpeed == 0:
        string1 = "Ship Design %s has partslist %s"%(design.name(False), [part for part in design.parts])
        if len(design.parts)==0 or design.parts[0] in [ "SH_DEFENSE_GRID", "SH_DEFLECTOR" , "SH_MULTISPEC" ] or (len(design.parts)==1 and design.parts[0]==''):
            #print string1, "-- classifying as Base Defense"
            return AIShipRoleType.SHIP_ROLE_BASE_DEFENSE
        else:
            #print string1, "-- classifying as Invalid"
            return AIShipRoleType.SHIP_ROLE_INVALID

    stats = foAI.foAIstate.get_design_id_stats(design.id)
    rating = stats['attack'] * ( stats['structure'] + stats['shields'] )
    scoutNames = AIShipDesignTypes.explorationShip.keys()
    if ( "SD_SCOUT" in design.name(False) )  or ( design.name(False).split('-')[0] in scoutNames ) :
        for part in design.parts:
            if "DT_DETECTOR" in part:
                return AIShipRoleType.SHIP_ROLE_CIVILIAN_EXPLORATION
    if rating > 0: #positive attack stat
        return AIShipRoleType.SHIP_ROLE_MILITARY
    else:
        return AIShipRoleType.SHIP_ROLE_CIVILIAN_EXPLORATION  #let this be the default since even without detection part a ship has some inherent


def generate_fleet_orders_for_fleet_missions():
    """generates fleet orders from targets"""
    print("Generating fleet orders")

    # The following fleet lists are based on *Roles* -- Secure type missions are done by fleets with Military Roles
    print "Fleets by Role\n"
    print "Exploration Fleets : " + str(get_empire_fleet_ids_by_role(AIFleetMissionType.FLEET_MISSION_EXPLORATION))
    print "Colonization Fleets: " + str(get_empire_fleet_ids_by_role(AIFleetMissionType.FLEET_MISSION_COLONISATION))
    print "Outpost Fleets : " + str(get_empire_fleet_ids_by_role(AIFleetMissionType.FLEET_MISSION_OUTPOST))
    print "Attack Fleets : " + str(get_empire_fleet_ids_by_role(AIFleetMissionType.FLEET_MISSION_ATTACK))
    print "Defend Fleets : " + str(get_empire_fleet_ids_by_role(AIFleetMissionType.FLEET_MISSION_DEFEND))
    print "Invasion Fleets : " + str(get_empire_fleet_ids_by_role(AIFleetMissionType.FLEET_MISSION_INVASION))
    print "Military Fleets : " + str(get_empire_fleet_ids_by_role(AIFleetMissionType.FLEET_MISSION_MILITARY))
    print "Orbital Defense Fleets : " + str(get_empire_fleet_ids_by_role(AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE))
    print "Outpost Base Fleets : " + str(get_empire_fleet_ids_by_role(AIFleetMissionType.FLEET_MISSION_ORBITAL_OUTPOST))
    print "Invasion Base Fleets : " + str(get_empire_fleet_ids_by_role(AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION))
    print "Securing Fleets : " + str(get_empire_fleet_ids_by_role(AIFleetMissionType.FLEET_MISSION_SECURE)) + " (currently FLEET_MISSION_MILITARY should be used instead of this Role)"
    print "Unclassifyable Fleets : " + str(get_empire_fleet_ids_by_role(AIFleetMissionType.FLEET_MISSION_INVALID))

    if fo.currentTurn() <50:
        print
        print "Explored systems :"
        print_systems(foAI.foAIstate.get_explorable_systems(AIExplorableSystemType.EXPLORABLE_SYSTEM_EXPLORED))
        print "Unexplored systems:"
        print_systems(foAI.foAIstate.get_explorable_systems(AIExplorableSystemType.EXPLORABLE_SYSTEM_UNEXPLORED))
        print

    explorationAIFleetMissions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([AIFleetMissionType.FLEET_MISSION_EXPLORATION])
    if len( explorationAIFleetMissions) >0:
        print "Exploration targets: "
    else:
        print "Exploration targets: None"
    for explorationAIFleetMission in explorationAIFleetMissions:
        print "    " + str(explorationAIFleetMission)

    colonisationAIFleetMissions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([AIFleetMissionType.FLEET_MISSION_COLONISATION])
    if len( colonisationAIFleetMissions) >0:
        print "Colonization targets: "
    else:
        print "Colonization targets: None"
    for colonisationAIFleetMission in colonisationAIFleetMissions:
        print "    " + str(colonisationAIFleetMission)

    outpostAIFleetMissions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([AIFleetMissionType.FLEET_MISSION_OUTPOST])
    if len( outpostAIFleetMissions) >0:
        print "Outpost targets: "
    else:
        print "Outpost targets: None"
    for outpostAIFleetMission in outpostAIFleetMissions:
        print "    " + str(outpostAIFleetMission)

    outpostBaseAIFleetMissions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([AIFleetMissionType.FLEET_MISSION_ORBITAL_OUTPOST])
    if len( outpostBaseAIFleetMissions) >0:
        print "Outpost Base targets (must have been interrupted by combat): "
    else:
        print "Outpost targets: None (as expected, due to expected timing of order submission and execution)"
    for outpostAIFleetMission in outpostBaseAIFleetMissions:
        print "    " + str(outpostAIFleetMission)

    invasionAIFleetMissions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([AIFleetMissionType.FLEET_MISSION_INVASION])
    if len( invasionAIFleetMissions) >0:
        print "Invasion targets: "
    else:
        print "Invasion targets: None"
    for invasionAIFleetMission in invasionAIFleetMissions:
        print "    " + str(invasionAIFleetMission)

    troopBaseAIFleetMissions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION])
    if len( troopBaseAIFleetMissions) >0:
        print "Invasion Base targets (must have been interrupted by combat): "
    else:
        print "Invasion Base targets: None (as expected, due to expected timing of order submission and execution)"
    for invasionAIFleetMission in troopBaseAIFleetMissions:
        print "    " + str(invasionAIFleetMission)

    militaryAIFleetMissions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([AIFleetMissionType.FLEET_MISSION_MILITARY])
    if len( militaryAIFleetMissions) >0:
        print "General Military targets: "
    else:
        print "General Military targets: None"
    for militaryAIFleetMission in militaryAIFleetMissions:
        print "    " + str(militaryAIFleetMission)

    secureAIFleetMissions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([AIFleetMissionType.FLEET_MISSION_SECURE])
    if len( secureAIFleetMissions) >0:
        print "Secure targets: "
    else:
        print "Secure targets: None"
    for secureAIFleetMission in secureAIFleetMissions:
        print "    " + str(secureAIFleetMission)

    orbDefenseAIFleetMissions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE])
    if len( orbDefenseAIFleetMissions) >0:
        print "Orbital Defense targets: "
    else:
        print "Orbital Defense targets: None"
    for ODAIFleetMission in orbDefenseAIFleetMissions:
        print "    " + str(ODAIFleetMission)

    aiFleetMissions = foAI.foAIstate.get_all_fleet_missions()

    for aiFleetMission in aiFleetMissions:
        aiFleetMission.generate_fleet_orders()


def issue_fleet_orders_for_fleet_missions():
    """issues fleet orders"""
    print
    universe=fo.getUniverse()
    aiFleetMissions = foAI.foAIstate.get_all_fleet_missions()
    thisround = 0
    while thisround <3:
        thisround += 1
        print "issuing fleet orders Round %d:"%thisround
        for aiFleetMission in aiFleetMissions:
            fleetID = aiFleetMission.target_id
            fleet = aiFleetMission.target.target_obj
            if (not fleet) or ( len(fleet.shipIDs)==0) or fleetID in universe.destroyedObjectIDs(fo.empireID()):  # in case fleet was merged into another previously during this turn
                continue
            aiFleetMission.issue_fleet_orders()
        aiFleetMissions = foAI.foAIstate.misc.get('ReassignedFleetMissions', [])
        foAI.foAIstate.misc['ReassignedFleetMissions']=[]
    print


def print_systems(systemIDs):
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
    for systemID in systemIDs:
        # determine if system is in supplied
        suppliedSystem = ""
        if systemID in fleetSupplyableSystemIDs:
            suppliedSystem = " supplied"

        system = universe.getSystem(systemID)
        if system:
            print "    name:" + system.name + " id:" + str(systemID) + suppliedSystem
        else:
            print "    name:??? id:" + str(systemID) + suppliedSystem
