from EnumsAI import AIFleetOrderType, AITargetType, AIShipRoleType,  AIFleetMissionType
import FleetUtilsAI
import freeOrionAIInterface as fo # pylint: disable=import-error
import FreeOrionAI as foAI
import MoveUtilsAI
import PlanetUtilsAI
from tools import dict_from_map

AIFleetOrderTypeNames=AIFleetOrderType()
AIFleetMissionTypeNames = AIFleetMissionType()

dumpTurn=0


class AIFleetOrder(object):
    """Stores information about orders which can be executed"""

    def __init__(self, fleet_order_type, source_target, target_target):
        self.__aiFleetOrderType = fleet_order_type
        self.__sourceAITarget = source_target
        self.__targetAITarget = target_target
        self.__executed = False
        self.__executionCompleted = False

    def get_fleet_order_type(self):
        return self.__aiFleetOrderType

    def get_source_target(self):
        return self.__sourceAITarget

    def get_target_target(self):
        return self.__targetAITarget

    def is_executed(self):
        return self.__executed

    def __set_executed(self):
        self.__executed = True

    def is_execution_completed(self):
        return self.__executionCompleted

    def __set_execution_completed(self):
        self.__executionCompleted = True

    def __check_validity_ship_in_fleet(self, ship_target, fleet_target):
        ship_id = ship_target.target_id
        fleet_id = fleet_target.target_id
        universe = fo.getUniverse()
        fleet = universe.getFleet(fleet_id)
        # ship is in fleet
        if ship_id in fleet.shipIDs:
            return True
        return False

    def is_valid(self):
        """check if FleetOrder could be somehow in future issued = is valid"""

        if self.is_executed() and self.is_execution_completed():
            return False
        if self.get_source_target().valid and self.get_target_target().valid:
            sourceAITargetTypeValid = False
            targetAITargetTypeValid = False
            universe = fo.getUniverse()

            # outpost
            if AIFleetOrderType.ORDER_OUTPOST == self.get_fleet_order_type():
                # with ship
                if AITargetType.TARGET_SHIP == self.get_source_target().target_type:
                    ship = universe.getShip(self.get_source_target().target_id)
                    if ship.canColonize:
                        sourceAITargetTypeValid = True
                # with fleet
                elif AITargetType.TARGET_FLEET == self.get_source_target().target_type:
                    fleet = universe.getFleet(self.get_source_target().target_id)
                    if fleet.hasOutpostShips:
                        sourceAITargetTypeValid = True
                # colonise planet
                if AITargetType.TARGET_PLANET == self.get_target_target().target_type:
                    planet = universe.getPlanet(self.get_target_target().target_id)
                    system = universe.getSystem(planet.systemID)
                    sysPartialVisTurn = dict_from_map(universe.getVisibilityTurnsMap(planet.systemID, fo.empireID())).get(fo.visibility.partial, -9999)
                    planetPartialVisTurn = dict_from_map(universe.getVisibilityTurnsMap(planet.id, fo.empireID())).get(fo.visibility.partial, -9999)
                    if (planetPartialVisTurn == sysPartialVisTurn) and planet.unowned:
                        targetAITargetTypeValid = True
                    else:#try to get order cancelled out
                        self.__set_executed()
                        self.__set_execution_completed()

            # colonise
            elif AIFleetOrderType.ORDER_COLONISE == self.get_fleet_order_type():
                # with ship
                if AITargetType.TARGET_SHIP == self.get_source_target().target_type:
                    ship = universe.getShip(self.get_source_target().target_id)
                    if ship.canColonize:
                        sourceAITargetTypeValid = True
                # with fleet
                elif AITargetType.TARGET_FLEET == self.get_source_target().target_type:
                    fleet = universe.getFleet(self.get_source_target().target_id)
                    if fleet.hasColonyShips:
                        sourceAITargetTypeValid = True
                # colonise planet
                if AITargetType.TARGET_PLANET == self.get_target_target().target_type:
                    planet = universe.getPlanet(self.get_target_target().target_id)
                    system = universe.getSystem(planet.systemID)
                    sysPartialVisTurn = dict_from_map(universe.getVisibilityTurnsMap(planet.systemID, fo.empireID())).get(fo.visibility.partial, -9999)
                    planetPartialVisTurn = dict_from_map(universe.getVisibilityTurnsMap(planet.id, fo.empireID())).get(fo.visibility.partial, -9999)

                    if (planetPartialVisTurn == sysPartialVisTurn) and ( planet.unowned or  (planet.ownedBy(fo.empireID()) and   planet.currentMeterValue(fo.meterType.population)==0 )):
                        targetAITargetTypeValid = True
                    else:#try to get order cancelled out
                        self.__set_executed()
                        self.__set_execution_completed()
            # invade
            elif AIFleetOrderType.ORDER_INVADE == self.get_fleet_order_type():
                # with ship
                if AITargetType.TARGET_SHIP == self.get_source_target().target_type:
                    ship = universe.getShip(self.get_source_target().target_id)
                    if ship.canInvade:
                        sourceAITargetTypeValid = True
                # with fleet
                elif AITargetType.TARGET_FLEET == self.get_source_target().target_type:
                    fleet = universe.getFleet(self.get_source_target().target_id)
                    if fleet.hasTroopShips:
                        sourceAITargetTypeValid = True
                # invade planet
                if AITargetType.TARGET_PLANET == self.get_target_target().target_type:
                    planet = universe.getPlanet(self.get_target_target().target_id)
                    planetPopulation = planet.currentMeterValue(fo.meterType.population)
                    if not planet.unowned or planetPopulation > 0:
                        targetAITargetTypeValid = True
                    else:#try to get order cancelled out
                        self.__set_executed()
                        self.__set_execution_completed()
                # military
                elif AIFleetOrderType.ORDER_MILITARY == self.get_fleet_order_type():
                    # with ship
                    if AITargetType.TARGET_SHIP == self.get_source_target().target_type:
                        ship = universe.getShip(self.get_source_target().target_id)
                        if ship.isArmed:
                            sourceAITargetTypeValid = True
                # with fleet
                elif AITargetType.TARGET_FLEET == self.get_source_target().target_type:
                    fleet = universe.getFleet(self.get_source_target().target_id)
                    if fleet.hasArmedShips:
                        sourceAITargetTypeValid = True
                # military system
                if AITargetType.TARGET_SYSTEM == self.get_target_target().target_type:
                    system = universe.getSystem(self.get_target_target().target_id)
                    targetAITargetTypeValid = True
            # move
            elif AIFleetOrderType.ORDER_MOVE == self.get_fleet_order_type():
                # with fleet
                if AITargetType.TARGET_FLEET == self.get_source_target().target_type:
                    sourceAITargetTypeValid = True
                # move to system
                if AITargetType.TARGET_SYSTEM == self.get_target_target().target_type:
                    targetAITargetTypeValid = True
            # resupply
            elif AIFleetOrderType.ORDER_RESUPPLY == self.get_fleet_order_type():
                # with fleet
                if AITargetType.TARGET_FLEET == self.get_source_target().target_type:
                    sourceAITargetTypeValid = True
                # move to system
                if AITargetType.TARGET_SYSTEM == self.get_target_target().target_type:
                    empire = fo.getEmpire()
                    fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
                    if self.get_target_target().target_id in fleetSupplyableSystemIDs:
                        targetAITargetTypeValid = True
            # repair
            elif AIFleetOrderType.ORDER_REPAIR == self.get_fleet_order_type():
                # with fleet
                if AITargetType.TARGET_FLEET == self.get_source_target().target_type:
                    sourceAITargetTypeValid = True
                # move to system
                if AITargetType.TARGET_SYSTEM == self.get_target_target().target_type:
                    empire = fo.getEmpire()
                    fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
                    if self.get_target_target().target_id in fleetSupplyableSystemIDs: #TODO: check for drydock still there/owned
                        targetAITargetTypeValid = True
            # split fleet
            elif AIFleetOrderType.ORDER_SPLIT_FLEET == self.get_fleet_order_type():
                # with fleet
                if AITargetType.TARGET_FLEET == self.get_source_target().target_type:
                    sourceAITargetTypeValid = True
                # split ship
                if AITargetType.TARGET_SHIP == self.get_target_target().target_type:
                    targetAITargetTypeValid = True
                if sourceAITargetTypeValid == True and targetAITargetTypeValid == True:
                    if self.__check_validity_ship_in_fleet(self.get_target_target(), self.get_source_target()):
                        return True
            elif AIFleetOrderType.ORDER_ATTACK == self.get_fleet_order_type():
                # with fleet
                if AITargetType.TARGET_FLEET == self.get_source_target().target_type:
                    sourceAITargetTypeValid = True
                # move to system
                if AITargetType.TARGET_SYSTEM == self.get_target_target().target_type or AITargetType.TARGET_PLANET == self.get_target_target().target_type:
                    targetAITargetTypeValid = True
            elif AIFleetOrderType.ORDER_DEFEND == self.get_fleet_order_type():
                # with fleet
                if AITargetType.TARGET_FLEET == self.get_source_target().target_type:
                    sourceAITargetTypeValid = True
                # move to system
                if AITargetType.TARGET_SYSTEM == self.get_target_target().target_type or AITargetType.TARGET_PLANET == self.get_target_target().target_type:
                    targetAITargetTypeValid = True

            if sourceAITargetTypeValid == True and targetAITargetTypeValid == True:
                return True

        return False

    def can_issue_order(self,  considerMergers=False,  verbose=False):
        """if FleetOrder can be issued now"""

        #for some orders, may need to re-issue if invasion/outposting/colonization was interrupted
        if self.is_executed()  and self.get_fleet_order_type() not in [ AIFleetOrderType.ORDER_OUTPOST,  AIFleetOrderType.ORDER_COLONISE,  AIFleetOrderType.ORDER_INVADE   ]:
            return False
        if not self.is_valid():
            return False

        reason = ""
        universe = fo.getUniverse()
        fleetID = None
        shipID = None
        if AITargetType.TARGET_SHIP == self.get_source_target().target_type:
            shipID = self.get_source_target().target_id
            ship = universe.getShip(shipID)
            fleetID = ship.fleetID
        elif AITargetType.TARGET_FLEET == self.get_source_target().target_type:
            fleetID = self.get_source_target().target_id
        fleet = universe.getFleet(fleetID)

        systemID = fleet.systemID
        sys1=universe.getSystem(systemID)
        sysName = (sys1 and sys1.name) or "unknown"
        targetID = self.get_target_target().target_id

        if verbose:
            mainFleetMission=foAI.foAIstate.get_fleet_mission(fleetID)
            mainMissionType = (mainFleetMission.get_mission_types() + [-1])[0]
            msgP1 = "** %s  -- Mission Type  %s (%s) , current loc sys %d  - %s"%(  self,   AIFleetMissionTypeNames.name(mainMissionType), mainMissionType,  systemID,  sysName )
            print msgP1

        #
        # outpost
        #
        if AIFleetOrderType.ORDER_OUTPOST == self.get_fleet_order_type():#TODO: check for separate fleet holding outpost  ships
            if AITargetType.TARGET_FLEET == self.get_source_target().target_type:
                shipID = FleetUtilsAI.get_ship_id_with_role(fleetID, AIShipRoleType.SHIP_ROLE_CIVILIAN_OUTPOST)
                if shipID is None:
                    shipID = FleetUtilsAI.get_ship_id_with_role(fleetID, AIShipRoleType.SHIP_ROLE_BASE_OUTPOST)
            ship = universe.getShip(shipID)
            planet = universe.getPlanet(self.get_target_target().target_id)
            if (ship is not None) and (fleet.systemID == planet.systemID) and ship.canColonize:
                return True
            return False
        #
        # colonise
        #
        elif AIFleetOrderType.ORDER_COLONISE == self.get_fleet_order_type():#TODO: check for separate fleet holding colony ships
            if AITargetType.TARGET_FLEET == self.get_source_target().target_type:
                shipID = FleetUtilsAI.get_ship_id_with_role(fleetID, AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION)
                if shipID is None:
                    shipID = FleetUtilsAI.get_ship_id_with_role(fleetID, AIShipRoleType.SHIP_ROLE_BASE_COLONISATION)
            ship = universe.getShip(shipID)
            planet = universe.getPlanet(self.get_target_target().target_id)
            if ship and not ship.canColonize:
                print "Error: colonization fleet %d has no colony ship"%fleetID
            if (ship is not None) and (fleet.systemID == planet.systemID) and ship.canColonize:
                return True
            return False
        #
        # invade
        #
        elif AIFleetOrderType.ORDER_INVADE == self.get_fleet_order_type():#TODO: check for separate fleet holding invasion ships
            if AITargetType.TARGET_FLEET == self.get_source_target().target_type:
                shipID = FleetUtilsAI.get_ship_id_with_role(fleetID, AIShipRoleType.SHIP_ROLE_MILITARY_INVASION,  False)
                if shipID is None:
                    shipID = FleetUtilsAI.get_ship_id_with_role(fleetID, AIShipRoleType.SHIP_ROLE_BASE_INVASION)
            ship = universe.getShip(shipID)
            planet = universe.getPlanet(self.get_target_target().target_id)
            if (ship is not None) and (fleet.systemID == planet.systemID) and ship.canInvade and ( planet.currentMeterValue(fo.meterType.shield) ==0):
                return True
            return False
        #
        # military
        #
        elif AIFleetOrderType.ORDER_MILITARY == self.get_fleet_order_type():
            if AITargetType.TARGET_FLEET == self.get_source_target().target_type:
                shipID = FleetUtilsAI.get_ship_id_with_role(fleetID, AIShipRoleType.SHIP_ROLE_MILITARY)
            ship = universe.getShip(shipID)
            system = universe.getSystem(self.get_target_target().target_id)
            if (ship is not None) and (fleet.systemID == system.systemID) and ship.isArmed:
                return True
            return False
        #
        # split fleet
        #
        elif AIFleetOrderType.ORDER_SPLIT_FLEET == self.get_fleet_order_type():
            fleet2 = universe.getFleet(self.get_source_target().target_id)
            if len(fleet2.shipIDs) <= 1:
                return False
        #
        # move -- have fleets will do a safety check, also check for potential military fleet mergers
        #
        elif AIFleetOrderType.ORDER_MOVE == self.get_fleet_order_type():
            #targetID = self.get_target_target().target_id
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
            fleetRating = foAI.foAIstate.get_rating(fleetID).get('overall', 0)
            target_sys_status = foAI.foAIstate.systemStatus.get(targetID,  {})
            f_threat = target_sys_status.get('fleetThreat',  0)
            m_threat = target_sys_status.get('monsterThreat',  0)
            p_threat = target_sys_status.get('planetThreat',  0)
            threat =  f_threat + m_threat + p_threat

            safetyFactor = 1.1
            if fleetRating >= safetyFactor* threat:
                return True
            else:
                #following line was poor because AIstate.militaryFleetIDs only covers fleets without current missions
                #myOtherFleetsRating =   sum([foAI.foAIstate.fleetStatus.get(fleetID, {}).get('rating', 0)  for fleetID in foAI.foAIstate.militaryFleetIDs   if ( foAI.foAIstate.fleetStatus.get(fleetID,  {}).get('sysID',  -1) == thisSystemID ) ])
                myOtherFleetsRatings =   [foAI.foAIstate.fleetStatus.get(fid, {}).get('rating', {})  for fid in foAI.foAIstate.systemStatus.get( targetID, {}).get('myfleets', [])  ]
                #myOtherFleetsRating =   sum([foAI.foAIstate.fleetStatus.get(fid, {}).get('rating', 0)  for fid in foAI.foAIstate.systemStatus.get( targetID, {}).get('myfleets', [])  ])
                myOtherFleetsRating =   foAI.foAIstate.systemStatus.get( targetID, {}).get('myFleetRating', 0)
                if  (myOtherFleetsRating > 1.5 * safetyFactor * threat) or (myOtherFleetsRating + fleetRating  > 2.0 * safetyFactor * threat):
                    if verbose:
                        print "\tAdvancing fleet %d (rating %d) at system %d (%s) into system %d (%s) with threat %d because of sufficient empire fleet strength already at desination"%(fleetID,  fleetRating,  systemID,  sys1Name,  targetID,  targ1Name,  threat)
                    return True
                elif (threat == p_threat) and ( not fleet.aggressive ) and (myOtherFleetsRating == 0) and (len(target_sys_status.get('localEnemyFleetIDs', [-1]))==0):
                    if verbose:
                        print ( "\tAdvancing fleet %d (rating %d) at system %d (%s) into system %d (%s) with planet threat %d because nonaggressive" +
                               " and no other fleets present to trigger combat")%(fleetID,  fleetRating,  systemID,  sys1Name,  targetID,  targ1Name,  threat)
                    return True
                else:
                    if verbose:
                        print "\tHolding fleet %d (rating %d) at system %d (%s) before travelling to system %d (%s) with threat %d"%(fleetID,  fleetRating,  systemID,  sys1Name,  targetID,  targ1Name,  threat)
                    return False
        else:  # default returns true
            return True

    def issue_order(self,  considerMergers=False):
        global dumpTurn
        if not self.can_issue_order(considerMergers=False):  #appears to be redundant with check in IAFleetMission?
            print "\tcan't issue %s"%self
        else:
            universe=fo.getUniverse()
            self.__set_executed()
            # outpost
            if AIFleetOrderType.ORDER_OUTPOST == self.get_fleet_order_type():
                planet=universe.getPlanet(self.get_target_target().target_id)
                if not planet.unowned:
                    self.__set_execution_completed()
                    return
                shipID = None
                if AITargetType.TARGET_SHIP == self.get_source_target().target_type:
                    shipID = self.get_source_target().target_id
                elif AITargetType.TARGET_FLEET == self.get_source_target().target_type:
                    fleetID = self.get_source_target().target_id
                    shipID = FleetUtilsAI.get_ship_id_with_role(fleetID, AIShipRoleType.SHIP_ROLE_CIVILIAN_OUTPOST)
                    if shipID is None:
                        shipID = FleetUtilsAI.get_ship_id_with_role(fleetID, AIShipRoleType.SHIP_ROLE_BASE_OUTPOST)
                result=fo.issueColonizeOrder(shipID, self.get_target_target().target_id)
                if result==0:
                    self.__executed = False
            # colonise
            elif AIFleetOrderType.ORDER_COLONISE == self.get_fleet_order_type():
                shipID = None
                if AITargetType.TARGET_SHIP == self.get_source_target().target_type:
                    shipID = self.get_source_target().target_id
                elif AITargetType.TARGET_FLEET == self.get_source_target().target_type:
                    fleetID = self.get_source_target().target_id
                    shipID = FleetUtilsAI.get_ship_id_with_role(fleetID, AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION)
                    if shipID is None:
                        shipID = FleetUtilsAI.get_ship_id_with_role(fleetID, AIShipRoleType.SHIP_ROLE_BASE_COLONISATION)

                planetID = self.get_target_target().target_id
                planet=universe.getPlanet(planetID)
                planetName = (planet and planet.name) or "apparently invisible"
                result = fo.issueColonizeOrder(shipID, planetID)
                print "Ordered colony ship ID %d to colonize %s, got result %d"%(shipID, planetName,  result)
                if result==0:
                    self.__executed = False
            # invade
            elif AIFleetOrderType.ORDER_INVADE == self.get_fleet_order_type():
                result = False
                shipID = None
                planetID = self.get_target_target().target_id
                planet=universe.getPlanet(planetID)
                planetName = (planet and planet.name) or "invisible"
                if AITargetType.TARGET_SHIP == self.get_source_target().target_type:
                    shipID = self.get_source_target().target_id
                elif AITargetType.TARGET_FLEET == self.get_source_target().target_type:
                    fleetID = self.get_source_target().target_id
                    fleet = fo.getUniverse().getFleet(fleetID)
                    for shipID in fleet.shipIDs:
                        ship = universe.getShip(shipID)
                        if foAI.foAIstate.get_ship_role(ship.design.id) in [AIShipRoleType.SHIP_ROLE_MILITARY_INVASION,  AIShipRoleType.SHIP_ROLE_BASE_INVASION]:
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
                if result:
                    print "Successfully ordered troop ship(s) to invade %s, with detail%s "%(planetName,  detailStr)
            # military
            elif AIFleetOrderType.ORDER_MILITARY == self.get_fleet_order_type():
                shipID = None
                if AITargetType.TARGET_SHIP == self.get_source_target().target_type:
                    shipID = self.get_source_target().target_id
                elif AITargetType.TARGET_FLEET == self.get_source_target().target_type:
                    fleetID = self.get_source_target().target_id
                    shipID = FleetUtilsAI.get_ship_id_with_role(fleetID, AIShipRoleType.SHIP_ROLE_MILITARY)
                #fo.issueFleetMoveOrder(fleetID, self.get_target_target().target_id) #moving is already taken care of separately
                targetSysID = self.get_target_target().target_id
                fleet = fo.getUniverse().getFleet(fleetID)
                systemStatus =  foAI.foAIstate.systemStatus.get(targetSysID,  {})
                if fleet and ( fleet.systemID==targetSysID ) and ((systemStatus.get('fleetThreat', 0) + systemStatus.get('planetThreat', 0)+ systemStatus.get('monsterThreat', 0))==0):
                    self.__set_execution_completed()

            # move or resupply
            elif self.get_fleet_order_type() in [ AIFleetOrderType.ORDER_MOVE,  AIFleetOrderType.ORDER_REPAIR, AIFleetOrderType.ORDER_RESUPPLY]:
                fleetID = self.get_source_target().target_id
                system_id = self.get_target_target().target_id
                fleet = fo.getUniverse().getFleet(fleetID)
                if  system_id not in [fleet.systemID,  fleet.nextSystemID] :
                    if self.get_fleet_order_type() == AIFleetOrderType.ORDER_MOVE:
                        dest_id = system_id
                    else:
                        if self.get_fleet_order_type() == AIFleetOrderType.ORDER_REPAIR:
                            fo.issueAggressionOrder(fleetID,  False)
                        start_id = [fleet.systemID,  fleet.nextSystemID][ fleet.systemID == -1 ]
                        dest_id = MoveUtilsAI.get_safe_path_leg_to_dest(fleetID,  start_id,  system_id)
                        print "fleet %d with order type(%s) sent to safe leg dest %s and ultimate dest %s"%(fleetID, AIFleetOrderTypeNames.name(self.get_fleet_order_type()),
                                                                                                            PlanetUtilsAI.sys_name_ids([dest_id]),
                                                                                                            PlanetUtilsAI.sys_name_ids([system_id]))
                    fo.issueFleetMoveOrder(fleetID, dest_id)
                if system_id == fleet.systemID:
                    if    foAI.foAIstate.get_fleet_role(fleetID) == AIFleetMissionType.FLEET_MISSION_EXPLORATION :
                        if system_id in foAI.foAIstate.needsEmergencyExploration:
                            del foAI.foAIstate.needsEmergencyExploration[ foAI.foAIstate.needsEmergencyExploration.index(system_id) ]
                    self.__set_execution_completed()

            # split fleet
            elif AIFleetOrderType.ORDER_SPLIT_FLEET == self.get_fleet_order_type():
                fleetID = self.get_source_target().target_id
                shipID = self.get_target_target().target_id
                fleet = fo.getUniverse().getFleet(fleetID)
                if shipID in fleet.shipIDs:
                    fo.issueNewFleetOrder(str(shipID), shipID)
                self.__set_execution_completed()
            # attack
            elif AIFleetOrderType.ORDER_ATTACK == self.get_fleet_order_type():
                fleetID = self.get_source_target().target_id
                systemID = self.get_target_target().get_required_system_ai_targets()[0].target_id

                fo.issueFleetMoveOrder(fleetID, systemID)

    def __str__(self):
        return "fleet order[%s] source:%26s | target %26s" %(  AIFleetOrderTypeNames.name(self.__aiFleetOrderType),  self.__sourceAITarget,   self.__targetAITarget)

    def __cmp__(self, other):
        """compares AIFleetOrders"""

        if other is None:
            return False
        if self.get_fleet_order_type() < other.get_fleet_order_type():
            return - 1
        elif self.get_fleet_order_type() == other.get_fleet_order_type():
            result = self.get_source_target().__cmp__(other.get_source_target())
            if result == 0:
                return self.get_target_target().__cmp__(other.get_target_target())
            else:
                return result
        return 1

    def __eq__(self, other):
        """returns equal to other object"""

        if other is None:
            return False
        if isinstance(other, AIFleetOrder):
            return self.__cmp__(other) == 0
        return NotImplemented

    def __ne__(self, other):
        """returns not equal to other object"""

        result = self.__eq__(other)
        if result is NotImplemented:
            return result
        return not result
