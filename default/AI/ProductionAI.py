import freeOrionAIInterface as fo
import FreeOrionAI as foAI
from EnumsAI import AIExplorableSystemType, AIPriorityType, getAIPriorityResourceTypes, getAIPriorityProductionTypes, AIFocusType
import PlanetUtilsAI
import AIstate

def generateProductionOrders():
    "generate production orders"

    print "Production:"

    empire = fo.getEmpire()
    totalPP = empire.productionPoints
    print "total Production Points: " + str(totalPP)

    print "possible building types to build:"
    possibleBuildingTypes = empire.availableBuildingTypes
    for buildingTypeID in possibleBuildingTypes:
        buildingType = fo.getBuildingType(buildingTypeID)
        print "    " + str(buildingType.name) + " cost:" + str(buildingType.productionCost) + " time:" + str(buildingType.productionTime)

    print "possible ship designs to build:"
    possibleShipDesigns = empire.availableShipDesigns
    for shipDesignID in possibleShipDesigns:
        shipDesign = fo.getShipDesign(shipDesignID)
        print "    " + str(shipDesign.name(True)) + " cost:" + str(shipDesign.productionCost) + " time:" + str(shipDesign.productionTime)

    print ""
    print "projects already in building queue:"
    productionQueue = empire.productionQueue
    for element in productionQueue:
        print "    " + element.name + " turns:" + str(element.turnsLeft) + " PP:" + str(element.allocation)

    print ""
    # get the highest production priorities
    print "Production Priorities"
    productionPriorities = {}
    for priorityType in getAIPriorityProductionTypes():
	productionPriorities[priorityType] = foAI.foAIstate.getPriority(priorityType)

    sortedPriorities = productionPriorities.items()
    sortedPriorities.sort(lambda x,y: cmp(x[1], y[1]), reverse=True)
    for evaluationPair in sortedPriorities:
        print "    ID|Score: " + str(evaluationPair)

    if productionQueue.empty:
        for shipDesignID in possibleShipDesigns:
            locationIDs = getAvailableBuildLocations(shipDesignID)
            shipDesign = fo.getShipDesign(shipDesignID)
            if len(locationIDs) > 0 and shipDesign.productionCost <= (totalPP * 30):
                if shipDesign.canColonize:
                    # colony ship
                    print "adding new ship to production queue: " + shipDesign.name(True)
                    fo.issueEnqueueShipProductionOrder(shipDesignID, locationIDs[0])
                elif shipDesign.attack > 0:
                    # attack ship
                    print "adding new ship to production queue: " + shipDesign.name(True)
                    fo.issueEnqueueShipProductionOrder(shipDesignID, locationIDs[0])

    print ""
    # get the highest resource priorities
    print "Resource Priorities"
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

    print "  topPriority: " + str(topPriority)
    if topPriority == AIPriorityType.PRIORITY_RESOURCE_FOOD:
	newFocus = AIFocusType.FOCUS_FARMING
    elif topPriority == AIPriorityType.PRIORITY_RESOURCE_MINERALS:
	newFocus = AIFocusType.FOCUS_MINING
    elif topPriority == AIPriorityType.PRIORITY_RESOURCE_PRODUCTION:
	newFocus = AIFocusType.FOCUS_INDUSTRY
    elif topPriority == AIPriorityType.PRIORITY_RESOURCE_RESEARCH:
	newFocus = AIFocusType.FOCUS_RESEARCH

    # what is the focus of available resource centers?
    print ""
    print "Resource Foci"
    empireID = empire.empireID
    universe = fo.getUniverse()
    ownedPlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    print "  ownedPlanetIDs:" + str(ownedPlanetIDs)
    for planetID in ownedPlanetIDs:
	planet = universe.getPlanet(planetID)
	print "  ID|Focus: " + str(planetID) + "|" + str(planet.focus)
	# for focus in planet.availableFoci:
	#     print "    >" + str(focus)
	if str(planet.focus) != str(newFocus) and str(newFocus) in planet.availableFoci:
	    fo.issueChangeFocusOrder(planetID, newFocus)
	    print "  issueChangeFocusOrder(" + str(planetID) + ", " + str(newFocus) + ")"

def getAvailableBuildLocations(shipDesignID):
    "returns locations where shipDesign can be built"

    result = []

    systemIDs = foAI.foAIstate.getExplorableSystems(AIExplorableSystemType.EXPLORABLE_SYSTEM_EXPLORED)
    planetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(systemIDs)
    shipDesign = fo.getShipDesign(shipDesignID)
    empire = fo.getEmpire()
    empireID = empire.empireID
    for planetID in planetIDs:
        if shipDesign.productionLocationForEmpire(empireID, planetID):
            result.append(planetID)

    return result
