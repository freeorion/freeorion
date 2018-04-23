def check_validity(value):
    """checks if value is valid"""
    return value is not None and value >= 0


class EnumItem(int):
    @staticmethod
    def __new__(cls, *more):
        return super(EnumItem, cls).__new__(cls, more[0])

    def __init__(self, number, name, enum_name):
        super(EnumItem, self).__init__(number)
        self.name = name
        self.enum_name = enum_name

    def __str__(self):
        return "%s.%s" % (self.enum_name, self.name)


class EnumMeta(type):
    @staticmethod
    def __new__(cls, name, bases, attrs):
        super_new = super(EnumMeta, cls).__new__
        parents = [b for b in bases if isinstance(b, EnumMeta)]
        if not parents:
            return super_new(cls, name, bases, attrs)

        for k, v in attrs.items():
            if not k.startswith('_') and isinstance(v, (int, long)):
                attrs[k] = EnumItem(v, k, name)
        return super_new(cls, name, bases, attrs)


class Enum(object):
    __metaclass__ = EnumMeta

    @classmethod
    def range(cls, start, end):
        result = []
        current_range = range(start, end)
        for key in dir(cls):
            if not key.startswith('_'):
                value = getattr(cls, key)
                if isinstance(value, EnumItem) and value in current_range:
                    result.append(value)
        return sorted(result)

    @classmethod
    def has_item(cls, item):
        """Return True if specified item is a member of the enum

        :type item: EnumItem
        :rtype: bool
        """
        return hasattr(cls, item.name) and isinstance(getattr(cls, item.name), EnumItem)


class PriorityType(Enum):
    RESOURCE_GROWTH = 1
    RESOURCE_PRODUCTION = 2
    RESOURCE_RESEARCH = 3
    RESOURCE_TRADE = 4
    RESOURCE_CONSTRUCTION = 5
    PRODUCTION_EXPLORATION = 6
    PRODUCTION_OUTPOST = 7
    PRODUCTION_COLONISATION = 8
    PRODUCTION_INVASION = 9
    PRODUCTION_MILITARY = 10
    PRODUCTION_BUILDINGS = 11
    RESEARCH_LEARNING = 12
    RESEARCH_GROWTH = 13
    RESEARCH_PRODUCTION = 14
    RESEARCH_CONSTRUCTION = 15
    RESEARCH_ECONOMICS = 16
    RESEARCH_SHIPS = 17
    RESEARCH_DEFENSE = 18
    PRODUCTION_ORBITAL_DEFENSE = 19
    PRODUCTION_ORBITAL_INVASION = 20
    PRODUCTION_ORBITAL_OUTPOST = 21
    PRODUCTION_ORBITAL_COLONISATION = 22


def get_priority_resource_types():
    return PriorityType.range(1, 6)


def get_priority_production_types():
    return PriorityType.range(6, 12)


class MissionType(Enum):
    OUTPOST = 1
    COLONISATION = 2
    SPLIT_FLEET = 3
    MERGE_FLEET = 4  # not really supported yet
    EXPLORATION = 5
    INVASION = 9
    MILITARY = 10
    # mostly same as MILITARY, but waits for system removal from all targeted system lists
    # (invasion, colonization, outpost, blockade) before clearing
    SECURE = 11
    ORBITAL_DEFENSE = 12
    ORBITAL_INVASION = 13
    ORBITAL_OUTPOST = 14
    # ORBITAL_COLONISATION = 15 Not implemented yet


class ShipRoleType(Enum):  # this is also used in determining fleetRoles
    INVALID = -1
    MILITARY_ATTACK = 1
    MILITARY_MISSILES = 2
    MILITARY_POINTDEFENSE = 3
    CIVILIAN_EXPLORATION = 4
    CIVILIAN_COLONISATION = 5
    CIVILIAN_OUTPOST = 6
    MILITARY_INVASION = 7
    MILITARY = 8
    BASE_DEFENSE = 9
    BASE_INVASION = 10
    BASE_OUTPOST = 11
    BASE_COLONISATION = 12


class EmpireProductionTypes(Enum):
    BT_NOT_BUILDING = 0  # ///< no building is taking place
    BT_BUILDING = 1  # ///< a Building object is being built
    BT_SHIP = 2  # ///< a Ship object is being built


class FocusType(object):
    FOCUS_PROTECTION = "FOCUS_PROTECTION"
    FOCUS_GROWTH = "FOCUS_GROWTH"
    FOCUS_INDUSTRY = "FOCUS_INDUSTRY"
    FOCUS_RESEARCH = "FOCUS_RESEARCH"
    FOCUS_TRADE = "FOCUS_TRADE"
    FOCUS_CONSTRUCTION = "FOCUS_CONSTRUCTION"


class EmpireMeters(object):
    DETECTION_STRENGTH = "METER_DETECTION_STRENGTH"
