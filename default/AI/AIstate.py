import copy
import freeOrionAIInterface as fo

# TO DO:
# hide gs_etc functions

# AI enumerators
missionTypes =       ["MT_NONE", "MT_EXPLORATION", "MT_COLONISATION", "MT_MOVE", "MT_MERGE", "MT_ATTACK", "MT_GUARD"]

# fleetRoles = ... <> conversion to/from missionType?

militaryRoles =      ["SR_ATTACK", "SR_LONGRANGE", "SR_MISSILES", "SR_POINTDEFENSE"]
civilianRoles =      ["SR_EXPLORATION", "SR_COLONISATION"]
shipRoles =          ["SR_NONE"] + militaryRoles + civilianRoles

priorityResources =  ["PR_FOOD", "PR_MINERALS", "PR_PRODUCTION", "PR_RESEARCH", "PR_TRADE"]
priorityProduction = ["PR_EXPLORATION", "PR_COLONISATION", "PR_MILITARY", "PR_BUILDINGS"]
priorityResearch =   ["PR_LEARNING", "PR_GROWTH", "PR_PRODUCTION", "PR_CONSTRUCTION", "PR_ECONOMICS", "PR_SHIPS"]
priorityTypes =      ["PR_NONE"] + priorityResources + priorityProduction + priorityResearch


# AIstate class
class AIstate:
    "stores, saves and loads AI gamestate"

    # def startTurn (cleans missions, fleetroles, calls colonisable planets + priorities)
    # def endTurn
    
    # - def getLostFleets (save fleets last turn, look if still there)
    #  => from AI interface SITREP?
    # - - def cleanMissions
    # - - def cleanFleetRoles

    # fleet missions
    def addMission(self, missionType, mission): gs_addMission(self, missionType, mission)
    def removeMission(self, missionType, missionID): gs_removeMission(self, missionType, missionID) 
    def removeAnyMission(self, missionID): gs_removeAnyMission(self, missionID)

    def getMission(self, missionType, missionID): return gs_getMission(self, missionType, missionID)
    def getMissions(self, missionType): return gs_getMissions(self, missionType)
    def getAllMissions(self): return gs_getAllMissions(self)

    def hasMission(self, missionType, missionID): return gs_hasMission(self, missionType, missionID)
    def hasAnyMission(self, mission): return gs_hasAnyMission(self, mission)

    def hasTarget(self, missionType, targetID): return gs_hasTarget(self, missionType, targetID)
    def hasAnyTarget(self, targetID): return gs_hasAnyTarget(self, targetID)

    def printMissions(self, missionType): gs_printMissions(self, missionType)

    # ship names/roles
    def getShipRole(self, shipDesignID): return gs_getShipRole(self, shipDesignID)
    def addShipRole(self, shipDesignID, shipRole): gs_addShipRole(self, shipDesignID, shipRole)
    def removeShipRole(self, shipDesignID): gs_removeShipRole(self, shipDesignID)

    # fleet IDs/roles
    def getFleetRole(self, fleetID): return gs_getFleetRole(self, fleetID)
    def addFleetRole(self, fleetID, missionType): gs_addFleetRole(self, fleetID, missionType)
    def removeFleetRole(self, fleetID): gs_removeFleetRole(self, fleetID)
    def cleanFleetRoles(self): gs_cleanFleetRoles(self)

    # colonisable planets (should be set at start of turn)
    # (hide?) setColonisablePlanets
    # getColonisablePlanets (deepcopy!)

    # demands (should be set at start of turn)
    def setPriority(self, priorityType, value): gs_setPriority(self, priorityType, value)
    def getPriority(self, priorityType): return gs_getPriority(self, priorityType)
    def getAllPriorities(self): return gs_getAllPriorities(self)

    # constructor / destructor
    def __init__(self):
        
        self.__missionsByType__ = {}
        for missionType in missionTypes: self.__missionsByType__[missionType] = {}

        self.__shipRoleByDesignID__ = {}
        self.__fleetRoleByID__ = {}
        self.__priorityByType__ = {}
        
    def __del__(self):
        del self.__missionsByType__
        del self.__shipRoleByDesignID__
        del self.__fleetRoleByID__
        del self.__priorityByType__




def missionTypeValid(missionType):
    "checks if missionType is valid"
    
    if not (missionType in missionTypes):
        print "Invalid missionType: " + missionType
        return False

    return True


# def fleetRoleValid => missionTypeValid




def gs_addMission(self, missionType, mission):
    "adds mission of a missionType"

    if not missionTypeValid(missionType): return

    # check if there already is a mission
    if self.hasAnyMission(mission[0]):
        print "Mission ID " + str(mission[0]) + " already exists."
        return

    missions = self.__missionsByType__[missionType]
    missions[mission[0]] = mission[1]


def gs_removeMission(self, missionType, missionID):
    "removes a mission of a missionType"

    if not missionTypeValid(missionType): return

    missions = self.__missionsByType__[missionType]

    if len(missions) == 0: return
    if (missionID in missions):
        print "Removed " + missionType + " ID: " + str(missionID)
        del missions[missionID]
        return


