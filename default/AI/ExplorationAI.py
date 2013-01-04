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
currentScoutFleetIDs = []

def dictFromMap(map):
    return dict(  [  (el.key(),  el.data() ) for el in map ] )
    
def getBorderExploredSystemIDs():
    return list( __borderExploredSystemIDs )

def updateScoutFleets():
    currentScoutFleetIDs[:] = []
    currentScoutFleetIDs.extend( FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_EXPLORATION) )

def getCurrentExplorationInfo(verbose=True):
    "returns ( [current target list] ,  [available scout list] ) "
    fleetIDs = list( currentScoutFleetIDs )
    availableScouts=[]
    alreadyCovered=set()
    for fleetID in fleetIDs:
        aiFleetMission = foAI.foAIstate.getAIFleetMission(fleetID)
        if len(aiFleetMission.getAIMissionTypes()) == 0:
            availableScouts.append(fleetID)
        else:
            targets = [targ.getTargetID() for targ in  aiFleetMission.getAITargets(AIFleetMissionType.FLEET_MISSION_EXPLORATION) ] 
            if verbose:
                if len(targets)==0:
                    print "problem determining existing exploration target systems from targets:\n%s"%(aiFleetMission.getAITargets(AIFleetMissionType.FLEET_MISSION_EXPLORATION))
                else:
                    print "found existing exploration targets: %s"%targets
            alreadyCovered.update( targets )
    return ( list(alreadyCovered),  availableScouts )

