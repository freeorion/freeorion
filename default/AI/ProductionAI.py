import freeOrionAIInterface as fo
import FreeOrionAI as foAI
from EnumsAI import AIExplorableSystemType, AIPriorityType, getAIPriorityResourceTypes, getAIPriorityProductionTypes, AIFocusType,  AIEmpireProductionTypes,  AIShipDesignTypes
import PlanetUtilsAI
import AIstate
from random import choice

def generateProductionOrders():
    
    empire = fo.getEmpire()
    universe = fo.getUniverse()
    homeworld = universe.getPlanet(empire.capitalID)
    
    "generate production orders"

    print "Production Queue Management:"
    empire = fo.getEmpire()
    totalPP = empire.productionPoints
    print ""
    print "  Total Available Production Points: " + str(totalPP)


    print "Empire Capital is:",  homeworld.name
    print "Buildings present at empire Capital (ID, Name, Type, Tags, Specials):"
    for bldg in homeworld.buildingIDs:
        thisObj=universe.getObject(bldg)
        tags=" ".join( thisObj.tags)
        specials=" ".join(thisObj.specials)
        print  "%10s \t %30s \t type:%30s \t tags:%20s \t specials: %20s "%(bldg,  thisObj.name,  thisObj.buildingTypeName,  tags,  specials )
    
    capitalBldgs = [universe.getObject(bldg).buildingTypeName for bldg in homeworld.buildingIDs]

    print "Possible building types to build:"
    possibleBuildingTypeIDs = empire.availableBuildingTypes
    for buildingTypeID in possibleBuildingTypeIDs:
        buildingType = fo.getBuildingType(buildingTypeID)
        print "    " + str(buildingType.name) + " cost:" + str(buildingType.productionCost) + " time:" + str(buildingType.productionTime)

    possibleBuildingTypes = [ fo.getBuildingType(buildingTypeID).name  for buildingTypeID in possibleBuildingTypeIDs ]

    print ""
    print "Buildings already in Production Queue:"
    productionQueue = empire.productionQueue
    queuedBldgs=[element for element in productionQueue if element.buildType == AIEmpireProductionTypes.BT_BUILDING]
    for bldg in queuedBldgs:
        print "    " + bldg.name + " turns:" + str(bldg.turnsLeft) + " PP:" + str(bldg.allocation)
    if queuedBldgs == []: print "None"
    print
    queuedBldgNames=[ bldg.name for bldg in queuedBldgs ]

    
    if  ("BLD_INDUSTRY_CENTER" in possibleBuildingTypes) and ("BLD_INDUSTRY_CENTER" not in (capitalBldgs+queuedBldgNames)):
        print "Enqueueing BLD_INDUSTRY_CENTER"
        fo.issueEnqueueBuildingProductionOrder("BLD_INDUSTRY_CENTER", empire.capitalID)
        
    if  ("BLD_EXOBOT_SHIP" in possibleBuildingTypes) and ("BLD_EXOBOT_SHIP" not in capitalBldgs+queuedBldgNames):
        print "Enqueueing BLD_EXOBOT_SHIP"
        fo.issueEnqueueBuildingProductionOrder("BLD_EXOBOT_SHIP", empire.capitalID)

    totalPPSpent = fo.getEmpire().productionQueue.totalSpent
    print "  Total Production Points Spent:     " + str(totalPPSpent)

    wastedPP = totalPP - totalPPSpent
    print "  Wasted Production Points:          " + str(wastedPP)

    print ""
    print "Possible ship designs to build:"
    possibleShipDesigns = empire.availableShipDesigns
    for shipDesignID in possibleShipDesigns:
        shipDesign = fo.getShipDesign(shipDesignID)
        print "    " + str(shipDesign.name(True)) + " cost:" + str(shipDesign.productionCost) + " time:" + str(shipDesign.productionTime)

    print ""
    print "Projects already in Production Queue:"
    productionQueue = empire.productionQueue
    for element in productionQueue:
        print "    " + element.name + " turns:" + str(element.turnsLeft) + " PP:" + str(element.allocation)

    print ""
    # get the highest production priorities
    print "Production Queue Priorities:"
    productionPriorities = {}
    for priorityType in getAIPriorityProductionTypes():
        productionPriorities[priorityType] = foAI.foAIstate.getPriority(priorityType)

    sortedPriorities = productionPriorities.items()
    sortedPriorities.sort(lambda x,y: cmp(x[1], y[1]), reverse=True)

    topPriority = -1
    topscore = -1
    priorityChoices=[]
    for ID,  score in sortedPriorities:
        if topscore < score:
            topPriority = ID
            topscore = score
        print "    ID|Score: " + str([ID, score])
        if ID != AIPriorityType.PRIORITY_PRODUCTION_BUILDINGS:
            priorityChoices.extend( int(score) * [ID] )

    print "  Top Production Queue Priority: " + str(topPriority)
    
    if priorityChoices == []:
        print "No non-building-production priorities with nonzero score, setting to default: Military"
        priorityChoices = [ AIPriorityType.PRIORITY_PRODUCTION_MILITARY ]
    print "\n ship priority selection list: \n %s \n\n"%str(priorityChoices)
    loopCount = 0
    shipTypeMap = dict( zip( [AIPriorityType.PRIORITY_PRODUCTION_EXPLORATION,  AIPriorityType.PRIORITY_PRODUCTION_OUTPOST,  AIPriorityType.PRIORITY_PRODUCTION_COLONISATION,  AIPriorityType.PRIORITY_PRODUCTION_INVASION,  AIPriorityType.PRIORITY_PRODUCTION_MILITARY], 
                                                [AIShipDesignTypes.explorationShip,  AIShipDesignTypes.outpostShip,  AIShipDesignTypes.colonyShip,  AIShipDesignTypes.troopShip,  AIShipDesignTypes.attackShip ] ) )
                                                
    if True: #if True use new code, if False use old code
        while (wastedPP > 0) and loopCount <100: #make sure don't get stuck in some nonbreaking loop like if all shipyards captured
            loopCount +=1
            thisPriority = choice( priorityChoices )
            print "selected priority: ",  thisPriority
            theseDesigns = [shipDesign for shipDesign in possibleShipDesigns if shipTypeMap.get(thisPriority,  "nomatch")  in fo.getShipDesign(shipDesign).name(False)  and getAvailableBuildLocations(shipDesign) != [] ]
            if theseDesigns == []: continue
            ships = [ ( fo.getShipDesign(shipDesign).name(False),  shipDesign) for shipDesign in theseDesigns ]
            bestShip = sorted( ships)[-1][-1]
            buildChoices = getAvailableBuildLocations(bestShip)
            loc = choice(buildChoices)
            print ""
            print "adding new ship to production queue: " + fo.getShipDesign(bestShip).name(True)
            fo.issueEnqueueShipProductionOrder(bestShip, loc)
            wastedPP -=  (fo.getShipDesign(bestShip).productionCost / fo.getShipDesign(bestShip).productionTime)
    else:
        locationIDs = getAvailableBuildLocations(shipDesignID)
        if len(locationIDs) > 0 and wastedPP > 0:
            for shipDesignID in possibleShipDesigns:
                shipDesign = fo.getShipDesign(shipDesignID)
                explorationShipName = "SD_SCOUT"
                colonyShipName = "SD_COLONY_SHIP"
                outpostShipName = "SD_OUTPOST_SHIP"
                troopShipName = "SD_TROOP_SHIP"
                if topPriority == 6 and shipDesign.name(False) == explorationShipName:
                    # exploration ship
                    print ""
                    print "adding new ship to production queue: " + shipDesign.name(True)
                    fo.issueEnqueueShipProductionOrder(shipDesignID, locationIDs[0])
                elif topPriority == 7 and shipDesign.canColonize and shipDesign.name(False) == outpostShipName:
                    # outpost ship
                    print ""
                    print "adding new ship to production queue: " + shipDesign.name(True)
                    fo.issueEnqueueShipProductionOrder(shipDesignID, locationIDs[0])
                elif topPriority == 8 and shipDesign.canColonize and shipDesign.name(False) == colonyShipName:
                    # colony ship
                    print ""
                    print "adding new ship to production queue: " + shipDesign.name(True)
                    fo.issueEnqueueShipProductionOrder(shipDesignID, locationIDs[0])
                elif topPriority == 9 and shipDesign.canInvade and shipDesign.name(False) == troopShipName:
                    # troop ship
                    print ""
                    print "adding new ship to production queue: " + shipDesign.name(True)
                    fo.issueEnqueueShipProductionOrder(shipDesignID, locationIDs[0])
                elif topPriority == 10 and shipDesign.isArmed:
                    # military ship
                    print ""
                    print "adding new ship to production queue: " + shipDesign.name(True)
                    fo.issueEnqueueShipProductionOrder(shipDesignID, locationIDs[0])
                elif shipDesign.attack > 0:
                    # military ship
                    print ""
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

def spentPP():
    "calculate PPs spent this turn so far"

    queue = fo.getEmpire().productionQueue
    return queue.totalSpent
