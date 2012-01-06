import freeOrionAIInterface as fo
import FreeOrionAI as foAI
import AIstate
import EnumsAI
from EnumsAI import AIPriorityType, AIFleetMissionType, AIExplorableSystemType, getAIPriorityProductionTypes
import ColonisationAI
import FleetUtilsAI
from ResearchAI import getCompletedTechs
import InvasionAI
import MilitaryAI

def calculatePriorities():
    "calculates the priorities of the AI player"
    ColonisationAI.getColonyFleets() # sets AIstate.colonisablePlanetIDs and AIstate.outpostPlanetIDs
    InvasionAI.getInvasionFleets() # sets AIstate.invasionFleetIDs, AIstate.opponentPlanetIDs, and AIstate.invasionTargetedPlanetIDs
    MilitaryAI.getMilitaryFleets() # sets AIstate.militaryFleetIDs and AIstate.militaryTargetedSystemIDs

    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_RESOURCE_FOOD, calculateFoodPriority())
    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_RESOURCE_MINERALS, calculateMineralsPriority())
    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_RESOURCE_PRODUCTION, calculateIndustryPriority())
    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_RESOURCE_RESEARCH, calculateResearchPriority())
    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_RESOURCE_TRADE, 0)
    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_RESOURCE_CONSTRUCTION, 0)

    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_PRODUCTION_EXPLORATION, calculateExplorationPriority())
    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_PRODUCTION_OUTPOST, calculateOutpostPriority())
    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_PRODUCTION_COLONISATION, calculateColonisationPriority())
    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_PRODUCTION_INVASION, calculateInvasionPriority())
    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_PRODUCTION_MILITARY, calculateMilitaryPriority())
    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_PRODUCTION_BUILDINGS, 25)

    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_RESEARCH_LEARNING, calculateLearningPriority())
    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_RESEARCH_GROWTH, calculateGrowthPriority())
    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_RESEARCH_PRODUCTION, calculateTechsProductionPriority())
    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_RESEARCH_CONSTRUCTION, calculateConstructionPriority())
    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_RESEARCH_ECONOMICS, 0)
    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_RESEARCH_SHIPS, calculateShipsPriority())
    foAI.foAIstate.setPriority(AIPriorityType.PRIORITY_RESEARCH_DEFENSE, 0)

    # foAI.foAIstate.printPriorities()

def calculateFoodPriority():
    "calculates the AI's need for food"
    # attempts to sustain a food stockpile whose size depends on the AI empires population
    # foodStockpile == 0 => returns 100, foodStockpile == foodTarget => returns 0

    empire = fo.getEmpire()
    foodProduction = empire.resourceProduction(fo.resourceType.food)
    foodStockpile = empire.resourceStockpile(fo.resourceType.food)
    foodTarget = 10 * empire.population() * AIstate.foodStockpileSize

    if (foodTarget == 0):
        return 0

    foodPriority = (foodTarget - foodStockpile) / foodTarget * 115

    print ""
    print "Food Production:        " + str(foodProduction)
    print "Size of Food Stockpile: " + str(foodStockpile)
    print "Target Food Stockpile : " + str (foodTarget)
    print "Priority for Food     : " + str(foodPriority)

    if foodPriority < 0:
        return 0

    return foodPriority

def calculateMineralsPriority():
    "calculates the demand for minerals by industry"

    empire = fo.getEmpire()

    # get current minerals and industry production
    mineralsProduction = empire.resourceProduction(fo.resourceType.minerals)
    mineralsStockpile = empire.resourceStockpile(fo.resourceType.minerals)
    mineralsTurns = mineralsStockpile / (mineralsProduction + 0.001)
    industryProduction = empire.resourceProduction(fo.resourceType.industry)
    mineralsTarget = 10 * industryProduction

    mineralsPriority = (mineralsTarget - mineralsStockpile) / mineralsTarget * 99

    print ""
    print "Minerals Production:        " + str(mineralsProduction)
    print "Size of Minerals Stockpile: " + str(mineralsStockpile)
    print "Target Minerals Stockpile:  " + str(mineralsTarget)
    # print "Minerals Production Turns:  " + str(mineralsTurns)
    # print "industry production  : " + str(industryProduction)
    print "Priority for Minerals:      " + str(mineralsPriority)

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
    industryPriority = 38 * (mineralsProduction - mineralsTurns) / (industryProduction + 0.001)

    print ""
    # print "minerals production  : " + str(mineralsProduction)
    # print "minerals stockpile   : " + str(mineralsStockpile)
    # print "minerals turns       : " + str(mineralsTurns)
    print "Industry Production  : " + str(industryProduction)
    print "Priority for Industry: " + str(industryPriority)

    return industryPriority

def calculateResearchPriority():
    "calculates the AI empire's demand for research"

    empire = fo.getEmpire()
    totalPP = empire.productionPoints
    totalRP = empire.resourceProduction(fo.resourceType.research)

    # increase demand for research if significantly lagging production capability
    researchPriority = 10 * totalPP / (totalRP + 1)

    print  ""
    print  "Research Production  : " + str(totalRP)
    print  "Priority for Research: " + str(researchPriority)

    return researchPriority

def calculateExplorationPriority():
    "calculates the demand for scouts by unexplored systems"

    numUnexploredSystems = len(foAI.foAIstate.getExplorableSystems(AIExplorableSystemType.EXPLORABLE_SYSTEM_UNEXPLORED))

    scoutIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_EXPLORATION)
    numScouts = len(scoutIDs)

    if (numUnexploredSystems == 0) or (numScouts >= 2): return 0

    explorationPriority = 95 * (numUnexploredSystems - numScouts) / numUnexploredSystems

    print ""
    print "Number of Scouts            : " + str(numScouts)
    print "Number of Unexplored systems: " + str(numUnexploredSystems)
    print "Priority for scouts         : " + str(explorationPriority)

    if explorationPriority < 0: return 0

    return explorationPriority

