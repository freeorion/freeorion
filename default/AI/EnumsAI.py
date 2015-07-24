def check_validity(value):
    """checks if value is valid"""
    return value is not None and value >= 0


class EnumsType(object):
    names = ()

    @classmethod
    def name(cls, mtype):
        if mtype is None:
            return 'None'
        try:
            return cls.names[mtype]
        except (IndexError):
            return cls.names[-1]


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
            return self.AIPriorityNames[mtype]
        except IndexError:
            return "invalidPriorityType"

AIPriorityNames = AIPriorityType.AIPriorityNames


def get_priority_resource_types():
    return range(0, 6)


def get_priority_production_types():
    return range(6, 12)


def get_priority_research_types():
    return range(12, 19)


def get_priority_types():
    return range(0, 19)


class AIExplorableSystemType(object):
    EXPLORABLE_SYSTEM_INVALID = -1
    EXPLORABLE_SYSTEM_UNREACHABLE = 0
    EXPLORABLE_SYSTEM_UNEXPLORED = 1
    EXPLORABLE_SYSTEM_EXPLORED = 2
    EXPLORABLE_SYSTEM_VISIBLE = 3


def get_explorable_system_types():
    return range(0, 4)


class AIFleetMissionType(EnumsType):
    FLEET_MISSION_INVALID = -1
    FLEET_MISSION_EXPLORATION = 0
    FLEET_MISSION_OUTPOST = 1
    FLEET_MISSION_COLONISATION = 2
    FLEET_MISSION_SPLIT_FLEET = 3
    FLEET_MISSION_MERGE_FLEET = 4  # not really supported yet
    FLEET_MISSION_INVASION = 9
    FLEET_MISSION_MILITARY = 10
    FLEET_MISSION_SECURE = 11  # mostly same as MILITARY, but waits for system removal from all targeted system lists (invasion, colonization, outpost, blockade) before clearing
    FLEET_MISSION_ORBITAL_DEFENSE = 12
    FLEET_MISSION_ORBITAL_INVASION = 13
    FLEET_MISSION_ORBITAL_OUTPOST = 14
    # FLEET_MISSION_ORBITAL_COLONISATION = 15 Not implemented yet

    names = ['explore', 'outpost', 'colonize', 'split_fleet', 'mergeFleet', 'hit&Run', 'attack', 'defend', 'last_stand', 'invasion', 'military', 'secure',
                                                    'orbitalDefense', 'orbitalInvasion', 'orbitalOutpost', 'orbitalColonisation', 'repair', 'invalid']


FLEET_MISSION_TYPES = range(0, 17)


class AIShipDesignTypes(object):
    explorationShip = {"SD_SCOUT":"A", "Scout":"B", "Tracker":"C"}
    colonyShip = {"SD_COLONY_SHIP":"A", "Seeder":"B", "Nest-Maker":"C", "Den-Maker":"D"}
    outpostShip = {"SD_OUTPOST_SHIP":"A", "Outposter":"B"}
    troopShip = {"SD_TROOP_SHIP":"A", "Basic-Troopers":"B", "Medium-Troopers":"C", "Heavy-Troopers":"D", "Very-Heavy-Troopers":"E"}
    # [(0, ('SD_MARK', 'A')), (1, ('Lynx', 'B')), (2, ('Griffon', 'C')), (3, ('Wyvern', 'D')), (4, ('Manticore', 'E')),
    # (5, ('Atlas', 'EA')), (6, ('Pele', 'EB')), (7, ('Xena', 'EC')), (8, ('Devil', 'F')), (9, ('Reaver', 'G')), (10, ('Obliterator', 'H'))]
    attackShip= {"SD_MARK":"A", 
                        "Lynx":"B",
                        "Griffon":"C",
                        "Bolo": "CA",
                        "Comet":"CB",
                        "Wyvern":"D", 
                        "Manticore":"E",
                        "Atlas":"EA",
                        "Pele":"EB",
                        "Xena":"EC",
                        "Devil":"F",
                        "Reaver":"G",
                        "Obliterator":"H",
                                    }

    colonyBase={"SD_COLONY_BASE":"A", "NestBase":"B"}
    outpostBase={"SD_OUTPOST_BASE":"A", "OutpostBase":"B"}
    troopBase={"SpaceInvaders":"A"}
    defenseBase={"Decoy":"A", "OrbitalGrid":"B", "OrbitalShield":"C", "OrbitalMultiShield":"D"}


