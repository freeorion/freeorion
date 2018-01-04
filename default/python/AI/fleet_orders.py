from EnumsAI import ShipRoleType, MissionType
import FleetUtilsAI
import freeOrionAIInterface as fo  # pylint: disable=import-error
import FreeOrionAI as foAI
import MilitaryAI
import MoveUtilsAI
import CombatRatingsAI
from universe_object import Fleet, System, Planet
from freeorion_tools import get_partial_visibility_turn, dump_universe

from common.configure_logging import convenience_function_references_for_logger
(debug, info, warn, error, fatal) = convenience_function_references_for_logger(__name__)


def trooper_move_reqs_met(main_fleet_mission, order, verbose):
    """
    Indicates whether or not move requirements specific to invasion troopers are met for the provided mission and order.
    :type main_fleet_mission: AIFleetMission.AIFleetMission
    :type order: OrderMove
    :param verbose: whether to print verbose decision details
    :type verbose: bool
    :rtype: bool
    """
    # Don't advance outside of our fleet-supply zone unless the target either has no shields at all or there
    # is already a military fleet assigned to secure the target, and don't take final jump unless the planet is
    # (to the AI's knowledge) down to zero shields.  Additional checks will also be done by the later
    # generic movement code
    invasion_target = main_fleet_mission.target
    invasion_planet = invasion_target.get_object()
    invasion_system = invasion_target.get_system()
    supplied_systems = fo.getEmpire().fleetSupplyableSystemIDs
    # if about to leave supply lines
    if order.target.id not in supplied_systems or fo.getUniverse().jumpDistance(order.fleet.id, invasion_system.id) < 5:
        if invasion_planet.currentMeterValue(fo.meterType.maxShield):
            military_support_fleets = MilitaryAI.get_military_fleets_with_target_system(invasion_system.id)
            if not military_support_fleets:
                if verbose:
                    print ("trooper_move_reqs_met() holding Invasion fleet %d before leaving supply "
                           "because target (%s) has nonzero max shields and there is not yet a military fleet "
                           "assigned to secure the target system.") % (order.fleet.id, invasion_planet)
                return False

            # if there is a threat in the enemy system, do give military ships at least 1 turn to clear it
            delay_to_move_troops = 1 if MilitaryAI.get_system_local_threat(order.target.id) else 0

            def eta(fleet_id):
                return FleetUtilsAI.calculate_estimated_time_of_arrival(fleet_id, invasion_system.id)

            eta_this_fleet = eta(order.fleet.id)
            if all(((eta_this_fleet - delay_to_move_troops) <= eta(fid) and eta(fid))
                   for fid in military_support_fleets):
                if verbose:
                    print ("trooper_move_reqs_met() holding Invasion fleet %d before leaving supply "
                           "because target (%s) has nonzero max shields and no assigned military fleet would arrive"
                           "at least %d turn earlier than the invasion fleet") % (
                                order.fleet.id, invasion_planet, delay_to_move_troops)
                return False

        if verbose:
            print ("trooper_move_reqs_met() allowing Invasion fleet %d to leave supply "
                   "because target (%s) has zero max shields or there is a military fleet assigned to secure "
                   "the target system which will arrive at least 1 turn before the invasion fleet.") % (order.fleet.id,
                                                                                                        invasion_planet)
    return True


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
            error("Order required fleet got %s" % type(fleet))

        if not isinstance(target, self.TARGET_TYPE):
            error("Target is not allowed, got %s expect %s" % (type(target), self.TARGET_TYPE))

        self.fleet = fleet
        self.target = target
        self.executed = False
        self.order_issued = False

    def __setstate__(self, state):
        # construct the universe objects from stored ids
        state["fleet"] = Fleet(state["fleet"])
        target_type = state.pop("target_type")
        if state["target"] is not None:
            assert self.TARGET_TYPE.object_name == target_type
            state["target"] = self.TARGET_TYPE(state["target"])
        self.__dict__ = state

    def __getstate__(self):
        retval = dict(self.__dict__)
        # do not store the universe object but only the fleet id
        retval['fleet'] = self.fleet.id
        if self.target is not None:
            retval["target"] = self.target.id
            retval["target_type"] = self.target.object_name
        else:
            retval["target_type"] = None
        return retval

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
            print "\t\t order not valid: fleet validity: %s and target validity %s" % (
                bool(self.fleet), bool(self.target))
            return False

    def can_issue_order(self, verbose=False):
        """If FleetOrder can be issued now."""
        # for some orders, may need to re-issue if invasion/outposting/colonization was interrupted
        if self.executed and not isinstance(self, (OrderOutpost, OrderColonize, OrderInvade)):
            return False
        if not self.is_valid():
            return False

        if verbose:
            sys1 = self.fleet.get_system()
            main_fleet_mission = foAI.foAIstate.get_fleet_mission(self.fleet.id)
            print "  Can issue %s - Mission Type %s (%s), current loc sys %d - %s" % (
                self, main_fleet_mission.type,
                main_fleet_mission.type, self.fleet.id, sys1)
        return True

    def issue_order(self):
        if not self.can_issue_order():  # appears to be redundant with check in IAFleetMission?
            print "  can't issue %s" % self
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
        return "[%s] of %s to %s %s" % (self.ORDER_NAME, self.fleet.get_object(),
                                        self.target.get_object(), execute_status)

    def __eq__(self, other):
        return type(self) == type(other) and self.fleet == other.fleet and self.target == other.target


