import freeOrionAIInterface as fo
import FreeOrionAI as foAI
import AIstate
import FleetUtils
import ExplorationAI    # move into AIstate
import ColonisationAI

priorityResources =  ["PR_FOOD", "PR_MINERALS", "PR_PRODUCTION", "PR_RESEARCH", "PR_TRADE"]
priorityProduction = ["PR_EXPLORATION", "PR_COLONISATION", "PR_MILITARY", "PR_BUILDINGS"]
priorityResearch =   ["PR_LEARNING", "PR_GROWTH", "PR_PRODUCTION", "PR_CONSTRUCTION", "PR_ECONOMICS", "PR_SHIPS"]
priorityTypes =      ["PR_NONE"] + priorityResources + priorityProduction + priorityResearch



def calculatePriorities():
    "calculates the priorities of the AI player"

    empire = fo.getEmpire()

    foAI.foAIstate.setPriority("PR_EXPLORATION", calculateExplorationPriority())
    foAI.foAIstate.setPriority("PR_COLONISATION", calculateColonisationPriority())
    foAI.foAIstate.setPriority("PR_BUILDINGS", 20)
    foAI.foAIstate.setPriority("PR_MILITARY", 20)

    foAI.foAIstate.setPriority("PR_FOOD", calculateFoodPriority())
    foAI.foAIstate.setPriority("PR_PRODUCTION", calculateProductionPriority()) # uses all prProds
    foAI.foAIstate.setPriority("PR_MINERALS", calculateMineralsPriority()) # uses PR_PRODUCTION
    foAI.foAIstate.setPriority("PR_RESEARCH", 10)

    foAI.foAIstate.printPriorities()


# attempts to sustain a food stockpile whose size depends on the AI empires population
# foodStockpile == 0 => returns 100, foodStockpile == foodTarget => returns 0
def calculateFoodPriority():
    "calculates the AI's need for food"

    empire = fo.getEmpire()

    foodStockpile = empire.resourceStockpile(fo.resourceType.food)
    foodTarget = empire.population() * AIstate.foodStockpileSize

    if (foodTarget == 0): return 0
    foodPriority = (foodTarget - foodStockpile) / foodTarget * 100

    print ""
    print "Size of food stockpile:  " + str(foodStockpile)
    print "Target food stockpile:   " + str (foodTarget)
    print "Priority for Food:         " + str(foodPriority)

    if foodPriority < 0: return 0

    return foodPriority


# looks if there are enough scouts to explore unknown systems
def calculateExplorationPriority():
    "calculates the demand for scouts"

    empireID = fo.empireID()

    # TODO: Fog of war
    numExplorableSystemIDs = len(foAI.foAIstate.getExplorableSystems("EST_UNEXPLORED"))

    scoutIDs = FleetUtils.getEmpireFleetIDsByRole("MT_EXPLORATION")
    numScouts = len(scoutIDs)

    if (numExplorableSystemIDs == 0): return 0

    explorationPriority = (numExplorableSystemIDs - numScouts) / numExplorableSystemIDs * 100

    print ""
    print "Number of Scouts: " + str(numScouts)
    print "Number of Systems: " + str(numExplorableSystemIDs)
    print "Priority for scouts: " + str(explorationPriority)

    if explorationPriority < 0: return 0

    return explorationPriority


# looks if there is enough Colony Ships for colonisable planets
def calculateColonisationPriority():
    "calculates the demand for colony ships"

    empireID = fo.empireID()

    numColonisablePlanetIDs = len(ColonisationAI.colonisablePlanetIDs)

    colonyshipIDs = FleetUtils.getEmpireFleetIDsByRole("MT_COLONISATION")
    numColonyships = len(FleetUtils.extractFleetIDsWithoutMission(colonyshipIDs))

    if (numColonisablePlanetIDs == 0): return 0

    colonisationPriority = (numColonisablePlanetIDs - numColonyships) / numColonisablePlanetIDs * 100

    print ""
    print "Number of Colony Ships: " + str(numColonyships)
    print "Number of Planets: " + str(numColonisablePlanetIDs)
    print "Priority for colony ships: " + str(colonisationPriority)

    if colonisationPriority < 0: return 0

    return colonisationPriority


# minerals must match production
def calculateMineralsPriority():
    "calculates the demand for minerals"

    empire = fo.getEmpire()

    # get current minerals and industry production
    mineralsProduction = empire.resourceProduction(fo.resourceType.minerals)
    industryProduction = empire.resourceProduction(fo.resourceType.industry)

    # match minerals to industry
    mineralsPriority = foAI.foAIstate.getPriority("PR_PRODUCTION")

    if (mineralsProduction == 0): return 0

    # increase demand for minerals if productionPoints are higher
    if (mineralsProduction < industryProduction): mineralsPriority = mineralsPriority * (industryProduction / mineralsProduction)

    print ""
    print "minerals prod: " + str(mineralsProduction)
    print "industry prod: " + str(industryProduction)
    print "Priority for Minerals: " + str(mineralsPriority)

    return mineralsPriority


def calculateProductionPriority():
    "calculates the demand for production"

    priorityValues = []

    for priority in priorityProduction:
        priorityValues.append(foAI.foAIstate.getPriority(priority))

    productionPriority = max(priorityValues)

    return productionPriority