class AIShipRoleType(EnumsType):  # this is also used in determining fleetRoles
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

    names = ('milAttack', 'milLongrange', 'milMissiles', 'MilPD', 'CivExplore', 'CivColonize', 'CivOutpost',
             'MilInvasion', 'MilMil', 'baseDef', 'baseInvasion', 'baseOutpost', 'baseColony', 'invalid')


def get_ship_roles_types():
    return range(0, 9)


class AIEmpireProductionTypes(object):
    INVALID_BUILD_TYPE = -1
    BT_NOT_BUILDING = 0  # ///< no building is taking place
    BT_BUILDING = 1  # ///< a Building object is being built
    BT_SHIP = 2  # ///< a Ship object is being built
    NUM_BUILD_TYPES = 3


class AIProductionDemandType(object):
    PRODUCTION_DEMAND_INVALID = -1
    PRODUCTION_DEMAND_SHIP = 0
    PRODUCTION_DEMAND_BUILDING = 1


def get_production_demand_types():
    return range(0, 2)


class AIProductionRequirementType(object):
    PRODUCTION_REQUIREMENT_INVALID = -1
    PRODUCTION_REQUIREMENT_MINERALS_POINTS = 0
    PRODUCTION_REQUIREMENT_FOOD_POINTS = 1
    PRODUCTION_REQUIREMENT_RESEARCH_POINTS = 2
    PRODUCTION_REQUIREMENT_TRADE_POINTS = 3
    PRODUCTION_REQUIREMENT_PRODUCTION_POINTS = 4
    PRODUCTION_REQUIREMENT_MINIMUM_TURNS = 5
    PRODUCTION_REQUIREMENT_MINIMUM_SHIPYARDS = 6


def get_production_requirement_types():
    return range(0, 7)


class AIResearchRequirementType(object):
    RESEARCH_REQUIREMENT_INVALID = -1
    RESEARCH_REQUIREMENT_THEORY = 0
    RESEARCH_REQUIREMENT_REFIMENT = 1
    RESEARCH_REQUIREMENT_APPLICATION = 2
    RESEARCH_REQUIREMENT_RESEARCH_POINTS = 3


def get_research_requirement_types():
    return range(0, 4)


def get_mission_types():
    return range(0, 4)


class AIEmpireWarMissionType(object):
    EMPIRE_WAR_MISSION_INVALID = -1
    EMPIRE_WAR_MISSION_DEFEND_SYSTEM = 0
    EMPIRE_WAR_MISSION_DEFEND_SHIP = 1
    EMPIRE_WAR_MISSION_DEFEND_FLEET = 2
    EMPIRE_WAR_MISSION_GET_PLANET = 3
    EMPIRE_WAR_MISSION_GET_SYSTEM = 4


def get_empire_war_mission_types():
    return range(0, 5)


class AIDemandType(object):
    DEMAND_INVALID = -1
    DEMAND_RESOURCE = 0
    DEMAND_PRODUCTION = 1
    DEMAND_RESEARCH = 2


def get_demand_types():
    return range(0, 3)


class AIFocusType(object):
    FOCUS_PROTECTION = "FOCUS_PROTECTION"
    FOCUS_GROWTH = "FOCUS_GROWTH"
    FOCUS_INDUSTRY = "FOCUS_INDUSTRY"
    FOCUS_RESEARCH = "FOCUS_RESEARCH"
    FOCUS_TRADE = "FOCUS_TRADE"
    FOCUS_CONSTRUCTION = "FOCUS_CONSTRUCTION"
    FOCUS_MINING = "FOCUS_MINING"
    FOCUS_HEAVY_MINING = "FOCUS_HEAVY_MINING"

