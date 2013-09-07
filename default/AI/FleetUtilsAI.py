import freeOrionAIInterface as fo
import FreeOrionAI as foAI
import EnumsAI
from EnumsAI import AIFleetMissionType, AIShipRoleType, AIExplorableSystemType,  AIShipDesignTypes

__designStats={}
__AIShipRoleTypeNames = AIShipRoleType()
__AIFleetMissionTypeNames = AIFleetMissionType()

def clearShipDesignInfo():
    __designRoles.clear()
    __designStats.clear()
    
def statsMeetReq(stats,  reqs,  reqName):
    if stats.get(reqName,  0) < reqs.get(reqName, 0):
        return False
    return True

def statsMeetReqs(stats,  reqs):
    try:
        for key in reqs:
            #print " -- FleetUtilsAI.statsMeetsReqs checking stat %s for value >= %f   :  found  val % f "%(key,   reqs[key],  stats.get(key,  0) )
            if stats.get(key,  0) < reqs[key]:
                return False
        return True
    except:
        return False

def countPartsFleetwide(fleetID,  partsList):
    tally=0
    universe = fo.getUniverse()
    fleet = universe.getFleet(fleetID)
    if not fleet:  return 0
    for shipID in fleet.shipIDs:
        ship = universe.getShip(shipID)
        if not ship: continue
        design = ship.design
        if not design: continue
        for part in design.parts:
            if part in partsList:
                tally += 1
    return tally

def getFleetsForMission(nships,  targetStats,  minStats,  curStats,  species,  systemsToCheck,  systemsChecked, fleetPoolSet,   fleetList, 
                                                            takeAny=False,  extendSearch=True,  triedFleets=set([]),  verbose=False,  depth=0): #implements breadth-first search through systems
    if verbose:
        print "getFleetsForMission: (nships:%1d,  targetStats:%s,  minStats:%s, curStats:%s,  species:%6s,  systemsToCheck:%8s,  systemsChecked:%8s, fleetPoolSet:%8s,   fleetList:%8s) "%(
                                                                                                                                        nships,  targetStats,  minStats, curStats,  species,  systemsToCheck,  systemsChecked, fleetPoolSet,   fleetList)
    universe = fo.getUniverse()
    if not (systemsToCheck and fleetPoolSet):
        if verbose: 
            print "no more systems or fleets to check"
        if takeAny or ( statsMeetReqs(curStats,  minStats)  and ( sum( [len(universe.getFleet(fID).shipIDs) for fID in fleetList] )  >= nships)):
            return fleetList
        else:
            return []
    thisSystemID = systemsToCheck.pop(0) #take the head of the line
    systemsChecked.append(thisSystemID)
    fleetsHere = [ fid for fid in foAI.foAIstate.systemStatus.get(thisSystemID,  {}).get('myFleetsAccessible',  []) if fid in fleetPoolSet]
    if verbose:
        print "found fleetPool Fleets  %s"%fleetsHere
    while fleetsHere !=[]:
        fleetID=fleetsHere.pop(0)
        fleet = universe.getFleet(fleetID)
        if not fleet: 
            "in getFleetsForMission,  fleetID %d appers invalid; cannot retrieve"%fleetID 
            fleetPoolSet.remove(  fleetID) 
            continue
        if len (list(fleet.shipIDs)) > 1:
            newFleets = splitFleet(fleetID) # try splitting fleet
            fleetPoolSet.update(newFleets)
            fleetsHere.extend(newFleets)
        meetsSpeciesReq=False
        needsSpecies=False
        if (species != ""): 
            needsSpecies=True
        hasSpecies=""
        for shipID in fleet.shipIDs:
            ship = universe.getShip(shipID)
            if ((foAI.foAIstate.getShipRole(ship.design.id) in [ AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION,  AIShipRoleType.SHIP_ROLE_BASE_COLONISATION]) ):
                hasSpecies = ship.speciesName
                if hasSpecies==species:
                    meetsSpeciesReq=True
                    break
        hasPods=0
        needsTroops =  'troopPods' in targetStats
        if needsTroops:
            hasPods =  countPartsFleetwide(fleetID,  ["GT_TROOP_POD"])
        
        useFleet = ( ( not needsSpecies and not hasSpecies) or  meetsSpeciesReq )  and (  (  needsTroops  and   hasPods >0)  or ( not needsTroops and hasPods==0   )  )
        if useFleet:
            fleetList.append(fleetID)
            fleetPoolSet.remove( fleetID)
            thisRating=foAI.foAIstate.getRating(fleetID)
            curStats['attack'] = curStats.get('attack',  0) + thisRating['attack']
            curStats['health'] = curStats.get('health',  0) + thisRating['health']
            curStats['rating'] = curStats['attack'] * curStats['health'] 
            if 'troopPods' in targetStats:
                curStats['troopPods'] = curStats.get('troopPods',  0) + countPartsFleetwide(fleetID,  ["GT_TROOP_POD"])
            if  ( sum( [len(universe.getFleet(fID).shipIDs) for fID in fleetList] )  >= nships ) and statsMeetReqs(curStats,  targetStats)    :
                if verbose: 
                    print  "returning fleetlist: %s"%fleetList
                return fleetList
    # finished loop without meeting reqs
    if extendSearch:
        thisSys = universe.getSystem(thisSystemID)
        for neighborID in [el.key() for el in universe.getSystemNeighborsMap(thisSystemID,  foAI.foAIstate.empireID) ]:
            if neighborID not in systemsChecked  and neighborID not in systemsToCheck   and neighborID in foAI.foAIstate.exploredSystemIDs:
                systemsToCheck.append(neighborID)
    try:
        resList = getFleetsForMission(nships,  targetStats,  minStats,  curStats,  species,  systemsToCheck,  systemsChecked, fleetPoolSet,  fleetList,  takeAny,  extendSearch,  verbose,  depth=depth+1)
        return resList
    except:
        s1=len(systemsToCheck)
        s2=len(systemsChecked)
        s3=len(set( systemsToCheck + systemsChecked ) )
        print "Error: exception triggered in 'getFleetsForMissions' and caught at depth  %d  w/s1/s2/s3 (%d/%d/%d):  "%(depth+2,  s1,  s2,  s3),  traceback.format_exc()
        #print ("Error: call parameters were targetStats: %s,  curStats: %s,  species: '%s',  systemsToCheck: %s,  systemsChecked: %s,  fleetPoolSet: %s,  fleetList: %s"%(
        #                            targetStats,  curStats,  species,  systemsToCheck,  systemsChecked,  fleetPoolSet,  fleetList))
        return []
