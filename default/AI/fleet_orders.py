from EnumsAI import AIShipRoleType, AIFleetMissionType
import FleetUtilsAI
import freeOrionAIInterface as fo  # pylint: disable=import-error
import FreeOrionAI as foAI
import MilitaryAI
import MoveUtilsAI
import PlanetUtilsAI
from freeorion_tools import print_error
from universe_object import Fleet, System, Planet

dumpTurn = 0


class AIFleetOrder(object):
    """Stores information about orders which can be executed."""
    TARGET_TYPE = None

    def __init__(self, fleet, target):
        """
        :param fleet: fleet to execute order
        :type fleet: universe_object.Fleet
        :param target: fleet target, depends of order type
        :type target: universe_object.UniverseObject
        """
        if not isinstance(fleet, Fleet):
            print_error("Order required fleet got %s" % type(fleet))

        if not isinstance(target, self.TARGET_TYPE):
            print_error("Target is not allowed, got %s expect %s" % (type(target), self.TARGET_TYPE))

        self.fleet = fleet
        self.target = target
        self.executed = False
        self.order_issued = False

    def ship_in_fleet(self):
        universe = fo.getUniverse()
        fleet = universe.getFleet(self.fleet.id)
        return self.target.id in fleet.shipIDs

    def is_valid(self):
        """Check if FleetOrder could be somehow in future issued = is valid."""
        if self.executed and self.order_issued:
            print "\t\t order not valid because already executed and completed"
            return False
        if self.fleet and self.target:
            return True
        else:
            print "\t\t order not valid: fleet validity: %s and target validity %s" % (bool(self.fleet), bool(self.target))
            return False

    def can_issue_order(self, verbose=False):
        """If FleetOrder can be issued now."""
        # for some orders, may need to re-issue if invasion/outposting/colonization was interrupted
        if self.executed and not isinstance(self, (OrderOutpost, OrderColonize, OrderInvade)):
            return False
        if not self.is_valid():
            return False

        if verbose:
            sys1 = self.fleet.get_object()
            sys_name = sys1 and sys1.name or "unknown"
            main_fleet_mission = foAI.foAIstate.get_fleet_mission(self.fleet.id)
            print "** %s -- Mission Type %s (%s) , current loc sys %d - %s" % (
                                                                self, AIFleetMissionType.name(main_fleet_mission.type),
                                                                main_fleet_mission.type, self.fleet.id, sys_name)
        return True

    def issue_order(self):
        if not self.can_issue_order():  # appears to be redundant with check in IAFleetMission?
            print "\tcan't issue %s" % self
            return False
        else:
            self.executed = True  # TODO check that it is really executed
            return True

    def __str__(self):
        execute_status = 'in progress'
        if self.executed:
            execute_status = 'executed'
        elif self.order_issued:
            execute_status = 'order issued'
        return "Fleet order[%s] source:%26s | target %26s %s" % (self.ORDER_NAME, self.fleet, self.target, execute_status)

    def __eq__(self, other):
        return type(self) == type(other) and self.fleet == other.fleet and self.target == other.target


