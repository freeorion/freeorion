from EnumsAI import AIFleetOrderType, AITargetType, AIShipRoleType,  AIFleetMissionType
import FleetUtilsAI
import freeOrionAIInterface as fo # pylint: disable=import-error
import FreeOrionAI as foAI

AIFleetOrderTypeNames=AIFleetOrderType()
AIFleetMissionTypeNames = AIFleetMissionType()

dumpTurn=0

def dictFromMap(thismap):
    return dict(  [  (el.key(),  el.data() ) for el in thismap ] )

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
        shipID = shipAITarget.target_id
        fleetID = fleetAITarget.target_id
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
        if self.getSourceAITarget().valid and self.getTargetAITarget().valid:
            sourceAITargetTypeValid = False
            targetAITargetTypeValid = False
            universe = fo.getUniverse()

            # outpost
            if AIFleetOrderType.ORDER_OUTPOST == self.getAIFleetOrderType():
                # with ship
                if AITargetType.TARGET_SHIP == self.getSourceAITarget().target_type:
                    ship = universe.getShip(self.getSourceAITarget().target_id)
                    if ship.canColonize:
                        sourceAITargetTypeValid = True
                # with fleet
                elif AITargetType.TARGET_FLEET == self.getSourceAITarget().target_type:
                    fleet = universe.getFleet(self.getSourceAITarget().target_id)
                    if fleet.hasOutpostShips:
                        sourceAITargetTypeValid = True
                # colonise planet
                if AITargetType.TARGET_PLANET == self.getTargetAITarget().target_type:
                    planet = universe.getPlanet(self.getTargetAITarget().target_id)
                    system = universe.getSystem(planet.systemID)
                    sysPartialVisTurn = dictFromMap(universe.getVisibilityTurnsMap(planet.systemID,  fo.empireID())).get(fo.visibility.partial, -9999)
                    planetPartialVisTurn = dictFromMap(universe.getVisibilityTurnsMap(planet.id,  fo.empireID())).get(fo.visibility.partial, -9999)
                    if (planetPartialVisTurn == sysPartialVisTurn) and planet.unowned:
                        targetAITargetTypeValid = True
                    else:#try to get order cancelled out
                        self.__setExecuted()
                        self.__setExecutionCompleted()

            # colonise
            elif AIFleetOrderType.ORDER_COLONISE == self.getAIFleetOrderType():
                # with ship
                if AITargetType.TARGET_SHIP == self.getSourceAITarget().target_type:
                    ship = universe.getShip(self.getSourceAITarget().target_id)
                    if ship.canColonize:
                        sourceAITargetTypeValid = True
                # with fleet
                elif AITargetType.TARGET_FLEET == self.getSourceAITarget().target_type:
                    fleet = universe.getFleet(self.getSourceAITarget().target_id)
                    if fleet.hasColonyShips:
                        sourceAITargetTypeValid = True
                # colonise planet
                if AITargetType.TARGET_PLANET == self.getTargetAITarget().target_type:
                    planet = universe.getPlanet(self.getTargetAITarget().target_id)
                    system = universe.getSystem(planet.systemID)
                    sysPartialVisTurn = dictFromMap(universe.getVisibilityTurnsMap(planet.systemID,  fo.empireID())).get(fo.visibility.partial, -9999)
                    planetPartialVisTurn = dictFromMap(universe.getVisibilityTurnsMap(planet.id,  fo.empireID())).get(fo.visibility.partial, -9999)

                    if (planetPartialVisTurn == sysPartialVisTurn) and ( planet.unowned or  (planet.ownedBy(fo.empireID()) and   planet.currentMeterValue(fo.meterType.population)==0 )):
                        targetAITargetTypeValid = True
                    else:#try to get order cancelled out
                        self.__setExecuted()
                        self.__setExecutionCompleted()
            # invade
            elif AIFleetOrderType.ORDER_INVADE == self.getAIFleetOrderType():
                # with ship
                if AITargetType.TARGET_SHIP == self.getSourceAITarget().target_type:
                    ship = universe.getShip(self.getSourceAITarget().target_id)
                    if ship.canInvade:
                        sourceAITargetTypeValid = True
                # with fleet
                elif AITargetType.TARGET_FLEET == self.getSourceAITarget().target_type:
                    fleet = universe.getFleet(self.getSourceAITarget().target_id)
                    if fleet.hasTroopShips:
                        sourceAITargetTypeValid = True
                # invade planet
                if AITargetType.TARGET_PLANET == self.getTargetAITarget().target_type:
                    planet = universe.getPlanet(self.getTargetAITarget().target_id)
                    planetPopulation = planet.currentMeterValue(fo.meterType.population)
                    if not planet.unowned or planetPopulation > 0:
                        targetAITargetTypeValid = True
                    else:#try to get order cancelled out
                        self.__setExecuted()
                        self.__setExecutionCompleted()
                # military
                elif AIFleetOrderType.ORDER_MILITARY == self.getAIFleetOrderType():
                    # with ship
                    if AITargetType.TARGET_SHIP == self.getSourceAITarget().target_type:
                        ship = universe.getShip(self.getSourceAITarget().target_id)
                        if ship.isArmed:
                            sourceAITargetTypeValid = True
                # with fleet
                elif AITargetType.TARGET_FLEET == self.getSourceAITarget().target_type:
                    fleet = universe.getFleet(self.getSourceAITarget().target_id)
                    if fleet.hasArmedShips:
                        sourceAITargetTypeValid = True
                # military system
                if AITargetType.TARGET_SYSTEM == self.getTargetAITarget().target_type:
                    system = universe.getSystem(self.getTargetAITarget().target_id)
                    targetAITargetTypeValid = True
            # move
            elif AIFleetOrderType.ORDER_MOVE == self.getAIFleetOrderType():
                # with fleet
                if AITargetType.TARGET_FLEET == self.getSourceAITarget().target_type:
                    sourceAITargetTypeValid = True
                # move to system
                if AITargetType.TARGET_SYSTEM == self.getTargetAITarget().target_type:
                    targetAITargetTypeValid = True
            # resupply
            elif AIFleetOrderType.ORDER_RESUPPLY == self.getAIFleetOrderType():
                # with fleet
                if AITargetType.TARGET_FLEET == self.getSourceAITarget().target_type:
                    sourceAITargetTypeValid = True
                # move to system
                if AITargetType.TARGET_SYSTEM == self.getTargetAITarget().target_type:
                    empire = fo.getEmpire()
                    fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
                    if (self.getTargetAITarget().target_id in fleetSupplyableSystemIDs):
                        targetAITargetTypeValid = True
            # split fleet
            elif AIFleetOrderType.ORDER_SPLIT_FLEET == self.getAIFleetOrderType():
                # with fleet
                if AITargetType.TARGET_FLEET == self.getSourceAITarget().target_type:
                    sourceAITargetTypeValid = True
                # split ship
                if AITargetType.TARGET_SHIP == self.getTargetAITarget().target_type:
                    targetAITargetTypeValid = True
                if sourceAITargetTypeValid == True and targetAITargetTypeValid == True:
                    if self.__checkValidityShipInFleet(self.getTargetAITarget(), self.getSourceAITarget()):
                        return True
            elif AIFleetOrderType.ORDER_ATTACK == self.getAIFleetOrderType():
                # with fleet
                if AITargetType.TARGET_FLEET == self.getSourceAITarget().target_type:
                    sourceAITargetTypeValid = True
                # move to system
                if AITargetType.TARGET_SYSTEM == self.getTargetAITarget().target_type or AITargetType.TARGET_PLANET == self.getTargetAITarget().target_type:
                    targetAITargetTypeValid = True
            elif AIFleetOrderType.ORDER_DEFEND == self.getAIFleetOrderType():
                # with fleet
                if AITargetType.TARGET_FLEET == self.getSourceAITarget().target_type:
                    sourceAITargetTypeValid = True
                # move to system
                if AITargetType.TARGET_SYSTEM == self.getTargetAITarget().target_type or AITargetType.TARGET_PLANET == self.getTargetAITarget().target_type:
                    targetAITargetTypeValid = True

            if sourceAITargetTypeValid == True and targetAITargetTypeValid == True:
                return True

        return False

    def canIssueOrder(self,  considerMergers=False,  verbose=False):
        "if FleetOrder can be issued now"

        #for some orders, may need to re-issue if invasion/outposting/colonization was interrupted
        if self.isExecuted()  and self.getAIFleetOrderType() not in [ AIFleetOrderType.ORDER_OUTPOST,  AIFleetOrderType.ORDER_COLONISE,  AIFleetOrderType.ORDER_INVADE   ]:
            return False
        if not self.isValid():
            return False

        reason = ""
        universe = fo.getUniverse()
        fleetID = None
        shipID = None
        if AITargetType.TARGET_SHIP == self.getSourceAITarget().target_type:
            shipID = self.getSourceAITarget().target_id
            ship = universe.getShip(shipID)
            fleetID = ship.fleetID
        elif AITargetType.TARGET_FLEET == self.getSourceAITarget().target_type:
            fleetID = self.getSourceAITarget().target_id
        fleet = universe.getFleet(fleetID)

        systemID = fleet.systemID
        sys1=universe.getSystem(systemID)
        sysName = (sys1 and sys1.name) or "unknown"
        targetID = self.getTargetAITarget().target_id

        if verbose:
            mainFleetMission=foAI.foAIstate.getAIFleetMission(fleetID)
            mainMissionType = (mainFleetMission.getAIMissionTypes() + [-1])[0]
            msgP1 = "** %s  -- Mission Type  %s , current loc sys %d  - %s"%(  self,   AIFleetMissionTypeNames.name(mainMissionType),  systemID,  sysName )
            print msgP1

        #
        # outpost
        #
        if AIFleetOrderType.ORDER_OUTPOST == self.getAIFleetOrderType():#TODO: check for separate fleet holding outpost  ships
            if AITargetType.TARGET_FLEET == self.getSourceAITarget().target_type:
                shipID = FleetUtilsAI.getShipIDWithRole(fleetID, AIShipRoleType.SHIP_ROLE_CIVILIAN_OUTPOST)
                if shipID is None:
                    shipID = FleetUtilsAI.getShipIDWithRole(fleetID, AIShipRoleType.SHIP_ROLE_BASE_OUTPOST)
            ship = universe.getShip(shipID)
            planet = universe.getPlanet(self.getTargetAITarget().target_id)
            if (ship != None) and (fleet.systemID == planet.systemID) and ship.canColonize:
                return True
            return False
        #
        # colonise
        #
        elif AIFleetOrderType.ORDER_COLONISE == self.getAIFleetOrderType():#TODO: check for separate fleet holding colony ships
            if AITargetType.TARGET_FLEET == self.getSourceAITarget().target_type:
                shipID = FleetUtilsAI.getShipIDWithRole(fleetID, AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION)
                if shipID is None:
                    shipID = FleetUtilsAI.getShipIDWithRole(fleetID, AIShipRoleType.SHIP_ROLE_BASE_COLONISATION)
            ship = universe.getShip(shipID)
            planet = universe.getPlanet(self.getTargetAITarget().target_id)
            if ship and not ship.canColonize:
                print "Error: colonization fleet %d has no colony ship"%fleetID
            if (ship != None) and (fleet.systemID == planet.systemID) and ship.canColonize:
                return True
            return False
        #
        # invade
        #
        elif AIFleetOrderType.ORDER_INVADE == self.getAIFleetOrderType():#TODO: check for separate fleet holding invasion ships
            if AITargetType.TARGET_FLEET == self.getSourceAITarget().target_type:
                shipID = FleetUtilsAI.getShipIDWithRole(fleetID, AIShipRoleType.SHIP_ROLE_MILITARY_INVASION)
                if shipID is None:
                    shipID = FleetUtilsAI.getShipIDWithRole(fleetID, AIShipRoleType.SHIP_ROLE_BASE_INVASION)
            ship = universe.getShip(shipID)
            planet = universe.getPlanet(self.getTargetAITarget().target_id)
            if (ship != None) and (fleet.systemID == planet.systemID) and ship.canInvade and ( planet.currentMeterValue(fo.meterType.shield) ==0 or planet.owner==-1):# native planets currently shouldnt have shields, but a bug sometimes makes it look like they do
                return True
            return False
        #
        # military
        #
        elif AIFleetOrderType.ORDER_MILITARY == self.getAIFleetOrderType():
            if AITargetType.TARGET_FLEET == self.getSourceAITarget().target_type:
                shipID = FleetUtilsAI.getShipIDWithRole(fleetID, AIShipRoleType.SHIP_ROLE_MILITARY)
            ship = universe.getShip(shipID)
            system = universe.getSystem(self.getTargetAITarget().target_id)
            if (ship != None) and (fleet.systemID == system.systemID) and ship.isArmed:
                return True
            return False
        #
        # split fleet
        #
        elif AIFleetOrderType.ORDER_SPLIT_FLEET == self.getAIFleetOrderType():
            fleet2 = universe.getFleet(self.getSourceAITarget().target_id)
            if len(fleet2.shipIDs) <= 1:
                return False
        #
        # move -- have fleets will do a safety check, also check for potential military fleet mergers
        #
        elif AIFleetOrderType.ORDER_MOVE == self.getAIFleetOrderType():
            #targetID = self.getTargetAITarget().target_id
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
            if systemID == targetID:
                return True #already there so no point to worry about threat
            sys1=universe.getSystem(systemID)
            sys1Name = (sys1 and sys1.name) or "unknown"
            targ1 = universe.getSystem(targetID)
            targ1Name = (targ1 and targ1.name) or "unknown"
            fleetRating = foAI.foAIstate.getRating(fleetID).get('overall', 0)
            threat = foAI.foAIstate.systemStatus.get(targetID,  {}).get('fleetThreat',  0) + foAI.foAIstate.systemStatus.get(targetID,  {}).get('planetThreat',  0)+ foAI.foAIstate.systemStatus.get(targetID,  {}).get('monsterThreat',  0)

            safetyFactor = 1.1
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
        global dumpTurn
        if not self.canIssueOrder(considerMergers=False):  #appears to be redundant with check in IAFleetMission?
            print "\tcan't issue %s"%self
        else:
            universe=fo.getUniverse()
            self.__setExecuted()
            # outpost
            if AIFleetOrderType.ORDER_OUTPOST == self.getAIFleetOrderType():
                planet=universe.getPlanet(self.getTargetAITarget().target_id)
                if not planet.unowned:
                    self.__setExecutionCompleted()
                    return
                shipID = None
                if AITargetType.TARGET_SHIP == self.getSourceAITarget().target_type:
                    shipID = self.getSourceAITarget().target_id
                elif AITargetType.TARGET_FLEET == self.getSourceAITarget().target_type:
                    fleetID = self.getSourceAITarget().target_id
                    shipID = FleetUtilsAI.getShipIDWithRole(fleetID, AIShipRoleType.SHIP_ROLE_CIVILIAN_OUTPOST)
                    if shipID is None:
                        shipID = FleetUtilsAI.getShipIDWithRole(fleetID, AIShipRoleType.SHIP_ROLE_BASE_OUTPOST)
                result=fo.issueColonizeOrder(shipID, self.getTargetAITarget().target_id)
                if result==0:
                    self.__executed = False
            # colonise
            elif AIFleetOrderType.ORDER_COLONISE == self.getAIFleetOrderType():
                shipID = None
                if AITargetType.TARGET_SHIP == self.getSourceAITarget().target_type:
                    shipID = self.getSourceAITarget().target_id
                elif AITargetType.TARGET_FLEET == self.getSourceAITarget().target_type:
                    fleetID = self.getSourceAITarget().target_id
                    shipID = FleetUtilsAI.getShipIDWithRole(fleetID, AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION)
                    if shipID is None:
                        shipID = FleetUtilsAI.getShipIDWithRole(fleetID, AIShipRoleType.SHIP_ROLE_BASE_COLONISATION)

                planetID = self.getTargetAITarget().target_id
                planet=universe.getPlanet(planetID)
                planetName = (planet and planet.name) or "apparently invisible"
                result = fo.issueColonizeOrder(shipID, planetID)
                print "Ordered colony ship ID %d to colonize %s, got result %d"%(shipID, planetName,  result)
                if result==0:
                    self.__executed = False
            # invade
            elif AIFleetOrderType.ORDER_INVADE == self.getAIFleetOrderType():
                result = False
                shipID = None
                planetID = self.getTargetAITarget().target_id
                planet=universe.getPlanet(planetID)
                planetName = (planet and planet.name) or "invisible"
                if AITargetType.TARGET_SHIP == self.getSourceAITarget().target_type:
                    shipID = self.getSourceAITarget().target_id
                elif AITargetType.TARGET_FLEET == self.getSourceAITarget().target_type:
                    fleetID = self.getSourceAITarget().target_id
                    fleet = fo.getUniverse().getFleet(fleetID)
                    for shipID in fleet.shipIDs:
                        ship = universe.getShip(shipID)
                        if (foAI.foAIstate.getShipRole(ship.design.id) in [AIShipRoleType.SHIP_ROLE_MILITARY_INVASION,  AIShipRoleType.SHIP_ROLE_BASE_INVASION]):
                            result = fo.issueInvadeOrder(shipID, planetID)  or  result #will track if at least one invasion troops successfully deployed
                            detailStr = ""
                            if result == 0:
                                pstealth = planet.currentMeterValue(fo.meterType.stealth)
                                pop = planet.currentMeterValue(fo.meterType.population)
                                shields = planet.currentMeterValue(fo.meterType.shield)
                                owner = planet.owner
                                detailStr= " -- planet has %.1f stealth, shields %.1f,  %.1f population and is owned by empire %d"%(pstealth,  shields,  pop,  owner)
                            print "Ordered troop ship ID %d to invade %s, got result %d"%(shipID, planetName,  result),  detailStr
                            if result == 0:
                                if 'needsEmergencyExploration' not in dir(foAI.foAIstate):
                                    foAI.foAIstate.needsEmergencyExploration=[]
                                if  fleet.systemID not in foAI.foAIstate.needsEmergencyExploration:
                                    foAI.foAIstate.needsEmergencyExploration.append(fleet.systemID)
                                    print "Due to trouble invading,  adding system %d to Emergency Exploration List"%fleet.systemID
                                    self.__executed = False
                                if shields >0 and owner==-1 and dumpTurn<fo.currentTurn():
                                    dumpTurn=fo.currentTurn()
                                    print "Universe Dump to debug invasions:"
                                    universe.dump()
                                break
            # military
            elif AIFleetOrderType.ORDER_MILITARY == self.getAIFleetOrderType():
                shipID = None
                if AITargetType.TARGET_SHIP == self.getSourceAITarget().target_type:
                    shipID = self.getSourceAITarget().target_id
                elif AITargetType.TARGET_FLEET == self.getSourceAITarget().target_type:
                    fleetID = self.getSourceAITarget().target_id
                    shipID = FleetUtilsAI.getShipIDWithRole(fleetID, AIShipRoleType.SHIP_ROLE_MILITARY)
                #fo.issueFleetMoveOrder(fleetID, self.getTargetAITarget().target_id) #moving is already taken care of separately
                targetSysID = self.getTargetAITarget().target_id
                fleet = fo.getUniverse().getFleet(fleetID)
                systemStatus =  foAI.foAIstate.systemStatus.get(targetSysID,  {})
                if (fleet )and ( fleet.systemID==targetSysID ) and ((systemStatus.get('fleetThreat', 0) + systemStatus.get('planetThreat', 0)+ systemStatus.get('monsterThreat', 0))==0):
                    self.__setExecutionCompleted()

            # move or resupply
            elif (AIFleetOrderType.ORDER_MOVE == self.getAIFleetOrderType()) or (AIFleetOrderType.ORDER_RESUPPLY == self.getAIFleetOrderType()):
                fleetID = self.getSourceAITarget().target_id
                systemID = self.getTargetAITarget().target_id
                fleet = fo.getUniverse().getFleet(fleetID)
                if  systemID not in [fleet.systemID,  fleet.nextSystemID] :
                    fo.issueFleetMoveOrder(fleetID, systemID)
                if systemID == fleet.systemID:
                    if    foAI.foAIstate.getFleetRole(fleetID) == AIFleetMissionType.FLEET_MISSION_EXPLORATION :
                        if 'needsEmergencyExploration' not in dir(foAI.foAIstate):
                            foAI.foAIstate.needsEmergencyExploration=[]
                        if systemID in foAI.foAIstate.needsEmergencyExploration:
                            del foAI.foAIstate.needsEmergencyExploration[ foAI.foAIstate.needsEmergencyExploration.index(systemID) ]
                    self.__setExecutionCompleted()

            # split fleet
            elif AIFleetOrderType.ORDER_SPLIT_FLEET == self.getAIFleetOrderType():
                fleetID = self.getSourceAITarget().target_id
                shipID = self.getTargetAITarget().target_id
                fleet = fo.getUniverse().getFleet(fleetID)
                if shipID in fleet.shipIDs:
                    fo.issueNewFleetOrder(str(shipID), shipID)
                self.__setExecutionCompleted()
            # attack
            elif (AIFleetOrderType.ORDER_ATTACK == self.getAIFleetOrderType()):
                fleetID = self.getSourceAITarget().target_id
                systemID = self.getTargetAITarget().get_required_system_ai_targets()[0].target_id

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
