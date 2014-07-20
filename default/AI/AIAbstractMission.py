from AITarget import AITarget
import EnumsAI
from EnumsAI import AIMissionType


class AIAbstractMission(object):
    def __init__(self, mission_type, target_type, target_id):
        target = AITarget(target_type, target_id)
        self.__aiTarget = target
        self.__aiMissionType = mission_type

        self.__aiMissionTypes = {}
        for __aiMissionType in self.get_any_mission_types():
            self.__aiMissionTypes[__aiMissionType] = []

    def get_target(self):
        """return mission AITarget"""

        return self.__aiTarget

    @property
    def target_id(self):
        """return id"""

        return self.get_target().target_id

    @property
    def target_type(self):
        """return mission AITargetType"""

        return self.get_target().target_type

    def get_mission_type(self):
        """return AIMissionType"""

        return self.__aiMissionType

    def get_any_mission_types(self):
        """return types of mission"""

        if AIMissionType.FLEET_MISSION == self.get_mission_type():
            return EnumsAI.get_fleet_mission_types()
        elif AIMissionType.EMPIRE_WAR_MISSION == self.get_mission_type():
            return EnumsAI.get_empire_war_mission_types()

        return NotImplemented

    def add_target(self, mission_type, target):
        targets = self.get_targets(mission_type)
        if not target in targets:
            targets.append(target)

    def remove_target(self, mission_type, target):
        targets = self.get_targets(mission_type)
        if target in targets:
            targets.remove(target)

    def clear_targets(self, mission_type):
        if mission_type == -1:
            targets = []
            for mission_type in self.get_mission_types():
                targets.extend(self.get_targets(mission_type))
        else:
            targets = self.get_targets(mission_type)
        for target in targets:
            self.remove_target(mission_type, target)

    def get_targets(self, mission_type):
        return self.__aiMissionTypes.get(mission_type, [])

    def has_target(self, mission_type, target):
        return target in self.get_targets(mission_type)

    def get_mission_types(self):
        return [mission_type for mission_type in self.get_any_mission_types() if self.get_targets(mission_type)]

    def has_any_mission_types(self):
        return bool(self.get_mission_types())

    def has_any_of_mission_types(self, wanted_mission_types):
        mission_types = self.get_mission_types()
        for wanted_mission_type in wanted_mission_types:
            if wanted_mission_type in mission_types:
                return True
        return False

    def __cmp__(self, other):
        """compares AIMissions"""

        if other is None:
            return False
        if self.target_id == other.target_id:
            return True
        return False

    def __eq__(self, other):
        """returns equal to other object"""

        if other is None:
            return False
        if self.get_mission_type() == other.get_mission_type() and self.target_type == self.target_type:
            return self.__cmp__(other) == 0

        print "NOT IMPLEMENTED AIAbstractMission eq\n"
        return NotImplemented

    def __ne__(self, other):
        """returns not equal to other object"""

        result = self.__eq__(other)
        if result is NotImplemented:
            print "NOT IMPLEMENTED AIAbstractMission ne\n"
            return result

        return not result
