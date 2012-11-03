from EnumsAI import AIFleetOrderType, AITargetType, AIShipRoleType,  AIFleetMissionType
import FleetUtilsAI
import freeOrionAIInterface as fo
import FreeOrionAI as foAI

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
                    else:
                        print "Fleet outpost order fails due to ship %d faile  ship.canColonize"%ship.id
                        print "from source target %s and targetTarget %s"%(self.__sourceAITarget ,  self.__targetAITarget) 
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
                    if planet.unowned:
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

    def mergeFleetAintoB(self,  fID,  fleetID):
        universe = fo.getUniverse()
        fleetA = universe.getFleet(fID)
        if not fleetA:
            return
        success = True
        for shipID in fleetA.shipIDs:
            print "\t\t\t\t *** transferring ship %4d,  formerly of fleet %4d,  into fleet %4d"%(shipID,  fID,  fleetID)
            success = success and ( fo.issueFleetTransferOrder(shipID,  fleetID) )#returns a zero if transfer failure
        fleetA = universe.getFleet(fID)
        if (not fleetA) or fleetA.empty:
            print "deleting fleet info for old fleet %d"%fID
            foAI.foAIstate.deleteFleetInfo(fID)
        elif  success:
            print "fleet all transferred,  but fleet.empty returns false.  remaining ship IDs: %s"%fleetA.shipIDs
            foAI.foAIstate.deleteFleetInfo(fID)
        foAI.foAIstate.updateFleetRating(fleetID)
        #fleet = universe.getFleet(fleetID)
        #fleet.setAggressive(True)

    def checkMergers(self,  fleetID):
        universe = fo.getUniverse()
        fleet = universe.getFleet(fleetID)
        systemID = fleet.systemID
        if systemID == -1:
            return # can't merge fleets in middle of starlane
        sys1=universe.getSystem(systemID)
        sysName = (sys1 and sys1.name) or "unknown"
        targetID = self.getTargetAITarget().getTargetID()
        mainFleetMission=foAI.foAIstate.getAIFleetMission(fleetID)
        if not mainFleetMission:
            #print "\t ****** no main mission for fleet %4d -- can't consider mergers"%fleetID
            return 
        mainMissionType = (mainFleetMission.getAIMissionTypes() + [-1])[0]
        if mainMissionType not in [  AIFleetMissionType.FLEET_MISSION_ATTACK,
                                                                        AIFleetMissionType.FLEET_MISSION_DEFEND, 
                                                                        AIFleetMissionType.FLEET_MISSION_LAST_STAND ,  
                                                                        AIFleetMissionType.FLEET_MISSION_INVASION, 
                                                                        AIFleetMissionType.FLEET_MISSION_MILITARY, 
                                                                    ]:
            return
        mainMissionTargets = mainFleetMission.getAITargets(mainMissionType)
        #print "\tconsidering  mergers into fleet %d at system %d (%s)"%(fleetID,  systemID,  sysName)
        if mainMissionTargets == []:
            #print "\t ****** no main-mission targets for fleet %4d -- mission type %s  -- can't consider mergers"%(fleetID,  AIFleetMissionTypeNames.name(mainMissionType) )
            return
        if len(mainMissionTargets)>1: 
            pass
            #print "\tSurprise,  fleet %d has multiple targets: %s"%(fleetID,  str(mainMissionTargets))
        mMT0=mainMissionTargets[0]
        mMT0ID = mMT0.getTargetID()
        #print "\tMain-Mission type %s  -- primary target :   %s"%( AIFleetMissionTypeNames.name(mainMissionType),  mMT0)
        otherFleetsHere = [ fID for fID,  val in foAI.foAIstate.fleetStatus.items() if (fID != fleetID) and (val.get('sysID',  -2)==systemID)]
        for fID in otherFleetsHere:
            fleet2 = universe.getFleet(fID)
            if not (fleet2 and (fleet2.systemID == systemID)):
                print "\t\t fleet2 (id %4d)  not actually in system %4d %s although AI records thought it should be"%(fID,  systemID,  sysName)
                continue
            if not fleet2.ownedBy(foAI.foAIstate.empireID):
                #print "\t\t tried fleet %d,  but not owned by this empire"%fID
                continue
            f2Mission=foAI.foAIstate.getAIFleetMission(fID)
            if not f2Mission:
                continue
            f2MType = (f2Mission.getAIMissionTypes()+[-1])[0]
            #print "\t\t fleet2 (id %d)  has  mission type %s "%(fID,   AIFleetMissionTypeNames.name(f2MType))
            if f2MType != mainMissionType:
                continue
            f2Targets = f2Mission.getAITargets(mainMissionType)
            if len(f2Targets)>1: 
                #print "\t\t\t Surprise,  fleet %d has multiple targets: %s"%(fID,  str(f2Targets))
                pass
            elif len(f2Targets)==0: 
                #print "\t\t\t  found no targets for  fleet2 (id: %4d) "%(fID)
                continue
            target = f2Targets[0]
            #print "\t\t\t  found  fleet2 (id: %4d) target %s"%(fID,  target)
            if target == mMT0:
                self.mergeFleetAintoB(fID,  fleetID)
        return

    def canIssueOrder(self,  considerMergers=True,  verbose=False):
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
        mainFleetMission=foAI.foAIstate.getAIFleetMission(fleetID)
        if not mainFleetMission:
            mainMissionType = -1
            if considerMergers:
                considerMergers=False
                reason = " -- no main mission"
                #if verbose:
                #    print "\t ****** no main mission for fleet %4d -- can't consider mergers"%fleetID
        else: 
            mainMissionType = (mainFleetMission.getAIMissionTypes() + [-1] )[0]
            if mainMissionType not in [  AIFleetMissionType.FLEET_MISSION_ATTACK,
                                                                            AIFleetMissionType.FLEET_MISSION_DEFEND, 
                                                                            AIFleetMissionType.FLEET_MISSION_LAST_STAND ,  
                                                                            AIFleetMissionType.FLEET_MISSION_INVASION, 
                                                                            AIFleetMissionType.FLEET_MISSION_MILITARY, 
                                                                        ]:
                considerMergers=False
                reason = " -- wrong main mission type: %s"
        if considerMergers:
            mainMissionTargets = mainFleetMission.getAITargets(mainMissionType)
            if mainMissionTargets == []:
                #if verbose:
                    #print "\t ****** no main-mission targets for fleet %4d -- mission type %s  -- can't consider mergers"%(fleetID,  AIFleetMissionTypeNames.name(mainMissionType) )
                considerMergers=False
                reason=" -- no mission targets"

        if verbose:
            #msgP2 = "\t-- Order %s -- currently %s considering mergers %s"%(  self,  [" not ", ""][considerMergers],  reason )
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
            if (ship != None) and (fleet.systemID == planet.systemID) and ship.canColonize:
                return True
            return False
        #
        # invade
        #
        elif AIFleetOrderType.ORDER_INVADE == self.getAIFleetOrderType():
            if (considerMergers):
                self.checkMergers(fleetID)
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
            if (considerMergers):
                self.checkMergers(fleetID)
            systemID = fleet.systemID
            sys1=universe.getSystem(systemID)
            sys1Name = (sys1 and sys1.name) or "unknown"
            targetID = self.getTargetAITarget().getTargetID()
            targ1 = universe.getSystem(targetID)
            targ1Name = (targ1 and targ1.name) or "unknown"
            fleetRating = foAI.foAIstate.rateFleet(fleetID)
            threat = foAI.foAIstate.systemStatus.get(targetID,  {}).get('fleetThreat',  -1)

            if threat==0 or fleetRating > threat:
                return True
            else:
                myOtherFleetsRating =   sum([foAI.foAIstate.fleetStatus.get(fleetID, {}).get('rating', 0)  for fleetID in foAI.foAIstate.militaryFleetIDs   if ( foAI.foAIstate.fleetStatus.get(fleetID,  {}).get('sysID',  -1) == thisSystemID ) ])
                if  myOtherFleetsRating > threat:
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

                fo.issueColonizeOrder(shipID, self.getTargetAITarget().getTargetID())
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
                            print "Ordered troop ship ID %d to invade %s"%(shipID, planetName)
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