import traceback
    
def splitFleet(fleetID):
    "splits a fleet into its ships"

    universe = fo.getUniverse()
    empireID = fo.empireID()

    fleet = universe.getFleet(fleetID)
    newfleets = []

    if fleet == None: return []
    if not fleet.ownedBy(empireID): return []

    if len(list(fleet.shipIDs)) <= 1:  # fleet with only one ship cannot be split
        return []
    shipIDs = list( fleet.shipIDs )
    for shipID in shipIDs[1:]:
        newFleetID = fo.issueNewFleetOrder("Fleet %d"%(shipID), shipID)
        if newFleetID:
            newFleet=universe.getFleet(newFleetID)
            if not newFleet:
                print "Error: newly split fleet %d not available from universe"%newFleetID
            fo.issueRenameOrder(newFleetID,  "Fleet %5d"%newFleetID) #to ease review of debugging logs
            fo.issueAggressionOrder(newFleetID,  True)
            role = foAI.foAIstate.getFleetRole(newFleetID) #and mission?
            foAI.foAIstate.getRating(newFleetID) #
            newfleets.append(newFleetID)
            foAI.foAIstate.newlySplitFleets[newFleetID]=True
        else:
            if fleet.systemID==-1:
                print "Error - tried to split ship id (%d) from fleet %d when fleet is in starlane"%(shipID,  fleetID)
            else:
                print "Error - got no fleet ID back after trying to split ship id (%d) from fleet %d"%(shipID,  fleetID)
    foAI.foAIstate.getFleetRole(fleetID, forceNew=True) #
    foAI.foAIstate.updateFleetRating(fleetID) #
    if newfleets !=[]:
        foAI.foAIstate.ensureHaveFleetMissions(newfleets)
    return newfleets

