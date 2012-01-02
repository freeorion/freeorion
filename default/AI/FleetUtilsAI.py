import freeOrionAIInterface as fo
import FreeOrionAI as foAI
import EnumsAI
from EnumsAI import AIFleetMissionType, AIShipRoleType, AIExplorableSystemType

def splitFleet(fleetID):
    "splits a fleet into its ships"

    universe = fo.getUniverse()
    empireID = fo.empireID()

    fleet = universe.getFleet(fleetID)

    if fleet == None: return
    if not fleet.ownedBy(empireID): return

    for shipID in fleet.shipIDs:
        if len(fleet.shipIDs) <= 1: break # fleet with only one ship cannot be split
        fo.issueNewFleetOrder(str(shipID), shipID)

def fleetHasShipWithRole(fleetID, shipRole):
    "returns True if a ship with shipRole is in the fleet"

    universe = fo.getUniverse()
    fleet = universe.getFleet(fleetID)

    if fleet == None: return False
    for shipID in fleet.shipIDs:
        ship = universe.getShip(shipID)
        if (foAI.foAIstate.getShipRole(ship.design.id) == shipRole):
            return True

    return False

def getShipIDWithRole(fleetID, shipRole):
    "returns a ship with the specified role in the fleet"

    if not fleetHasShipWithRole(fleetID, shipRole):
        print "No ship with role " + shipRole + " found."
        return None

    universe = fo.getUniverse()
    fleet = universe.getFleet(fleetID)

    for shipID in fleet.shipIDs:
        ship = universe.getShip(shipID)
        if (foAI.foAIstate.getShipRole(ship.design.id) == shipRole):
            return shipID

def getEmpireFleetIDs():
    "returns all fleetIDs of an empire"

    universe = fo.getUniverse()
    empireID = fo.empireID()

    fleetIDs = universe.fleetIDs
    empireFleetIDs = []

    for fleetID in fleetIDs:
        fleet = universe.getFleet(fleetID)

        if (fleet == None): continue
        if (not fleet.ownedBy(empireID)): continue

        empireFleetIDs = empireFleetIDs + [fleetID]

    return empireFleetIDs

def getEmpireFleetIDsByRole(fleetRole):
    "returns a list with fleetIDs that have the specified role"

    fleetIDs = getEmpireFleetIDs()
    fleetIDsWithRole = []

    for fleetID in fleetIDs:
        if not (foAI.foAIstate.getFleetRole(fleetID) == fleetRole): continue

        fleetIDsWithRole.append(fleetID)

    return fleetIDsWithRole

def extractFleetIDsWithoutMissionTypes(fleetIDs):
    "extracts a list with fleetIDs that have no mission"

    fleetIDsWithoutMission = []

    for fleetID in fleetIDs:
        aiFleetMission = foAI.foAIstate.getAIFleetMission(fleetID)
        if not aiFleetMission.hasAnyAIMissionTypes():
            fleetIDsWithoutMission.append(fleetID)

    return fleetIDsWithoutMission

def assessFleetRole(fleetID):
    "counts the number of ShipRoles in a fleet and returns a corresponding fleetRole"
    # TODO: one colony ship in fleet should mean it's a colony fleet

    universe = fo.getUniverse()

    shipRoles = {}
    for shipRole in EnumsAI.getAIShipRolesTypes():
        shipRoles[shipRole] = 0

    fleet = universe.getFleet(fleetID)
    if not fleet:
        print "couldn't get fleet with id " + str(fleetID)
        return AIShipRoleType.SHIP_ROLE_INVALID

    # count shipRoles
    for shipID in fleet.shipIDs:
        ship = universe.getShip(shipID)
        if ship.design:
            role = foAI.foAIstate.getShipRole(ship.design.id)
        else:
            role = AIShipRoleType.SHIP_ROLE_INVALID

        if role != AIShipRoleType.SHIP_ROLE_INVALID:
            shipRoles[role] = shipRoles[role] + 1

    # determine most common shipRole
    favouriteRole = AIShipRoleType.SHIP_ROLE_INVALID
    for shipRole in shipRoles:
        if shipRoles[shipRole] == max(shipRoles.values()):
            favouriteRole = shipRole

    # assign fleet role
    if favouriteRole == AIShipRoleType.SHIP_ROLE_CIVILIAN_EXPLORATION:
        return AIFleetMissionType.FLEET_MISSION_EXPLORATION
    if favouriteRole == AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION:
        return AIFleetMissionType.FLEET_MISSION_COLONISATION
    if favouriteRole == AIShipRoleType.SHIP_ROLE_CIVILIAN_OUTPOST:
        return AIFleetMissionType.FLEET_MISSION_OUTPOST
    if favouriteRole == AIShipRoleType.SHIP_ROLE_MILITARY_INVASION:
        return AIFleetMissionType.FLEET_MISSION_INVASION
    if favouriteRole == AIShipRoleType.SHIP_ROLE_MILITARY_ATTACK:
        return AIFleetMissionType.FLEET_MISSION_ATTACK
    if favouriteRole == AIShipRoleType.SHIP_ROLE_MILITARY:
        return AIFleetMissionType.FLEET_MISSION_MILITARY

    return AIShipRoleType.SHIP_ROLE_INVALID

