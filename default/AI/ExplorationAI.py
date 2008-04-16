import freeOrionAIInterface as fo   # interface used to interact with FreeOrion AI client
import FreeOrionAI as foAI
import FleetUtils

def generateExplorationOrders():

    # retreive objects from freeOrionAIInterface
    empire = fo.getEmpire()
    empireID = fo.empireID()

    # get explorable systems and scouting fleets
    systemIDs = getExplorableSystemIDs(getHomeSystemID(empire), empireID)

    removeInvalidExploreMissions(empire)

    mFleetIDs = FleetUtils.getEmpireFleetIDsByRole(empireID, "MT_EXPLORATION")
    fleetIDs = FleetUtils.extractFleetIDsWithoutMission(mFleetIDs)

    # order fleets to explore
    for fleetID in fleetIDs:        
             
        # if fleet already has a mission, continue
        if foAI.foAIstate.hasMission("MT_EXPLORATION", fleetID): continue
        
        # else send fleet to a system
        for systemID in systemIDs:
            
            # if system is already being explored, continue
            if foAI.foAIstate.hasTarget("MT_EXPLORATION", systemID): continue

            # send fleet, register an exploration mission
            fo.issueFleetMoveOrder(fleetID, systemID)
            foAI.foAIstate.addMission("MT_EXPLORATION", [fleetID, systemID])
            break

    print "Scouts: " + str(FleetUtils.getEmpireFleetIDsByRole(empireID, "MT_EXPLORATION"))
    print "Systems being explored (fleet|system): " + str(foAI.foAIstate.getMissions("MT_EXPLORATION"))





def getHomeSystemID(empire):
    "returns the systemID of the home world"

    universe = fo.getUniverse()
    homeworld = universe.getPlanet(empire.homeworldID)

    return homeworld.systemID


# returns list of systems ids known of by but not explored by empireID,
# that a ship located in startSystemID could reach via starlanes
def getExplorableSystemIDs(startSystemID, empireID):
    "returns explorable systems"

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


def removeInvalidExploreMissions(empire):
    "removes missions if fleet is destroyed or system already explored"

    print "Removing invalid exploration missions:"

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

