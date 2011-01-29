import freeOrionAIInterface as fo
import FreeOrionAI as foAI
import AIstate
import FleetUtilsAI
from EnumsAI import AIFleetMissionType, AIExplorableSystemType, AITargetType
import AITarget
import PlanetUtilsAI

def getColonyFleets():
    # get colony fleets
    allColonyFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_COLONISATION)
    AIstate.colonyFleetIDs = FleetUtilsAI.extractFleetIDsWithoutMissionTypes(allColonyFleetIDs)

    # get supplyable systems
    empire = fo.getEmpire()
    universe = fo.getUniverse()
    capitolID = empire.capitolID
    homeworld = universe.getPlanet(capitolID)
    speciesName = homeworld.speciesName
    species = fo.getSpecies(speciesName)

    fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
    fleetSupplyablePlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(fleetSupplyableSystemIDs)
    print "    fleetSupplyablePlanetIDs:" + str(fleetSupplyablePlanetIDs)

    # get planets
    systemIDs = foAI.foAIstate.getExplorableSystems(AIExplorableSystemType.EXPLORABLE_SYSTEM_EXPLORED)
    planetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(systemIDs)

    removeAlreadyOwnedPlanetIDs(planetIDs, AIFleetMissionType.FLEET_MISSION_COLONISATION)
    removeAlreadyOwnedPlanetIDs(planetIDs, AIFleetMissionType.FLEET_MISSION_OUTPOST)

    evaluatedPlanets = assignColonisationValues(planetIDs, AIFleetMissionType.FLEET_MISSION_COLONISATION, fleetSupplyablePlanetIDs, species, empire)
    removeLowValuePlanets(evaluatedPlanets)

    sortedPlanets = evaluatedPlanets.items()
    sortedPlanets.sort(lambda x, y: cmp(x[1], y[1]), reverse=True)

    print "Colonisable planets:"
    for evaluationPair in sortedPlanets:
        print "    ID|Score: " + str(evaluationPair)
    print ""

    # export planets for other AI modules
    AIstate.colonisablePlanetIDs = sortedPlanets

    # get outpost fleets
    allOutpostFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_OUTPOST)
    AIstate.outpostFleetIDs = FleetUtilsAI.extractFleetIDsWithoutMissionTypes(allOutpostFleetIDs)

    evaluatedOutposts = assignColonisationValues(planetIDs, AIFleetMissionType.FLEET_MISSION_OUTPOST, fleetSupplyablePlanetIDs, species, empire)
    removeLowValuePlanets(evaluatedOutposts)

    sortedOutposts = evaluatedOutposts.items()
    sortedOutposts.sort(lambda x, y: cmp(x[1], y[1]), reverse=True)

    print "Colonisable outposts:"
    for evaluationPair in sortedOutposts:
        print "    ID|Score: " + str(evaluationPair)
    print ""

    # export outposts for other AI modules
    AIstate.colonisableOutpostIDs = sortedOutposts

def assignColonyFleetsToColonise():
    # assign fleet targets to colonisable planets
    sendColonyShips(AIstate.colonyFleetIDs, AIstate.colonisablePlanetIDs, AIFleetMissionType.FLEET_MISSION_COLONISATION)

    # assign fleet targets to colonisable outposts
    sendColonyShips(AIstate.outpostFleetIDs, AIstate.colonisableOutpostIDs, AIFleetMissionType.FLEET_MISSION_OUTPOST)

def removeAlreadyOwnedPlanetIDs(planetIDs, missionType):
    "removes planets that already are being colonised or owned"

    universe = fo.getUniverse()

    coloniseAIFleetMissions = foAI.foAIstate.getAIFleetMissionsWithAnyMissionTypes([missionType])
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
            if coloniseAIFleetMission.hasTarget(missionType, aiTarget):
                deletePlanets.append(planetID)

    for ID in deletePlanets:
        planetIDs.remove(ID)
        # print "removed planet " + str(ID)