class OrderMove(AIFleetOrder):
    ORDER_NAME = 'move'
    TARGET_TYPE = System

    def can_issue_order(self, verbose=False):
        if not super(OrderMove, self).can_issue_order(verbose=verbose):
            return False
        # TODO: figure out better way to have invasions (& possibly colonizations) require visibility on target without needing visibility of all intermediate systems
        # if False and main_mission_type not in [AIFleetMissionType.FLEET_MISSION_ATTACK,  # TODO: consider this later
        #                                      AIFleetMissionType.FLEET_MISSION_MILITARY,
        #                                      AIFleetMissionType.FLEET_MISSION_SECURE,
        #                                      AIFleetMissionType.FLEET_MISSION_HIT_AND_RUN,
        #                                      AIFleetMissionType.FLEET_MISSION_EXPLORATION]:
        #     if not universe.getVisibility(target_id, foAI.foAIstate.empireID) >= fo.visibility.partial:
        #         #if not target_id in interior systems
        #         foAI.foAIstate.needsEmergencyExploration.append(fleet.systemID)
        #         return False
        system_id = self.fleet.get_system().id
        if system_id == self.target.get_system().id:
            return True  # TODO: already there, but could consider retreating

        fleet_rating = foAI.foAIstate.get_rating(self.fleet.id).get('overall', 0)
        target_sys_status = foAI.foAIstate.systemStatus.get(self.target.id, {})
        f_threat = target_sys_status.get('fleetThreat', 0)
        m_threat = target_sys_status.get('monsterThreat', 0)
        p_threat = target_sys_status.get('planetThreat', 0)
        threat = f_threat + m_threat + p_threat
        safety_factor = MilitaryAI.get_safety_factor()
        universe = fo.getUniverse()
        if fleet_rating >= safety_factor * threat:
            return True
        elif not p_threat and self.target.id in fo.getEmpire().supplyUnobstructedSystems:
                    return True
        else:
            sys1 = universe.getSystem(system_id)
            sys1_name = sys1 and sys1.name or "unknown"
            targ1 = self.target.get_system()
            targ1_name = (targ1 and targ1.get_object().name) or "unknown"
            # following line was poor because AIstate.militaryFleetIDs only covers fleets without current missions
            # my_other_fleet_rating = sum([foAI.foAIstate.fleetStatus.get(fleet_id, {}).get('rating', 0) for fleet_id in foAI.foAIstate.militaryFleetIDs if ( foAI.foAIstate.fleetStatus.get(fleet_id, {}).get('sysID', -1) == thisSystemID ) ])
            # myOtherFleetsRatings = [foAI.foAIstate.fleetStatus.get(fid, {}).get('rating', {}) for fid in foAI.foAIstate.systemStatus.get(target_id, {}).get('myfleets', [])]
            # my_other_fleet_rating = sum([foAI.foAIstate.fleetStatus.get(fid, {}).get('rating', 0) for fid in foAI.foAIstate.systemStatus.get( target_id, {}).get('myfleets', []) ])
            my_other_fleet_rating = foAI.foAIstate.systemStatus.get(self.target.id, {}).get('myFleetRating', 0)  # TODO: adjust calc for any departing fleets
            is_military = foAI.foAIstate.get_fleet_role(self.fleet.id) == AIFleetMissionType.FLEET_MISSION_MILITARY
            if ((my_other_fleet_rating > 3 * safety_factor * threat) or
                        (is_military and my_other_fleet_rating + fleet_rating > safety_factor * threat) or
                        (is_military and my_other_fleet_rating + fleet_rating > 0.8 * safety_factor * threat and fleet_rating > 0.2 * threat)):
                if verbose:
                    print "\tAdvancing fleet %d (rating %d) at system %d (%s) into system %d (%s) with threat %d because of sufficient empire fleet strength already at destination" % (
                        self.fleet.id, fleet_rating, system_id, sys1_name, self.target.id, targ1_name, threat)
                return True
            elif threat == p_threat and not self.fleet.get_object().aggressive and not my_other_fleet_rating and not target_sys_status.get('localEnemyFleetIDs', [-1]):
                if verbose:
                    print ("\tAdvancing fleet %d (rating %d) at system %d (%s) into system %d (%s) with planet threat %d because nonaggressive" +
                           " and no other fleets present to trigger combat") % (self.fleet.id, fleet_rating, system_id, sys1_name, self.target.id, targ1_name, threat)
                return True
            else:
                if verbose:
                    print "\tHolding fleet %d (rating %d) at system %d (%s) before travelling to system %d (%s) with threat %d" % (self.fleet.id, fleet_rating, system_id, sys1_name, self.target.id, targ1_name, threat)
                needs_vis = foAI.foAIstate.misc.setdefault('needs_vis', [])
                if self.target.id not in needs_vis:
                    needs_vis.append(self.target.id)
                return False
        return True

    def issue_order(self):
        if not super(OrderMove, self).issue_order():
            return
        fleet_id = self.fleet.id
        system_id = self.target.get_system().id
        fleet = self.fleet.get_object()
        if system_id not in [fleet.systemID, fleet.nextSystemID]:
            dest_id = system_id
            fo.issueFleetMoveOrder(fleet_id, dest_id)
            print "Order issued: %s fleet: %s target: %s" % (self.ORDER_NAME, self.fleet, self.target)

        if system_id == fleet.systemID:
            if foAI.foAIstate.get_fleet_role(fleet_id) == AIFleetMissionType.FLEET_MISSION_EXPLORATION:
                if system_id in foAI.foAIstate.needsEmergencyExploration:
                    del foAI.foAIstate.needsEmergencyExploration[foAI.foAIstate.needsEmergencyExploration.index(system_id)]
            self.order_issued = True


