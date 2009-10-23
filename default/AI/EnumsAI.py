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
    PRIORITY_RESOURCE_FOOD = 0
    PRIORITY_RESOURCE_MINERALS = 1
    PRIORITY_RESOURCE_PRODUCTION = 2
    PRIORITY_RESOURCE_RESEARCH = 3
    PRIORITY_RESOURCE_TRADE = 4
    PRIORITY_PRODUCTION_EXPLORATION = 5
    PRIORITY_PRODUCTION_COLONISATION = 6
    PRIORITY_PRODUCTION_MILITARY = 7
    PRIORITY_PRODUCTION_BUILDINGS = 8
    PRIORITY_RESEARCH_LEARNING = 9
    PRIORITY_RESEARCH_GROWTH = 10
    PRIORITY_RESEARCH_PRODUCTION = 11
    PRIORITY_RESEARCH_CONSTRUCTION = 12
    PRIORITY_RESEARCH_ECONOMICS = 13
    PRIORITY_RESEARCH_SHIPS = 14

def getAIPriorityResourceTypes():
    return __getInterval(0, 4)
def getAIPriorityProductionTypes():
    return __getInterval(5, 8)
def getAIPriorityResearchTypes():
    return __getInterval(9, 14)
def getAIPriorityTypes():
    return __getInterval(0, 14)


class AIExplorableSystemType(object):
    EXPLORABLE_SYSTEM_INVALID = -1
    EXPLORABLE_SYSTEM_UNEXPLORED = 0
    EXPLORABLE_SYSTEM_EXPLORED = 1
    EXPLORABLE_SYSTEM_VISIBLE = 2

def getAIExplorableSystemTypes():
    return __getInterval(0, 2)


class AIFleetMissionType(object):
    FLEET_MISSION_INVALID = -1
    FLEET_MISSION_EXPLORATION = 0
    FLEET_MISSION_COLONISATION = 1
    FLEET_MISSION_SPLIT_FLEET = 2
    FLEET_MISSION_MERGE_FLEET = 3
    FLEET_MISSION_HIT_AND_RUN = 4
    FLEET_MISSION_ATTACK = 5
    FLEET_MISSION_DEFEND = 6
    FLEET_MISSION_LAST_STAND = 7

def getAIFleetMissionTypes():
    return __getInterval(0, 7)


class AIFleetOrderType(object):
    ORDER_INVALID = -1
    ORDER_SCRAP = 0
    ORDER_MOVE = 1
    ORDER_RESUPPLY = 2
    ORDER_SPLIT_FLEET = 3
    ORDER_MERGE_FLEET = 4
    ORDER_COLONISE = 5
    ORDER_ATACK = 6
    ORDER_DEFEND = 7

def getAIFleetOrderTypes():
    return __getInterval(0, 7)


class AIShipRoleType(object):
    SHIP_ROLE_INVALID = -1
    SHIP_ROLE_MILITARY_ATTACK = 0
    SHIP_ROLE_MILITARY_LONGRANGE = 1
    SHIP_ROLE_MILITARY_MISSILES = 2
    SHIP_ROLE_MILITARY_POINTDEFENSE = 3
    SHIP_ROLE_CIVILIAN_EXPLORATION = 4
    SHIP_ROLE_CIVILIAN_COLONISATION = 5

def getAIShipRolesTypes():
    return __getInterval(0, 5)


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

def getAITargetTypes():
    return __getInterval(0, 7)


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