def assignColonisationValues(planetIDs, missionType, fleetSupplyablePlanetIDs, species, empire):
    "creates a dictionary that takes planetIDs as key and their colonisation score as value"

    planetValues = {}

    for planetID in planetIDs:
        planetValues[planetID] = evaluatePlanet(planetID, missionType, fleetSupplyablePlanetIDs, species, empire)

    return planetValues

def evaluatePlanet(planetID, missionType, fleetSupplyablePlanetIDs, species, empire):
    "returns the colonisation value of a planet"
    # TODO: in planet evaluation consider specials and distance

    universe = fo.getUniverse()

    planet = universe.getPlanet(planetID)
    if (planet == None): return 0

    # give preference to closest worlds
    empireID = empire.empireID
    capitolID = empire.capitolID
    homeworld = universe.getPlanet(capitolID)
    homeSystemID = homeworld.systemID
    evalSystemID = planet.systemID
    leastJumpsPath = len(universe.leastJumpsPath(homeSystemID, evalSystemID, empireID))
    distanceFactor = 1.001 / (leastJumpsPath + 1)

    # print ">>> evaluatePlanet ID:" + str(planetID) + "/" + str(planet.type) + "/" + str(planet.size) + "/" + str(leastJumpsPath) + "/" + str(distanceFactor)
    if missionType == AIFleetMissionType.FLEET_MISSION_COLONISATION:
        # planet size ranges from 1-5
	if (planetID in fleetSupplyablePlanetIDs):
	    return getPlanetHospitality(planetID, species) * planet.size + distanceFactor
	else:
	    return getPlanetHospitality(planetID, species) * planet.size - distanceFactor
    elif missionType == AIFleetMissionType.FLEET_MISSION_OUTPOST:
	planetEnvironment = species.getPlanetEnvironment(planet.type)
        if planetEnvironment == fo.planetEnvironment.uninhabitable:
	    # prevent outposts from being built when they cannot get food
	    if (planetID in fleetSupplyablePlanetIDs):
		return AIstate.minimalColoniseValue + distanceFactor
	    elif (str("GRO_ORBIT_FARMING") in empire.availableTechs):
		return AIstate.minimalColoniseValue + distanceFactor
	    else:
		return AIstate.minimalColoniseValue - distanceFactor

def getPlanetHospitality(planetID, species):
    "returns a value depending on the planet type"

    universe = fo.getUniverse()

    planet = universe.getPlanet(planetID)
    if planet == None: return 0

    planetEnvironment = species.getPlanetEnvironment(planet.type)
    # print ":: planet:" + str(planetID) + " type:" + str(planet.type) + " size:" + str(planet.size) + " env:" + str(planetEnvironment)

    # reworked with races
    if planetEnvironment == fo.planetEnvironment.good: return 2.75
    if planetEnvironment == fo.planetEnvironment.adequate: return 1
    if planetEnvironment == fo.planetEnvironment.poor: return 0.5
    if planetEnvironment == fo.planetEnvironment.hostile: return 0.25
    if planetEnvironment == fo.planetEnvironment.uninhabitable: return 0.1

    return 0

def removeLowValuePlanets(evaluatedPlanets):
    "removes all planets with a colonisation value < minimalColoniseValue"

    removeIDs = []

    # print ":: min:" + str(AIstate.minimalColoniseValue)
    for planetID in evaluatedPlanets.iterkeys():
	# print ":: eval:" + str(planetID) + " val:" + str(evaluatedPlanets[planetID])
        if (evaluatedPlanets[planetID] < AIstate.minimalColoniseValue):
            removeIDs.append(planetID)

    for ID in removeIDs: del evaluatedPlanets[ID]

def sendColonyShips(colonyFleetIDs, evaluatedPlanets, missionType):
    "sends a list of colony ships to a list of planet_value_pairs"

    i = 0

    for planetID_value_pair in evaluatedPlanets:
        if i >= len(colonyFleetIDs): return

        fleetID = colonyFleetIDs[i]
        planetID = planetID_value_pair[0]

        aiTarget = AITarget.AITarget(AITargetType.TARGET_PLANET, planetID)
        aiFleetMission = foAI.foAIstate.getAIFleetMission(fleetID)
        aiFleetMission.addAITarget(missionType, aiTarget)

        i = i + 1
