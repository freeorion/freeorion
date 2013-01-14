import traceback
import sys

def checkValidity(value):
    "checks if value is valid"

    if (value == None or value < 0):
        return False
    return True

def __getInterval(low, high):
    "returns integer numbers from interval <low, high>"

    result = []
    # <low, high)
    for i in range(low, high):
        result.append(i)
    # <high, high>
    result.append(high)

    return result

class AIPriorityType(object):
    PRIORITY_INVALID = -1
    PRIORITY_RESOURCE_GROWTH = 0
    PRIORITY_RESOURCE_PRODUCTION = 2
    PRIORITY_RESOURCE_RESEARCH = 3
    PRIORITY_RESOURCE_TRADE = 4
    PRIORITY_RESOURCE_CONSTRUCTION = 5
    PRIORITY_PRODUCTION_EXPLORATION = 6
    PRIORITY_PRODUCTION_OUTPOST = 7
    PRIORITY_PRODUCTION_COLONISATION = 8
    PRIORITY_PRODUCTION_INVASION = 9
    PRIORITY_PRODUCTION_MILITARY = 10
    PRIORITY_PRODUCTION_BUILDINGS = 11
    PRIORITY_RESEARCH_LEARNING = 12
    PRIORITY_RESEARCH_GROWTH = 13
    PRIORITY_RESEARCH_PRODUCTION = 14
    PRIORITY_RESEARCH_CONSTRUCTION = 15
    PRIORITY_RESEARCH_ECONOMICS = 16
    PRIORITY_RESEARCH_SHIPS = 17
    PRIORITY_RESEARCH_DEFENSE = 18
    PRIORITY_PRODUCTION_ORBITAL_DEFENSE = 19
    PRIORITY_PRODUCTION_ORBITAL_INVASION = 20
    PRIORITY_PRODUCTION_ORBITAL_OUTPOST = 21
    PRIORITY_PRODUCTION_ORBITAL_COLONISATION = 22

    AIPriorityNames = [
            "RESOURCE_GROWTH", 
            "slot_empty", 
            "RESOURCE_PRODUCTION", 
            "RESOURCE_RESEARCH", 
            "RESOURCE_TRADE", 
            "RESOURCE_CONSTRUCTION", 
            "PRODUCTION_EXPLORATION", 
            "PRODUCTION_OUTPOST", 
            "PRODUCTION_COLONISATION", 
            "PRODUCTION_INVASION", 
            "PRODUCTION_MILITARY", 
            "PRODUCTION_BUILDINGS", 
            "RESEARCH_LEARNING", 
            "RESEARCH_GROWTH", 
            "RESEARCH_PRODUCTION", 
            "RESEARCH_CONSTRUCTION", 
            "RESEARCH_ECONOMICS", 
            "RESEARCH_SHIPS", 
            "RESEARCH_DEFENSE", 
            "PRODUCTION_ORBITAL_DEFENSE", 
            "PRODUCTION_ORBITAL_INVASION", 
            "PRODUCTION_ORBITAL_OUTPOST", 
            "PRODUCTION_ORBITAL_COLONISATION", 
            "INVALID" ]

    def name(self, mtype):
        try:
            name=self.AIPriorityNames[mtype]
            return name
        except:
            return "invalidPriorityType"
            print "Error: exception triggered and caught:  ",  traceback.format_exc()

AIPriorityNames = AIPriorityType.AIPriorityNames

def getAIPriorityResourceTypes():
    return __getInterval(0, 5)
def getAIPriorityProductionTypes():
    return __getInterval(6, 11)
def getAIPriorityResearchTypes():
    return __getInterval(12, 18)
def getAIPriorityTypes():
    return __getInterval(0, 18)
   

class AIExplorableSystemType(object):
    EXPLORABLE_SYSTEM_INVALID = -1
    EXPLORABLE_SYSTEM_UNREACHABLE = 0
    EXPLORABLE_SYSTEM_UNEXPLORED = 1
    EXPLORABLE_SYSTEM_EXPLORED = 2
    EXPLORABLE_SYSTEM_VISIBLE = 3

def getAIExplorableSystemTypes():
    return __getInterval(0, 3)