class OrderMove(AIFleetOrder):
    ORDER_NAME = 'move'
    TARGET_TYPE = System

    def can_issue_order(self, verbose=False):
        if not super(OrderMove, self).can_issue_order(verbose=verbose):
            return False
        # TODO: figure out better way to have invasions (& possibly colonizations)
        #       require visibility on target without needing visibility of all intermediate systems
        # if False and main_mission_type not in [MissionType.ATTACK,  # TODO: consider this later
        #                                        MissionType.MILITARY,
        #                                        MissionType.SECURE,
        #                                        MissionType.HIT_AND_RUN,
        #                                        MissionType.EXPLORATION]:
        #     if not universe.getVisibility(target_id, foAI.foAIstate.empireID) >= fo.visibility.partial:
        #         #if not target_id in interior systems
        #         foAI.foAIstate.needsEmergencyExploration.append(fleet.systemID)
        #         return False
        system_id = self.fleet.get_system().id
        if system_id == self.target.get_system().id:
            return True  # TODO: already there, but could consider retreating

        main_fleet_mission = foAI.foAIstate.get_fleet_mission(self.fleet.id)

        fleet_rating = CombatRatingsAI.get_fleet_rating(self.fleet.id)
        fleet_rating_vs_planets = CombatRatingsAI.get_fleet_rating_against_planets(self.fleet.id)
        target_sys_status = foAI.foAIstate.systemStatus.get(self.target.id, {})
        f_threat = target_sys_status.get('fleetThreat', 0)
        m_threat = target_sys_status.get('monsterThreat', 0)
        p_threat = target_sys_status.get('planetThreat', 0)
        threat = f_threat + m_threat + p_threat
        safety_factor = foAI.foAIstate.character.military_safety_factor()
        universe = fo.getUniverse()
        if main_fleet_mission.type == MissionType.INVASION and not trooper_move_reqs_met(main_fleet_mission,
                                                                                         self, verbose):
            return False
        if fleet_rating >= safety_factor * threat and fleet_rating_vs_planets >= p_threat:
            return True
        elif not p_threat and self.target.id in fo.getEmpire().supplyUnobstructedSystems:
            return True
        else:
            sys1 = universe.getSystem(system_id)
            sys1_name = sys1 and sys1.name or "unknown"
            target_system = self.target.get_system()
            target_system_name = (target_system and target_system.get_object().name) or "unknown"
            # TODO: adjust calc for any departing fleets
            my_other_fleet_rating = foAI.foAIstate.systemStatus.get(self.target.id, {}).get('myFleetRating', 0)
            my_other_fleet_rating_vs_planets = foAI.foAIstate.systemStatus.get(self.target.id, {}).get(
                'myFleetRatingVsPlanets', 0)
            is_military = foAI.foAIstate.get_fleet_role(self.fleet.id) == MissionType.MILITARY

            # TODO(Morlic): Is there any reason to add this linearly instead of using CombineRatings?
            total_rating = my_other_fleet_rating + fleet_rating
            total_rating_vs_planets = my_other_fleet_rating_vs_planets + fleet_rating_vs_planets
            if (my_other_fleet_rating > 3 * safety_factor * threat or
                    (is_military and total_rating_vs_planets > 2.5*p_threat and total_rating > safety_factor * threat)):
                if verbose:
                    print ("\tAdvancing fleet %d (rating %d) at system %d (%s) "
                           "into system %d (%s) with threat %d because of "
                           "sufficient empire fleet strength already at destination") % (
                        self.fleet.id, fleet_rating, system_id, sys1_name, self.target.id, target_system_name, threat)
                return True
            elif (threat == p_threat and
                  not self.fleet.get_object().aggressive and
                  not my_other_fleet_rating and
                  not target_sys_status.get('localEnemyFleetIDs', [-1])):
                if verbose:
                    print ("\tAdvancing fleet %d (rating %d) at system %d (%s) "
                           "into system %d (%s) with planet threat %d because non aggressive"
                           " and no other fleets present to trigger combat") % (
                        self.fleet.id, fleet_rating, system_id, sys1_name, self.target.id, target_system_name, threat)
                return True
            else:
                if verbose:
                    print ("\tHolding fleet %d (rating %d) at system %d (%s) "
                           "before travelling to system %d (%s) with threat %d") % (
                        self.fleet.id, fleet_rating, system_id, sys1_name, self.target.id, target_system_name, threat)
                needs_vis = foAI.foAIstate.misc.setdefault('needs_vis', [])
                if self.target.id not in needs_vis:
                    needs_vis.append(self.target.id)
                return False

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
            if foAI.foAIstate.get_fleet_role(fleet_id) == MissionType.EXPLORATION:
                if system_id in foAI.foAIstate.needsEmergencyExploration:
                    foAI.foAIstate.needsEmergencyExploration.remove(system_id)
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
            start_id = FleetUtilsAI.get_fleet_system(fleet)
            dest_id = MoveUtilsAI.get_safe_path_leg_to_dest(fleet_id, start_id, system_id)
            universe = fo.getUniverse()
            print "fleet %d with order type(%s) sent to safe leg dest %s and ultimate dest %s" % (
                fleet_id, self.ORDER_NAME, universe.getSystem(dest_id), universe.getSystem(system_id))
            fo.issueFleetMoveOrder(fleet_id, dest_id)
            print "Order issued: %s fleet: %s target: %s" % (self.ORDER_NAME, self.fleet, self.target)

        if system_id == fleet.systemID:
            if foAI.foAIstate.get_fleet_role(fleet_id) == MissionType.EXPLORATION:
                if system_id in foAI.foAIstate.needsEmergencyExploration:
                    foAI.foAIstate.needsEmergencyExploration.remove(system_id)
            self.order_issued = True