def calculateColonisationPriority():
    "calculates the demand for colony ships by colonisable planets"

    numColonisablePlanetIDs = len(AIstate.colonisablePlanetIDs)
    if (numColonisablePlanetIDs == 0): return 0

    colonyshipIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_COLONISATION)
    numColonyships = len(FleetUtilsAI.extractFleetIDsWithoutMissionTypes(colonyshipIDs))
    colonisationPriority = 101 * (numColonisablePlanetIDs - numColonyships) / numColonisablePlanetIDs

    # print ""
    # print "Number of Colony Ships        : " + str(numColonyships)
    # print "Number of Colonisable planets : " + str(numColonisablePlanetIDs)
    # print "Priority for colony ships     : " + str(colonisationPriority)

    if colonisationPriority < 0: return 0

    return colonisationPriority

def calculateOutpostPriority():
    "calculates the demand for outpost ships by colonisable planets"

    numOutpostPlanetIDs = len(AIstate.colonisableOutpostIDs)
    completedTechs = getCompletedTechs()
    if numOutpostPlanetIDs == 0 or not 'GRO_ENV_ENCAPSUL' in completedTechs:
        return 0

    outpostShipIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_OUTPOST)
    numOutpostShips = len(FleetUtilsAI.extractFleetIDsWithoutMissionTypes(outpostShipIDs))
    outpostPriority = 102 * (numOutpostPlanetIDs - numOutpostShips) / numOutpostPlanetIDs

    # print ""
    # print "Number of Outpost Ships       : " + str(numOutpostShips)
    # print "Number of Colonisable outposts: " + str(numOutpostPlanetIDs)
    # print "Priority for outpost ships    : " + str(outpostPriority)

    if outpostPriority < 0: return 0

    return outpostPriority

def calculateInvasionPriority():
    "calculates the demand for troop ships by opponent planets"

    numOpponentPlanetIDs = len(AIstate.opponentPlanetIDs)
    if numOpponentPlanetIDs == 0: return 0

    troopShipIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_INVASION)
    numTroopShips = len(FleetUtilsAI.extractFleetIDsWithoutMissionTypes(troopShipIDs))
    invasionPriority = 110 * (numOpponentPlanetIDs - numTroopShips) / numOpponentPlanetIDs

    # print ""
    # print "Number of Troop Ships Without Missions: " + str(numTroopShips)
    # print "Number of Opponent Planets:             " + str(numOpponentPlanetIDs)
    # print "Priority for Troop Ships  :             " + str(invasionPriority)

    if invasionPriority < 0: return 0

    return invasionPriority

def calculateMilitaryPriority():
    "calculates the demand for military ships by military targeted systems"

    numMilitaryTargetedSystemIDs = len(AIstate.militaryTargetedSystemIDs)
    militaryShipIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_MILITARY)
    numMilitaryShips = len(FleetUtilsAI.extractFleetIDsWithoutMissionTypes(militaryShipIDs))

    # build one more military ship than military targeted systems
    militaryPriority = 100 * ((numMilitaryTargetedSystemIDs +2) - numMilitaryShips) / (numMilitaryTargetedSystemIDs + 1)

    # print ""
    # print "Number of Military Ships Without Missions: " + str(numMilitaryShips)
    # print "Number of Military Targeted Systems: " + str(numMilitaryTargetedSystemIDs)
    # print "Priority for Military Ships: " + str(militaryPriority)

    if militaryPriority < 0: return 0

    return militaryPriority

def calculateTopProductionQueuePriority():
    "calculates the top production queue priority"

    productionQueuePriorities = {}
    for priorityType in getAIPriorityProductionTypes():
        productionQueuePriorities[priorityType] = foAI.foAIstate.getPriority(priorityType)

    sortedPriorities = productionQueuePriorities.items()
    sortedPriorities.sort(lambda x,y: cmp(x[1], y[1]), reverse=True)
    topProductionQueuePriority = -1
    for evaluationPair in sortedPriorities:
        if topProductionQueuePriority < 0:
            topProductionQueuePriority = evaluationPair[0]

    return topProductionQueuePriority

def calculateLearningPriority():
    "calculates the demand for techs learning category"

    currentturn = fo.currentTurn()
    if currentturn == 1:
        return 100
    elif currentturn > 1:
        return 0

def calculateGrowthPriority():
    "calculates the demand for techs growth category"

    productionPriority = calculateTopProductionQueuePriority()
    if productionPriority == 8:
        return 70
    elif productionPriority != 8:
        return 0

def calculateTechsProductionPriority():
    "calculates the demand for techs production category"

    productionPriority = calculateTopProductionQueuePriority()
    if productionPriority == 7 or productionPriority == 9:
        return 60
    elif productionPriority != 7 or productionPriority != 9:
        return 0

def calculateConstructionPriority():
    "calculates the demand for techs construction category"

    productionPriority = calculateTopProductionQueuePriority()
    if productionPriority == 6 or productionPriority == 11:
        return 80
    elif productionPriority != 6 or productionPriority != 11:
        return 30

def calculateShipsPriority():
    "calculates the demand for techs ships category"

    productionPriority = calculateTopProductionQueuePriority()
    if productionPriority == 10:
        return 90
    elif productionPriority != 10:
        return 0
