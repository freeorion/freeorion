from EnumsAI import AIFleetOrderType, AITargetType, AIShipRoleType, AIFleetMissionType
import FleetUtilsAI
import freeOrionAIInterface as fo  # pylint: disable=import-error
import FreeOrionAI as foAI
import MoveUtilsAI
import PlanetUtilsAI
from freeorion_tools import dict_from_map

AIFleetOrderTypeNames = AIFleetOrderType()
AIFleetMissionTypeNames = AIFleetMissionType()
dumpTurn = 0


class AIFleetOrder(object):
    """Stores information about orders which can be executed"""
    def __init__(self, fleet_order_type, fleet, target):
        self.order_type = fleet_order_type
        self.fleet = fleet
        self.target = target
        self.executed = False
        self.execution_completed = False

    def ship_in_fleet(self, ship, fleet):
        universe = fo.getUniverse()
        fleet = universe.getFleet(fleet.target_id)
        return ship.target_id in fleet.shipIDs

    def is_valid(self):
        """Check if FleetOrder could be somehow in future issued = is valid."""
        if self.executed and self.execution_completed:
            return False
        if self.fleet.valid and self.target.valid:
            sourceAITargetTypeValid = False
            targetAITargetTypeValid = False
            universe = fo.getUniverse()

            # outpost
            if AIFleetOrderType.ORDER_OUTPOST == self.order_type:
                # with ship
                if AITargetType.TARGET_SHIP == self.fleet.target_type:
                    ship = universe.getShip(self.fleet.target_id)
                    if ship.canColonize:
                        sourceAITargetTypeValid = True
                # with fleet
                elif AITargetType.TARGET_FLEET == self.fleet.target_type:
                    fleet = universe.getFleet(self.fleet.target_id)
                    if fleet.hasOutpostShips:
                        sourceAITargetTypeValid = True
                # colonise planet
                if AITargetType.TARGET_PLANET == self.target.target_type:
                    planet = universe.getPlanet(self.target.target_id)
                    sysPartialVisTurn = dict_from_map(universe.getVisibilityTurnsMap(planet.systemID, fo.empireID())).get(fo.visibility.partial, -9999)
                    planetPartialVisTurn = dict_from_map(universe.getVisibilityTurnsMap(planet.id, fo.empireID())).get(fo.visibility.partial, -9999)
                    if (planetPartialVisTurn == sysPartialVisTurn) and planet.unowned:
                        targetAITargetTypeValid = True
                    else:  # try to get order cancelled out
                        self.executed = True
                        self.execution_completed = True

            # colonise
            elif AIFleetOrderType.ORDER_COLONISE == self.order_type:
                # with ship
                if AITargetType.TARGET_SHIP == self.fleet.target_type:
                    ship = universe.getShip(self.fleet.target_id)
                    if ship.canColonize:
                        sourceAITargetTypeValid = True
                # with fleet
                elif AITargetType.TARGET_FLEET == self.fleet.target_type:
                    fleet = universe.getFleet(self.fleet.target_id)
                    if fleet.hasColonyShips:
                        sourceAITargetTypeValid = True
                # colonise planet
                if AITargetType.TARGET_PLANET == self.target.target_type:
                    planet = universe.getPlanet(self.target.target_id)
                    sysPartialVisTurn = dict_from_map(universe.getVisibilityTurnsMap(planet.systemID, fo.empireID())).get(fo.visibility.partial, -9999)
                    planetPartialVisTurn = dict_from_map(universe.getVisibilityTurnsMap(planet.id, fo.empireID())).get(fo.visibility.partial, -9999)

                    if planetPartialVisTurn == sysPartialVisTurn and planet.unowned or (planet.ownedBy(fo.empireID()) and not planet.currentMeterValue(fo.meterType.population)):
                        targetAITargetTypeValid = True
                    else:  # try to get order cancelled out
                        self.executed = True
                        self.execution_completed = True
            # invade
            elif AIFleetOrderType.ORDER_INVADE == self.order_type:
                # with ship
                if AITargetType.TARGET_SHIP == self.fleet.target_type:
                    ship = universe.getShip(self.fleet.target_id)
                    if ship.canInvade:
                        sourceAITargetTypeValid = True
                # with fleet
                elif AITargetType.TARGET_FLEET == self.fleet.target_type:
                    fleet = universe.getFleet(self.fleet.target_id)
                    if fleet.hasTroopShips:
                        sourceAITargetTypeValid = True
                # invade planet
                if AITargetType.TARGET_PLANET == self.target.target_type:
                    planet = universe.getPlanet(self.target.target_id)
                    planetPopulation = planet.currentMeterValue(fo.meterType.population)
                    if not planet.unowned or planetPopulation > 0:
                        targetAITargetTypeValid = True
                    else:  # try to get order cancelled out
                        self.executed = True
                        self.execution_completed = True
            # military
            elif AIFleetOrderType.ORDER_MILITARY == self.order_type:
                # with ship
                if AITargetType.TARGET_SHIP == self.fleet.target_type:
                    ship = universe.getShip(self.fleet.target_id)
                    if ship.isArmed:
                        sourceAITargetTypeValid = True
                # with fleet
                elif AITargetType.TARGET_FLEET == self.fleet.target_type:
                    fleet = universe.getFleet(self.fleet.target_id)
                    if fleet.hasArmedShips:
                        sourceAITargetTypeValid = True
                # military system
                if AITargetType.TARGET_SYSTEM == self.target.target_type:
                    targetAITargetTypeValid = True
            # move
            elif AIFleetOrderType.ORDER_MOVE == self.order_type:
                # with fleet
                if AITargetType.TARGET_FLEET == self.fleet.target_type:
                    sourceAITargetTypeValid = True
                # move to system
                if AITargetType.TARGET_SYSTEM == self.target.target_type:
                    targetAITargetTypeValid = True
            # resupply
            elif AIFleetOrderType.ORDER_RESUPPLY == self.order_type:
                # with fleet
                if AITargetType.TARGET_FLEET == self.fleet.target_type:
                    sourceAITargetTypeValid = True
                # move to system
                if AITargetType.TARGET_SYSTEM == self.target.target_type:
                    empire = fo.getEmpire()
                    fleet_suppliable_system_ids = empire.fleetSupplyableSystemIDs
                    if self.target.target_id in fleet_suppliable_system_ids:
                        targetAITargetTypeValid = True
            # repair
            elif AIFleetOrderType.ORDER_REPAIR == self.order_type:
                # with fleet
                if AITargetType.TARGET_FLEET == self.fleet.target_type:
                    sourceAITargetTypeValid = True
                # move to system
                if AITargetType.TARGET_SYSTEM == self.target.target_type:
                    empire = fo.getEmpire()
                    fleet_suppliable_system_ids = empire.fleetSupplyableSystemIDs
                    if self.target.target_id in fleet_suppliable_system_ids:  # TODO: check for drydock still there/owned
                        targetAITargetTypeValid = True
            # split fleet
            elif AIFleetOrderType.ORDER_SPLIT_FLEET == self.order_type:
                # with fleet
                if AITargetType.TARGET_FLEET == self.fleet.target_type:
                    sourceAITargetTypeValid = True
                # split ship
                if AITargetType.TARGET_SHIP == self.target.target_type:
                    targetAITargetTypeValid = True
                if sourceAITargetTypeValid and targetAITargetTypeValid:
                    if self.ship_in_fleet(self.target, self.fleet):
                        return True
            elif AIFleetOrderType.ORDER_ATTACK == self.order_type:
                # with fleet
                if AITargetType.TARGET_FLEET == self.fleet.target_type:
                    sourceAITargetTypeValid = True
                # move to system
                if AITargetType.TARGET_SYSTEM == self.target.target_type or AITargetType.TARGET_PLANET == self.target.target_type:
                    targetAITargetTypeValid = True
            elif AIFleetOrderType.ORDER_DEFEND == self.order_type:
                # with fleet
                if AITargetType.TARGET_FLEET == self.fleet.target_type:
                    sourceAITargetTypeValid = True
                # move to system
                if AITargetType.TARGET_SYSTEM == self.target.target_type or AITargetType.TARGET_PLANET == self.target.target_type:
                    targetAITargetTypeValid = True

            if sourceAITargetTypeValid and targetAITargetTypeValid:
                return True

        return False

    def can_issue_order(self, verbose=False):
        """If FleetOrder can be issued now."""
        # for some orders, may need to re-issue if invasion/outposting/colonization was interrupted
        if self.executed and self.order_type not in [AIFleetOrderType.ORDER_OUTPOST, AIFleetOrderType.ORDER_COLONISE, AIFleetOrderType.ORDER_INVADE]:
            return False
        if not self.is_valid():
            return False
        universe = fo.getUniverse()
        fleet_id = None
        ship_id = None
        if AITargetType.TARGET_SHIP == self.fleet.target_type:
            ship_id = self.fleet.target_id
            ship = universe.getShip(ship_id)
            fleet_id = ship.fleetID
        elif AITargetType.TARGET_FLEET == self.fleet.target_type:
            fleet_id = self.fleet.target_id
        fleet = universe.getFleet(fleet_id)

        system_id = fleet.systemID
        sys1 = universe.getSystem(system_id)
        sys_name = sys1 and sys1.name or "unknown"
        target_id = self.target.target_id

        if verbose:
            main_fleet_mission = foAI.foAIstate.get_fleet_mission(fleet_id)
            main_mission_type = (main_fleet_mission.get_mission_types() + [-1])[0]
            print "** %s -- Mission Type  %s (%s) , current loc sys %d  - %s" % (self, AIFleetMissionTypeNames.name(main_mission_type), main_mission_type, system_id, sys_name)
        #
        # outpost
        #
        if AIFleetOrderType.ORDER_OUTPOST == self.order_type:  # TODO: check for separate fleet holding outpost ships
            if AITargetType.TARGET_FLEET == self.fleet.target_type:
                ship_id = FleetUtilsAI.get_ship_id_with_role(fleet_id, AIShipRoleType.SHIP_ROLE_CIVILIAN_OUTPOST)
                if ship_id is None:
                    ship_id = FleetUtilsAI.get_ship_id_with_role(fleet_id, AIShipRoleType.SHIP_ROLE_BASE_OUTPOST)
            ship = universe.getShip(ship_id)
            planet = universe.getPlanet(self.target.target_id)
            if (ship is not None) and (fleet.systemID == planet.systemID) and ship.canColonize:
                return True
            return False
        #
        # colonise
        #
        elif AIFleetOrderType.ORDER_COLONISE == self.order_type:  # TODO: check for separate fleet holding colony ships
            if AITargetType.TARGET_FLEET == self.fleet.target_type:
                ship_id = FleetUtilsAI.get_ship_id_with_role(fleet_id, AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION)
                if ship_id is None:
                    ship_id = FleetUtilsAI.get_ship_id_with_role(fleet_id, AIShipRoleType.SHIP_ROLE_BASE_COLONISATION)
            ship = universe.getShip(ship_id)
            planet = universe.getPlanet(self.target.target_id)
            if ship and not ship.canColonize:
                print "Error: colonization fleet %d has no colony ship" % fleet_id
            if (ship is not None) and (fleet.systemID == planet.systemID) and ship.canColonize:
                return True
            return False
        #
        # invade
        #
        elif AIFleetOrderType.ORDER_INVADE == self.order_type:  # TODO: check for separate fleet holding invasion ships
            if AITargetType.TARGET_FLEET == self.fleet.target_type:
                ship_id = FleetUtilsAI.get_ship_id_with_role(fleet_id, AIShipRoleType.SHIP_ROLE_MILITARY_INVASION, False)
                if ship_id is None:
                    ship_id = FleetUtilsAI.get_ship_id_with_role(fleet_id, AIShipRoleType.SHIP_ROLE_BASE_INVASION)
            ship = universe.getShip(ship_id)
            planet = universe.getPlanet(self.target.target_id)
            if ship is not None and fleet.systemID == planet.systemID and ship.canInvade and not planet.currentMeterValue(fo.meterType.shield):
                return True
            return False
        #
        # military
        #
        elif AIFleetOrderType.ORDER_MILITARY == self.order_type:
            if AITargetType.TARGET_FLEET == self.fleet.target_type:
                ship_id = FleetUtilsAI.get_ship_id_with_role(fleet_id, AIShipRoleType.SHIP_ROLE_MILITARY)
            ship = universe.getShip(ship_id)
            system = universe.getSystem(self.target.target_id)
            if (ship is not None) and (fleet.systemID == system.systemID) and ship.isArmed:
                return True
            return False
        #
        # split fleet
        #
        elif AIFleetOrderType.ORDER_SPLIT_FLEET == self.order_type:
            fleet2 = universe.getFleet(self.fleet.target_id)
            if len(fleet2.shipIDs) <= 1:
                return False
        #
        # move -- have fleets will do a safety check, also check for potential military fleet mergers
        #
        elif AIFleetOrderType.ORDER_MOVE == self.order_type:
            #  target_id = self.target.target_id
            # TODO: figure out better way to have invasions (& possibly colonizations) require visibility on target without needing visibility of all intermediate systems
            if False and main_mission_type not in [AIFleetMissionType.FLEET_MISSION_ATTACK,  # TODO: consider this later
                                                 AIFleetMissionType.FLEET_MISSION_MILITARY,
                                                 AIFleetMissionType.FLEET_MISSION_SECURE,
                                                 AIFleetMissionType.FLEET_MISSION_HIT_AND_RUN,
                                                 AIFleetMissionType.FLEET_MISSION_EXPLORATION]:
                if not (universe.getVisibility(target_id, foAI.foAIstate.empireID) >= fo.visibility.partial):
                    #if not target_id in interior systems
                    foAI.foAIstate.needsEmergencyExploration.append(fleet.systemID)
                    return False

            system_id = fleet.systemID
            if system_id == target_id:
                return True  # already there so no point to worry about threat
            sys1 = universe.getSystem(system_id)
            sys1_name = sys1 and sys1.name or "unknown"
            targ1 = universe.getSystem(target_id)
            targ1_name = (targ1 and targ1.name) or "unknown"
            fleetRating = foAI.foAIstate.get_rating(fleet_id).get('overall', 0)
            target_sys_status = foAI.foAIstate.systemStatus.get(target_id, {})
            f_threat = target_sys_status.get('fleetThreat', 0)
            m_threat = target_sys_status.get('monsterThreat', 0)
            p_threat = target_sys_status.get('planetThreat', 0)
            threat = f_threat + m_threat + p_threat

            safetyFactor = 1.1
            if fleetRating >= safetyFactor * threat:
                return True
            else:
                # following line was poor because AIstate.militaryFleetIDs only covers fleets without current missions
                # myOtherFleetsRating = sum([foAI.foAIstate.fleetStatus.get(fleet_id, {}).get('rating', 0) for fleet_id in foAI.foAIstate.militaryFleetIDs if ( foAI.foAIstate.fleetStatus.get(fleet_id, {}).get('sysID', -1) == thisSystemID ) ])
                # myOtherFleetsRatings = [foAI.foAIstate.fleetStatus.get(fid, {}).get('rating', {}) for fid in foAI.foAIstate.systemStatus.get(target_id, {}).get('myfleets', [])]
                # myOtherFleetsRating = sum([foAI.foAIstate.fleetStatus.get(fid, {}).get('rating', 0) for fid in foAI.foAIstate.systemStatus.get( target_id, {}).get('myfleets', []) ])
                myOtherFleetsRating = foAI.foAIstate.systemStatus.get(target_id, {}).get('myFleetRating', 0)
                if myOtherFleetsRating > 1.5 * safetyFactor * threat or myOtherFleetsRating + fleetRating > 2.0 * safetyFactor * threat:
                    if verbose:
                        print "\tAdvancing fleet %d (rating %d) at system %d (%s) into system %d (%s) with threat %d because of sufficient empire fleet strength already at destination" % (fleet_id, fleetRating, system_id, sys1_name, target_id, targ1_name, threat)
                    return True
                elif threat == p_threat and not fleet.aggressive and not myOtherFleetsRating and not target_sys_status.get('localEnemyFleetIDs', [-1]):
                    if verbose:
                        print ("\tAdvancing fleet %d (rating %d) at system %d (%s) into system %d (%s) with planet threat %d because nonaggressive" +
                               " and no other fleets present to trigger combat") % (fleet_id, fleetRating, system_id, sys1_name, target_id, targ1_name, threat)
                    return True
                else:
                    if verbose:
                        print "\tHolding fleet %d (rating %d) at system %d (%s) before travelling to system %d (%s) with threat %d" % (fleet_id, fleetRating, system_id, sys1_name, target_id, targ1_name, threat)
                    return False
        else:  # default returns true
            return True

    def issue_order(self):
        global dumpTurn
        if not self.can_issue_order():  # appears to be redundant with check in IAFleetMission?
            print "\tcan't issue %s" % self
        else:
            universe = fo.getUniverse()
            self.executed = True
            # outpost
            if AIFleetOrderType.ORDER_OUTPOST == self.order_type:
                planet = universe.getPlanet(self.target.target_id)
                if not planet.unowned:
                    self.execution_completed = True
                    return
                ship_id = None
                if AITargetType.TARGET_SHIP == self.fleet.target_type:
                    ship_id = self.fleet.target_id
                elif AITargetType.TARGET_FLEET == self.fleet.target_type:
                    fleet_id = self.fleet.target_id
                    ship_id = FleetUtilsAI.get_ship_id_with_role(fleet_id, AIShipRoleType.SHIP_ROLE_CIVILIAN_OUTPOST)
                    if ship_id is None:
                        ship_id = FleetUtilsAI.get_ship_id_with_role(fleet_id, AIShipRoleType.SHIP_ROLE_BASE_OUTPOST)
                result = fo.issueColonizeOrder(ship_id, self.target.target_id)
                if not result:
                    self.executed = False
            # colonise
            elif AIFleetOrderType.ORDER_COLONISE == self.order_type:
                ship_id = None
                if AITargetType.TARGET_SHIP == self.fleet.target_type:
                    ship_id = self.fleet.target_id
                elif AITargetType.TARGET_FLEET == self.fleet.target_type:
                    fleet_id = self.fleet.target_id
                    ship_id = FleetUtilsAI.get_ship_id_with_role(fleet_id, AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION)
                    if ship_id is None:
                        ship_id = FleetUtilsAI.get_ship_id_with_role(fleet_id, AIShipRoleType.SHIP_ROLE_BASE_COLONISATION)

                planet_id = self.target.target_id
                planet = universe.getPlanet(planet_id)
                planet_name = planet and planet.name or "apparently invisible"
                result = fo.issueColonizeOrder(ship_id, planet_id)
                print "Ordered colony ship ID %d to colonize %s, got result %d" % (ship_id, planet_name, result)
                if not result:
                    self.executed = False
            # invade
            elif AIFleetOrderType.ORDER_INVADE == self.order_type:
                result = False
                ship_id = None
                planet_id = self.target.target_id
                planet = universe.getPlanet(planet_id)
                planet_name = planet and planet.name or "invisible"
                if AITargetType.TARGET_SHIP == self.fleet.target_type:
                    ship_id = self.fleet.target_id
                elif AITargetType.TARGET_FLEET == self.fleet.target_type:
                    fleet_id = self.fleet.target_id
                    fleet = fo.getUniverse().getFleet(fleet_id)
                    for ship_id in fleet.shipIDs:
                        ship = universe.getShip(ship_id)
                        if foAI.foAIstate.get_ship_role(ship.design.id) in [AIShipRoleType.SHIP_ROLE_MILITARY_INVASION, AIShipRoleType.SHIP_ROLE_BASE_INVASION]:
                            result = fo.issueInvadeOrder(ship_id, planet_id) or result  # will track if at least one invasion troops successfully deployed
                            detailStr = ""
                            if not result:
                                pstealth = planet.currentMeterValue(fo.meterType.stealth)
                                pop = planet.currentMeterValue(fo.meterType.population)
                                shields = planet.currentMeterValue(fo.meterType.shield)
                                owner = planet.owner
                                detailStr = " -- planet has %.1f stealth, shields %.1f, %.1f population and is owned by empire %d" % (pstealth, shields, pop, owner)
                            print "Ordered troop ship ID %d to invade %s, got result %d" % (ship_id, planet_name, result), detailStr
                            if not result:
                                if 'needsEmergencyExploration' not in dir(foAI.foAIstate):
                                    foAI.foAIstate.needsEmergencyExploration = []
                                if fleet.systemID not in foAI.foAIstate.needsEmergencyExploration:
                                    foAI.foAIstate.needsEmergencyExploration.append(fleet.systemID)
                                    print "Due to trouble invading, adding system %d to Emergency Exploration List" % fleet.systemID
                                    self.executed = False
                                if shields > 0 and owner == -1 and dumpTurn < fo.currentTurn():
                                    dumpTurn = fo.currentTurn()
                                    print "Universe Dump to debug invasions:"
                                    universe.dump()
                                break
                if result:
                    print "Successfully ordered troop ship(s) to invade %s, with detail %s" % (planet_name, detailStr)
            # military
            elif AIFleetOrderType.ORDER_MILITARY == self.order_type:
                ship_id = None
                if AITargetType.TARGET_SHIP == self.fleet.target_type:
                    ship_id = self.fleet.target_id
                elif AITargetType.TARGET_FLEET == self.fleet.target_type:
                    fleet_id = self.fleet.target_id
                    ship_id = FleetUtilsAI.get_ship_id_with_role(fleet_id, AIShipRoleType.SHIP_ROLE_MILITARY)
                #fo.issueFleetMoveOrder(fleet_id, self.target.target_id) #moving is already taken care of separately
                targetSysID = self.target.target_id
                fleet = fo.getUniverse().getFleet(fleet_id)
                systemStatus = foAI.foAIstate.systemStatus.get(targetSysID, {})
                if fleet and fleet.systemID == targetSysID and not (systemStatus.get('fleetThreat', 0) + systemStatus.get('planetThreat', 0) + systemStatus.get('monsterThreat', 0)):
                    self.execution_completed = True

            # move or resupply
            elif self.order_type in [AIFleetOrderType.ORDER_MOVE, AIFleetOrderType.ORDER_REPAIR, AIFleetOrderType.ORDER_RESUPPLY]:
                fleet_id = self.fleet.target_id
                system_id = self.target.target_id
                fleet = fo.getUniverse().getFleet(fleet_id)
                if system_id not in [fleet.systemID, fleet.nextSystemID]:
                    if self.order_type == AIFleetOrderType.ORDER_MOVE:
                        dest_id = system_id
                    else:
                        if self.order_type == AIFleetOrderType.ORDER_REPAIR:
                            fo.issueAggressionOrder(fleet_id, False)
                        start_id = [fleet.systemID, fleet.nextSystemID][fleet.systemID == -1]
                        dest_id = MoveUtilsAI.get_safe_path_leg_to_dest(fleet_id, start_id, system_id)
                        print "fleet %d with order type(%s) sent to safe leg dest %s and ultimate dest %s" % (fleet_id, AIFleetOrderTypeNames.name(self.order_type),
                                                                                                            PlanetUtilsAI.sys_name_ids([dest_id]),
                                                                                                            PlanetUtilsAI.sys_name_ids([system_id]))
                    fo.issueFleetMoveOrder(fleet_id, dest_id)
                if system_id == fleet.systemID:
                    if foAI.foAIstate.get_fleet_role(fleet_id) == AIFleetMissionType.FLEET_MISSION_EXPLORATION:
                        if system_id in foAI.foAIstate.needsEmergencyExploration:
                            del foAI.foAIstate.needsEmergencyExploration[foAI.foAIstate.needsEmergencyExploration.index(system_id)]
                    self.execution_completed = True

            # split fleet
            elif AIFleetOrderType.ORDER_SPLIT_FLEET == self.order_type:
                fleet_id = self.fleet.target_id
                ship_id = self.target.target_id
                fleet = fo.getUniverse().getFleet(fleet_id)
                if ship_id in fleet.shipIDs:
                    fo.issueNewFleetOrder(str(ship_id), ship_id)
                self.execution_completed = True
            # attack
            elif AIFleetOrderType.ORDER_ATTACK == self.order_type:
                fleet_id = self.fleet.target_id
                systemID = self.target.get_required_system_ai_targets()[0].target_id

                fo.issueFleetMoveOrder(fleet_id, systemID)

    def __str__(self):
        return "fleet order[%s] source:%26s | target %26s" % (AIFleetOrderTypeNames.name(self.order_type), self.fleet, self.target)

    def __cmp__(self, other):
        """compares AIFleetOrders"""
        if other is None:
            return False
        if self.order_type < other.order_type:
            return - 1
        elif self.order_type == other.order_type:
            result = self.fleet.__cmp__(other.fleet)
            if result == 0:
                return self.target.__cmp__(other.target)
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