class OrderOutpost(AIFleetOrder):
    ORDER_NAME = 'outpost'
    TARGET_TYPE = Planet

    def is_valid(self):
        if not super(OrderOutpost, self).is_valid():
            return False
        planet = self.target.get_object()
        sys_partial_vis_turn = get_partial_visibility_turn(planet.systemID)
        planet_partial_vis_turn = get_partial_visibility_turn(planet.id)
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
        ship_id = FleetUtilsAI.get_ship_id_with_role(self.fleet.id, ShipRoleType.CIVILIAN_OUTPOST)
        if ship_id is None:
            ship_id = FleetUtilsAI.get_ship_id_with_role(self.fleet.id, ShipRoleType.BASE_OUTPOST)
        ship = universe.getShip(ship_id)
        return ship is not None and self.fleet.get_object().systemID == self.target.get_system().id and ship.canColonize

    def issue_order(self):
        if not super(OrderOutpost, self).issue_order():
            return
        planet = self.target.get_object()
        if not planet.unowned:
            self.order_issued = True
            return
        fleet_id = self.fleet.id
        ship_id = FleetUtilsAI.get_ship_id_with_role(fleet_id, ShipRoleType.CIVILIAN_OUTPOST)
        if ship_id is None:
            ship_id = FleetUtilsAI.get_ship_id_with_role(fleet_id, ShipRoleType.BASE_OUTPOST)
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
        ship_id = FleetUtilsAI.get_ship_id_with_role(fleet_id, ShipRoleType.CIVILIAN_COLONISATION)
        if ship_id is None:
            ship_id = FleetUtilsAI.get_ship_id_with_role(fleet_id, ShipRoleType.BASE_COLONISATION)

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
        planet = self.target.get_object()

        sys_partial_vis_turn = get_partial_visibility_turn(planet.systemID)
        planet_partial_vis_turn = get_partial_visibility_turn(planet.id)
        if (planet_partial_vis_turn == sys_partial_vis_turn and planet.unowned or
                (planet.ownedBy(fo.empireID()) and not planet.currentMeterValue(fo.meterType.population))):
            return self.fleet.get_object().hasColonyShips
        self.executed = True
        self.order_issued = True
        return False

    def can_issue_order(self, verbose=False):
        if not super(OrderColonize, self).is_valid():
            return False
        # TODO: check for separate fleet holding colony ships
        ship_id = FleetUtilsAI.get_ship_id_with_role(self.fleet.id, ShipRoleType.CIVILIAN_COLONISATION)
        if ship_id is None:
            ship_id = FleetUtilsAI.get_ship_id_with_role(self.fleet.id, ShipRoleType.BASE_COLONISATION)
        universe = fo.getUniverse()
        ship = universe.getShip(ship_id)
        if ship and not ship.canColonize:
            warn("colonization fleet %d has no colony ship" % self.fleet.id)
        return ship is not None and self.fleet.get_object().systemID == self.target.get_system().id and ship.canColonize


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
            print "\t\t invasion order not valid due to target planet status-- owned: %s and population %.1f" % (
                not planet.unowned, planet_population)
            self.executed = True
            self.order_issued = True
            return False
        else:
            return self.fleet.get_object().hasTroopShips

    def can_issue_order(self, verbose=False):
        if not super(OrderInvade, self).is_valid():
            return False
        # TODO: check for separate fleet holding invasion ships
        ship_id = FleetUtilsAI.get_ship_id_with_role(self.fleet.id, ShipRoleType.MILITARY_INVASION, False)
        if ship_id is None:
            ship_id = FleetUtilsAI.get_ship_id_with_role(self.fleet.id, ShipRoleType.BASE_INVASION)
        universe = fo.getUniverse()
        ship = universe.getShip(ship_id)
        planet = self.target.get_object()
        return all((
            ship is not None,
            self.fleet.get_object().systemID == planet.systemID,
            ship.canInvade,
            not planet.currentMeterValue(fo.meterType.shield)
        ))

    def issue_order(self):
        if not super(OrderInvade, self).can_issue_order():
            return

        universe = fo.getUniverse()
        planet_id = self.target.id
        planet = self.target.get_object()
        fleet = self.fleet.get_object()

        invasion_roles = (ShipRoleType.MILITARY_INVASION,
                          ShipRoleType.BASE_INVASION)

        print "Issuing order: %s fleet: %s target: %s" % (self.ORDER_NAME, self.fleet, self.target)
        # will track if at least one invasion troops successfully deployed
        result = False
        for ship_id in fleet.shipIDs:
            ship = universe.getShip(ship_id)
            role = foAI.foAIstate.get_ship_role(ship.design.id)
            if role not in invasion_roles:
                continue

            print "Ordering troop ship %d to invade %s" % (ship_id, planet)
            result = fo.issueInvadeOrder(ship_id, planet_id) or result
            if not result:
                shields = planet.currentMeterValue(fo.meterType.shield)
                planet_stealth = planet.currentMeterValue(fo.meterType.stealth)
                pop = planet.currentMeterValue(fo.meterType.population)
                warn("Invasion order failed!")
                print (" -- planet has %.1f stealth, shields %.1f, %.1f population and "
                       "is owned by empire %d") % (planet_stealth, shields, pop, planet.owner)
                if 'needsEmergencyExploration' not in dir(foAI.foAIstate):
                    foAI.foAIstate.needsEmergencyExploration = []
                if fleet.systemID not in foAI.foAIstate.needsEmergencyExploration:
                    foAI.foAIstate.needsEmergencyExploration.append(fleet.systemID)
                    print "Due to trouble invading, adding system %d to Emergency Exploration List" % fleet.systemID
                    self.executed = False
                
                if shields > 0 and planet.unowned:
                    dump_universe()
                break
        if result:
            print "Successfully ordered troop ship(s) to invade %s" % planet


