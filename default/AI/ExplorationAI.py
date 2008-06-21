import freeOrionAIInterface as fo   # interface used to interact with FreeOrion AI client
import FreeOrionAI as foAI
import FleetUtils

# to do: assign scout fleets to closest planets (put in FleetUtils)
# do NOT pass empire from main function

# globals
explorableSystemIDs = []   # move out to AIstate


def generateExplorationOrders():

    empireID = fo.empireID()

    # get explorable systems and scouting fleets
    global explorableSystemIDs
    explorableSystemIDs = getExplorableSystemIDs(getHomeSystemID())

    removeInvalidExploreMissions()

    mFleetIDs = FleetUtils.getEmpireFleetIDsByRole("MT_EXPLORATION")
    fleetIDs = FleetUtils.extractFleetIDsWithoutMission(mFleetIDs) # line can probably be removed

    # order fleets to explore
    for fleetID in fleetIDs:        
             
        # if fleet already has a mission, continue
        if foAI.foAIstate.hasMission("MT_EXPLORATION", fleetID): continue
        
        # else send fleet to a system
        for systemID in explorableSystemIDs:
            
            # if system is already being explored, continue
            if foAI.foAIstate.hasTarget("MT_EXPLORATION", systemID): continue

            # send fleet, register an exploration mission
            fo.issueFleetMoveOrder(fleetID, systemID)
            foAI.foAIstate.addMission("MT_EXPLORATION", [fleetID, systemID])
            break

    print "Scouts: " + str(FleetUtils.getEmpireFleetIDsByRole("MT_EXPLORATION"))
    print "Systems being explored (fleet|system): " + str(foAI.foAIstate.getMissions("MT_EXPLORATION"))




def getHomeSystemID():
    "returns the systemID of the home world"

    empire = fo.getEmpire()
    universe = fo.getUniverse()
    homeworld = universe.getPlanet(empire.homeworldID)

    return homeworld.systemID


# returns list of systems ids known of by but not explored by empireID,
# that a ship located in startSystemID could reach via starlanes
def getExplorableSystemIDs(startSystemID):
    "returns explorable systems"

    universe = fo.getUniverse()
    objectIDs = universe.allObjectIDs
    empireID = fo.empireID()
    empire = fo.getEmpire()

    systemIDs = []

    for objectID in objectIDs:
        system = universe.getSystem(objectID)
        if (system == None): continue
        if (empire.hasExploredSystem(objectID)): continue
        if (not universe.systemsConnected(objectID, startSystemID, empireID)): continue
        systemIDs = systemIDs + [objectID]

    return systemIDs


def removeInvalidExploreMissions():
    "removes missions if fleet is destroyed or system already explored"

    print "Removing invalid exploration missions:"

    empire = fo.getEmpire()
    universe = fo.getUniverse()
    exploreMissions = foAI.foAIstate.getMissions("MT_EXPLORATION")

    # look for invalid missions
    for fleetID in exploreMissions:

        if empire.hasExploredSystem(exploreMissions[fleetID]):
            foAI.foAIstate.removeMission("MT_EXPLORATION", fleetID)
            # print "removed mission " + str(fleetID)
            continue

        fleet = universe.getFleet(fleetID)
        
        if (fleet == None):
            foAI.foAIstate.removeMission("MT_EXPLORATION", fleetID)
            continue
        
        if not (fleet.whollyOwnedBy(empire.empireID)):
            foAI.foAIstate.removeMission("MT_EXPLORATION", fleetID)

