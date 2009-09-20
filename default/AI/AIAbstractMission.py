from AITarget import AITarget
import EnumsAI
from EnumsAI import AIMissionType

class AIAbstractMission(object):
    ""

    def __init__(self, aiMissionType, aiTargetType, aiTargetID):
        "constructor"

        aiTarget = AITarget(aiTargetType, aiTargetID)
        self.__aiTarget = aiTarget
        self.__aiMissionType = aiMissionType

        self.__aiMissionTypes = {}
        for __aiMissionType in self.getAnyAIMissionTypes():
            self.__aiMissionTypes[__aiMissionType] = []

    def getAITarget(self):
        "return mission AITarget"

        return self.__aiTarget

    def getAITargetID(self):
        "return id"

        return self.getAITarget().getTargetID()

    def getAITargetType(self):
        "return mission AITargetType"

        return self.getAITarget().getAITargetType()

    def getAIMissionType(self):
        "return AIMissionType"

        return self.__aiMissionType

    def getAnyAIMissionTypes(self):
        "return types of mission"

        if AIMissionType.FLEET_MISSION == self.getAIMissionType():
            return EnumsAI.getAIFleetMissionTypes()
        elif AIMissionType.EMPIRE_WAR_MISSION == self.getAIMissionType():
            return EnumsAI.getAIEmpireWarMissionTypes()

        return NotImplemented

    def addAITarget(self, aiMissionType, aiTarget):
        targets = self.getAITargets(aiMissionType)
        if not targets.__contains__(aiTarget):
            targets.append(aiTarget)

    def removeAITarget(self, aiMissionType, aiTarget):
        targets = self.getAITargets(aiMissionType)
        if targets.__contains__(aiTarget):
            result = []
            for target in targets:
                if aiTarget.__cmp__(target) != 0:
                    result.append(target)
            self.__aiMissionTypes[aiMissionType] = result

            del aiTarget

    def clearAITargets(self, aiMissionType):
        aiTargets = self.getAITargets(aiMissionType)
        for aiTarget in aiTargets:
            self.removeAITarget(aiMissionType, aiTarget)

    def getAITargets(self, aiMissionType):
        "getter"

        return self.__aiMissionTypes[aiMissionType]

    def hasTarget(self, aiMissionType, aiTarget):
        targets = self.getAITargets(aiMissionType)
        for target in targets:
            if target.__eq__(aiTarget):
                return True
        return False

    def getAIMissionTypes(self):
        result = []
        for aiMissionType in self.getAnyAIMissionTypes():
            aiTargets = self.getAITargets(aiMissionType)
            if len(aiTargets) > 0:
                result.append(aiMissionType)

        return result

    def hasAnyAIMissionTypes(self):
        aiMissionTypes = self.getAIMissionTypes()
        if len(aiMissionTypes) > 0:
            return True
        return False

    def hasAnyOfAIMissionTypes(self, wantedAIMissionTypes):
        aiMissionTypes = self.getAIMissionTypes()

        for wantedAIMissionType in wantedAIMissionTypes:
            if wantedAIMissionType in aiMissionTypes:
                return True
        return False

    def __cmp__(self, other):
        "compares AIMissions"

        if other == None:
            return False
        if self.getAITargetID() == other.getAITargetID():
            return True
        return False

    def __eq__(self, other):
        "returns equal to other object"

        if other == None:
            return False
        if self.getAIMissionType() == other.getAIMissionType() and self.getAITargetType() == self.getAITargetType():
            return self.__cmp__(other) == 0

        print "NOT IMPLEMENTED AIAbstractMission eq\n"
        return NotImplemented

    def __ne__(self, other):
        "returns not equal to other object"

        result = self.__eq__(other)
        if result is NotImplemented:
            print "NOT IMPLEMENTED AIAbstractMission ne\n"
            return result

        return not result