def assignScoutsToExploreSystems():
    # TODO: use Graph Theory to explore closest systems
    universe = fo.getUniverse()
    capitalSysID = PlanetUtilsAI.getCapitalSysID()
    # order fleets to explore
    #explorableSystemIDs = foAI.foAIstate.getExplorableSystems(AIExplorableSystemType.EXPLORABLE_SYSTEM_UNEXPLORED)
    explorableSystemIDs =  list(__borderUnexploredSystemIDs) 
    if not explorableSystemIDs: 
        return
    expSystemsByDist = sorted(  map( lambda x: ( universe.linearDistance(capitalSysID, x),  x) ,  explorableSystemIDs ) )
    print "Exploration system considering following system-distance pairs:\n %s"%("[ "+ ",  ".join(["%3d : %5.1f"%(sys, dist) for dist, sys in expSystemsByDist]) +" ]")
    exploreList = [sysID for dist, sysID in expSystemsByDist ]

    alreadyCovered,  availableScouts  = getCurrentExplorationInfo()
    

    print "explorable sys IDs: %s"%exploreList
    print "already targeted: %s"%alreadyCovered
    if 'needsEmergencyExploration' not in dir(foAI.foAIstate):
        foAI.foAIstate.needsEmergencyExploration=[]
    needsCoverage= foAI.foAIstate.needsEmergencyExploration + [sysID for sysID in exploreList if sysID not in  alreadyCovered ] # emergency coverage cane be due to invasion detection trouble, etc.
    print "needs coverage: %s"%needsCoverage
    print "available scouts & AIstate locs: %s"%(map( lambda x: (x,  foAI.foAIstate.fleetStatus.get(x, {}).get('sysID', -1)),    availableScouts) )
    print "available scouts & universe locs: %s"%(map( lambda x: (x,  universe.getFleet(x).systemID),  availableScouts) )
    if not needsCoverage or not availableScouts:
        return

    availableScouts = set(availableScouts)
    sentList=[]
    while (len(availableScouts) > 0 ) and ( len(needsCoverage) >0):
        thisSysID = needsCoverage.pop(0)
        if (foAI.foAIstate.systemStatus.setdefault(thisSysID, {}).setdefault('monsterThreat', 0) > 2000*(1+foAI.foAIstate.aggression) ) or (fo.currentTurn() <20  and foAI.foAIstate.systemStatus[thisSysID]['monsterThreat'] > 200):
            print "Skipping exploration of system %d due to Big Monster,  threat %d"%(thisSysID,  foAI.foAIstate.systemStatus[thisSysID]['monsterThreat'])
            continue
        foundFleets=[]
        thisFleetList = FleetUtilsAI.getFleetsForMission(nships=1,  targetStats={},  minStats={},  curStats={},  species="",  systemsToCheck=[thisSysID],  systemsChecked=[], 
                                                     fleetPoolSet = availableScouts,   fleetList=foundFleets,  verbose=False)
        if thisFleetList==[]:
            print "seem to have run out of scouts while trying to cover sysID %d"%thisSysID
            break #must have ran out of scouts
        fleetID = thisFleetList[0]
        aiFleetMission = foAI.foAIstate.getAIFleetMission( fleetID )
        aiTarget = AITarget.AITarget(AITargetType.TARGET_SYSTEM, thisSysID )
        if len(MoveUtilsAI.canTravelToSystemAndReturnToResupply(fleetID, aiFleetMission.getLocationAITarget(), aiTarget, fo.empireID())) > 0:
            aiFleetMission.addAITarget(AIFleetMissionType.FLEET_MISSION_EXPLORATION, aiTarget)
            sentList.append(thisSysID)
        else: #system too far out, skip it, but can add scout back to available pool
            print "sysID %d too far out for fleet ( ID %d ) to readch"%(thisSysID,  fleetID)
            availableScouts.update(thisFleetList)
    print "sent scouting fleets to sysIDs : %s"%sentList
    return 
    """
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
    print "sent scouting fleets to sysIDs : %s"%sentList
    """

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
        if curSystemID in graphFlags:
            continue
        graphFlags[curSystemID] = 1
        system=universe.getSystem(curSystemID)
        if not system: 
            sysName=foAI.foAIstate.systemStatus.get(curSystemID, {}).get('name',  "name unknown")
        else:
            sysName = system.name or foAI.foAIstate.systemStatus.get(curSystemID, {}).get('name',  "name unknown")
        if curSystemID in foAI.foAIstate.visBorderSystemIDs:
            preVis = "a border system"
        elif curSystemID in foAI.foAIstate.visInteriorSystemIDs:
            preVis = "an interior system"
        else:
            preVis = "an unknown system"
        print "*** system ID %d ( %s ) ; previously %s, new visibility_turns vector is %s "%(curSystemID, sysName, preVis,   [turn for turn in universe.getVisibilityTurns(curSystemID,  empireID)])
        statusStr = "*** system ID %d  ( %s ) ; "%(curSystemID, sysName)
        isVisible = ( universe.getVisibilityTurns(curSystemID,  empireID)[fo.visibility.partial] > 0 ) # more precisely, this means HAS BEEN visible
        #print "previous visTurns result: %s"% ([val for val in universe.getVisibilityTurns(curSystemID,  empireID)],  )
        #print "new visTurns result: %s"% (dictFromMap( universe.getVisibilityTurnsMap(curSystemID,  empireID)),  )
        isConnected = universe.systemsConnected(curSystemID, homeSystemID, -1)  #self.empireID) 
        statusStr += " -- is %s partially visible "%(["not",  ""][isVisible])
        statusStr += " -- is %s visibly connected to homesystem "%(["not",  ""][isConnected])
        if isVisible:
            sysStatus = foAI.foAIstate.systemStatus.get(curSystemID, {})
            foAI.foAIstate.visInteriorSystemIDs[curSystemID] = 1
            if curSystemID in foAI.foAIstate.visBorderSystemIDs:
                del foAI.foAIstate.visBorderSystemIDs[curSystemID]
            #neighbors= dict( [(el.key(), el.data()) for el in  universe.getSystemNeighborsMap(curSystemID,  empireID)] )  #
            neighbors = dictFromMap( universe.getSystemNeighborsMap(curSystemID,  empireID) ).keys()
            if neighbors !={}:
                sysNeighbors=sysStatus.get('neighbors', {})
                for neighbor in neighbors:
                    if neighbor not in sysNeighbors:
                        sysNeighbors.setdefault(neighbor, {}).setdefault('placeholder',"")
                sysStatus['neighbors']=sysNeighbors
            neighbors=sysStatus['neighbors'].keys()
            sysPlanets=sysStatus.get('planets', {})
            print "    previously knew of system %d planets %s"%(curSystemID,  sysPlanets.keys())
            if system:
                for planet in system.planetIDs:
                    targPop=sysPlanets.setdefault(planet, {}).setdefault('targetPop',0)
                    planetObj=universe.getPlanet(planet)
                    troops=sysPlanets[planet].setdefault('troops', 0)
                    if planetObj:
                        newPop=planetObj.currentMeterValue(fo.meterType.targetPopulation)
                        if newPop != sysPlanets[planet]['targetPop']:
                            print "  * updating targetPop of planet %d ( %s ) to %.2f  from %.2f"%(planet,  planetObj.name,  newPop,  sysPlanets[planet]['targetPop'])
                        troops = planetObj.currentMeterValue(fo.meterType.troops)
                        if troops != sysPlanets[planet].get('troops', 0):
                            print "  * updating troops of planet %d ( %s ) to %.2f  from %.2f"%(planet,  planetObj.name,  troops,  sysPlanets[planet]['troops'])
                        sysPlanets[planet]['targetPop']= newPop
                        sysPlanets[planet]['troops']= troops
                sysStatus['planets']=sysPlanets
            print "    now know of system %d planets %s"%(curSystemID,  sysPlanets.keys())
            foAI.foAIstate.systemStatus[curSystemID]=sysStatus
            #neighbors = list(  universe.getImmediateNeighbors(curSystemID,  empireID) )   #imNeighbors
            #if set(neighbors) != set(neighbors2):
            #    print "Error with neighbors: imn giving %s ; giN giving %s"%(neighbors2,  neighbors)
            if len(neighbors)>0:
                statusStr += " -- has neighbors %s "%neighbors
                for sysID in neighbors:#.keys() :
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
    obsLanes = empire.obstructedStarlanes()
    print "object is: %s"%(obsLanes,  )
    obsLanesList = [el for el in obsLanes]
    if obsLanesList:
        print "obstructed starlanes  are: %s"%( obsLanesList,  )
    else:
        print "No obstructed Starlanes"
    empireID = foAI.foAIstate.empireID
    newlyExplored=[]
    stillUnexplored=[]
    for sysID in list(foAI.foAIstate.unexploredSystemIDs):
        if  (empire.hasExploredSystem(sysID)): 
            del foAI.foAIstate.unexploredSystemIDs[sysID]
            foAI.foAIstate.exploredSystemIDs[sysID] = 1
            sys=universe.getSystem(sysID)
            print "Moved system %d ( %s ) from unexplored list to explored list"%( sysID,  (sys and sys.name) or "name unknown")
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
        
    return newlyExplored