class OrderResupply(AIFleetOrder):
    ORDER_NAME = 'resupply'
    TARGET_TYPE = System

    def is_valid(self):
        if not super(OrderResupply, self).is_valid():
            return False
        return self.target.id in fo.getEmpire().fleetSupplyableSystemIDs

    def issue_order(self):
        if not super(OrderResupply, self).issue_order():
            return
        fleet_id = self.fleet.id
        system_id = self.target.get_system().id
        fleet = self.fleet.get_object()
        if system_id not in [fleet.systemID, fleet.nextSystemID]:
            # if self.order_type == AIFleetOrderType.ORDER_MOVE:
            #     dest_id = system_id
            # else:
            # if self.order_type == AIFleetOrderType.ORDER_REPAIR:
            #     fo.issueAggressionOrder(fleet_id, False)
            start_id = [fleet.systemID, fleet.nextSystemID][fleet.systemID == -1]
            dest_id = MoveUtilsAI.get_safe_path_leg_to_dest(fleet_id, start_id, system_id)
            print "fleet %d with order type(%s) sent to safe leg dest %s and ultimate dest %s" % (fleet_id, self.ORDER_NAME,
                                                                                                PlanetUtilsAI.sys_name_ids([dest_id]),
                                                                                                PlanetUtilsAI.sys_name_ids([system_id]))
            fo.issueFleetMoveOrder(fleet_id, dest_id)
            print "Order issued: %s fleet: %s target: %s" % (self.ORDER_NAME, self.fleet, self.target)

        if system_id == fleet.systemID:
            if foAI.foAIstate.get_fleet_role(fleet_id) == AIFleetMissionType.FLEET_MISSION_EXPLORATION:
                if system_id in foAI.foAIstate.needsEmergencyExploration:
                    del foAI.foAIstate.needsEmergencyExploration[foAI.foAIstate.needsEmergencyExploration.index(system_id)]
            self.order_issued = True


class OrderSplitFleet(AIFleetOrder):
    ORDER_NAME = 'split_fleet'
    TARGET_TYPE = System  # TODO check real usage

    def can_issue_order(self, verbose=False):
        if not super(OrderSplitFleet, self).is_valid():
            return False
        return len(self.fleet.get_object().shipIDs) > 1

    def issue_order(self):
        if not super(OrderSplitFleet, self).issue_order():
            return
        ship_id = self.target.id
        fleet = self.fleet.get_object()
        if ship_id in fleet.shipIDs:
            fo.issueNewFleetOrder(str(ship_id), ship_id)
            print "Order issued: %s fleet: %s target: %s" % (self.ORDER_NAME, self.fleet, self.target)
        self.order_issued = True


# class OrderMergeFleet(AIFleetOrder):  # TODO check remove
#     pass


class OrderOutpost(AIFleetOrder):
    ORDER_NAME = 'outpost'
    TARGET_TYPE = Planet

    def is_valid(self):
        if not super(OrderOutpost, self).is_valid():
            return False
        universe = fo.getUniverse()
        planet = self.target.get_object()
        sys_partial_vis_turn = universe.getVisibilityTurnsMap(planet.systemID, fo.empireID()).get(fo.visibility.partial, -9999)
        planet_partial_vis_turn = universe.getVisibilityTurnsMap(planet.id, fo.empireID()).get(fo.visibility.partial, -9999)
        if not (planet_partial_vis_turn == sys_partial_vis_turn and planet.unowned):
            self.executed = True
            self.order_issued = True
            return False
        else:
            return self.fleet.get_object().hasOutpostShips

    def can_issue_order(self, verbose=False):
        # TODO: check for separate fleet holding outpost ships
        if not super(OrderOutpost, self).can_issue_order(verbose=verbose):
            return False
        universe = fo.getUniverse()
        ship_id = FleetUtilsAI.get_ship_id_with_role(self.fleet.id, AIShipRoleType.SHIP_ROLE_CIVILIAN_OUTPOST)
        if ship_id is None:
            ship_id = FleetUtilsAI.get_ship_id_with_role(self.fleet.id, AIShipRoleType.SHIP_ROLE_BASE_OUTPOST)
        ship = universe.getShip(ship_id)
        return ship is not None and self.fleet.get_system() == self.target.get_system() and ship.canColonize

    def issue_order(self):
        if not super(OrderOutpost, self).issue_order():
            return
        planet = self.target.get_object()
        if not planet.unowned:
            self.order_issued = True
            return
        fleet_id = self.fleet.id
        ship_id = FleetUtilsAI.get_ship_id_with_role(fleet_id, AIShipRoleType.SHIP_ROLE_CIVILIAN_OUTPOST)
        if ship_id is None:
            ship_id = FleetUtilsAI.get_ship_id_with_role(fleet_id, AIShipRoleType.SHIP_ROLE_BASE_OUTPOST)
        result = fo.issueColonizeOrder(ship_id, self.target.id)
        print "Order issued: %s fleet: %s target: %s" % (self.ORDER_NAME, self.fleet, self.target)
        if not result:
            self.executed = False


