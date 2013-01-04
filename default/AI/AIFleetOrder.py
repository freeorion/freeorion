from EnumsAI import AIFleetOrderType, AITargetType, AIShipRoleType,  AIFleetMissionType
import FleetUtilsAI
import freeOrionAIInterface as fo
import FreeOrionAI as foAI
import ExplorationAI

AIFleetOrderTypeNames=AIFleetOrderType()
AIFleetMissionTypeNames = AIFleetMissionType()

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
                    if fleet.hasOutpostShips:
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
                    if planet.unowned or  (planet.ownedBy(fo.empireID()) and   planet.currentMeterValue(fo.meterType.population)==0 ):
                        targetAITargetTypeValid = True
            # invade
            elif AIFleetOrderType.ORDER_INVADE == self.getAIFleetOrderType():
                # with ship
                if AITargetType.TARGET_SHIP == self.getSourceAITarget().getAITargetType():
                    ship = universe.getShip(self.getSourceAITarget().getTargetID())
                    if ship.canInvade:
                        sourceAITargetTypeValid = True
                # with fleet
                elif AITargetType.TARGET_FLEET == self.getSourceAITarget().getAITargetType():
                    fleet = universe.getFleet(self.getSourceAITarget().getTargetID())
                    if fleet.hasTroopShips:
                        sourceAITargetTypeValid = True
                # invade planet
                if AITargetType.TARGET_PLANET == self.getTargetAITarget().getAITargetType():
                    planet = universe.getPlanet(self.getTargetAITarget().getTargetID())
                    planetPopulation = planet.currentMeterValue(fo.meterType.population)
                    if not planet.unowned or planetPopulation > 0:
                        targetAITargetTypeValid = True
                # military
                elif AIFleetOrderType.ORDER_MILITARY == self.getAIFleetOrderType():
                    # with ship
                    if AITargetType.TARGET_SHIP == self.getSourceAITarget().getAITargetType():
                        ship = universe.getShip(self.getSourceAITarget().getTargetID())
                        if ship.isArmed:
                            sourceAITargetTypeValid = True
                # with fleet
                elif AITargetType.TARGET_FLEET == self.getSourceAITarget().getAITargetType():
                    fleet = universe.getFleet(self.getSourceAITarget().getTargetID())
                    if fleet.hasArmedShips:
                        sourceAITargetTypeValid = True
                # military system
                if AITargetType.TARGET_SYSTEM == self.getTargetAITarget().getAITargetType():
                    system = universe.getSystem(self.getTargetAITarget().getTargetID())
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
            elif AIFleetOrderType.ORDER_ATTACK == self.getAIFleetOrderType():
                # with fleet
                if AITargetType.TARGET_FLEET == self.getSourceAITarget().getAITargetType():
                    sourceAITargetTypeValid = True
                # move to system
                if AITargetType.TARGET_SYSTEM == self.getTargetAITarget().getAITargetType() or AITargetType.TARGET_PLANET == self.getTargetAITarget().getAITargetType():
                    targetAITargetTypeValid = True

            if sourceAITargetTypeValid == True and targetAITargetTypeValid == True:
                return True

        return False
    
    def canIssueOrder(self,  considerMergers=False,  verbose=False):
        "if FleetOrder can be issued now"

        if self.isExecuted():
            return False
        if not self.isValid():
            return False

        reason = ""
        universe = fo.getUniverse()
        fleetID = None
        shipID = None
        if AITargetType.TARGET_SHIP == self.getSourceAITarget().getAITargetType():
            shipID = self.getSourceAITarget().getTargetID()
            ship = universe.getShip(shipID)
            fleetID = ship.fleetID
        elif AITargetType.TARGET_FLEET == self.getSourceAITarget().getAITargetType():
            fleetID = self.getSourceAITarget().getTargetID()
        fleet = universe.getFleet(fleetID)

        systemID = fleet.systemID
        sys1=universe.getSystem(systemID)
        sysName = (sys1 and sys1.name) or "unknown"
        targetID = self.getTargetAITarget().getTargetID()

        if verbose:
            mainFleetMission=foAI.foAIstate.getAIFleetMission(fleetID)
            mainMissionType = (mainFleetMission.getAIMissionTypes() + [-1])[0]
            msgP1 = "** %s  -- Mission Type  %s , current loc sys %d  - %s"%(  self,   AIFleetMissionTypeNames.name(mainMissionType),  systemID,  sysName )
            print msgP1 

        #
        # outpost
        #
        if AIFleetOrderType.ORDER_OUTPOST == self.getAIFleetOrderType():
            if AITargetType.TARGET_FLEET == self.getSourceAITarget().getAITargetType():
                shipID = FleetUtilsAI.getShipIDWithRole(fleetID, AIShipRoleType.SHIP_ROLE_CIVILIAN_OUTPOST)
            ship = universe.getShip(shipID)
            planet = universe.getPlanet(self.getTargetAITarget().getTargetID())
            if (ship != None) and (fleet.systemID == planet.systemID) and ship.canColonize:
                return True
            return False
        #
        # colonise
        #
        elif AIFleetOrderType.ORDER_COLONISE == self.getAIFleetOrderType():
            if AITargetType.TARGET_FLEET == self.getSourceAITarget().getAITargetType():
                shipID = FleetUtilsAI.getShipIDWithRole(fleetID, AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION)
            ship = universe.getShip(shipID)
            planet = universe.getPlanet(self.getTargetAITarget().getTargetID())
            if ship and not ship.canColonize:
                print "Error: colonization fleet %d has no colony ship"%fleetID
            if (ship != None) and (fleet.systemID == planet.systemID) and ship.canColonize:
                return True
            return False
        #
        # invade
        #
        elif AIFleetOrderType.ORDER_INVADE == self.getAIFleetOrderType():
            if AITargetType.TARGET_FLEET == self.getSourceAITarget().getAITargetType():
                shipID = FleetUtilsAI.getShipIDWithRole(fleetID, AIShipRoleType.SHIP_ROLE_MILITARY_INVASION)
            ship = universe.getShip(shipID)
            planet = universe.getPlanet(self.getTargetAITarget().getTargetID())
            if (ship != None) and (fleet.systemID == planet.systemID) and ship.canInvade:
                return True
            return False
        #
        # military
        #
        elif AIFleetOrderType.ORDER_MILITARY == self.getAIFleetOrderType():
            if AITargetType.TARGET_FLEET == self.getSourceAITarget().getAITargetType():
                shipID = FleetUtilsAI.getShipIDWithRole(fleetID, AIShipRoleType.SHIP_ROLE_MILITARY)
            ship = universe.getShip(shipID)
            system = universe.getSystem(self.getTargetAITarget().getTargetID())
            if (ship != None) and (fleet.systemID == system.systemID) and ship.isArmed:
                return True
            return False
        #
        # split fleet 
        #
        elif AIFleetOrderType.ORDER_SPLIT_FLEET == self.getAIFleetOrderType():
            fleet2 = universe.getFleet(self.getSourceAITarget().getTargetID())
            if len(fleet2.shipIDs) <= 1:
                return False
        #
        # move -- have fleets will do a safety check, also check for potential military fleet mergers
        #
        elif AIFleetOrderType.ORDER_MOVE == self.getAIFleetOrderType():
            #targetID = self.getTargetAITarget().getTargetID()
            #TODO: figure out better way to have invasions (& possibly colonizations) require visibility on target without needing visibility of all intermediate systems
            if False and mainMissionType not in [  AIFleetMissionType.FLEET_MISSION_ATTACK,     #TODO: consider this later
                                                                            AIFleetMissionType.FLEET_MISSION_MILITARY, 
                                                                            AIFleetMissionType.FLEET_MISSION_SECURE, 
                                                                            AIFleetMissionType.FLEET_MISSION_HIT_AND_RUN, 
                                                                            AIFleetMissionType.FLEET_MISSION_EXPLORATION, 
                                                                        ]:
                if  not (universe.getVisibility(targetID,  foAI.foAIstate.empireID) >= fo.visibility.partial):
                    #if not targetID in  interior systems
                    foAI.foAIstate.needsEmergencyExploration.append(fleet.systemID)
                    return False

            systemID = fleet.systemID
            sys1=universe.getSystem(systemID)
            sys1Name = (sys1 and sys1.name) or "unknown"
            targ1 = universe.getSystem(targetID)
            targ1Name = (targ1 and targ1.name) or "unknown"
            fleetRating = foAI.foAIstate.getRating(fleetID).get('overall', 0)
            threat = foAI.foAIstate.systemStatus.get(targetID,  {}).get('fleetThreat',  0) + foAI.foAIstate.systemStatus.get(targetID,  {}).get('planetThreat',  0)

            safetyFactor = 1.0
            if fleetRating >= safetyFactor* threat:
                return True
            else:
                #following line was poor because AIstate.militaryFleetIDs only covers fleets without current missions
                #myOtherFleetsRating =   sum([foAI.foAIstate.fleetStatus.get(fleetID, {}).get('rating', 0)  for fleetID in foAI.foAIstate.militaryFleetIDs   if ( foAI.foAIstate.fleetStatus.get(fleetID,  {}).get('sysID',  -1) == thisSystemID ) ])
                myOtherFleetsRatings =   [foAI.foAIstate.fleetStatus.get(fid, {}).get('rating', {})  for fid in foAI.foAIstate.systemStatus.get( targetID, {}).get('myfleets', [])  ]
                #myOtherFleetsRating =   sum([foAI.foAIstate.fleetStatus.get(fid, {}).get('rating', 0)  for fid in foAI.foAIstate.systemStatus.get( targetID, {}).get('myfleets', [])  ])
                myOtherFleetsRating =   foAI.foAIstate.systemStatus.get( targetID, {}).get('myFleetRating', 0) 
                if  (myOtherFleetsRating > safetyFactor* threat) or (myOtherFleetsRating + fleetRating  > 1.5*safetyFactor*threat):
                    if verbose:
                        print "\tAdvancing fleet %d (rating %d) at system %d (%s) into system %d (%s) with threat %d because of sufficient empire fleet strength already at desination"%(fleetID,  fleetRating,  systemID,  sys1Name,  targetID,  targ1Name,  threat)
                    return True
                else:
                    if verbose:
                        print "\tHolding fleet %d (rating %d) at system %d (%s) before travelling to system %d (%s) with threat %d"%(fleetID,  fleetRating,  systemID,  sys1Name,  targetID,  targ1Name,  threat)
                    return False
        else:  # default returns true
            return True

    def issueOrder(self,  considerMergers=False):
        if not self.canIssueOrder(considerMergers=False):  #appears to be redundant with check in IAFleetMission?
            print "\tcan't issue %s"%self
        else:
            universe=fo.getUniverse()
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

                planetID = self.getTargetAITarget().getTargetID()
                planet=universe.getPlanet(planetID)
                planetName = (planet and planet.name) or "apparently nvisible"
                result = fo.issueColonizeOrder(shipID, planetID)
                print "Ordered colony ship ID %d to colonize %s, got result %d"%(shipID, planetName,  result)
            # invade
            elif AIFleetOrderType.ORDER_INVADE == self.getAIFleetOrderType():
                result = False
                shipID = None
                planetID = self.getTargetAITarget().getTargetID()
                planet=universe.getPlanet(planetID)
                planetName = (planet and planet.name) or "invisible"
                if AITargetType.TARGET_SHIP == self.getSourceAITarget().getAITargetType():
                    shipID = self.getSourceAITarget().getTargetID()
                elif AITargetType.TARGET_FLEET == self.getSourceAITarget().getAITargetType():
                    fleetID = self.getSourceAITarget().getTargetID()
                    fleet = fo.getUniverse().getFleet(fleetID)
                    for shipID in fleet.shipIDs:
                        ship = universe.getShip(shipID)
                        if (foAI.foAIstate.getShipRole(ship.design.id) == AIShipRoleType.SHIP_ROLE_MILITARY_INVASION):
                            result = fo.issueInvadeOrder(shipID, planetID)  or  result #will track if at least one invasion troops successfully deployed
                            print "Ordered troop ship ID %d to invade %s, got result %d"%(shipID, planetName,  result)
                            if result == 0:
                                if 'needsEmergencyExploration' not in dir(foAI.foAIstate):
                                    foAI.foAIstate.needsEmergencyExploration=[]
                                foAI.foAIstate.needsEmergencyExploration.append(fleet.systemID)
                                print "Due to trouble invading,  adding system %d to Emergency Exploration List"%fleet.systemID
            # military
            elif AIFleetOrderType.ORDER_MILITARY == self.getAIFleetOrderType():
                shipID = None
                if AITargetType.TARGET_SHIP == self.getSourceAITarget().getAITargetType():
                    shipID = self.getSourceAITarget().getTargetID()
                elif AITargetType.TARGET_FLEET == self.getSourceAITarget().getAITargetType():
                    fleetID = self.getSourceAITarget().getTargetID()
                    shipID = FleetUtilsAI.getShipIDWithRole(fleetID, AIShipRoleType.SHIP_ROLE_MILITARY)
                #fo.issueFleetMoveOrder(fleetID, self.getTargetAITarget().getTargetID()) #moving is already taken care of separately
                targetSysID = self.getTargetAITarget().getTargetID()
                fleet = fo.getUniverse().getFleet(fleetID)
                systemStatus =  foAI.foAIstate.systemStatus.get(targetSysID,  {})
                if (fleet )and ( fleet.systemID==targetSysID ) and ((systemStatus.get('fleetThreat', 0) + systemStatus.get('planetThreat', 0))==0):
                    self.__setExecutionCompleted()

            # move or resupply
            elif (AIFleetOrderType.ORDER_MOVE == self.getAIFleetOrderType()) or (AIFleetOrderType.ORDER_RESUPPLY == self.getAIFleetOrderType()):
                fleetID = self.getSourceAITarget().getTargetID()
                systemID = self.getTargetAITarget().getTargetID()
                fleet = fo.getUniverse().getFleet(fleetID)
                if  systemID not in [fleet.systemID,  fleet.nextSystemID] :
                    fo.issueFleetMoveOrder(fleetID, systemID)
                if systemID == fleet.systemID:
                    if  fleetID in ExplorationAI.currentScoutFleetIDs:
                        if 'needsEmergencyExploration' not in dir(foAI.foAIstate):
                            foAI.foAIstate.needsEmergencyExploration=[]
                        if systemID in foAI.foAIstate.needsEmergencyExploration:
                            del foAI.foAIstate.needsEmergencyExploration[ foAI.foAIstate.needsEmergencyExploration.index(systemID) ]
                    self.__setExecutionCompleted()

            # split fleet
            elif AIFleetOrderType.ORDER_SPLIT_FLEET == self.getAIFleetOrderType():
                fleetID = self.getSourceAITarget().getTargetID()
                shipID = self.getTargetAITarget().getTargetID()
                fleet = fo.getUniverse().getFleet(fleetID)
                if shipID in fleet.shipIDs:
                    fo.issueNewFleetOrder(str(shipID), shipID)
                self.__setExecutionCompleted()
            # attack
            elif (AIFleetOrderType.ORDER_ATTACK == self.getAIFleetOrderType()):
                fleetID = self.getSourceAITarget().getTargetID()
                systemID = self.getTargetAITarget().getRequiredSystemAITargets()[0].getTargetID()

                fo.issueFleetMoveOrder(fleetID, systemID)

    def __str__(self):
        "returns describing string"

        return "fleet order[%s] source:%26s | target %26s" %(  AIFleetOrderTypeNames.name(self.__aiFleetOrderType),  self.__sourceAITarget,   self.__targetAITarget)

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
