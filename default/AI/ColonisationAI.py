import freeOrionAIInterface as fo
import FreeOrionAI as foAI
import AIstate
import FleetUtilsAI
from EnumsAI import AIFleetMissionType, AIExplorableSystemType, AITargetType
import AITarget
import PlanetUtilsAI

# globals
colonisablePlanetIDs = []  # TODO: move into AIstate

def assignColonyFleetsToColonise():
    # get colony fleets
    allColonyFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_COLONISATION)
    colonyFleetIDs = FleetUtilsAI.extractFleetIDsWithoutMissionTypes(allColonyFleetIDs)

    # get planets
    systemIDs = foAI.foAIstate.getExplorableSystems(AIExplorableSystemType.EXPLORABLE_SYSTEM_EXPLORED)
    planetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(systemIDs)

    removeAlreadyOwnedPlanetIDs(planetIDs)

    evaluatedPlanets = assignColonisationValues(planetIDs)
    removeLowValuePlanets(evaluatedPlanets)

    sortedPlanets = evaluatedPlanets.items()
    sortedPlanets.sort(lambda x, y: cmp(x[1], y[1]), reverse=True)

    print "Colonisable planets:"
    for evaluationPair in sortedPlanets:
        print "    ID|Score: " + str(evaluationPair)
    print ""

    # export planets for other AI modules
    global colonisablePlanetIDs
    colonisablePlanetIDs = sortedPlanets   # !!! move into AIstate?

    # assign fleet targets to colonisable planets
    sendColonyShips(colonyFleetIDs, sortedPlanets)

def removeAlreadyOwnedPlanetIDs(planetIDs):
    "removes planets that already are being colonised or owned"

    universe = fo.getUniverse()

    coloniseAIFleetMissions = foAI.foAIstate.getAIFleetMissionsWithAnyMissionTypes([AIFleetMissionType.FLEET_MISSION_COLONISATION])
    deletePlanets = []

    for planetID in planetIDs:

        planet = universe.getPlanet(planetID)
        # remove owned planets
        if (not planet.unowned):
            deletePlanets.append(planetID)
            continue

        # remove planets that are target of a mission
        for coloniseAIFleetMission in coloniseAIFleetMissions:
            aiTarget = AITarget.AITarget(AITargetType.TARGET_PLANET, planetID)
            if coloniseAIFleetMission.hasTarget(AIFleetMissionType.FLEET_MISSION_COLONISATION, aiTarget):
                deletePlanets.append(planetID)

    for ID in deletePlanets:
        planetIDs.remove(ID)
        # print "removed planet " + str(ID)

def assignColonisationValues(planetIDs):
    "creates a dictionary that takes planetIDs as key and their colonisation score as value"

    planetValues = {}

    for planetID in planetIDs:
        planetValues[planetID] = evaluatePlanet(planetID)

    return planetValues

def evaluatePlanet(planetID):
    "returns the colonisation value of a planet"
    # TODO: in planet evaluation consider specials and distance

    universe = fo.getUniverse()

    planet = universe.getPlanet(planetID)
    if (planet == None): return 0

    return getPlanetHospitality(planetID) * planet.size
    # planet size ranges from 1-5

def getPlanetHospitality(planetID):
    "returns a value depending on the planet type"

    universe = fo.getUniverse()

    planet = universe.getPlanet(planetID)
    if planet == None: return 0

    empire = fo.getEmpire()
    capitolID = empire.capitolID
    homeworld = universe.getPlanet(capitolID)
    speciesName = homeworld.speciesName
    species = fo.getSpecies(speciesName)
    planetEnvironment = species.getPlanetEnvironment(planet.type)
    # print ">> planet:" + str(planetID) + " type:" + str(planet.type) + " planetEnvironment:" + str(planetEnvironment)

    # should be reworked with races
    # if planet.type == fo.planetType.terran: return 2
    # if planet.type == fo.planetType.ocean: return 1
    # if planet.type == fo.planetType.desert: return 1
    # if planet.type == fo.planetType.tundra: return 0.5
    # if planet.type == fo.planetType.swamp: return 0.5
    # reworked with races
    if planetEnvironment == fo.planetEnvironment.good: return 2
    if planetEnvironment == fo.planetEnvironment.adequate: return 1
    if planetEnvironment == fo.planetEnvironment.poor: return 1
    if planetEnvironment == fo.planetEnvironment.hostile: return 0.5
    if planetEnvironment == fo.planetEnvironment.uninhabitable: return 0.5

    return 0

def removeLowValuePlanets(evaluatedPlanets):
    "removes all planets with a colonisation value < minimalColoniseValue"

    removeIDs = []

    # print ">> min:" + str(AIstate.minimalColoniseValue)
    for planetID in evaluatedPlanets.iterkeys():
	# print ">> eval:" + str(planetID) + " val:" + str(evaluatedPlanets[planetID])
        if (evaluatedPlanets[planetID] < AIstate.minimalColoniseValue):
            removeIDs.append(planetID)

    for ID in removeIDs: del evaluatedPlanets[ID]

def sendColonyShips(colonyFleetIDs, evaluatedPlanets):
    "sends a list of colony ships to a list of planet_value_pairs"

    i = 0

    for planetID_value_pair in evaluatedPlanets:
        if i >= len(colonyFleetIDs): return

        fleetID = colonyFleetIDs[i]
        planetID = planetID_value_pair[0]

        aiTarget = AITarget.AITarget(AITargetType.TARGET_PLANET, planetID)
        aiFleetMission = foAI.foAIstate.getAIFleetMission(fleetID)
        aiFleetMission.addAITarget(AIFleetMissionType.FLEET_MISSION_COLONISATION, aiTarget)

        i = i + 1
