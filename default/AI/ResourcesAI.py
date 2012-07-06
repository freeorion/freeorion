import freeOrionAIInterface as fo
import FreeOrionAI as foAI
import AIstate
from EnumsAI import AIPriorityType, getAIPriorityResourceTypes, AIFocusType
import PlanetUtilsAI

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

def setCapitalIDResourceFocus():
    "set resource focus of CapitalID planet"

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    ownedPlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    capitalID = empire.capitalID
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

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    empirePlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    capitalID = [empire.capitalID]
    asteroids = PlanetUtilsAI.getTypePlanetEmpireOwned(fo.planetType.asteroids)
    gasGiants = PlanetUtilsAI.getTypePlanetEmpireOwned(fo.planetType.gasGiant)
    generalPlanetIDs = list(set(empirePlanetIDs) - (set(capitalID)|set(asteroids)|set(gasGiants)))
    topPriority = topResourcePriority()
    fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
    fleetSupplyablePlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(fleetSupplyableSystemIDs)

    if topPriority == AIPriorityType.PRIORITY_RESOURCE_GROWTH:
        newFocus = AIFocusType.FOCUS_GROWTH
        for planetID in generalPlanetIDs:
            planet = universe.getPlanet(planetID)
            focus = newFocus
            if focus in planet.availableFoci:
                fo.issueChangeFocusOrder(planetID, focus)
    elif topPriority == AIPriorityType.PRIORITY_RESOURCE_PRODUCTION:
        newFocus = AIFocusType.FOCUS_INDUSTRY
        for planetID in generalPlanetIDs:
            planet = universe.getPlanet(planetID)
            focus = newFocus
            if planetID in fleetSupplyablePlanetIDs and focus in planet.availableFoci:
                fo.issueChangeFocusOrder(planetID, focus)
    elif topPriority == AIPriorityType.PRIORITY_RESOURCE_RESEARCH:
        newFocus = AIFocusType.FOCUS_RESEARCH
        for planetID in generalPlanetIDs:
            planet = universe.getPlanet(planetID)
            focus = newFocus
            if planetID in fleetSupplyablePlanetIDs and focus in planet.availableFoci:
                fo.issueChangeFocusOrder(planetID, focus)
    else:
        focus = AIFocusType.FOCUS_FARMING
        if focus in planet.availableFoci:
            fo.issueChangeFocusOrder(planetID, focus)

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

def generateResourcesOrders():
    "generate resources focus orders"

    # calculate top resource priority
    topResourcePriority()

    # set resource foci of planets
    setCapitalIDResourceFocus()
    setGeneralPlanetResourceFocus()
    setAsteroidsResourceFocus()
    setGasGiantsResourceFocus()

    printResourcesPriority()
