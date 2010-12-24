from EnumsAI import AIFleetOrderType, AITargetType, AIShipRoleType
import FleetUtilsAI
import freeOrionAIInterface as fo

class AIFleetOrder(object):
    "Stores information about orders which can be executed"

    def __init__(self, aiFleetOrderType, sourceAITarget, targetAITarget):
        "constructor"
        self.__aiFleetOrderType = aiFleetOrderType
        self.__sourceAITarget = sourceAITarget
        self.__targetAITarget = targetAITarget
        self.__executed = False
        self.__executionCompleted = False

    def getAIFleetOrderType(self):
        return self.__aiFleetOrderType

    def getSourceAITarget(self):
        return self.__sourceAITarget

    def getTargetAITarget(self):
        return self.__targetAITarget

    def isExecuted(self):
        return self.__executed

    def __setExecuted(self):
        self.__executed = True

    def isExecutionCompleted(self):
        return self.__executionCompleted

    def __setExecutionCompleted(self):
        self.__executionCompleted = True

    def __checkValidityShipInFleet(self, shipAITarget, fleetAITarget):
        shipID = shipAITarget.getTargetID()
        fleetID = fleetAITarget.getTargetID()
        universe = fo.getUniverse()
        fleet = universe.getFleet(fleetID)
        # ship is in fleet
        if (shipID in fleet.shipIDs):
            return True
        return False

    def isValid(self):
        "check if FleetOrder could be somehow in future issued = is valid"

        if (self.isExecuted() and self.isExecutionCompleted()):
            return False
        if self.getSourceAITarget().isValid() and self.getTargetAITarget().isValid():
            sourceAITargetTypeValid = False
            targetAITargetTypeValid = False
            universe = fo.getUniverse()

            # outpost
            if AIFleetOrderType.ORDER_OUTPOST == self.getAIFleetOrderType():
                # with ship
                if AITargetType.TARGET_SHIP == self.getSourceAITarget().getAITargetType():
                    ship = universe.getShip(self.getSourceAITarget().getTargetID())
                    if ship.canColonize:
                        sourceAITargetTypeValid = True
                # with fleet
                elif AITargetType.TARGET_FLEET == self.getSourceAITarget().getAITargetType():
                    fleet = universe.getFleet(self.getSourceAITarget().getTargetID())
                    if fleet.hasColonyShips:
                        sourceAITargetTypeValid = True
                # colonise planet
                if AITargetType.TARGET_PLANET == self.getTargetAITarget().getAITargetType():
                    planet = universe.getPlanet(self.getTargetAITarget().getTargetID())
                    if planet.unowned:
                        targetAITargetTypeValid = True
            # colonise
            elif AIFleetOrderType.ORDER_COLONISE == self.getAIFleetOrderType():
                # with ship
                if AITargetType.TARGET_SHIP == self.getSourceAITarget().getAITargetType():
                    ship = universe.getShip(self.getSourceAITarget().getTargetID())
                    if ship.canColonize:
                        sourceAITargetTypeValid = True
                # with fleet
                elif AITargetType.TARGET_FLEET == self.getSourceAITarget().getAITargetType():
                    fleet = universe.getFleet(self.getSourceAITarget().getTargetID())
                    if fleet.hasColonyShips:
                        sourceAITargetTypeValid = True
                # colonise planet
                if AITargetType.TARGET_PLANET == self.getTargetAITarget().getAITargetType():
                    planet = universe.getPlanet(self.getTargetAITarget().getTargetID())
                    if planet.unowned:
                        targetAITargetTypeValid = True
            # move
            elif AIFleetOrderType.ORDER_MOVE == self.getAIFleetOrderType():
                # with fleet
                if AITargetType.TARGET_FLEET == self.getSourceAITarget().getAITargetType():
                    sourceAITargetTypeValid = True
                # move to system
                if AITargetType.TARGET_SYSTEM == self.getTargetAITarget().getAITargetType():
                    targetAITargetTypeValid = True
            # resupply
            elif AIFleetOrderType.ORDER_RESUPPLY == self.getAIFleetOrderType():
                # with fleet
                if AITargetType.TARGET_FLEET == self.getSourceAITarget().getAITargetType():
                    sourceAITargetTypeValid = True
                # move to system
                if AITargetType.TARGET_SYSTEM == self.getTargetAITarget().getAITargetType():
                    empire = fo.getEmpire()
                    fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
                    if (self.getTargetAITarget().getTargetID() in fleetSupplyableSystemIDs):
                        targetAITargetTypeValid = True
            # split fleet
            elif AIFleetOrderType.ORDER_SPLIT_FLEET == self.getAIFleetOrderType():
                # with fleet
                if AITargetType.TARGET_FLEET == self.getSourceAITarget().getAITargetType():
                    sourceAITargetTypeValid = True
                # split ship
                if AITargetType.TARGET_SHIP == self.getTargetAITarget().getAITargetType():
                    targetAITargetTypeValid = True
                if sourceAITargetTypeValid == True and targetAITargetTypeValid == True:
                    if self.__checkValidityShipInFleet(self.getTargetAITarget(), self.getSourceAITarget()):
                        return True
            elif AIFleetOrderType.ORDER_ATACK == self.getAIFleetOrderType():
                # with fleet
                if AITargetType.TARGET_FLEET == self.getSourceAITarget().getAITargetType():
                    sourceAITargetTypeValid = True
                # move to system
                if AITargetType.TARGET_SYSTEM == self.getTargetAITarget().getAITargetType() or AITargetType.TARGET_PLANET == self.getTargetAITarget().getAITargetType():
                    targetAITargetTypeValid = True

            if sourceAITargetTypeValid == True and targetAITargetTypeValid == True:
                return True

        return False

    def canIssueOrder(self):
        "if FleetOrder can be issued now"

        if self.isExecuted():
            return False
        if not self.isValid():
            return False

        universe = fo.getUniverse()
        # outpost
        if AIFleetOrderType.ORDER_OUTPOST == self.getAIFleetOrderType():
            fleetID = None
            shipID = None
            if AITargetType.TARGET_SHIP == self.getSourceAITarget().getAITargetType():
                shipID = self.getSourceAITarget().getTargetID()
                ship = universe.getShip(shipID)
                fleetID = ship.fleetID
            elif AITargetType.TARGET_FLEET == self.getSourceAITarget().getAITargetType():
                fleetID = self.getSourceAITarget().getTargetID()
                shipID = FleetUtilsAI.getShipIDWithRole(fleetID, AIShipRoleType.SHIP_ROLE_CIVILIAN_OUTPOST)

            ship = universe.getShip(shipID)
            fleet = universe.getFleet(fleetID)
            planet = universe.getPlanet(self.getTargetAITarget().getTargetID())
            if (ship != None) and (fleet.systemID == planet.systemID) and ship.canColonize:
                return True
            return False
        # colonise
        elif AIFleetOrderType.ORDER_COLONISE == self.getAIFleetOrderType():
            fleetID = None
            shipID = None
            if AITargetType.TARGET_SHIP == self.getSourceAITarget().getAITargetType():
                shipID = self.getSourceAITarget().getTargetID()
                ship = universe.getShip(shipID)
                fleetID = ship.fleetID
            elif AITargetType.TARGET_FLEET == self.getSourceAITarget().getAITargetType():
                fleetID = self.getSourceAITarget().getTargetID()
                shipID = FleetUtilsAI.getShipIDWithRole(fleetID, AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION)

            ship = universe.getShip(shipID)
            fleet = universe.getFleet(fleetID)
            planet = universe.getPlanet(self.getTargetAITarget().getTargetID())
            if (ship != None) and (fleet.systemID == planet.systemID) and ship.canColonize:
                return True
            return False
        # split fleet            
        elif AIFleetOrderType.ORDER_SPLIT_FLEET == self.getAIFleetOrderType():
            fleet = universe.getFleet(self.getSourceAITarget().getTargetID())
            if len(fleet.shipIDs) <= 1:
                return False

        return True

    def issueOrder(self):
        if not self.canIssueOrder():
            print "can't issue " + self
        else:
            self.__setExecuted()

            # outpost
            if AIFleetOrderType.ORDER_OUTPOST == self.getAIFleetOrderType():
                shipID = None
                if AITargetType.TARGET_SHIP == self.getSourceAITarget().getAITargetType():
                    shipID = self.getSourceAITarget().getTargetID()
                elif AITargetType.TARGET_FLEET == self.getSourceAITarget().getAITargetType():
                    fleetID = self.getSourceAITarget().getTargetID()
                    shipID = FleetUtilsAI.getShipIDWithRole(fleetID, AIShipRoleType.SHIP_ROLE_CIVILIAN_OUTPOST)

                fo.issueColonizeOrder(shipID, self.getTargetAITarget().getTargetID())
            # colonise
            elif AIFleetOrderType.ORDER_COLONISE == self.getAIFleetOrderType():
                shipID = None
                if AITargetType.TARGET_SHIP == self.getSourceAITarget().getAITargetType():
                    shipID = self.getSourceAITarget().getTargetID()
                elif AITargetType.TARGET_FLEET == self.getSourceAITarget().getAITargetType():
                    fleetID = self.getSourceAITarget().getTargetID()
                    shipID = FleetUtilsAI.getShipIDWithRole(fleetID, AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION)

                fo.issueColonizeOrder(shipID, self.getTargetAITarget().getTargetID())
            # move or resupply
            elif (AIFleetOrderType.ORDER_MOVE == self.getAIFleetOrderType()) or (AIFleetOrderType.ORDER_RESUPPLY == self.getAIFleetOrderType()):
                fleetID = self.getSourceAITarget().getTargetID()
                systemID = self.getTargetAITarget().getTargetID()

                fo.issueFleetMoveOrder(fleetID, systemID)
            # split fleet
            elif AIFleetOrderType.ORDER_SPLIT_FLEET == self.getAIFleetOrderType():
                fleetID = self.getSourceAITarget().getTargetID()
                shipID = self.getTargetAITarget().getTargetID()

                fo.issueNewFleetOrder(str(shipID), shipID)
                self.__setExecutionCompleted()
            elif (AIFleetOrderType.ORDER_ATACK == self.getAIFleetOrderType()):
                fleetID = self.getSourceAITarget().getTargetID()
                systemID = self.getTargetAITarget().getRequiredSystemAITargets()[0].getTargetID()

                fo.issueFleetMoveOrder(fleetID, systemID)

    def __str__(self):
        "returns describing string"

        return "fleet order[" + str(self.getAIFleetOrderType()) + "] source:" + str(self.getSourceAITarget()) + " target:" + str(self.getTargetAITarget())

    def __cmp__(self, other):
        "compares AIFleetOrders"

        if other == None:
            return False
        if self.getAIFleetOrderType() < other.getAIFleetOrderType():
            return - 1
        elif self.getAIFleetOrderType() == other.getAIFleetOrderType():
            result = self.getSourceAITarget().__cmp__(other.getSourceAITarget())
            if result == 0:
                result = self.getTargetAITarget().__cmp__(other.getTargetAITarget())
                return result
            else:
                return result
        return 1

    def __eq__(self, other):
        "returns equal to other object"

        if other == None:
            return False
        if isinstance(other, AIFleetOrder):
            return self.__cmp__(other) == 0
        return NotImplemented

    def __ne__(self, other):
        "returns not equal to other object"

        result = self.__eq__(other)
        if result is NotImplemented:
            return result
        return not result