def mergeFleetAintoB(fleetA_ID,  fleetB_ID,  leaveRating=0,  needRating=0,  context=""):
    universe = fo.getUniverse()
    fleetA = universe.getFleet(fleetA_ID)
    sysID=fleetA.systemID
    fleetB = universe.getFleet(fleetB_ID)
    if not fleetA or not fleetB:
        return 0
    success = True
    initRating = foAI.foAIstate.getRating(fleetA_ID)
    remainingRating = initRating.copy()
    transferredRating = 0
    transferredAttack=0
    transferredHealth=0
    BHasMonster=False
    for shipID in fleetB.shipIDs:
        thisShip=universe.getShip(shipID)
        if (not thisShip):
            continue
        if  thisShip.isMonster:
            BHasMonster = True
            break
    for shipID in fleetA.shipIDs:
        thisShip=universe.getShip(shipID)
        if (not thisShip) or  ( thisShip.isMonster != BHasMonster ) : 
            continue  
        stats = foAI.foAIstate.getDesignIDStats(thisShip.designID)
        thisRating = stats['attack'] * ( stats['structure'] + stats['shields'] )
        if (remainingRating['attack'] -stats['attack'])*(remainingRating['health'] -( stats['structure'] + stats['shields'] ))  < leaveRating:
            continue
        #remainingRating -= thisRating
        remainingRating['attack'] -= stats['attack']
        remainingRating['health'] -= ( stats['structure'] + stats['shields'] )
        thisSuccess =  ( fo.issueFleetTransferOrder(shipID,  fleetB_ID) )#returns a zero if transfer failure
        if thisSuccess:
            transferredRating += thisRating
            transferredAttack += stats['attack']
            transferredHealth += ( stats['structure'] + stats['shields'] )
        else:
            print "\t\t\t\t *** attempted transfer of ship %4d,  formerly of fleet %4d,  into fleet %4d  with result %d; %s"%(shipID,  fleetA_ID,  fleetB_ID,  thisSuccess,   [" context is %s"%context, ""][context==""])
        success = success and thisSuccess
        if needRating !=0 and needRating <=  transferredAttack*transferredHealth:  #transferredRating:
            break
    fleetA = universe.getFleet(fleetA_ID)
    if (not fleetA) or fleetA.empty  or fleetA_ID in universe.destroyedObjectIDs(fo.empireID()):
        #print "\t\t\t\t\tdeleting fleet info for old fleet %d after transfers into fleet %d"%(fleetA_ID,  fleetB_ID)
        foAI.foAIstate.deleteFleetInfo(fleetA_ID)
    else:
        newARating = foAI.foAIstate.updateFleetRating(fleetA_ID)
        if  success : #and ( newARating==remainingRating) :
            #print "\t\t\t\t\t\t\%d rating from fleet %d successfully transferred to fleet %d,  leaving %d"%(transferredAttack*transferredHealth,  fleetA_ID,  fleetB_ID,  newARating['overall'])
            pass
        else:
            #print "\t\t\t\t\t\t transfer of %d rating from fleet %d  to fleet %d was attempted but appears to have had problems, leaving %d"%(transferredAttack*transferredHealth,  fleetA_ID,  fleetB_ID,  newARating['overall'])
            pass
    foAI.foAIstate.updateFleetRating(fleetB_ID)
    return transferredAttack*transferredHealth,  transferredAttack,  transferredHealth

def fleetHasShipWithRole(fleetID, shipRole):
    "returns True if a ship with shipRole is in the fleet"

    universe = fo.getUniverse()
    fleet = universe.getFleet(fleetID)

    if fleet == None: return False
    for shipID in fleet.shipIDs:
        ship = universe.getShip(shipID)
        if (foAI.foAIstate.getShipRole(ship.design.id) == shipRole):
            return True
    return False

def getShipIDWithRole(fleetID, shipRole):
    "returns a ship with the specified role in the fleet"

    if not fleetHasShipWithRole(fleetID, shipRole):
        print "No ship with role " + __AIShipRoleTypeNames.name(shipRole) + " found."
        return None

    universe = fo.getUniverse()
    fleet = universe.getFleet(fleetID)

    for shipID in fleet.shipIDs:
        ship = universe.getShip(shipID)
        if (foAI.foAIstate.getShipRole(ship.design.id) == shipRole):
            return shipID

def getAllEverVisibleFleetIDs(): #may be only currently visible
    return  fo.getUniverse().fleetIDs

