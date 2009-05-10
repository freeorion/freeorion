from EnumsAI import AITargetType
import freeOrionAIInterface as fo

class AITarget(object):
    "stores information about AI target - its id and type"

    def __init__(self, aiTargetType, targetID):
        "constructor"
        
        self.__aiTargetType__ = aiTargetType
        self.__targetID__ = targetID        

    def getTargetID(self):
        "getter"
        
        return self.__targetID__

    def getAITargetType(self):
        "getter"
        
        return self.__aiTargetType__

    def setTargetID(self, value):
        "setter"
        
        self.__targetID__ = value

    def setAITargetType(self, value):
        "setter"
        
        self.__aiTargetType__ = value
        
    def __cmp__(self, other):
        "compares AITargets"
        
        if self.getTargetID() < other.getTargetID():
            return -1
        elif self.getTargetID() == other.getTargetID():
            if self.getAITargetType() < other.getAITargetType():
                return -1
            elif self.getAITargetType() == other.getAITargetType():
                return 0
            return 1
        return 1
    
    def __eq__(self, other):
        "returns equal to other object"
        
        if other == None:
            return False
        if isinstance(other, AITarget):
            return self.__cmp__(other) == 0
        return NotImplemented
    
    def __ne__(self, other):
        "returns not equal to other object"
        
        result = self.__eq__(other)
        if result is NotImplemented:
            return result
        return not result
    
    def __str__(self):
        "returns describing string"
        
        return "{" + str(self.getAITargetType()) + ":" + str(self.getTargetID())+"}"
    
    def isValid(self):
        "returns if this object is valid"
        
        if self.getTargetID() == None or self.getAITargetType() == None:
            return False
        
        universe = fo.getUniverse()
        if AITargetType.TARGET_FLEET == self.getAITargetType():            
            fleet = universe.getFleet(self.getTargetID())
            if fleet == None:
                return False
            return True
        
        elif AITargetType.TARGET_SYSTEM == self.getAITargetType():
            system = universe.getSystem(self.getTargetID())
            if system == None:
                return False
            return True
        
        elif AITargetType.TARGET_PLANET == self.getAITargetType():
            planet = universe.getPlanet(self.getTargetID())
            if planet == None:
                return False
            return True
        
        elif AITargetType.TARGET_BUILDING == self.getAITargetType():
            building = universe.getBuilding(self.getTargetID())
            if building == None:
                return False
            return True
        
        elif AITargetType.TARGET_EMPIRE == self.getAITargetType():
            empireIDs = fo.AllEmpireIDs()
            if (empireIDs == None) or (not self.getTargetID() in empireIDs):
                return False
            return True
        
        return False
    
    def getRequiredSystemAITargets(self):
        "returns all system AITargets required to visit in this object"
        
        # TODO: add parameter turn
        
        result = []
        if AITargetType.TARGET_SYSTEM == self.getAITargetType():
            result.append(self)
        
        elif AITargetType.TARGET_PLANET == self.getAITargetType():
            universe = fo.getUniverse()
            planet = universe.getPlanet(self.getTargetID())
            aiTarget = AITarget(AITargetType.TARGET_SYSTEM, planet.systemID)
            
            result.append(aiTarget)
            
        elif AITargetType.TARGET_FLEET == self.getAITargetType():
            # Fleet systemID is where is fleet going.
            # If fleet is going nowhere, then it is location of fleet
            universe = fo.getUniverse()
            fleet = universe.getFleet(self.getTargetID())
            systemID = fleet.nextSystemID
            if (systemID == -1):
                systemID = fleet.systemID
            aiTarget = AITarget(AITargetType.TARGET_SYSTEM, systemID)
            
            result.append(aiTarget)
        
        return result
