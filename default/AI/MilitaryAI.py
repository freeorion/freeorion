import freeOrionAIInterface as fo
import FreeOrionAI as foAI
import AIstate
import AITarget
from EnumsAI import AIFleetMissionType, AITargetType
import FleetUtilsAI
import PlanetUtilsAI

def getMilitaryFleets():
    "get armed military fleets"

    allMilitaryFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_MILITARY)
    AIstate.militaryFleetIDs = FleetUtilsAI.extractFleetIDsWithoutMissionTypes(allMilitaryFleetIDs)

    # get systems to defend
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    capitalID = empire.capitalID
    capitalPlanet = universe.getPlanet(capitalID)
    capitalPlanetSystem = capitalPlanet.systemID

    capitalSystemID = []
    capitalSystemID.append(capitalPlanetSystem)

    fleetSupplyableSystemIDs = list(empire.fleetSupplyableSystemIDs)

    exploredSystemIDs = empire.exploredSystemIDs
    # print "Explored SystemIDs: " + str(list(exploredSystemIDs))

    exploredPlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(exploredSystemIDs)
    allOwnedPlanetIDs = PlanetUtilsAI.getAllOwnedPlanetIDs(exploredPlanetIDs)
    allPopulatedSystemIDs = PlanetUtilsAI.getAllPopulatedSystemIDs(allOwnedPlanetIDs)
    print ""
    print "All Populated SystemIDs:             " + str(list(set(allPopulatedSystemIDs)))

    empirePlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    empireOccupiedSystemIDs = list(set(PlanetUtilsAI.getSystemsOccupiedByEmpire(empirePlanetIDs, empireID)))
    print ""
    print "Empire Capital SystemID:             " + str(capitalSystemID)
    # print "Empire Occupied SystemIDs:    " + str(empireOccupiedSystemIDs)

    empireProvinceSystemIDs = list(set(empireOccupiedSystemIDs) - set(capitalSystemID))
    print "Empire Province SystemIDs:           " + str(empireProvinceSystemIDs)

    competitorSystemIDs = list(set(allPopulatedSystemIDs) - set(empireOccupiedSystemIDs))
    print "Competitor SystemIDs:                " + str(competitorSystemIDs)

    otherTargetedSystemIDs = list(set(AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs + AIstate.invasionTargetedSystemIDs))
    print "Other Targeted SystemIDs:            " + str(otherTargetedSystemIDs)

    militaryTheaterSystemIDs = list(set(fleetSupplyableSystemIDs + empireOccupiedSystemIDs + competitorSystemIDs + otherTargetedSystemIDs))
    print "Military Theater SystemIDs:          " + str(militaryTheaterSystemIDs)

    allMilitaryTargetedSystemIDs = getMilitaryTargetedSystemIDs(universe.systemIDs, AIFleetMissionType.FLEET_MISSION_MILITARY, empireID)
    # export military targeted systems for other AI modules
    AIstate.militaryTargetedSystemIDs = allMilitaryTargetedSystemIDs
    print ""
    print "Military Targeted SystemIDs:         " + str(allMilitaryTargetedSystemIDs)

    militaryFleetIDs = allMilitaryFleetIDs
    if not militaryFleetIDs:
        print "Available Military Fleets:             0"
    else:
        print "Military FleetIDs:                   " + str(allMilitaryFleetIDs)

    numMilitaryFleets = len(FleetUtilsAI.extractFleetIDsWithoutMissionTypes(militaryFleetIDs))
    print "Military Fleets Without Missions:      " + str(numMilitaryFleets)

    evaluatedSystemIDs = list(set(militaryTheaterSystemIDs) - set(allMilitaryTargetedSystemIDs))
    # print "Evaluated SystemIDs:               " +str(evaluatedSystemIDs)

    evaluatedSystems = assignMilitaryValues(evaluatedSystemIDs, AIFleetMissionType.FLEET_MISSION_MILITARY, empireProvinceSystemIDs, otherTargetedSystemIDs, empire)
 
    sortedSystems = evaluatedSystems.items()
    sortedSystems.sort(lambda x, y: cmp(x[1], y[1]), reverse=True)

    print ""
    print "Military SystemIDs:"
    for evaluationPair in sortedSystems:
        print "    ID|Score: " + str(evaluationPair)

    # export military systems for other AI modules
    AIstate.militarySystemIDs = sortedSystems

def getMilitaryTargetedSystemIDs(systemIDs, missionType, empireID):
    "return list of military targeted systems"

    universe = fo.getUniverse()
    militaryAIFleetMissions = foAI.foAIstate.getAIFleetMissionsWithAnyMissionTypes([missionType])

    targetedSystems = []

    for systemID in systemIDs:
        system = universe.getSystem(systemID)
        # add systems that are target of a mission
        for militaryAIFleetMission in militaryAIFleetMissions:
            aiTarget = AITarget.AITarget(AITargetType.TARGET_SYSTEM, systemID)
            if militaryAIFleetMission.hasTarget(missionType, aiTarget):
                targetedSystems.append(systemID)

    return targetedSystems

def assignMilitaryValues(systemIDs, missionType, empireProvinceSystemIDs, otherTargetedSystemIDs, empire):
    "creates a dictionary that takes systemIDs as key and their military score as value"

    systemValues = {}

    for systemID in systemIDs:
        systemValues[systemID] = evaluateSystem(systemID, missionType, empireProvinceSystemIDs, otherTargetedSystemIDs, empire)

    return systemValues

def evaluateSystem(systemID, missionType, empireProvinceSystemIDs, otherTargetedSystemIDs, empire):
    "return the military value of a system"

    universe = fo.getUniverse()
    system = universe.getSystem(systemID)
    if (system == None): return 0

    # give preference to home system then closest systems
    empireID = empire.empireID
    capitalID = empire.capitalID
    homeworld = universe.getPlanet(capitalID)
    homeSystemID = homeworld.systemID
    evalSystemID = system.systemID
    leastJumpsPath = len(universe.leastJumpsPath(homeSystemID, evalSystemID, empireID))
    distanceFactor = 1.001/(leastJumpsPath + 1)

    if systemID == homeSystemID:
        return 10
    elif systemID in empireProvinceSystemIDs:
        return 3 + distanceFactor
    elif systemID in otherTargetedSystemIDs:
        return 2 + distanceFactor
    else:
        return 1 + .25 * distanceFactor

def sendMilitaryFleets(militaryFleetIDs, evaluatedSystems, missionType):
    "sends a list of military fleets to a list of system_value_pairs"

    i = 0

    for systemID_value_pair in evaluatedSystems: # evaluatedSystems is a dictionary
        if i >= len(militaryFleetIDs): return

        fleetID = militaryFleetIDs[i]
        systemID = systemID_value_pair[0]

        aiTarget = AITarget.AITarget(AITargetType.TARGET_SYSTEM, systemID)
        aiFleetMission = foAI.foAIstate.getAIFleetMission(fleetID)
        aiFleetMission.addAITarget(missionType, aiTarget)

        i = i + 1

def assignMilitaryFleetsToSystems():
    # assign military fleets to military theater systems

    sendMilitaryFleets(AIstate.militaryFleetIDs, AIstate.militarySystemIDs, AIFleetMissionType.FLEET_MISSION_MILITARY)