def getEmpireFleetIDs( empireID=None):
    "returns all fleetIDs of specified empire, defauls to current empire"
    if empireID is None:
        empireID = foAI.foAIstate.empireID
    universe = fo.getUniverse()
    empireFleetIDs = []
    destroyedObjectIDs = universe.destroyedObjectIDs(empireID)
    for fleetID in set(list(universe.fleetIDs) + list(foAI.foAIstate.newlySplitFleets)):
        fleet = universe.getFleet(fleetID)
        if (fleet == None): continue
        if ( fleet.ownedBy(empireID))  and (fleetID not in destroyedObjectIDs) and (not(fleet.empty) )and  (not (len(fleet.shipIDs)==0) ):
            empireFleetIDs.append( fleetID )
    return empireFleetIDs

def getEmpireFleetIDsByRole(fleetRole):
    "returns a list with fleetIDs that have the specified role"
    fleetIDs = getEmpireFleetIDs()
    fleetIDsWithRole = []
    for fleetID in fleetIDs:
        if not (foAI.foAIstate.getFleetRole(fleetID) == fleetRole): continue
        fleetIDsWithRole.append(fleetID)
    return fleetIDsWithRole

def extractFleetIDsWithoutMissionTypes(fleetIDs):
    "extracts a list with fleetIDs that have no mission"
    fleetIDsWithoutMission = []
    for fleetID in fleetIDs:
        aiFleetMission = foAI.foAIstate.getAIFleetMission(fleetID)
        if not aiFleetMission.hasAnyAIMissionTypes():
            fleetIDsWithoutMission.append(fleetID)
    return fleetIDsWithoutMission

def assessFleetRole(fleetID):
    "assesses ShipRoles represented in a fleet and returns a corresponding overall fleetRole"
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
            role = foAI.foAIstate.getShipRole(ship.design.id)
        else:
            role = AIShipRoleType.SHIP_ROLE_INVALID

        if role != AIShipRoleType.SHIP_ROLE_INVALID:
            shipRoles[role] = shipRoles.get(role,  0)  + 1
    # determine most common shipRole
    favouriteRole = AIShipRoleType.SHIP_ROLE_INVALID
    for shipRole in shipRoles:
        if shipRoles[shipRole] == max(shipRoles.values()):
            favouriteRole = shipRole

    # assign fleet role
    if  AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION in shipRoles:
        selectedRole= AIFleetMissionType.FLEET_MISSION_COLONISATION
    elif AIShipRoleType.SHIP_ROLE_BASE_COLONISATION in shipRoles:
        selectedRole= AIFleetMissionType.FLEET_MISSION_ORBITAL_COLONISATION
    elif AIShipRoleType.SHIP_ROLE_CIVILIAN_OUTPOST in shipRoles:
        selectedRole= AIFleetMissionType.FLEET_MISSION_OUTPOST
    elif AIShipRoleType.SHIP_ROLE_BASE_OUTPOST in shipRoles:
        selectedRole= AIFleetMissionType.FLEET_MISSION_ORBITAL_OUTPOST
    elif AIShipRoleType.SHIP_ROLE_BASE_DEFENSE in shipRoles:
        selectedRole= AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE
    elif AIShipRoleType.SHIP_ROLE_BASE_INVASION in shipRoles:
        selectedRole= AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION
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
    #print "fleetID %d : primary fleet mission type %d: '%s' ; found ship roles %s : %s ; rating %d"%(fleetID,selectedRole,   __AIFleetMissionTypeNames.name(selectedRole),  
    #                                                                           shipRoles,  [ "%s: %d "%(__AIShipRoleTypeNames.name(rtype),  rnum) for rtype, rnum in  shipRoles.items()] ,  foAI.foAIstate.getRating(fleetID).get('overall', 0))
    return selectedRole