class OrderColonize(AIFleetOrder):
    ORDER_NAME = 'colonize'
    TARGET_TYPE = Planet

    def issue_order(self):
        if not super(OrderColonize, self).issue_order():
            return

        fleet_id = self.fleet.id
        ship_id = FleetUtilsAI.get_ship_id_with_role(fleet_id, AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION)
        if ship_id is None:
            ship_id = FleetUtilsAI.get_ship_id_with_role(fleet_id, AIShipRoleType.SHIP_ROLE_BASE_COLONISATION)

        planet = self.target.get_object()
        planet_name = planet and planet.name or "apparently invisible"
        result = fo.issueColonizeOrder(ship_id, self.target.id)
        print "Order issued: %s fleet: %s target: %s" % (self.ORDER_NAME, self.fleet, self.target)
        print "Ordered colony ship ID %d to colonize %s, got result %d" % (ship_id, planet_name, result)
        if not result:
            self.executed = False

    def is_valid(self):
        if not super(OrderColonize, self).is_valid():
            return False
        universe = fo.getUniverse()
        planet = self.target.get_object()
        sys_partial_vis_turn = universe.getVisibilityTurnsMap(planet.systemID, fo.empireID()).get(fo.visibility.partial, -9999)
        planet_partial_vis_turn = universe.getVisibilityTurnsMap(planet.id, fo.empireID()).get(fo.visibility.partial, -9999)
        if not (planet_partial_vis_turn == sys_partial_vis_turn and planet.unowned or (planet.ownedBy(fo.empireID()) and not planet.currentMeterValue(fo.meterType.population))):
            self.executed = True
            self.order_issued = True
            return False
        else:
            return self.fleet.get_object().hasColonyShips

    def can_issue_order(self, verbose=False):
        if not super(OrderColonize, self).is_valid():
            return False
        # TODO: check for separate fleet holding colony ships
        ship_id = FleetUtilsAI.get_ship_id_with_role(self.fleet.id, AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION)
        if ship_id is None:
            ship_id = FleetUtilsAI.get_ship_id_with_role(self.fleet.id, AIShipRoleType.SHIP_ROLE_BASE_COLONISATION)
        universe = fo.getUniverse()
        ship = universe.getShip(ship_id)
        if ship and not ship.canColonize:
            print "Error: colonization fleet %d has no colony ship" % self.fleet.id
        return ship is not None and self.fleet.get_system() == self.target.get_system() and ship.canColonize


class OrderAttack(AIFleetOrder):
    ORDER_NAME = 'attack'
    TARGET_TYPE = System

    def issue_order(self, verbose=False):
        if not super(OrderAttack, self).is_valid():
            return
        fo.issueFleetMoveOrder(self.fleet.id, self.target.get_system().id)
        print "Order issued: %s fleet: %s target: %s" % (self.ORDER_NAME, self.fleet, self.target)


class OrderDefend(AIFleetOrder):
    """
        Used for orbital defense, have no real orders.
    """
    ORDER_NAME = 'defend'
    TARGET_TYPE = System


