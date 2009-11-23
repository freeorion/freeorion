import freeOrionAIInterface as fo   # interface used to interact with FreeOrion AI client
import FreeOrionAI as foAI
import FleetUtilsAI
from EnumsAI import AIFleetMissionType, AIExplorableSystemType, AITargetType
import AITarget
import MoveUtilsAI

def assignScoutsToExploreSystems():
    # TODO: use Graph Theory to explore closest systems

    fleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_EXPLORATION)

    # order fleets to explore
    explorableSystemIDs = foAI.foAIstate.getExplorableSystems(AIExplorableSystemType.EXPLORABLE_SYSTEM_UNEXPLORED)
    for fleetID in fleetIDs:
        # if fleet already has a mission, continue
        aiFleetMission = foAI.foAIstate.getAIFleetMission(fleetID)
        if len(aiFleetMission.getAIMissionTypes()) > 0:
            continue

        # else send fleet to a system
        for systemID in explorableSystemIDs:
            # if system is already being explored, continue
            aiTarget = AITarget.AITarget(AITargetType.TARGET_SYSTEM, systemID)
            if foAI.foAIstate.hasAITarget(AIFleetMissionType.FLEET_MISSION_EXPLORATION, aiTarget):
                continue

            # add exploration mission to fleet with target unexplored system and this system is in range
            if len(MoveUtilsAI.canTravelToSystemAndReturnToResupply(fleetID, aiFleetMission.getLocationAITarget(), aiTarget, fo.empireID())) > 0:
                aiFleetMission.addAITarget(AIFleetMissionType.FLEET_MISSION_EXPLORATION, aiTarget)
                break

def getHomeSystemID():
    "returns the systemID of the home world"

    empire = fo.getEmpire()
    universe = fo.getUniverse()
    homeworld = universe.getPlanet(empire.homeworldID)

    if homeworld:
        return homeworld.systemID

    return -1