def assessShipRole(shipID):
    "decides which role a ship has"
    # TODO: analyze parts

    universe = fo.getUniverse()
    ship = universe.getShip(shipID)

    if ship.canColonize:
        # look for the CO_OUTPOST_MOD
        if ship.design.parts.__contains__("CO_OUTPOST_MOD"):
            # print ">> assessShipRole Outpost Ship:" + str(shipID)
            return AIShipRoleType.SHIP_ROLE_CIVILIAN_OUTPOST
        if ship.design.parts.__contains__("CO_COLONY_POD"):
            # print ">> assessShipRole Colony Ship:" + str(shipID)
            return AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION
        if ship.design.parts.__contains__("CO_SUSPEND_ANIM_POD"):
            # print ">> assessShipRole SuspAnim Ship:" + str(shipID)
            return AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION
    elif ship.canInvade:
        if ship.design.parts.__contains__("GT_TROOP_POD"):
            return AIShipRoleType.SHIP_ROLE_MILITARY_INVASION
    elif ship.isArmed:
        return AIShipRoleType.SHIP_ROLE_MILITARY
    else:
        return AIShipRoleType.SHIP_ROLE_CIVILIAN_EXPLORATION

    return AIShipRoleType.SHIP_ROLE_INVALID

def generateAIFleetOrdersForAIFleetMissions():
    "generates fleet orders from targets"

    print ""
    print "Exploration Fleets : " + str(getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_EXPLORATION))
    print "Colonization Fleets: " + str(getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_COLONISATION))
    print "Outpost Fleets     : " + str(getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_OUTPOST))
    print "Attack Fleets      : " + str(getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_ATTACK))
    print "Defend Fleets      : " + str(getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_DEFEND))
    print "Invasion Fleets    : " + str(getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_INVASION))
    print "Military Fleets    : " + str(getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_MILITARY))

    print ""
    print "Explored systems  :"
    printSystems(foAI.foAIstate.getExplorableSystems(AIExplorableSystemType.EXPLORABLE_SYSTEM_EXPLORED))
    print "Unexplored systems:"
    printSystems(foAI.foAIstate.getExplorableSystems(AIExplorableSystemType.EXPLORABLE_SYSTEM_UNEXPLORED))
    print ""

    print "Exploration targets: fleetID[MissionType]:{TargetType:targetID}"
    explorationAIFleetMissions = foAI.foAIstate.getAIFleetMissionsWithAnyMissionTypes([AIFleetMissionType.FLEET_MISSION_EXPLORATION])
    for explorationAIFleetMission in explorationAIFleetMissions:
        print "    " + str(explorationAIFleetMission)

    print "Colonization targets: fleetID[MissionType]:{TargetType:targetID}"
    colonisationAIFleetMissions = foAI.foAIstate.getAIFleetMissionsWithAnyMissionTypes([AIFleetMissionType.FLEET_MISSION_COLONISATION])
    for colonisationAIFleetMission in colonisationAIFleetMissions:
        print "    " + str(colonisationAIFleetMission)

    print "Invasion targets: fleetID[MissionType]:{TargetType:targetID}"
    invasionAIFleetMissions = foAI.foAIstate.getAIFleetMissionsWithAnyMissionTypes([AIFleetMissionType.FLEET_MISSION_INVASION])
    for invasionAIFleetMission in invasionAIFleetMissions:
        print "    " + str(invasionAIFleetMission)

    print "Military targets: fleetID[MissionType]:{TargetType:targetID}"
    militaryAIFleetMissions = foAI.foAIstate.getAIFleetMissionsWithAnyMissionTypes([AIFleetMissionType.FLEET_MISSION_MILITARY])
    for militaryAIFleetMission in militaryAIFleetMissions:
        print "    " + str(militaryAIFleetMission)

    aiFleetMissions = foAI.foAIstate.getAllAIFleetMissions()
    for aiFleetMission in aiFleetMissions:
        aiFleetMission.generateAIFleetOrders()

def issueAIFleetOrdersForAIFleetMissions():
    "issues fleet orders"

    print ""
    print "issuing fleet orders:"
    aiFleetMissions = foAI.foAIstate.getAllAIFleetMissions()
    for aiFleetMission in aiFleetMissions:
        aiFleetMission.issueAIFleetOrders()
    print ""

def printSystems(systemIDs):
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
    for systemID in systemIDs:
        # determine if system is in supplied
        suppliedSystem = ""
        if systemID in fleetSupplyableSystemIDs:
            suppliedSystem = " supplied"

        system = universe.getSystem(systemID)
        if system:
            print "    name:" + system.name + " id:" + str(systemID) + suppliedSystem
        else:
            print "    name:??? id:" + str(systemID) + suppliedSystem

