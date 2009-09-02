from EnumsAI import AIFleetOrderType, AIFleetMissionType, AITargetType, AIMissionType
import MoveUtilsAI
import AITarget
import AIFleetOrder
import freeOrionAIInterface as fo
from AIAbstractMission import AIAbstractMission

class AIFleetMission(AIAbstractMission):
    '''
    Stores information about AI mission. Every mission has fleetID and AI targets depending upon AI fleet mission type.
    '''

    def __init__(self, fleetID):
        "constructor"

        AIAbstractMission.__init__(self, AIMissionType.FLEET_MISSION, AITargetType.TARGET_FLEET, fleetID)
        self.__aiFleetOrders = []

    def __str__(self):
        "returns describing string"

        result = ""
        for aiFleetMissionType in self.getAIMissionTypes():
            universe = fo.getUniverse()
            fleet = universe.getFleet(self.getAITargetID())
            targetsString = "fleet name:" + fleet.name + " id:" + str(self.getAITargetID()) + "[" + str(aiFleetMissionType) + "]:"
            targets = self.getAITargets(aiFleetMissionType)
            for target in targets:
                targetsString = targetsString + str(target)
            result = result + targetsString + "\n"
        return result

    def __getRequiredToVisitSystemAITargets(self):
        "returns all system AITargets required to visit in this object"

        result = []
        for aiFleetMissionType in self.getAIMissionTypes():
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
        fleetAITarget = AITarget.AITarget(AITargetType.TARGET_FLEET, self.getAITargetID())
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
            fleet = universe.getFleet(self.getAITargetID())
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

        allAIFleetMissionTypes = self.getAIMissionTypes()
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
                print "    " + str(aiFleetOrder)
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
        aiFleetOrdersToVisitSystems = MoveUtilsAI.getAIFleetOrdersFromSystemAITargets(self.getAITarget(), systemAITargets)

        # if fleet is in some system = fleet.systemID >=0, then also generate system AIFleetOrders
        universe = fo.getUniverse()
        fleet = universe.getFleet(self.getAITargetID())
        systemID = fleet.systemID
        if systemID >= 0:
            # system in where fleet is
            systemAITarget = AITarget.AITarget(AITargetType.TARGET_SYSTEM, systemID)
            # if mission aiTarget has required system where fleet is, then generate aiFleetOrder from this aiTarget 
            aiMissionTypes = self.getAIMissionTypes()
            # for all targets in all mission types get required systems to visit 
            for aiFleetMissionType in aiMissionTypes:
                aiTargets = self.getAITargets(aiFleetMissionType)
                for aiTarget in aiTargets:
                    if systemAITarget in aiTarget.getRequiredSystemAITargets():
                        # from target required to visit get fleet orders to accomplish target
                        aiFleetOrder = self.__getAIFleetOrderFromAITarget(aiFleetMissionType, aiTarget)
                        self.appendAIFleetOrder(aiFleetOrder)

        for aiFleetOrder in aiFleetOrdersToVisitSystems:
            self.appendAIFleetOrder(aiFleetOrder)

        # if fleet don't have any mission, then resupply if is current location not in supplyable system
        empire = fo.getEmpire()
        fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
        if (not self.hasAnyAIMissionTypes()) and not(self.getLocationAITarget().getTargetID() in fleetSupplyableSystemIDs):
            resupplyAIFleetOrder = MoveUtilsAI.getResupplyAIFleetOrder(self.getAITarget(), self.getLocationAITarget())
            if resupplyAIFleetOrder.isValid():
                self.appendAIFleetOrder(resupplyAIFleetOrder)

    def getLocationAITarget(self):
        "system AITarget where fleet is or will be"
        # TODO add parameter turn

        universe = fo.getUniverse()
        fleet = universe.getFleet(self.getAITargetID())
        systemID = fleet.systemID
        if systemID >= 0:
            return AITarget.AITarget(AITargetType.TARGET_SYSTEM, systemID)
        else:
            return AITarget.AITarget(AITargetType.TARGET_SYSTEM, fleet.nextSystemID)


def getFleetIDsFromAIFleetMissions(aiFleetMissions):
    result = []
    for aiFleetMission in aiFleetMissions:
        result.append(aiFleetMission.getMissionAITargetID())

    return result
