import copy
import freeOrionAIInterface as fo
import EnumsAI
from EnumsAI import AIFleetMissionType, AIShipRoleType, AIExplorableSystemType
import AIFleetMission

# global variables
foodStockpileSize = 1     # food stored per population
minimalColoniseValue = 4  # minimal value for a planet to be colonised, now a size 2 terran world

# AIstate class
class AIstate(object):
    "stores AI game state"

    # def colonisablePlanets (should be set at start of turn)
    # getColonisablePlanets (deepcopy!)

    def __init__(self):
        "constructor"

        self.__missionsByType = {}
        for missionType in EnumsAI.getAIFleetMissionTypes():
            self.__missionsByType[missionType] = {}

        self.__aiMissionsByFleetID = {}

        self.__shipRoleByDesignID = {}
        self.__fleetRoleByID = {}
        self.__priorityByType = {}

        self.__explorableSystemByType = {}
        for explorableSystemsType in EnumsAI.getAIExplorableSystemTypes():
            self.__explorableSystemByType[explorableSystemsType] = {}

    def __del__(self):
        "destructor"

        del self.__missionsByType
        del self.__shipRoleByDesignID
        del self.__fleetRoleByID
        del self.__priorityByType
        del self.__explorableSystemByType
        del self.__aiMissionsByFleetID

    def clean(self, startSystemID, fleetIDs):
        "turn AIstate cleanup"

        # cleanup explorable systems
        self.__cleanExplorableSystems(startSystemID)
        # TODO: cleanup colonisable planets
        # cleanup fleet roles
        self.__cleanFleetRoles()

        self.__cleanAIFleetMissions(fleetIDs)

    def afterTurnCleanup(self):
        "removes not required information to save from AI state after AI complete its turn"

        # some ships in fleet can be destroyed between turns and then fleet may have have different roles
        self.__fleetRoleByID = {}

    def __hasAIFleetMission(self, fleetID):
        "returns True if fleetID has AIFleetMission"

        return self.__aiMissionsByFleetID.__contains__(fleetID)

    def getAIFleetMission(self, fleetID):
        "returns AIFleetMission with fleetID"

        if self.__hasAIFleetMission(fleetID):
            return self.__aiMissionsByFleetID[fleetID]
        return None

    def getAllAIFleetMissions(self):
        "returns all AIFleetMissions"

        return self.__aiMissionsByFleetID.values()

    def getAIFleetMissionsWithAnyMissionTypes(self, fleetMissionTypes):
        "returns all AIFleetMissions which contains any of fleetMissionTypes"

        result = []

        aiFleetMissions = self.getAllAIFleetMissions()
        for aiFleetMission in aiFleetMissions:
            if aiFleetMission.hasAnyOfAIMissionTypes(fleetMissionTypes):
                result.append(aiFleetMission)
        return result

    def __addAIFleetMission(self, fleetID):
        "add new AIFleetMission with fleetID if it already not exists"

        if self.getAIFleetMission(fleetID) == None:
            aiFleetMission = AIFleetMission.AIFleetMission(fleetID)
            self.__aiMissionsByFleetID[fleetID] = aiFleetMission

    def __removeAIFleetMission(self, fleetID):
        "remove invalid AIFleetMission with fleetID if it exists"

        aiFleetMission = self.getAIFleetMission(fleetID)
        if aiFleetMission != None:
            self.__aiMissionsByFleetID[fleetID] = None
            del aiFleetMission
            del self.__aiMissionsByFleetID[fleetID]

    def __cleanAIFleetMissions(self, fleetIDs):
        "cleanup of AIFleetMissions"

        for fleetID in fleetIDs:
            if self.getAIFleetMission(fleetID) == None:
                self.__addAIFleetMission(fleetID)

        aiFleetMissions = self.getAllAIFleetMissions()
        deletedFleetIDs = []
        for aiFleetMission in aiFleetMissions:
            if not(aiFleetMission.getAITargetID() in fleetIDs):
                deletedFleetIDs.append(aiFleetMission.getAITargetID())
        for deletedFleetID in deletedFleetIDs:
            self.__removeAIFleetMission(deletedFleetID)

        aiFleetMissions = self.getAllAIFleetMissions()
        for aiFleetMission in aiFleetMissions:
            aiFleetMission.cleanInvalidAITargets()

    def hasAITarget(self, aiFleetMissionType, aiTarget):
        aiFleetMissions = self.getAIFleetMissionsWithAnyMissionTypes([aiFleetMissionType])
        for mission in aiFleetMissions:
            if mission.hasTarget(aiFleetMissionType, aiTarget):
                return True
        return False

    def getShipRole(self, shipDesignID):
        "returns ship role by name"

        if shipDesignID in self.__shipRoleByDesignID:
            return self.__shipRoleByDesignID[shipDesignID]

        print "'" + str(shipDesignID) + "'" + " not found."
        return AIShipRoleType.SHIP_ROLE_INVALID

    def addShipRole(self, shipDesignID, shipRole):
        "adds a ship name/role pair"

        if not (shipRole in EnumsAI.getAIShipRolesTypes()):
            print "Invalid shipRole: " + str(shipRole)
            return

        if shipDesignID in self.__shipRoleByDesignID:
            # print shipDesignID + " already exists."
            return

        self.__shipRoleByDesignID[shipDesignID] = shipRole

    def removeShipRole(self, shipDesignID):
        "removes a ship name/role pair"

        if shipDesignID in self.__shipRoleByDesignID:
            print "Removed ship named: " + str(shipDesignID)
            del self.__shipRoleByDesignID[shipDesignID]
            return

        print str(shipDesignID) + " not found."

    def getFleetRole(self, fleetID):
        "returns fleet role by ID"

        if fleetID in self.__fleetRoleByID:
            return self.__fleetRoleByID[fleetID]

        print "Fleet ID" + str(fleetID) + " not found."
        return AIFleetMissionType.FLEET_MISSION_INVALID

    def addFleetRole(self, fleetID, missionType):
        "adds a fleet ID/role pair"

        if not EnumsAI.checkValidity(missionType): return

        if fleetID in self.__fleetRoleByID:
            #print "Fleet ID " + str(fleetID) + " already exists."
            return

        self.__fleetRoleByID[fleetID] = missionType

    def removeFleetRole(self, fleetID):
        "removes a fleet ID/role pair"

        if fleetID in self.__fleetRoleByID:
            print "Removed fleet ID: " + str(fleetID)
            del self.__fleetRoleByID[fleetID]
            return

        #print "Fleet ID " + str(fleetID) + " not found."

    def __cleanFleetRoles(self):
        "removes fleetRoles if a fleet has been lost"

        deleteRoles = []
        universe = fo.getUniverse()

        for fleetID in self.__fleetRoleByID:
            fleet = universe.getFleet(fleetID)
            if (fleet == None):
                deleteRoles.append(fleetID)

        for fleetID in deleteRoles:
            del self.__fleetRoleByID[fleetID]
            print "Deleted fleetRole: " + str(fleetID)

    def getExplorableSystem(self, systemID):
        "determines system type from ID and returns it"

        for explorableSystemsType in EnumsAI.getAIExplorableSystemTypes():
            systems = self.getExplorableSystems(explorableSystemsType)
            if systemID in systems:
                return explorableSystemsType

        # print "SystemID " + str(systemID) + " not found."
        return AIExplorableSystemType.EXPLORABLE_SYSTEM_INVALID

    def addExplorableSystem(self, explorableSystemsType, systemID):
        "add explorable system ID with type"

        if not (explorableSystemsType in EnumsAI.getAIExplorableSystemTypes()):
            return

        systems = self.__explorableSystemByType[explorableSystemsType]
        if systemID in systems:
            return
        systems[systemID] = systemID

    def removeExplorableSystem(self, explorableSystemsType, systemID):
        "removes explorable system ID with type"

        systems = self.__explorableSystemByType[explorableSystemsType]
        if len(systems) == 0:
            return
        if systemID in systems:
            del systems[systemID]

    def __cleanExplorableSystems(self, startSystemID):
        "cleanup of all explorable systems"

        universe = fo.getUniverse()
        systemIDs = universe.systemIDs
        empireID = fo.empireID()
        empire = fo.getEmpire()

        for systemID in systemIDs:
            system = universe.getSystem(systemID)
            if not system: continue

            #print "system with id: " + str(systemID)

            if (empire.hasExploredSystem(systemID)):
                self.addExplorableSystem(AIExplorableSystemType.EXPLORABLE_SYSTEM_EXPLORED, systemID)
                self.removeExplorableSystem(AIExplorableSystemType.EXPLORABLE_SYSTEM_UNEXPLORED, systemID)
                #print "    has been explored"
                continue
            if (not universe.systemsConnected(systemID, startSystemID, empireID)):
                for explorableSystemsType in EnumsAI.getAIExplorableSystemTypes():
                    self.removeExplorableSystem(explorableSystemsType, systemID)
                #print "    is not connected to system with id: " + str(startSystemID)
                continue
            explorableSystemsType = self.getExplorableSystem(systemID)
            if (explorableSystemsType == AIExplorableSystemType.EXPLORABLE_SYSTEM_VISIBLE):
                #print "    is already explored system target"
                continue
            #print "    is now an unexplored system"
            self.addExplorableSystem(AIExplorableSystemType.EXPLORABLE_SYSTEM_UNEXPLORED, systemID)

    def getExplorableSystems(self, explorableSystemsType):
        "get all explorable systems determined by type "

        return copy.deepcopy(self.__explorableSystemByType[explorableSystemsType])

    def setPriority(self, priorityType, value):
        "sets a priority of the specified type"

        self.__priorityByType[priorityType] = value

    def getPriority(self, priorityType):
        "returns the priority value of the specified type"

        if priorityType in self.__priorityByType:
            return copy.deepcopy(self.__priorityByType[priorityType])

        return 0

    def getAllPriorities(self):
        "returns a dictionary with all priority values"

        return copy.deepcopy(self.__priorityByType)

    def printPriorities(self):
        "prints all priorities"

        print "all priorities:"
        for priority in self.__priorityByType:
            print "    " + str(priority) + ": " + str(self.__priorityByType[priority])
        print ""
