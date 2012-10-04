import freeOrionAIInterface as fo   # interface used to interact with FreeOrion AI client
import FreeOrionAI as foAI
import FleetUtilsAI
from EnumsAI import AIFleetMissionType, AIExplorableSystemType, AITargetType
import AITarget
import MoveUtilsAI
import PlanetUtilsAI
from random import shuffle

graphFlags={}
__interiorExploredSystemIDs = {} # explored systems whose neighbors are also all 
__borderExploredSystemIDs = {}
__borderUnexploredSystemIDs = {}
__blockedSystems={}
currentScoutFleetIDs = []

def updateScoutFleets():
    currentScoutFleetIDs[:] = []
    currentScoutFleetIDs.extend( FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_EXPLORATION) )
    
def assignScoutsToExploreSystems():
    # TODO: use Graph Theory to explore closest systems
    universe = fo.getUniverse()
    # order fleets to explore
    #explorableSystemIDs = foAI.foAIstate.getExplorableSystems(AIExplorableSystemType.EXPLORABLE_SYSTEM_UNEXPLORED)
    explorableSystemIDs =  set(__borderUnexploredSystemIDs) - set( __blockedSystems)
    if not explorableSystemIDs: 
        return
    fleetIDs = currentScoutFleetIDs
    availableScouts=[]
    alreadyCovered=set()
    for fleetID in fleetIDs:
        aiFleetMission = foAI.foAIstate.getAIFleetMission(fleetID)
        if len(aiFleetMission.getAIMissionTypes()) == 0:
            availableScouts.append(fleetID)
        else:
           alreadyCovered.union(  [targ.getTargetID() for targ in  aiFleetMission.getAITargets(AIFleetMissionType.FLEET_MISSION_EXPLORATION) ] )
                
    print "explorable sys IDs: %s"%explorableSystemIDs
    print "already targeted: %s"%alreadyCovered
    needsCoverage= explorableSystemIDs - alreadyCovered
    if not needsCoverage or not availableScouts:
        return
            
    #TODO: consider matching sys to closest scout, also consider rejecting scouts that would travel a blockaded  path
    sentList=[]
    sysList=  list(explorableSystemIDs) 
    shuffle( sysList ) #so that a monster defended system wont always be selected  early
    fleetList = list(availableScouts)
    isys= -1
    jfleet= -1
    while ( jfleet < len(fleetList) -1) :
        jfleet += 1
        fleetID =  fleetList[ jfleet ]
        while ( isys  < len(sysList) -1) :
            isys += 1
            sysID = sysList[ isys] 
            aiFleetMission = foAI.foAIstate.getAIFleetMission( fleetID )
            aiTarget = AITarget.AITarget(AITargetType.TARGET_SYSTEM, sysID )
            # add exploration mission to fleet with target unexplored system and this system is in range
            #print "try to assign scout to system %d"%systemID
            if len(MoveUtilsAI.canTravelToSystemAndReturnToResupply(fleetID, aiFleetMission.getLocationAITarget(), aiTarget, fo.empireID())) > 0:
                aiFleetMission.addAITarget(AIFleetMissionType.FLEET_MISSION_EXPLORATION, aiTarget)
                sentList.append(sysID)
                break
    print "sent scouting fleets to sysIDs : %s"%sysList

def getHomeSystemID():
    "returns the systemID of the home world"

    empire = fo.getEmpire()
    universe = fo.getUniverse()
    homeworld = universe.getPlanet(PlanetUtilsAI.getCapital())

    if homeworld:
        return homeworld.systemID

    return -1