class AIFleetMissionType(object):
    FLEET_MISSION_INVALID = -1
    FLEET_MISSION_EXPLORATION = 0
    FLEET_MISSION_OUTPOST = 1
    FLEET_MISSION_COLONISATION = 2
    FLEET_MISSION_SPLIT_FLEET = 3
    FLEET_MISSION_MERGE_FLEET = 4 # not really supported yet
    FLEET_MISSION_HIT_AND_RUN = 5 # currently same as MILITARY
    FLEET_MISSION_ATTACK = 6                # currently same as MILITARY
    FLEET_MISSION_DEFEND = 7            # currently same as MILITARY
    FLEET_MISSION_LAST_STAND = 8    # currently same as MILITARY
    FLEET_MISSION_INVASION = 9
    FLEET_MISSION_MILITARY = 10
    FLEET_MISSION_SECURE = 11   # mostly same as MILITARY, but waits for system removal from all targeted system lists (invasion, colonization, outpost, blockade) before clearing
    FLEET_MISSION_ORBITAL_DEFENSE = 12
    FLEET_MISSION_ORBITAL_INVASION = 13
    FLEET_MISSION_ORBITAL_OUTPOST = 14
    FLEET_MISSION_ORBITAL_COLONISATION = 15

    MissionTypeNames=['explore',  'outpost',  'colonize',  'splitFleet',  'mergeFleet',  'hit&Run',  'attack',  'defend',  'last_stand', 'invasion', 'military', 'secure', 
                                                    'orbitalDefense', 'orbitalInvasion', 'orbitalOutpost', 'orbitalColonisation', 'invalid']
    
    def name(self, mtype):
        try:
            name=self.MissionTypeNames[mtype]
            return name
        except:
            return "invalidMissionType"
            print "Error: exception triggered and caught:  ",  traceback.format_exc()

def getAIFleetMissionTypes():
    return __getInterval(0, 15)

class AIFleetOrderType(object):
    ORDER_INVALID = -1
    ORDER_SCRAP = 0
    ORDER_MOVE = 1
    ORDER_RESUPPLY = 2
    ORDER_SPLIT_FLEET = 3
    ORDER_MERGE_FLEET = 4
    ORDER_OUTPOST = 5
    ORDER_COLONISE = 6
    ORDER_ATTACK = 7
    ORDER_DEFEND = 8
    ORDER_INVADE = 9
    ORDER_MILITARY = 10
    ORDRER_STAGING = 11
    FleetOrderTypeNames = [ "scrap",  "move",  "resupply",  "splitFleet",  "mergeFleet",  "outpost",  "colonize",  "attack",  "defend",  "invade",  "military",  "staging", "invalid"]
    def name(self, otype):
        try:
            name=self.FleetOrderTypeNames[otype]
            return name
        except:
            return "invalidFleetOrderType"
            print "Error: exception triggered and caught:  ",  traceback.format_exc()

def getAIFleetOrderTypes():
    return __getInterval(0, 10)

