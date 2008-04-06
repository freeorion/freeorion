import freeOrionAIInterface as fo   # interface used to interact with FreeOrion AI client


exploreMissions = {}


def generateExplorationOrders():
    print "Generating Exploration Orders"

    # retreive objects from freeOrionAIInterface that can be used to access the gamestate
    empire = fo.getEmpire()
    empireID = fo.empireID()
    universe = fo.getUniverse()    
    global exploreMissions

    homeSystemID = getHomeSystemID(empire, universe)
    fleetIDs = getEmpireStationaryFleetIDs(empireID)
    systemIDs = getExplorableSystemIDs(homeSystemID, empireID)
     
    # print "stationary fleet IDs: " + str(fleetIDs)
    # print "explorable systems: " + str(systemIDs)

    # order stationary fleets to explore
    for fleetID in fleetIDs:

        removeInvalidExploreMissions(exploreMissions, empire, universe)
             
        # if fleet already has a mission, continue
        if exploreMissions.has_key(fleetID): continue
        
        # else send fleet to a system
        for systemID in systemIDs:
            
            # if system is already being explored, continue
            if dictHasValue(exploreMissions, systemID): continue

            # send fleet, register an explore mission
            fo.issueFleetMoveOrder(fleetID, systemID)
            exploreMissions[fleetID] = systemID
            break

    print "Systems being explored (fleet|system): " + str(exploreMissions)






def dictHasValue(dictionary, value):

    for entry in dictionary:
        if dictionary[entry] == value: return True

    return False


# returns the systemID of the home world
def getHomeSystemID(empire, universe):
    
    homeworldObject = universe.getPlanet(empire.homeworldID)

    return homeworldObject.systemID


# returns list of systems ids known of by but not explored by empireID,
# that a ship located in startSystemID could reach via starlanes
def getExplorableSystemIDs(startSystemID, empireID):

    universe = fo.getUniverse()
    objectIDs = universe.allObjectIDs
    empire = fo.getEmpire(empireID)

    systemIDs = []

    for objectID in objectIDs:
        system = universe.getSystem(objectID)
        if (system == None): continue
        if (empire.hasExploredSystem(objectID)): continue
        if (not universe.systemsConnected(objectID, startSystemID, empireID)): continue
        systemIDs = systemIDs + [objectID]

    return systemIDs


# returns list of staitionary fleet ids owned by empireID
def getEmpireStationaryFleetIDs(empireID):
    universe = fo.getUniverse()
    objectIDs = universe.allObjectIDs

    fleetIDs = []

    for objectID in objectIDs:
        fleet = universe.getFleet(objectID)
        if (fleet == None): continue

        if (not fleet.whollyOwnedBy(empireID)): continue    

        # has no target
        if (fleet.nextSystemID != universe.invalidObjectID): continue

        # is at a sytem
        if fleet.systemID == universe.invalidObjectID: continue

        fleetIDs = fleetIDs + [objectID]

    return fleetIDs


def removeInvalidExploreMissions(exploreMissions, empire, universe):

    removeMissions = []

    for fleetID in exploreMissions:

        # system explored? => delete mission
        if empire.hasExploredSystem(exploreMissions[fleetID]):
            removeMissions.append(fleetID)
            continue

        # fleet not there (or from a different empire)? => delete mission
        fleet = universe.getFleet(fleetID)
        
        if (fleet == None):
            removeMissions.append(fleetID)
            continue
        
        if not (fleet.whollyOwnedBy(empire.empireID)): removeMissions.append(fleetID)

    for fleetID in removeMissions: del exploreMissions[fleetID]