class OrderMilitary(AIFleetOrder):
    ORDER_NAME = 'military'
    TARGET_TYPE = System

    def is_valid(self):
        if not super(OrderMilitary, self).is_valid():
            return False
        fleet = self.fleet.get_object()
        # TODO: consider bombardment-only fleets/orders
        return fleet is not None and (fleet.hasArmedShips or fleet.hasFighterShips)

    def can_issue_order(self, verbose=False):
        # TODO: consider bombardment
        # TODO: consider simmply looking at fleet characteristics, as is done for is_valid()
        ship_id = FleetUtilsAI.get_ship_id_with_role(self.fleet.id, ShipRoleType.MILITARY)
        universe = fo.getUniverse()
        ship = universe.getShip(ship_id)
        return (ship is not None and
                self.fleet.get_object().systemID == self.target.id and
                (ship.isArmed or ship.hasFighters))

    def issue_order(self):
        if not super(OrderMilitary, self).issue_order():
            return
        target_sys_id = self.target.id
        fleet = self.target.get_object()
        system_status = foAI.foAIstate.systemStatus.get(target_sys_id, {})
        total_threat = sum(system_status.get(threat, 0) for threat in ('fleetThreat', 'planetThreat', 'monsterThreat'))
        if all((
                fleet,
                fleet.systemID == target_sys_id,
                system_status.get('currently_visible', False),
                not total_threat
        )):
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
            start_id = FleetUtilsAI.get_fleet_system(fleet)
            dest_id = MoveUtilsAI.get_safe_path_leg_to_dest(fleet_id, start_id, system_id)
            universe = fo.getUniverse()
            print "fleet %d with order type(%s) sent to safe leg dest %s and ultimate dest %s" % (
                fleet_id, self.ORDER_NAME, universe.getSystem(dest_id), universe.getSystem(system_id))
            fo.issueFleetMoveOrder(fleet_id, dest_id)
            print "Order issued: %s fleet: %s target: %s" % (self.ORDER_NAME, self.fleet, self.target)

        if system_id == fleet.systemID:
            if foAI.foAIstate.get_fleet_role(fleet_id) == MissionType.EXPLORATION:
                if system_id in foAI.foAIstate.needsEmergencyExploration:
                    foAI.foAIstate.needsEmergencyExploration.remove(system_id)
            self.order_issued = True
