import freeOrionAIInterface as fo
import FreeOrionAI as foAI
import AIstate
from EnumsAI import AIPriorityType, getAIPriorityResourceTypes, AIFocusType
import PlanetUtilsAI
from random import shuffle
from time import time

resourceTimerFile=None
doResourceTiming=True
__timerEntries1=["TopResources",  "SetCapital",  "SetPlanets",  "SetAsteroids",  "SetGiants",  "PrintResources" ]
__timerEntries2=["getPlanets",  "Filter",  "Priority",  "Shuffle",  "Targets",  "Loop" ]
__timerEntries = __timerEntries2
__timerFileFmt = "%8d"+ (len(__timerEntries)*"\t %8d")




def topResourcePriority():
    "calculate top resource priority"

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    ownedPlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)

    resourcePriorities = {}
    for priorityType in getAIPriorityResourceTypes():
        resourcePriorities[priorityType] = foAI.foAIstate.getPriority(priorityType)

    sortedPriorities = resourcePriorities.items()
    sortedPriorities.sort(lambda x,y: cmp(x[1], y[1]), reverse=True)
    topPriority = -1
    for evaluationPair in sortedPriorities:
        if topPriority < 0:
            topPriority = evaluationPair[0]

    return topPriority

def getResourceTargetTotals():
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    ownedPlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    capitalID = PlanetUtilsAI.getCapital()
    #    planet = universe.getPlanet(planetID)
    #   if planet.currentMeterValue(fo.meterType.population) >0:
    planets = map(universe.getPlanet,  ownedPlanetIDs)
    targetPP = sum( map( lambda x: x.currentMeterValue(fo.meterType.targetIndustry),  planets) )
    targetRP = sum( map( lambda x: x.currentMeterValue(fo.meterType.targetResearch),  planets) )
    return targetPP,  targetRP
    
def setCapitalIDResourceFocus():
    "set resource focus of CapitalID planet"

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    ownedPlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    capitalID = PlanetUtilsAI.getCapital()
    topPriority = topResourcePriority()

    if topPriority == AIPriorityType.PRIORITY_RESOURCE_GROWTH:
        newFocus = AIFocusType.FOCUS_GROWTH
        for planetID in ownedPlanetIDs:
            planet = universe.getPlanet(planetID)
            focus = newFocus
            if planetID == capitalID and focus in planet.availableFoci:
                fo.issueChangeFocusOrder(planetID, focus)
    elif topPriority == AIPriorityType.PRIORITY_RESOURCE_PRODUCTION:
        newFocus = AIFocusType.FOCUS_INDUSTRY
        for planetID in ownedPlanetIDs:
            planet = universe.getPlanet(planetID)
            focus = newFocus
            if planetID == capitalID and focus in planet.availableFoci:
                fo.issueChangeFocusOrder(planetID, focus)
    elif topPriority == AIPriorityType.PRIORITY_RESOURCE_RESEARCH:
        newFocus = AIFocusType.FOCUS_RESEARCH
        for planetID in ownedPlanetIDs:
            planet = universe.getPlanet(planetID)
            focus = newFocus
            if planetID == capitalID and focus in planet.availableFoci:
                fo.issueChangeFocusOrder(planetID, focus)

def setGeneralPlanetResourceFocus():
    "set resource focus of planets except capitalID, asteroids, and gas giants"
    global __timerFile

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    timer= [ time() ] # getPlanets
    empirePlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    timer.append( time() ) #filter
    capitalIDL = [PlanetUtilsAI.getCapital()]
    asteroids = PlanetUtilsAI.getTypePlanetEmpireOwned(fo.planetType.asteroids)
    gasGiants = PlanetUtilsAI.getTypePlanetEmpireOwned(fo.planetType.gasGiant)
    generalPlanetIDs = list(set(empirePlanetIDs) - (set(capitalIDL)|set(asteroids)|set(gasGiants)))
    timer.append( time() ) #Priority
    #fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
    #fleetSupplyablePlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(fleetSupplyableSystemIDs)
    ppPrio = foAI.foAIstate.getPriority(AIPriorityType.PRIORITY_RESOURCE_PRODUCTION)
    rpPrio = foAI.foAIstate.getPriority(AIPriorityType.PRIORITY_RESOURCE_RESEARCH)
    priorityRatio = float(ppPrio)/rpPrio
    timer.append( time() ) # shuffle
    # not supporting Growth for general planets until also adding code to make sure would actually benefit
    shuffle(generalPlanetIDs)
    timer.append( time() ) # targets
    pp, rp = getResourceTargetTotals()
    targetRatio = pp/rp
    nplanets = len(generalPlanetIDs)
    nChanges = int(  (  nplanets *  max( 1.0/rp,  2.0/pp)  *  ( (pp - rp* priorityRatio)/(priorityRatio +2)))  +0.5 ) # weird formula I came up with to estimate desired number of changes
    print "current target totals -- pp: %.1f   rp: %.1f   ; ratio %.2f  ; desired ratio %.2f  will change up to %d  foci from total of %d planets"%(pp, rp, targetRatio,  priorityRatio,  nChanges,  nplanets)
    timer.append( time() ) #loop
    iChanges = 0
    for planetID in generalPlanetIDs:
        if iChanges >= nChanges: break
        planet = universe.getPlanet(planetID)
        oldFocus=planet.focus
        targetRatio = pp/rp
        ratioRatio = priorityRatio / targetRatio
        if  (AIFocusType.FOCUS_MINING in planet.availableFoci) and ((priorityRatio > 0.5 ) or ( ratioRatio > 0.9  ) ) :  #could be a more complex decision here, 
            if oldFocus != AIFocusType.FOCUS_MINING:
                iChanges +=1 #even if not necessarily  directed towards desired ratio
                fo.issueChangeFocusOrder(planetID, AIFocusType.FOCUS_MINING)
                print "Changing planet focus for ID %s : %s  from %s to %s "%(planetID,  planet.name,  oldFocus,   AIFocusType.FOCUS_MINING )
            continue
        newFocus=None
        if ( ratioRatio > 1.1 ): #needs industry
            if oldFocus == AIFocusType.FOCUS_INDUSTRY:
                continue
            else:
                newFocus = AIFocusType.FOCUS_INDUSTRY
        elif ( ratioRatio < 0.91 ):
            if oldFocus == AIFocusType.FOCUS_RESEARCH:
                continue
            else:
                newFocus = AIFocusType.FOCUS_RESEARCH
        if newFocus:
            #pp -= planet.currentMeterValue(fo.meterType.targetIndustry)
            #rp -= planet.currentMeterValue(fo.meterType.targetResearch)
            fo.issueChangeFocusOrder(planetID, newFocus)
            iChanges += 1
            #pp += planet.currentMeterValue(fo.meterType.targetIndustry)
            #rp += planet.currentMeterValue(fo.meterType.targetResearch)
            print "Changing planet focus for ID %s : %s  from %s to %s "%(planetID,  planet.name,  oldFocus,   newFocus )
    timer.append( time() ) #end
    if doResourceTiming and __timerEntries==__timerEntries2:
        times = [timer[i] - timer[i-1] for i in range(1,  len(timer) ) ]
        timeFmt = "%30s: %8d msec  "
        print "ResourcesAI Time Requirements:"
        for mod,  modTime in zip(__timerEntries,  times):
            print timeFmt%((30*' '+mod)[-30:],  int(1000*modTime))
        if resourceTimerFile:
            print "len times: %d  ;  len entries: %d "%(len(times),  len(__timerEntries))
            resourceTimerFile.write(  __timerFileFmt%tuple( [ fo.currentTurn() ]+map(lambda x: int(1000*x),  times )) +'\n')
            resourceTimerFile.flush()