def assessShipDesignRole(design):
    if design.parts.__contains__("CO_OUTPOST_POD"):
        if design.starlaneSpeed > 0:
            return AIShipRoleType.SHIP_ROLE_CIVILIAN_OUTPOST
        else:
            return AIShipRoleType.SHIP_ROLE_BASE_OUTPOST
            
    if design.parts.__contains__("CO_COLONY_POD") or design.parts.__contains__("CO_SUSPEND_ANIM_POD"):
        if design.starlaneSpeed > 0:
            return AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION
        else:
            return AIShipRoleType.SHIP_ROLE_BASE_COLONISATION
            
    if design.parts.__contains__("GT_TROOP_POD"):
        if design.starlaneSpeed > 0:
            return AIShipRoleType.SHIP_ROLE_MILITARY_INVASION
        else:
            return AIShipRoleType.SHIP_ROLE_BASE_INVASION
            
    if design.starlaneSpeed == 0:
        string1 =  "Ship Design %s has partslist %s"%(design.name(False),  [part for part in design.parts])
        if len(design.parts)==0 or  design.parts[0] in [ "SH_DEFENSE_GRID", "SH_DEFLECTOR" ,  "SH_MULTISPEC" ] or (len(design.parts)==1 and design.parts[0]==''):
            #print string1,  "-- classifying as Base Defense"
            return AIShipRoleType.SHIP_ROLE_BASE_DEFENSE
        else:
            #print string1,  "-- classifying as Invalid"
            return AIShipRoleType.SHIP_ROLE_INVALID        
            
    stats = foAI.foAIstate.getDesignIDStats(design.id)
    rating = stats['attack'] * ( stats['structure'] + stats['shields'] )
    scoutNames = AIShipDesignTypes.explorationShip.keys()
    if  ( "SD_SCOUT" in design.name(False)  )  or  ( design.name(False).split('-')[0] in scoutNames )    :
        for part in design.parts:
            if "DT_DETECTOR" in part:
                return AIShipRoleType.SHIP_ROLE_CIVILIAN_EXPLORATION
    if rating > 0: #positive attack stat
        return AIShipRoleType.SHIP_ROLE_MILITARY
    else:
        return AIShipRoleType.SHIP_ROLE_CIVILIAN_EXPLORATION  #let this be the default since even without detection part a ship has some inherent

def assessDesignIDStats(designID):
    design = fo.getShipDesign(designID)
    if design is None:
        return  {'attack':0, 'structure':0, 'shields':0}
    else:
        return  {'attack':design.attack, 'structure':design.structure, 'shields':design.shields}

def assessShipRole(shipID):
    "decides which role a ship has"
    ship = fo.getUniverse().getShip(shipID)
    if ship:
        return assessShipDesignRole( fo.getShipDesign(ship.designID) )
    else:
        return AIShipRoleType.SHIP_ROLE_INVALID

