from EnumsAI import AIPriorityType, AIFleetMissionType, AIExplorableSystemType
import AIstate
import ColonisationAI
import EnumsAI
import FleetUtilsAI
import FreeOrionAI as foAI
import freeOrionAIInterface as fo

def calculatePriorities():
    "calculates the priorities of the AI player"

    print "Priority:"

    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_PRODUCTION_EXPLORATION, calculateExplorationPriority())
    ColonisationAI.getColonyFleets() # sets AIstate.colonisablePlanetIDs and AIstate.outpostPlanetIDs
    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_PRODUCTION_COLONISATION, calculateColonisationPriority())
    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_PRODUCTION_OUTPOST, calculateOutpostPriority())
    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_PRODUCTION_BUILDINGS, 20)
    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_PRODUCTION_MILITARY, 20)

    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_RESOURCE_FOOD, calculateFoodPriority())
    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_RESOURCE_PRODUCTION, calculateIndustryPriority())
    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_RESOURCE_MINERALS, calculateMineralsPriority())
    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_RESOURCE_RESEARCH, 10)

    foAI.foAIstate.printPriorities()

def calculateFoodPriority():
    "calculates the AI's need for food"
    # attempts to sustain a food stockpile whose size depends on the AI empires population
    # foodStockpile == 0 => returns 100, foodStockpile == foodTarget => returns 0

    empire = fo.getEmpire()
    foodStockpile = empire.resourceStockpile(fo.resourceType.food)
    foodTarget = 10 * empire.population() * AIstate.foodStockpileSize

    if (foodTarget == 0):
        return 0

    foodPriority = (foodTarget - foodStockpile) / foodTarget * 100

    print ""
    print "Size of food stockpile:  " + str(foodStockpile)
    print "Target food stockpile:   " + str (foodTarget)
    print "Priority for Food:       " + str(foodPriority)

    if foodPriority < 0:
        return 0

    return foodPriority

def calculateExplorationPriority():
    "calculates the demand for scouts by unexplored systems"

    numUnexploredSystems = len(foAI.foAIstate.getExplorableSystems(AIExplorableSystemType.EXPLORABLE_SYSTEM_UNEXPLORED))

    scoutIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_EXPLORATION)
    numScouts = len(scoutIDs)

    if (numUnexploredSystems == 0): return 0

    explorationPriority = (numUnexploredSystems - numScouts) / numUnexploredSystems * 100

    print ""
    print "Number of Scouts: " + str(numScouts)
    print "Number of Unexplored systems: " + str(numUnexploredSystems)
    print "Priority for scouts: " + str(explorationPriority)

    if explorationPriority < 0: return 0

    return explorationPriority

def calculateColonisationPriority():
    "calculates the demand for colony ships by colonisable planets"

    numColonisablePlanetIDs = len(AIstate.colonisablePlanetIDs)
    if (numColonisablePlanetIDs == 0): return 0

    colonyshipIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_COLONISATION)
    numColonyships = len(FleetUtilsAI.extractFleetIDsWithoutMissionTypes(colonyshipIDs))
    colonisationPriority = 100 * (numColonisablePlanetIDs - numColonyships) / numColonisablePlanetIDs

    print ""
    print "Number of Colony Ships        : " + str(numColonyships)
    print "Number of Colonisable planets : " + str(numColonisablePlanetIDs)
    print "Priority for colony ships     : " + str(colonisationPriority)

    if colonisationPriority < 0: return 0

    return colonisationPriority

def calculateOutpostPriority():
    "calculates the demand for outpost ships by colonisable planets"

    numOutpostPlanetIDs = len(AIstate.colonisableOutpostIDs)
    if (numOutpostPlanetIDs == 0): return 0

    outpostShipIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_OUTPOST)
    numOutpostShips = len(FleetUtilsAI.extractFleetIDsWithoutMissionTypes(outpostShipIDs))
    outpostPriority = 101 * (numOutpostPlanetIDs - numOutpostShips) / numOutpostPlanetIDs

    print ""
    print "Number of Outpost Ships       : " + str(numOutpostShips)
    print "Number of Colonisable outposts: " + str(numOutpostPlanetIDs)
    print "Priority for outpost ships    : " + str(outpostPriority)

    if outpostPriority < 0: return 0

    return outpostPriority

def calculateMineralsPriority():
    "calculates the demand for minerals by industry"

    empire = fo.getEmpire()

    # get current minerals and industry production
    mineralsProduction = empire.resourceProduction(fo.resourceType.minerals)
    mineralsStockpile = empire.resourceStockpile(fo.resourceType.minerals)
    mineralsTurns = mineralsStockpile / (mineralsProduction + 0.001)
    industryProduction = empire.resourceProduction(fo.resourceType.industry)

    mineralsPriority = 50 * industryProduction / (1 + mineralsProduction) - mineralsTurns

    print ""
    print "minerals production  : " + str(mineralsProduction)
    print "minerals stockpile   : " + str(mineralsStockpile)
    print "minerals turns       : " + str(mineralsTurns)
    print "industry production  : " + str(industryProduction)
    print "Priority for Minerals: " + str(mineralsPriority)

    return mineralsPriority

def calculateIndustryPriority():
    "calculates the demand for industry"

    empire = fo.getEmpire()

    # get current minerals and industry production
    mineralsProduction = empire.resourceProduction(fo.resourceType.minerals)
    mineralsStockpile = empire.resourceStockpile(fo.resourceType.minerals)
    mineralsTurns = mineralsStockpile / (mineralsProduction + 0.001)
    industryProduction = empire.resourceProduction(fo.resourceType.industry)

    # increase demand for industry if mineralsProduction is higher
    industryPriority = 50 * (mineralsProduction - mineralsTurns) / (industryProduction + 0.001)

    print ""
    print "minerals production  : " + str(mineralsProduction)
    print "minerals stockpile   : " + str(mineralsStockpile)
    print "minerals turns       : " + str(mineralsTurns)
    print "industry production  : " + str(industryProduction)
    print "Priority for Industry: " + str(industryPriority)

    return industryPriority

def calculateProductionPriority():
    "calculates the demand for production"

    priorityValues = []

    for priority in EnumsAI.getAIPriorityProductionTypes():
        priorityValues.append(foAI.foAIstate.getPriority(priority))

    print "priority production values: " + str(priorityValues)
    productionPriority = max(priorityValues)

    return productionPriority