def getFleetOrderTypeForMission(aiFleetMissionType,  option=None):
        if aiFleetMissionType == AIFleetMissionType.FLEET_MISSION_EXPLORATION:
            return AIFleetOrderType.ORDER_MOVE
        elif aiFleetMissionType == AIFleetMissionType.FLEET_MISSION_OUTPOST:
            return AIFleetOrderType.ORDER_OUTPOST
        elif aiFleetMissionType == AIFleetMissionType.FLEET_MISSION_COLONISATION:
            return AIFleetOrderType.ORDER_COLONISE
        elif aiFleetMissionType == AIFleetMissionType.FLEET_MISSION_SPLIT_FLEET: #not really supported in this fashion currently
            return AIFleetOrderType.ORDER_SPLIT_FLEET 
        elif aiFleetMissionType == AIFleetMissionType.FLEET_MISSION_MERGE_FLEET: #not really supported in this fashion currently
            return AIFleetOrderType.ORDER_MERGE_FLEET
        elif aiFleetMissionType == AIFleetMissionType.FLEET_MISSION_HIT_AND_RUN: # currently same as MILITARY
            return AIFleetOrderType.ORDER_MILITARY
        elif aiFleetMissionType == AIFleetMissionType.FLEET_MISSION_ATTACK:                # currently same as MILITARY
            return AIFleetOrderType.ORDER_MILITARY
        elif aiFleetMissionType == AIFleetMissionType.FLEET_MISSION_DEFEND:            # currently same as MILITARY
            return AIFleetOrderType.ORDER_MILITARY
        elif aiFleetMissionType == AIFleetMissionType.FLEET_MISSION_LAST_STAND:    # currently same as MILITARY
            return AIFleetOrderType.ORDER_MILITARY
        elif aiFleetMissionType == AIFleetMissionType.FLEET_MISSION_INVASION:
            return AIFleetOrderType.ORDER_INVADE
        elif aiFleetMissionType == AIFleetMissionType.FLEET_MISSION_MILITARY:
            return AIFleetOrderType.ORDER_MILITARY
        elif aiFleetMissionType == AIFleetMissionType.FLEET_MISSION_SECURE: # mostly same as MILITARY, but waits for system removal from all targeted system lists (invasion, colonization, outpost, blockade) before clearing
            return AIFleetOrderType.ORDER_MILITARY
        elif aiFleetMissionType == AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE:
            return AIFleetOrderType.ORDER_DEFEND
        elif aiFleetMissionType == AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION:
            return AIFleetOrderType.ORDER_INVADE
        elif aiFleetMissionType == AIFleetMissionType.FLEET_MISSION_ORBITAL_OUTPOST:
            return AIFleetOrderType.ORDER_OUTPOST
        elif aiFleetMissionType == AIFleetMissionType.FLEET_MISSION_ORBITAL_COLONISATION:
            return AIFleetOrderType.ORDER_COLONISE
        else: 
            return AIFleetOrderType.ORDER_INVALID

class AIShipDesignTypes(object):
    explorationShip = {"SD_SCOUT":"A",  "Scout":"B",  "Tracker":"C"}
    colonyShip = {"SD_COLONY_SHIP":"A", "Seeder":"B", "Nest-Maker":"C", "Den-Maker":"D"}
    outpostShip = {"SD_OUTPOST_SHIP":"A", "Outposter":"B"}
    troopShip = {"SD_TROOP_SHIP":"A", "Basic-Troopers":"B", "Medium-Troopers":"C",  "Heavy-Troopers":"D", "Very-Heavy-Troopers":"D"}
    attackShip= {"SD_MARK":"A", "Lynx":"B","Griffon":"C",  "Wyvern":"D", "Manticore":"E",  "Devil":"F",  "Reaver":"G"}
    colonyBase={"SD_COLONY_BASE":"A",  "NestBase":"B"}
    outpostBase={"SD_OUTPOST_BASE":"A",  "OutpostBase":"B"}
    troopBase={"SpaceInvaders":"A"}
    defenseBase={"OrbitalGrid":"A",  "OrbitalShield":"B",  "OrbitalMultiShield":"C"}
    
class AIShipRoleType(object):  #this is also used in determining fleetRoles
    SHIP_ROLE_INVALID = -1
    SHIP_ROLE_MILITARY_ATTACK = 0
    SHIP_ROLE_MILITARY_LONGRANGE = 1
    SHIP_ROLE_MILITARY_MISSILES = 2
    SHIP_ROLE_MILITARY_POINTDEFENSE = 3
    SHIP_ROLE_CIVILIAN_EXPLORATION = 4
    SHIP_ROLE_CIVILIAN_COLONISATION = 5
    SHIP_ROLE_CIVILIAN_OUTPOST = 6
    SHIP_ROLE_MILITARY_INVASION = 7
    SHIP_ROLE_MILITARY = 8
    SHIP_ROLE_BASE_DEFENSE = 9
    SHIP_ROLE_BASE_INVASION = 10
    SHIP_ROLE_BASE_OUTPOST = 11
    SHIP_ROLE_BASE_COLONISATION = 12
    ShipRoleNames=['milAttack',  'milLongrange',  'milMissiles',  'MilPD',  'CivExplore',  'CivColonize', 'CivOutpost', 'MilInvasion', 'MilMil', 
                                            'baseDef',  'baseInvasion',  'baseOutpost',  'baseColony',  'invalid']
    def name(self, roletype):
        try:
            name=self.ShipRoleNames[roletype]
            return name
        except:
            return "invalidRoleType"
            print "Error: exception triggered and caught:  ",  traceback.format_exc()

def getAIShipRolesTypes():
    return __getInterval(0, 8)

