import EnumsAI
from EnumsAI import AIFleetOrderType, AIFleetMissionType, AITargetType
import MoveUtilsAI
import AITarget
import AIFleetOrder
import freeOrionAIInterface as fo

class AIFleetMission(object):
    '''
    Stores information about AI mission. Every mission has fleetID and AI targets depending upon AI fleet mission type.
    '''
    
    def __init__(self, fleetID):
        "constructor"
        
        self.__fleetID = fleetID
        self.__aiFleetOrders = []        
        self.__aiFleetMissionTypes = {}
        for aiFleetMissionType in EnumsAI.getAIFleetMissionTypes():
            self.__aiFleetMissionTypes[aiFleetMissionType] = []
            
    def getFleetID(self):
        "getter"
        
        return self.__fleetID        
    
    def addAITarget(self, aiFleetMissionType, aiTarget):
        targets = self.getAITargets(aiFleetMissionType)
        if not targets.__contains__(aiTarget):
            targets.append(aiTarget)
    
    def removeAITarget(self, aiFleetMissionType, aiTarget):
        targets = self.getAITargets(aiFleetMissionType)
        if targets.__contains__(aiTarget):
            result = []
            for target in targets:
                if aiTarget.__cmp__(target) !=0:
                    result.append(target)
            self.__aiFleetMissionTypes[aiFleetMissionType] = result
            
            del aiTarget
    
    def clearAITargets(self, aiFleetMissionType):
        aiTargets = self.getAITargets(aiFleetMissionType)
        for aiTarget in aiTargets:
            self.removeAITarget(aiFleetMissionType, aiTarget)
    
    def getAITargets(self, aiFleetMissionType):
        "getter"
        
        return self.__aiFleetMissionTypes[aiFleetMissionType]
    
    def hasTarget(self, aiFleetMissionType, aiTarget):
        targets = self.getAITargets(aiFleetMissionType)
        for target in targets:
            if target.__eq__(aiTarget):
                return True
        return False
    
    def getAIFleetMissionTypes(self):
        result = []
        for aiFleetMissionType in EnumsAI.getAIFleetMissionTypes():
            aiTargets = self.getAITargets(aiFleetMissionType)
            if len(aiTargets) > 0:
                result.append(aiFleetMissionType)
        
        return result
    
    def hasAnyAIFleetMissionTypes(self):
        aiFleetMissionTypes = self.getAIFleetMissionTypes()
        if len(aiFleetMissionTypes) > 0:
            return True
        return False
    
    def hasAnyOfAIFleetMissionTypes(self, wantedAIFleetMissionTypes):
        aiFleetMissionTypes = self.getAIFleetMissionTypes()
        
        for wantedAIFleetMissionType in wantedAIFleetMissionTypes:
            if wantedAIFleetMissionType in aiFleetMissionTypes:
                return True
        return False
    
    def __cmp__(self, other):
        "compares AIFleetMissions"
        
        if other == None:
            return False
        if self.getFleetID() == other.getFleetID():
            return True
        return False
    
    def __eq__(self, other):
        "returns equal to other object"
        
        if other == None:
            return False
        if isinstance(other, AIFleetMission):
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
         
        result = ""
        for aiFleetMissionType in self.getAIFleetMissionTypes():
            universe = fo.getUniverse()
            fleet = universe.getFleet(self.getFleetID())
            targetsString = "fleet name:" + fleet.name + " id:"+ str(self.getFleetID()) + "[" + str(aiFleetMissionType) + "]:" 
            targets = self.getAITargets(aiFleetMissionType)
            for target in targets:
                targetsString = targetsString + str(target) 
            result = result + targetsString + "\n"
        return result
    
    def __getRequiredToVisitSystemAITargets(self):
        "returns all system AITargets required to visit in this object"
        
        result = []
        for aiFleetMissionType in self.getAIFleetMissionTypes():
            aiTargets = self.getAITargets(aiFleetMissionType)
            for aiTarget in aiTargets:
                result.extend(aiTarget.getRequiredSystemAITargets())
        
        return result
    
    def getVisitingSystemAITargets(self):
        "returns all system AITargets which will be visited"
        
        result = []
        for aiFleetOrder in self.getAIFleetOrders():
            if aiFleetOrder.getAIFleetOrderType() == AIFleetOrderType.ORDER_MOVE:
                result.append(aiFleetOrder.getTargetAITarget())
        return result
    
    def getAIFleetOrders(self):
        return self.__aiFleetOrders
    
    def appendAIFleetOrder(self, aiFleetOrder):
        self.__aiFleetOrders.append(aiFleetOrder)
        
    def hasAIFleetOrder(self, aiFleetOrder):
        aiFleetOrders = self.getAIFleetOrders()
        return aiFleetOrders.__contains__(aiFleetOrder)
        
    def removeAIFleetOrder(self, aiFleetOrder):
        result = []
        for fleetOrder in self.__aiFleetOrders: 
            if fleetOrder.__cmp__(aiFleetOrder) != 0:
                result.append(fleetOrder)
        self.__aiFleetOrders = result
        
        del aiFleetOrder
        
    def clearAIFleetOrders(self):
        self.__aiFleetOrders = []
        
    def __getAIFleetOrderFromAITarget(self, aiFleetMissionType, aiTarget):
        result = None
        fleetAITarget = AITarget.AITarget(AITargetType.TARGET_FLEET, self.getFleetID())
        if aiFleetMissionType == AIFleetMissionType.FLEET_MISSION_COLONISATION:
            result = AIFleetOrder.AIFleetOrder(AIFleetOrderType.ORDER_COLONISE, fleetAITarget, aiTarget)
        # TODO: implement other mission types
                     
        return result
    
    def isValidFleetMissionAITarget(self, aiFleetMissionType, aiTarget):
        if aiTarget.isValid() == False:
            return False
        
        if aiFleetMissionType == AIFleetMissionType.FLEET_MISSION_EXPLORATION:
            if aiTarget.getAITargetType() == AITargetType.TARGET_SYSTEM:
                empire = fo.getEmpire()
                if not empire.hasExploredSystem(aiTarget.getTargetID()):
                    return True
        elif aiFleetMissionType == AIFleetMissionType.FLEET_MISSION_COLONISATION:
            universe = fo.getUniverse()
            fleet = universe.getFleet(self.getFleetID())
            if not fleet.hasColonyShips:
                return False
            if aiTarget.getAITargetType() == AITargetType.TARGET_PLANET:                
                planet = universe.getPlanet(aiTarget.getTargetID())
                if planet.unowned:
                    return True
        
        # TODO: implement other mission types            
        return False
    
    def cleanInvalidAITargets(self):
        "clean invalid AITargets"
        
        allAIFleetMissionTypes = self.getAIFleetMissionTypes()
        for aiFleetMissionType in allAIFleetMissionTypes:
            allAITargets = self.getAITargets(aiFleetMissionType)
            for aiTarget in allAITargets:
                if not self.isValidFleetMissionAITarget(aiFleetMissionType, aiTarget):
                    self.removeAITarget(aiFleetMissionType, aiTarget)
            
    def issueAIFleetOrders(self):
        "issues AIFleetOrders which can be issued in system and moves to next one if is possible"
        
        # TODO: priority
        ordersInSystemCompleted = True
        for aiFleetOrder in self.getAIFleetOrders():
            if aiFleetOrder.canIssueOrder():
                print "issuing AIFleetOrders: " + str(aiFleetOrder)
                if aiFleetOrder.getAIFleetOrderType() == AIFleetOrderType.ORDER_MOVE and ordersInSystemCompleted:
                    aiFleetOrder.issueOrder()
                elif aiFleetOrder.getAIFleetOrderType() != AIFleetOrderType.ORDER_MOVE:
                    aiFleetOrder.issueOrder()
                if not aiFleetOrder.isExecutionCompleted():
                    ordersInSystemCompleted = False
            # moving to another system stops issuing all orders in system where fleet is
            # move order is also the last order in system
            if aiFleetOrder.getAIFleetOrderType() == AIFleetOrderType.ORDER_MOVE:
                break
                
    def generateAIFleetOrders(self):
        "generates AIFleetOrders from fleets targets to accomplish"
        
        # TODO: priority
        self.clearAIFleetOrders()
        # for some targets fleet has to visit systems and therefore fleet visit them
        systemAITargets = self.__getRequiredToVisitSystemAITargets()
        aiFleetOrdersToVisitSystems = MoveUtilsAI.getAIFleetOrdersFromSystemAITargets(self.getFleetID(), systemAITargets)
        
        # if fleet is in some system = fleet.systemID >=0, then also generate system AIFleetOrders
        universe = fo.getUniverse()
        fleet = universe.getFleet(self.getFleetID())
        systemID = fleet.systemID        
        if systemID >=0:
            # system in where fleet is
            systemAITarget = AITarget.AITarget(AITargetType.TARGET_SYSTEM, systemID)
            # if mission aiTarget has required system where fleet is, then generate aiFleetOrder from this aiTarget 
            aiMissionTypes = self.getAIFleetMissionTypes()
            for aiFleetMissionType in aiMissionTypes:
                aiTargets = self.getAITargets(aiFleetMissionType)
                for aiTarget in aiTargets:
                    if systemAITarget in aiTarget.getRequiredSystemAITargets():
                        aiFleetOrder = self.__getAIFleetOrderFromAITarget(aiFleetMissionType, aiTarget)
                        self.appendAIFleetOrder(aiFleetOrder)
            
        for aiFleetOrder in aiFleetOrdersToVisitSystems:
            self.appendAIFleetOrder(aiFleetOrder)
            
    def getLocationAITarget(self):
        "system AITarget where fleet is or will be"
        # TODO add parameter turn
        
        universe = fo.getUniverse()
        fleet = universe.getFleet(self.getFleetID())
        systemID = fleet.systemID
        if systemID >=0:
            return AITarget.AITarget(AITargetType.TARGET_SYSTEM, systemID)
        else:
            return AITarget.AITarget(AITargetType.TARGET_SYSTEM, fleet.nextSystemID)
        
        
def getFleetIDsFromAIFleetMissions(aiFleetMissions):
    result = []
    for aiFleetMission in aiFleetMissions:
        result.append(aiFleetMission.getFleetID())
        
    return result
