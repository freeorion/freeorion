from AITarget import AITarget

class AIDemand(object):
    "encapsulate AI demand"

    def __init__(self, aiDemandType, aiTargetType, aiTargetID, amount):
        "constructor"

        aiTarget = AITarget(aiTargetType, aiTargetID)
        self.__aiTarget = aiTarget

        self.__aiDemandType = aiDemandType
        self.__amount = amount

    def getAIDemandType(self):
        "returns demand type"

        return self.__aiDemandType

    def getAITarget(self):
        "returns demand AITarget"

        return self.__aiTarget

    def getAmount(self):
        "returns amount"

        return self.__amount

    def setAmount(self, amount):
        "set amount"

        self.__amount = amount