class OrderInvade(AIFleetOrder):
    ORDER_NAME = 'invade'
    TARGET_TYPE = Planet

    def is_valid(self):
        if not super(OrderInvade, self).is_valid():
            return False
        planet = self.target.get_object()
        planet_population = planet.currentMeterValue(fo.meterType.population)
        if planet.unowned and not planet_population:
            print "\t\t invasion order not valid due to target planet status-- owned: %s and population %.1f" % (not planet.unowned, planet_population)
            self.executed = True
            self.order_issued = True
            return False
        else:
            return self.fleet.get_object().hasTroopShips

    def can_issue_order(self, verbose=False):
        if not super(OrderInvade, self).is_valid():
            return False
        # TODO: check for separate fleet holding invasion ships
        ship_id = FleetUtilsAI.get_ship_id_with_role(self.fleet.id, AIShipRoleType.SHIP_ROLE_MILITARY_INVASION, False)
        if ship_id is None:
            ship_id = FleetUtilsAI.get_ship_id_with_role(self.fleet.id, AIShipRoleType.SHIP_ROLE_BASE_INVASION)
        universe = fo.getUniverse()
        ship = universe.getShip(ship_id)
        planet = self.target.get_object()
        return ship is not None and self.fleet.get_system().id == planet.systemID \
               and ship.canInvade and not planet.currentMeterValue(fo.meterType.shield)

    def issue_order(self):
        if not super(OrderInvade, self).can_issue_order():
            return
        result = False
        planet_id = self.target.id
        planet = self.target.get_object()
        planet_name = planet and planet.name or "invisible"
        fleet = self.fleet.get_object()
        detail_str = ""
        universe = fo.getUniverse()

        global dumpTurn
        for ship_id in fleet.shipIDs:
            ship = universe.getShip(ship_id)
            if foAI.foAIstate.get_ship_role(ship.design.id) in [AIShipRoleType.SHIP_ROLE_MILITARY_INVASION, AIShipRoleType.SHIP_ROLE_BASE_INVASION]:
                result = fo.issueInvadeOrder(ship_id, planet_id) or result  # will track if at least one invasion troops successfully deployed
                print "Order issued: %s fleet: %s target: %s" % (self.ORDER_NAME, self.fleet, self.target)
                shields = planet.currentMeterValue(fo.meterType.shield)
                owner = planet.owner
                if not result:
                    pstealth = planet.currentMeterValue(fo.meterType.stealth)
                    pop = planet.currentMeterValue(fo.meterType.population)
                    detail_str = " -- planet has %.1f stealth, shields %.1f, %.1f population and is owned by empire %d" % (pstealth, shields, pop, owner)
                print "Ordered troop ship ID %d to invade %s, got result %d" % (ship_id, planet_name, result), detail_str
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
            print "Successfully ordered troop ship(s) to invade %s, with detail %s" % (planet_name, detail_str)


class OrderMilitary(AIFleetOrder):
    ORDER_NAME = 'military'
    TARGET_TYPE = System

    def is_valid(self):
        if not super(OrderMilitary, self).is_valid():
            return False
        return self.fleet.get_object().hasArmedShips

    def can_issue_order(self, verbose=False):
        ship_id = FleetUtilsAI.get_ship_id_with_role(self.fleet.id, AIShipRoleType.SHIP_ROLE_MILITARY)
        universe = fo.getUniverse()
        ship = universe.getShip(ship_id)
        return ship is not None and self.fleet.get_system() == self.target and ship.isArmed

    def issue_order(self):
        if not super(OrderMilitary, self).issue_order():
            return
        target_sys_id = self.target.id
        fleet = self.target.get_object()
        system_status = foAI.foAIstate.systemStatus.get(target_sys_id, {})
        if fleet and fleet.systemID == target_sys_id and not (system_status.get('fleetThreat', 0) + system_status.get('planetThreat', 0) + system_status.get('monsterThreat', 0)):
            self.order_issued = True


class OrderRepair(AIFleetOrder):
    ORDER_NAME = 'repair'
    TARGET_TYPE = System

    def is_valid(self):
        if not super(OrderRepair, self).is_valid():
            return False
        return self.target.id in fo.getEmpire().fleetSupplyableSystemIDs  # TODO: check for drydock still there/owned

    def issue_order(self):
        if not super(OrderRepair, self).issue_order():
            return
        fleet_id = self.fleet.id
        system_id = self.target.get_system().id
        fleet = self.fleet.get_object()
        if system_id not in [fleet.systemID, fleet.nextSystemID]:
            fo.issueAggressionOrder(fleet_id, False)
            start_id = [fleet.systemID, fleet.nextSystemID][fleet.systemID == -1]
            dest_id = MoveUtilsAI.get_safe_path_leg_to_dest(fleet_id, start_id, system_id)
            print "fleet %d with order type(%s) sent to safe leg dest %s and ultimate dest %s" % (fleet_id, self.ORDER_NAME,
                                                                                                PlanetUtilsAI.sys_name_ids([dest_id]),
                                                                                                PlanetUtilsAI.sys_name_ids([system_id]))
            fo.issueFleetMoveOrder(fleet_id, dest_id)
            print "Order issued: %s fleet: %s target: %s" % (self.ORDER_NAME, self.fleet, self.target)

        if system_id == fleet.systemID:
            if foAI.foAIstate.get_fleet_role(fleet_id) == AIFleetMissionType.FLEET_MISSION_EXPLORATION:
                if system_id in foAI.foAIstate.needsEmergencyExploration:
                    del foAI.foAIstate.needsEmergencyExploration[foAI.foAIstate.needsEmergencyExploration.index(system_id)]
            self.order_issued = True

