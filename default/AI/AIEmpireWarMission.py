from AIAbstractMission import AIAbstractMission
from EnumsAI import AITargetType, AIMissionType

class AIEmpireWarMission(AIAbstractMission):
    "Stores information about how to achieve war objectives of the empire via war"

    def __init__(self, empireID):
        "constructor"

        AIAbstractMission.__init__(self, AIMissionType.EMPIRE_WAR_MISSION, AITargetType.TARGET_EMPIRE, empireID)