def gs_removeAnyMission(self, missionID):
    "removes a mission"

    if self.hasAnyMission(missionID):
        for missionType in missionTypes:
            self.removeMission(missionType, missionID)
    

def gs_printMissions(self, missionType):
    "prints all missions of a missionType"

    if not missionTypeValid(missionType): return
 
    print "Missions: " + str(self.__missionsByType__[missionType])


def gs_getMission(self, missionType, missionID):
    "returns the mission of missionID"

    if not missionTypeValid(missionType): return

    missions = self.getMissions(missionType)

    if missionID in missions:
        return [missionID, missions[missionID]]

    print "Mission ID " + str(missionID) + " not found."
    return None
    

def gs_getMissions(self, missionType):
    "returns a list with all missions of a missionType"

    if not missionTypeValid(missionType): return

    return copy.deepcopy(self.__missionsByType__[missionType])


def gs_getAllMissions(self):
    "returns a list with all missions"

    pass
#    missions = []

#    for missionType in missionTypes:
#        missions = missions + zip(self.__missionsByType__[missionType].keys(), self.__missionsByType__[missionType].values())

def gs_hasMission(self, missionType, missionID):
    "returns True if the requested mission is stored in gamestate"

    if not missionTypeValid(missionType): return

    missions = self.__missionsByType__[missionType]

    if len(missions) == 0: return False
    
    if (missionID in missions.keys()):
        return True

    return False


def gs_hasAnyMission(self, missionID):
    "returns True if the requested mission is stored in gamestate, independent of missionType"

    for missionType in missionTypes:
        if self.hasMission(missionType, missionID): return True

    return False


def gs_hasTarget(self, missionType, targetID):
    "returns true if targetID is the target of a mission"

    if not missionTypeValid(missionType): return

    missions = self.__missionsByType__[missionType]
    targets = missions.values()

    if len(targets) == 0: return False
    if targetID in targets: return True

    return False


def gs_hasAnyTarget(self, targetID):
    "returns true if targetID is the target of any mission"

    for missionType in missionTypes:
        if self.hasTarget(missionType, targetID): return True

    return False




def gs_getShipRole(self, shipDesignID):
    "returns ship role by name"

    if shipDesignID in self.__shipRoleByDesignID__:
        return self.__shipRoleByDesignID__[shipDesignID]

    return shipRoles[0]
    print "'" + str(shipDesignID) + "'" + " not found."


def gs_addShipRole(self, shipDesignID, shipRole):
    "adds a ship name/role pair"

    if not (shipRole in shipRoles):
        print "Invalid shipRole: " + shipRole
        return

    if shipDesignID in self.__shipRoleByDesignID__:
        # print shipDesignID + " already exists."
        return
    
    self.__shipRoleByDesignID__[shipDesignID] = shipRole
    

def gs_removeShipRole(self, shipDesignID): 
    "removes a ship name/role pair"

    if shipDesignID in self.__shipRoleByDesignID__:
        print "Removed ship named: " + shipDesignID
        del self.__shipRoleByDesignID__[shipDesignID]
        return

    print str(shipDesignID) + " not found."




def gs_getFleetRole(self, fleetID):
    "returns fleet role by ID"

    if fleetID in self.__fleetRoleByID__:
        return self.__fleetRoleByID__[fleetID]

    return missionTypes[0]
    print "Fleet ID" + str(fleetID) + " not found."


def gs_addFleetRole(self, fleetID, missionType):
    "adds a fleet ID/role pair"

    if not missionTypeValid(missionType): return

    if fleetID in self.__fleetRoleByID__:
        print "Fleet ID " + str(fleetID) + " already exists."
        return

    self.__fleetRoleByID__[fleetID] = missionType


def gs_removeFleetRole(self, fleetID):
    "removes a fleet ID/role pair"

    if fleetID in self.__fleetRoleByID__:
        print "Removed fleet ID: " + str(fleetID)
        del self.__fleetRoleByID__[fleetID]
        return

    print "Fleet ID " + str(fleetID) + " not found."


def gs_cleanFleetRoles(self):
    "removes fleetRoles if a fleet has been lost"

    deleteRoles = []
    universe = fo.getUniverse()

    for fleetID in self.__fleetRoleByID__:
        fleet = universe.getFleet(fleetID)
        if (fleet == None):
            deleteRoles.append(fleetID)

    for fleetID in deleteRoles:
        del self.__fleetRoleByID__[fleetID]
        print "Deleted fleetRole: " + str(fleetID)




def gs_setPriority(self, priorityType, value):
    "sets a priority of the specified type"

    self.__priorityByType__[priorityType] = value


def gs_getPriority(self, priorityType):
    "returns the priority value of the specified type"

    if priorityType in self.__priorityByType__:
        return self.__priorityByType__[priorityType]

    return 0
    # todo: check for valid types


def gs_getAllPriorities(self):
    "returns a dictionary with all priority values"

    return copy.deepcopy(self.__priorityByType__)