def generateAIFleetOrdersForAIFleetMissions():
    "generates fleet orders from targets"
    print("Generating fleet orders")

    # The following fleet lists are based on *Roles* -- Secure type missions are done by fleets with Military Roles
    print "Fleets by Role\n"
    print "Exploration Fleets : " + str(getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_EXPLORATION))
    print "Colonization Fleets: " + str(getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_COLONISATION))
    print "Outpost Fleets     : " + str(getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_OUTPOST))
    print "Attack Fleets      : " + str(getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_ATTACK))
    print "Defend Fleets      : " + str(getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_DEFEND))
    print "Invasion Fleets    : " + str(getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_INVASION))
    print "Military Fleets     : " + str(getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_MILITARY))
    print "Orbital Defense Fleets    : " + str(getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE))
    print "Outpost Base Fleets    : " + str(getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_ORBITAL_OUTPOST))
    print "Securing Fleets    : " + str(getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_SECURE)) + " (currently FLEET_MISSION_MILITARY should be used instead of this Role)"
    print "Unclassifyable Fleets  : " + str(getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_INVALID))

    if fo.currentTurn() <50:
        print ""
        print "Explored systems  :"
        printSystems(foAI.foAIstate.getExplorableSystems(AIExplorableSystemType.EXPLORABLE_SYSTEM_EXPLORED))
        print "Unexplored systems:"
        printSystems(foAI.foAIstate.getExplorableSystems(AIExplorableSystemType.EXPLORABLE_SYSTEM_UNEXPLORED))
        print ""

    explorationAIFleetMissions = foAI.foAIstate.getAIFleetMissionsWithAnyMissionTypes([AIFleetMissionType.FLEET_MISSION_EXPLORATION])
    if len( explorationAIFleetMissions) >0:
        print "Exploration targets: "
    else:
        print "Exploration targets:  None"
    for explorationAIFleetMission in explorationAIFleetMissions:
        print "    " + str(explorationAIFleetMission)

    colonisationAIFleetMissions = foAI.foAIstate.getAIFleetMissionsWithAnyMissionTypes([AIFleetMissionType.FLEET_MISSION_COLONISATION])
    if len( colonisationAIFleetMissions) >0:
        print "Colonization targets: "
    else:
        print "Colonization targets:  None"
    for colonisationAIFleetMission in colonisationAIFleetMissions:
        print "    " + str(colonisationAIFleetMission)

    outpostAIFleetMissions = foAI.foAIstate.getAIFleetMissionsWithAnyMissionTypes([AIFleetMissionType.FLEET_MISSION_OUTPOST])
    if len( outpostAIFleetMissions) >0:
        print "Outpost targets: "
    else:
        print "Outpost targets:  None"
    for outpostAIFleetMission in outpostAIFleetMissions:
        print "    " + str(outpostAIFleetMission)

    outpostBaseAIFleetMissions = foAI.foAIstate.getAIFleetMissionsWithAnyMissionTypes([AIFleetMissionType.FLEET_MISSION_ORBITAL_OUTPOST])
    if len( outpostBaseAIFleetMissions) >0:
        print "Outpost Base targets (must have been interrupted by combat): "
    else:
        print "Outpost targets:  None (as expected, due to expected timing of order submission and execution)"
    for outpostAIFleetMission in outpostBaseAIFleetMissions:
        print "    " + str(outpostAIFleetMission)

    invasionAIFleetMissions = foAI.foAIstate.getAIFleetMissionsWithAnyMissionTypes([AIFleetMissionType.FLEET_MISSION_INVASION])
    if len( invasionAIFleetMissions) >0:
        print "Invasion targets: "
    else:
        print "Invasion targets:  None"
    for invasionAIFleetMission in invasionAIFleetMissions:
        print "    " + str(invasionAIFleetMission)

    militaryAIFleetMissions = foAI.foAIstate.getAIFleetMissionsWithAnyMissionTypes([AIFleetMissionType.FLEET_MISSION_MILITARY])
    if len( militaryAIFleetMissions) >0:
        print "General Military targets: "
    else:
        print "General Military targets:  None"
    for militaryAIFleetMission in militaryAIFleetMissions:
        print "    " + str(militaryAIFleetMission)
        
    secureAIFleetMissions = foAI.foAIstate.getAIFleetMissionsWithAnyMissionTypes([AIFleetMissionType.FLEET_MISSION_SECURE])
    if len( secureAIFleetMissions) >0:
        print "Secure targets: "
    else:
        print "Secure targets:  None"
    for secureAIFleetMission in secureAIFleetMissions:
        print "    " + str(secureAIFleetMission)
        
    orbDefenseAIFleetMissions = foAI.foAIstate.getAIFleetMissionsWithAnyMissionTypes([AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE])
    if len( orbDefenseAIFleetMissions) >0:
        print "Orbital Defense targets: "
    else:
        print "Orbital Defense targets:  None"
    for ODAIFleetMission in orbDefenseAIFleetMissions:
        print "    " + str(ODAIFleetMission)

    aiFleetMissions = foAI.foAIstate.getAllAIFleetMissions()

    for aiFleetMission in aiFleetMissions:
        aiFleetMission.generateAIFleetOrders()

def issueAIFleetOrdersForAIFleetMissions():
    "issues fleet orders"

    print ""
    universe=fo.getUniverse()
    aiFleetMissions = foAI.foAIstate.getAllAIFleetMissions()
    round = 0
    while (round <3):
        round += 1
        print "issuing fleet orders Round %d:"%round
        for aiFleetMission in aiFleetMissions:
            fleetID = aiFleetMission.getAITargetID()
            fleet = aiFleetMission.getAITarget().getTargetObj()
            if (not fleet) or ( len(fleet.shipIDs)==0)  or fleetID in universe.destroyedObjectIDs(fo.empireID()):  # in case fleet was merged into another previously during this turn
                continue
            aiFleetMission.issueAIFleetOrders()
        aiFleetMissions = foAI.foAIstate.misc.get('ReassignedFleetMissions',  [])
        foAI.foAIstate.misc['ReassignedFleetMissions']=[]
    print ""

def printSystems(systemIDs):
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

