import freeOrionAIInterface as fo
import FreeOrionAI as foAI
import AIstate
from EnumsAI import AIFleetMissionType, AIExplorableSystemType, AITargetType
import FleetUtilsAI
import PlanetUtilsAI
import AITarget

def getInvasionFleets():
    "get invasion fleets"

    allInvasionFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_INVASION)
    AIstate.invasionFleetIDs = FleetUtilsAI.extractFleetIDsWithoutMissionTypes(allInvasionFleetIDs)

    # get supplyable planets
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID

    fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
    fleetSupplyablePlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(fleetSupplyableSystemIDs)

    # get competitor planets    
    allOwnedPlanetIDs = getAllOwnedPlanetIDs(universe.planetIDs)
    print "All Owned and Populated PlanetIDs: " + str(allOwnedPlanetIDs)

    empireOwnedPlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    print "Empire Owned PlanetIDs:            " + str(empireOwnedPlanetIDs)

    invasionTargetedPlanetIDs = getInvasionTargetedPlanetIDs(universe.planetIDs, AIFleetMissionType.FLEET_MISSION_INVASION, empireID)
    print "Invasion Targeted PlanetIDs:       " + str(invasionTargetedPlanetIDs)

    competitorPlanetIDs = list(set(allOwnedPlanetIDs) - set(empireOwnedPlanetIDs))
    print "Competitor PlanetIDs:              " + str(competitorPlanetIDs)

    planetIDs = list(set(competitorPlanetIDs) - set(invasionTargetedPlanetIDs))
    print "Evaluated PlanetIDs:               " + str(planetIDs)

    evaluatedPlanets = assignInvasionValues(planetIDs, AIFleetMissionType.FLEET_MISSION_INVASION, fleetSupplyablePlanetIDs, empire)

    sortedPlanets = evaluatedPlanets.items()
    sortedPlanets.sort(lambda x, y: cmp(x[1], y[1]), reverse=True)

    print ""
    print "Invadable planetIDs:"
    for evaluationPair in sortedPlanets:
        print "    ID|Score: " + str(evaluationPair)

    # export opponent planets for other AI modules
    AIstate.opponentPlanetIDs = sortedPlanets

def getInvasionTargetedPlanetIDs(planetIDs, missionType, empireID):
    "return list of Empire owned or being invaded planets"

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    invasionAIFleetMissions = foAI.foAIstate.getAIFleetMissionsWithAnyMissionTypes([missionType])

    targetedPlanets = []

    for planetID in planetIDs:
        planet = universe.getPlanet(planetID)
        # add planets that are target of a mission
        for invasionAIFleetMission in invasionAIFleetMissions:
            aiTarget = AITarget.AITarget(AITargetType.TARGET_PLANET, planetID)
            if invasionAIFleetMission.hasTarget(missionType, aiTarget):
                targetedPlanets.append(planetID)

        return targetedPlanets

def getAllOwnedPlanetIDs(planetIDs):
    "return list of owned planetIDs"

    universe = fo.getUniverse()
    allOwnedPlanetIDs = []
    for planetID in planetIDs:
        planet = universe.getPlanet(planetID)
        planetPopulation = planet.currentMeterValue(fo.meterType.population)
        if not planet.unowned or planetPopulation > 0:
            allOwnedPlanetIDs.append(planetID)

    return allOwnedPlanetIDs

def assignInvasionValues(planetIDs, missionType, fleetSupplyablePlanetIDs, empire):
    "creates a dictionary that takes planetIDs as key and their invasion score as value"

    planetValues = {}

    for planetID in planetIDs:
        planetValues[planetID] = evaluatePlanet(planetID, missionType, fleetSupplyablePlanetIDs, empire)

    return planetValues

def evaluatePlanet(planetID, missionType, fleetSupplyablePlanetIDs, empire):
    "return the invasion value of a planet"

    universe = fo.getUniverse()
    planet = universe.getPlanet(planetID)
    if (planet == None): return 0

    # give preference to closest invadable planets
    empireID = empire.empireID
    capitalID = empire.capitalID
    homeworld = universe.getPlanet(capitalID)
    homeSystemID = homeworld.systemID
    evalSystemID = planet.systemID
    leastJumpsPath = len(universe.leastJumpsPath(homeSystemID, evalSystemID, empireID))
    distanceFactor = 1.001/(leastJumpsPath + 1)

    if planetID in fleetSupplyablePlanetIDs:
        return getPlanetPopulation(planetID) * planet.size * planet.type * distanceFactor
    else:
        return getPlanetPopulation(planetID) * planet.size * planet.type * .25 * distanceFactor

def getPlanetPopulation(planetID):
    "return planet population"

    universe = fo.getUniverse()

    planet = universe.getPlanet(planetID)
    planetPopulation = planet.currentMeterValue(fo.meterType.population)
 
    if planet == None: return 0
    else:
        return planetPopulation

def sendInvasionFleets(invasionFleetIDs, evaluatedPlanets, missionType):
    "sends a list of invasion fleets to a list of planet_value_pairs"

    i = 0

    for planetID_value_pair in evaluatedPlanets:
        if i >= len(invasionFleetIDs): return

        fleetID = invasionFleetIDs[i]
        planetID = planetID_value_pair[0]

        aiTarget = AITarget.AITarget(AITargetType.TARGET_PLANET, planetID)
        aiFleetMission = foAI.foAIstate.getAIFleetMission(fleetID)
        aiFleetMission.addAITarget(missionType, aiTarget)

        i = i + 1

def assignInvasionFleetsToInvade():
    # assign fleet targets to invadable planets

    sendInvasionFleets(AIstate.invasionFleetIDs, AIstate.opponentPlanetIDs, AIFleetMissionType.FLEET_MISSION_INVASION)