def setAsteroidsResourceFocus():
    "change resource focus of asteroids from farming to mining"

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    ownedPlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    newFocus = AIFocusType.FOCUS_INDUSTRY
    for planetID in ownedPlanetIDs:
        planet = universe.getPlanet(planetID)
        focus = newFocus
        if planet.type == fo.planetType.asteroids and 'GRO_ORBIT_FARMING' in empire.availableTechs and focus in planet.availableFoci:
            fo.issueChangeFocusOrder(planetID, focus)

def setGasGiantsResourceFocus():
    "change resource focus of gas giants from farming to research"

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    ownedPlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    newFocus = AIFocusType.FOCUS_RESEARCH
    for planetID in ownedPlanetIDs:
        planet = universe.getPlanet(planetID)
        focus = newFocus
        if planet.type == fo.planetType.gasGiant and 'GRO_ORBIT_FARMING' in empire.availableTechs and focus in planet.availableFoci:
            fo.issueChangeFocusOrder(planetID, focus)

def printResourcesPriority():
    "calculate top resource priority"

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    ownedPlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    print "Resource Management:"
    print ""
    print "Resource Priorities:"
    resourcePriorities = {}
    for priorityType in getAIPriorityResourceTypes():
        resourcePriorities[priorityType] = foAI.foAIstate.getPriority(priorityType)

    sortedPriorities = resourcePriorities.items()
    sortedPriorities.sort(lambda x,y: cmp(x[1], y[1]), reverse=True)
    topPriority = -1
    for evaluationPair in sortedPriorities:
        if topPriority < 0:
            topPriority = evaluationPair[0]
        print "    ID|Score: " + str(evaluationPair)

    print "  Top Resource Priority: " + str(topPriority)
    # what is the focus of available resource centers?
    print ""
    print "Planet Resources Foci:"
    for planetID in ownedPlanetIDs:
        planet = universe.getPlanet(planetID)
        planetPopulation = planet.currentMeterValue(fo.meterType.population)
        print "  ID: " + str(planetID) + " Name: " + str(planet.name) + " Type: " + str(planet.type) + " Size: " + str(planet.size) + " Focus: " + str(planet.focus) + " Species: " + str(planet.speciesName) + " Population: " + str(planetPopulation)
    print "\n\nEmpire Totals:\nPopulation: %5d \nProduction: %5d\nResearch: %5d\n"%(empire.population(),  empire.productionPoints,  empire.resourceProduction(fo.resourceType.research))


def generateResourcesOrders():
    global __timerFile
    "generate resources focus orders"

    timer= [ time() ]
    # calculate top resource priority
    topResourcePriority()
    timer.append( time() )
    # set resource foci of planets
    setCapitalIDResourceFocus()
    timer.append( time() )
    setGeneralPlanetResourceFocus()
    timer.append( time() )
    setAsteroidsResourceFocus()
    timer.append( time() )
    setGasGiantsResourceFocus()
    timer.append( time() )

    printResourcesPriority()
    timer.append( time() )

    if doResourceTiming and __timerEntries==__timerEntries1:
        times = [timer[i] - timer[i-1] for i in range(1,  len(timer) ) ]
        timeFmt = "%30s: %8d msec  "
        print "ResourcesAI Time Requirements:"
        for mod,  modTime in zip(__timerEntries,  times):
            print timeFmt%((30*' '+mod)[-30:],  int(1000*modTime))
        if resourceTimerFile:
            resourceTimerFile.write(  __timerFileFmt%tuple( [ fo.currentTurn() ]+map(lambda x: int(1000*x),  times )) +'\n')
            resourceTimerFile.flush()
