import freeOrionAIInterface as fo
import FreeOrionAI as foAI
from EnumsAI import AIPriorityType, getAIPriorityResourceTypes, AIFocusType
import PlanetUtilsAI
import AIstate

def generateResourcesOrders():
    "generate resources focus orders"

    # get the highest resource priorities
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    ownedPlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    capitalID = PlanetUtilsAI.getCapitalID()
    print "Resources Management:"
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
    if topPriority == AIPriorityType.PRIORITY_RESOURCE_FOOD:
        newFocus = AIFocusType.FOCUS_FARMING
        print "  New Resource Focus:    " + str(newFocus)
        for planetID in ownedPlanetIDs:
            planet = universe.getPlanet(planetID)
            focus = newFocus
            if planetID == capitalID and newFocus in planet.availableFoci:
                print "  Capital ID: " + str(planetID) + " Resource Focus: " + str(newFocus)
                fo.issueChangeFocusOrder(planetID, newFocus)
    elif topPriority == AIPriorityType.PRIORITY_RESOURCE_MINERALS:
        newFocus = AIFocusType.FOCUS_MINING
        print "  Top Resource Focus:    " + str(newFocus)
        for planetID in ownedPlanetIDs:
            planet = universe.getPlanet(planetID)
            focus = newFocus
            if planetID == capitalID and newFocus in planet.availableFoci:
                print "  Capital ID: " + str(planetID) + " Resource Focus: " + str(newFocus)
                fo.issueChangeFocusOrder(planetID, newFocus) 
    elif topPriority == AIPriorityType.PRIORITY_RESOURCE_PRODUCTION:
        newFocus = AIFocusType.FOCUS_INDUSTRY
        print "  New Resource Focus:    " + str(newFocus)
        for planetID in ownedPlanetIDs:
            planet = universe.getPlanet(planetID)
            focus = newFocus
            if planetID == capitalID and newFocus in planet.availableFoci:
                print "  Capital ID: " + str(planetID) + " Resource Focus: " + str(newFocus)
                fo.issueChangeFocusOrder(planetID, newFocus)

    elif topPriority == AIPriorityType.PRIORITY_RESOURCE_RESEARCH:
        newFocus = AIFocusType.FOCUS_RESEARCH
        print "  New Resource Focus:    " + str(newFocus)

    # what is the focus of available resource centers?
    print ""
    print "Planet Resources Foci:"

    print ""  
    print "  Empire Owned planetIDs:" + str(ownedPlanetIDs)       
    for planetID in ownedPlanetIDs:
        planet = universe.getPlanet(planetID)
        planetPopulation = planet.currentMeterValue(fo.meterType.population)                
        print "  ID: " + str(planetID) + " Name: " + str(planet.name) + " Type: " + str(planet.type) + " Size: " + str(planet.size) + " Focus: " + str(planet.focus) + " Species: " + str(planet.speciesName) + " Population: " + str(planetPopulation)