class AITargetType(object):
    TARGET_INVALID = -1
    TARGET_BUILDING = 0
    TARGET_TECHNOLOGY = 1
    TARGET_PLANET = 2
    TARGET_SYSTEM = 3
    TARGET_SHIP = 4
    TARGET_FLEET = 5
    TARGET_EMPIRE = 6
    TARGET_ALL_OTHER_EMPIRES = 7
    TargetTypeNames = ["building",  "technology",  "planet",  "system",  "ship",  "fleet",  "empire",  "Opponents",  "invalid"]
    def name(self, targettype):
        try:
            name=self.TargetTypeNames[targettype]
            return name
        except:
            return "invalidTargetType"
            print "Error: exception triggered and caught:  ",  traceback.format_exc()

def getAITargetTypes():
    return __getInterval(0, 7)

class AIEmpireProductionTypes(object):
    INVALID_BUILD_TYPE = -1
    BT_NOT_BUILDING=0 #         ///< no building is taking place
    BT_BUILDING=1 #,             ///< a Building object is being built
    BT_SHIP = 2 #,                 ///< a Ship object is being built
    NUM_BUILD_TYPES =3
    
class AIProductionDemandType(object):
    PRODUCTION_DEMAND_INVALID = -1
    PRODUCTION_DEMAND_SHIP = 0
    PRODUCTION_DEMAND_BUILDING = 1

def getAIProductionDemandTypes():
    return __getInterval(0, 1)

class AIProductionRequirementType(object):
    PRODUCTION_REQUIREMENT_INVALID = -1
    PRODUCTION_REQUIREMENT_MINERALS_POINTS = 0
    PRODUCTION_REQUIREMENT_FOOD_POINTS = 1
    PRODUCTION_REQUIREMENT_RESEARCH_POINTS = 2
    PRODUCTION_REQUIREMENT_TRADE_POINTS = 3
    PRODUCTION_REQUIREMENT_PRODUCTION_POINTS = 4
    PRODUCTION_REQUIREMENT_MINIMUM_TURNS = 5
    PRODUCTION_REQUIREMENT_MINIMUM_SHIPYARDS = 6

def getAIProductionRequirementTypes():
    return __getInterval(0, 6)

class AIResearchRequirementType(object):
    RESEARCH_REQUIREMENT_INVALID = -1
    RESEARCH_REQUIREMENT_THEORY = 0
    RESEARCH_REQUIREMENT_REFIMENT = 1
    RESEARCH_REQUIREMENT_APPLICATION = 2
    RESEARCH_REQUIREMENT_RESEARCH_POINTS = 3

def getAIResearchRequirementTypes():
    return __getInterval(0, 3)

class AIMissionType(object):
    MISSION_INVALID = -1
    FLEET_MISSION = 0
    EMPIRE_WAR_MISSION = 1
    ALLIANCE_WAR_MISSION = 2
    DIPLOMATIC_MISSION = 3

def getAIMissionTypes():
    return __getInterval(0, 3)

class AIEmpireWarMissionType(object):
    EMPIRE_WAR_MISSION_INVALID = -1
    EMPIRE_WAR_MISSION_DEFEND_SYSTEM = 0
    EMPIRE_WAR_MISSION_DEFEND_SHIP = 1
    EMPIRE_WAR_MISSION_DEFEND_FLEET = 2
    EMPIRE_WAR_MISSION_GET_PLANET = 3
    EMPIRE_WAR_MISSION_GET_SYSTEM = 4

def getAIEmpireWarMissionTypes():
    return __getInterval(0, 4)

class AIDemandType(object):
    DEMAND_INVALID = -1
    DEMAND_RESOURCE = 0
    DEMAND_PRODUCTION = 1
    DEMAND_RESEARCH = 2

def getAIDemandTypes():
    return __getInterval(0, 2)

class AIFocusType(object):
    FOCUS_GROWTH = "FOCUS_GROWTH"
    FOCUS_INDUSTRY = "FOCUS_INDUSTRY"
    FOCUS_RESEARCH = "FOCUS_RESEARCH"
    FOCUS_TRADE = "FOCUS_TRADE"
    FOCUS_CONSTRUCTION = "FOCUS_CONSTRUCTION"
    FOCUS_MINING="FOCUS_MINING"
    FOCUS_HEAVY_MINING="FOCUS_HEAVY_MINING"

