import freeOrionAIInterface as fo
import FreeOrionAI as foAI
from EnumsAI import AIExplorableSystemType
import PlanetUtilsAI

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
        print "    " + str(buildingType.name) + " cost per turn:" + str(buildingType.buildCost) + " minimum build time:" + str(buildingType.buildTime)

    print "possible ship designs to build:"
    possibleShipDesigns = empire.availableShipDesigns
    for shipDesignID in possibleShipDesigns:
        shipDesign = fo.getShipDesign(shipDesignID)
        print "    " + str(shipDesign.name(True)) + " cost per turn:" + str(shipDesign.cost) + " minimum build time:" + str(shipDesign.buildTime)

    print "projects already in building queue:"
    productionQueue = empire.productionQueue
    for element in productionQueue:
        print "    " + element.name + " turns left:" + str(element.turnsLeft) + " allocated PP:" + str(element.allocation)

    if productionQueue.empty:
        for shipDesignID in possibleShipDesigns:
            locationIDs = getAvailableBuildLocations(shipDesignID)
            shipDesign = fo.getShipDesign(shipDesignID)
            if len(locationIDs) > 0 and shipDesign.cost <= (totalPP * 2):
                if shipDesign.attack > 0:
                    # attack ship
                    print "adding new ship to production queue: " + shipDesign.name(True)
                    fo.issueEnqueueShipProductionOrder(shipDesignID, locationIDs[0])
                elif shipDesign.canColonize:
                    # colony ship
                    print "adding new ship to production queue: " + shipDesign.name(True)
                    fo.issueEnqueueShipProductionOrder(shipDesignID, locationIDs[0])

    print ""

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