def followVisSystemConnections(startSystemID,  homeSystemID):
    universe = fo.getUniverse()
    systemIDs = universe.systemIDs 
    empireID = foAI.foAIstate.empireID
    empire = fo.getEmpire()
    explorationList=[  startSystemID ]
    while explorationList !=[]:
        curSystemID = explorationList.pop(0)
        graphFlags[curSystemID] = 1
        system=universe.getSystem(curSystemID)
        if not system: 
            sysName="name unknown"
        else:
            sysName="named %s"%system.name
        if curSystemID in foAI.foAIstate.visBorderSystemIDs:
            preVis = "a border system"
        elif curSystemID in foAI.foAIstate.visInteriorSystemIDs:
            preVis = "an interior system"
        else:
            preVis = "an unknown system"
        print "*** system ID %d  %s ; previously %s, new visibility_turns vector is %s "%(curSystemID, sysName, preVis,   [turn for turn in universe.getVisibilityTurns(curSystemID,  empireID)])
        statusStr = "*** system ID %d  %s ; "%(curSystemID, sysName)
        isVisible = ( universe.getVisibilityTurns(curSystemID,  empireID)[fo.visibility.partial] > 0 ) # more precisely, this means HAS BEEN visible
        isConnected = universe.systemsConnected(curSystemID, homeSystemID, -1)  #self.empireID) 
        statusStr += " -- is %s partially visible "%(["not",  ""][isVisible])
        statusStr += " -- is %s visibly connected to homesystem "%(["not",  ""][isConnected])
        if isVisible:
            foAI.foAIstate.visInteriorSystemIDs[curSystemID] = 1
            if curSystemID in foAI.foAIstate.visBorderSystemIDs:
                del foAI.foAIstate.visBorderSystemIDs[curSystemID]
            #neighbors2= [sysID for dist, sysID in  universe.getImN(curSystemID,  empireID)]   #imNeighbors
            neighbors = list(  universe.getImmediateNeighbors(curSystemID,  empireID) )   #imNeighbors
            #if set(neighbors) != set(neighbors2):
            #    print "Error with neighbors: imn giving %s ; giN giving %s"%(neighbors,  neighbors2)
            if neighbors:
                statusStr += " -- has neighbors %s "%neighbors
                for sysID in neighbors :
                    if (sysID not in foAI.foAIstate.exploredSystemIDs):
                        foAI.foAIstate.unexploredSystemIDs[sysID] = 1
                    if  (sysID not in graphFlags) and (sysID not in foAI.foAIstate.visInteriorSystemIDs ):
                        foAI.foAIstate.visBorderSystemIDs[sysID] = 1
                        explorationList.append(sysID)
        print statusStr
        print "----------------------------------------------------------"


def updateExploredSystems():
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    #obsLanes = empire.supplyObstructedStarlaneTraversals
    #print "obstructed starlanes are: %s"%obsLanes
    empireID = foAI.foAIstate.empireID
    newlyExplored=[]
    stillUnexplored=[]
    for sysID in list(foAI.foAIstate.unexploredSystemIDs):
        if  (empire.hasExploredSystem(sysID)): 
            del foAI.foAIstate.unexploredSystemIDs[sysID]
            foAI.foAIstate.exploredSystemIDs[sysID] = 1
            if sysID in __borderUnexploredSystemIDs:
                del __borderUnexploredSystemIDs[sysID]
            newlyExplored.append(sysID)
        else:
            stillUnexplored.append(sysID)
            
    neighborList=[]
    dummy=[]
    for idList, nextList in [ (newlyExplored,  neighborList),  (neighborList,  dummy) ]:
        for sysID in idList:
            neighbors = list(  universe.getImmediateNeighbors(sysID,  empireID) )
            allExplored=True
            for neighborID in neighbors:
                if neighborID  in foAI.foAIstate.unexploredSystemIDs: #when it matters, unexplored will be smaller than explored
                    allExplored=False
                else:
                    nextList.append(neighborID)
            if allExplored:
                __interiorExploredSystemIDs[sysID] = 1
                if (sysID  in __borderExploredSystemIDs) :
                    del __borderExploredSystemIDs[sysID]
            else:
                __borderExploredSystemIDs[sysID] = 1
                
    for sysID in stillUnexplored:
        neighbors = list(  universe.getImmediateNeighbors(sysID,  empireID) )
        anyExplored=False
        for neighborID in neighbors:
            if neighborID  in foAI.foAIstate.exploredSystemIDs: #consider changing to unexplored test -- when it matters, unexplored will be smaller than explored, but need to not get previously untreated neighbors
                anyExplored=True
        if anyExplored:
            __borderUnexploredSystemIDs[sysID] = 1
        
