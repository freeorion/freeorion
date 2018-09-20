from logging import debug, error, warn

from EnumsAI import ShipRoleType, MissionType
import EspionageAI
import FleetUtilsAI
import freeOrionAIInterface as fo  # pylint: disable=import-error
from aistate_interface import get_aistate
import MilitaryAI
import MoveUtilsAI
import CombatRatingsAI
from target import TargetFleet, TargetSystem, TargetPlanet


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
                    debug("trooper_move_reqs_met() holding Invasion fleet %d before leaving supply "
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
                    debug("trooper_move_reqs_met() holding Invasion fleet %d before leaving supply "
                          "because target (%s) has nonzero max shields and no assigned military fleet would arrive"
                          "at least %d turn earlier than the invasion fleet") % (
                                order.fleet.id, invasion_planet, delay_to_move_troops)
                return False

        if verbose:
            debug("trooper_move_reqs_met() allowing Invasion fleet %d to leave supply "
                  "because target (%s) has zero max shields or there is a military fleet assigned to secure "
                  "the target system which will arrive at least 1 turn before the invasion fleet.",
                  order.fleet.id, invasion_planet)
    return True


class AIFleetOrder(object):
    """Stores information about orders which can be executed."""
    TARGET_TYPE = None
    ORDER_NAME = ''
    fleet = None  # type: target.TargetFleet
    target = None  # type: target.Target

    def __init__(self, fleet, target):
        """
        :param fleet: fleet to execute order
        :type fleet: target.TargetFleet
        :param target: fleet target, depends of order type
        :type target: target.Target
        """
        if not isinstance(fleet, TargetFleet):
            error("Order required fleet got %s" % type(fleet))

        if not isinstance(target, self.TARGET_TYPE):
            error("Target is not allowed, got %s expect %s" % (type(target), self.TARGET_TYPE))

        self.fleet = fleet
        self.target = target
        self.executed = False
        self.order_issued = False

    def __setstate__(self, state):
        # construct the universe objects from stored ids
        state["fleet"] = TargetFleet(state["fleet"])
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

    def is_valid(self):
        """Check if FleetOrder could be somehow in future issued = is valid."""
        if self.executed and self.order_issued:
            debug("\t\t order not valid because already executed and completed")
            return False
        if self.fleet and self.target:
            return True
        else:
            debug("\t\t order not valid: fleet validity: %s and target validity %s" % (
                bool(self.fleet), bool(self.target)))
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
            main_fleet_mission = get_aistate().get_fleet_mission(self.fleet.id)
            debug("  Can issue %s - Mission Type %s (%s), current loc sys %d - %s" % (
                self, main_fleet_mission.type,
                main_fleet_mission.type, self.fleet.id, sys1))
        return True

    def issue_order(self):
        if not self.can_issue_order():  # appears to be redundant with check in IAFleetMission?
            debug("  can't issue %s" % self)
            return False
        # by default we now set the order as issue and executed.  For any subclass where order issuence and execution
        # is not necessarily sure, these values can be reset after any appropriate checks in the respective
        # subclass issue_order()
        self.order_issued = True
        self.executed = True
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
    TARGET_TYPE = TargetSystem

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
        #     if not universe.getVisibility(target_id, get_aistate().empireID) >= fo.visibility.partial:
        #         #if not target_id in interior systems
        #         get_aistate().needsEmergencyExploration.append(fleet.systemID)
        #         return False
        system_id = self.fleet.get_system().id
        if system_id == self.target.get_system().id:
            return True  # TODO: already there, but could consider retreating

        aistate = get_aistate()
        main_fleet_mission = aistate.get_fleet_mission(self.fleet.id)

        fleet_rating = CombatRatingsAI.get_fleet_rating(self.fleet.id)
        fleet_rating_vs_planets = CombatRatingsAI.get_fleet_rating_against_planets(self.fleet.id)
        target_sys_status = aistate.systemStatus.get(self.target.id, {})
        f_threat = target_sys_status.get('fleetThreat', 0)
        m_threat = target_sys_status.get('monsterThreat', 0)
        p_threat = target_sys_status.get('planetThreat', 0)
        threat = f_threat + m_threat + p_threat
        safety_factor = aistate.character.military_safety_factor()
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
            my_other_fleet_rating = aistate.systemStatus.get(self.target.id, {}).get('myFleetRating', 0)
            my_other_fleet_rating_vs_planets = aistate.systemStatus.get(self.target.id, {}).get(
                'myFleetRatingVsPlanets', 0)
            is_military = aistate.get_fleet_role(self.fleet.id) == MissionType.MILITARY

            total_rating = CombatRatingsAI.combine_ratings(my_other_fleet_rating, fleet_rating)
            total_rating_vs_planets = CombatRatingsAI.combine_ratings(my_other_fleet_rating_vs_planets,
                                                                      fleet_rating_vs_planets)
            if (my_other_fleet_rating > 3 * safety_factor * threat or
                    (is_military and total_rating_vs_planets > 2.5*p_threat and total_rating > safety_factor * threat)):
                debug(("\tAdvancing fleet %d (rating %d) at system %d (%s) into system %d (%s) with threat %d"
                       " because of sufficient empire fleet strength already at destination") %
                      (self.fleet.id, fleet_rating, system_id, sys1_name, self.target.id, target_system_name, threat))
                return True
            elif (threat == p_threat and
                  not self.fleet.get_object().aggressive and
                  not my_other_fleet_rating and
                  not target_sys_status.get('localEnemyFleetIDs', [-1])):
                if verbose:
                    debug("\tAdvancing fleet %d (rating %d) at system %d (%s) "
                          "into system %d (%s) with planet threat %d because non aggressive"
                          " and no other fleets present to trigger combat") % (
                        self.fleet.id, fleet_rating, system_id, sys1_name, self.target.id, target_system_name, threat)
                return True
            else:
                if verbose:
                    debug("\tHolding fleet %d (rating %d) at system %d (%s) "
                          "before travelling to system %d (%s) with threat %d") % (
                        self.fleet.id, fleet_rating, system_id, sys1_name, self.target.id, target_system_name, threat)
                needs_vis = aistate.misc.setdefault('needs_vis', [])
                if self.target.id not in needs_vis:
                    needs_vis.append(self.target.id)
                return False

    def issue_order(self):
        if not super(OrderMove, self).issue_order():
            return False
        fleet_id = self.fleet.id
        system_id = self.target.get_system().id
        fleet = self.fleet.get_object()
        if system_id not in [fleet.systemID, fleet.nextSystemID]:
            dest_id = system_id
            fo.issueFleetMoveOrder(fleet_id, dest_id)
            debug("Order issued: %s fleet: %s target: %s" % (self.ORDER_NAME, self.fleet, self.target))
        aistate = get_aistate()
        if system_id == fleet.systemID:
            if aistate.get_fleet_role(fleet_id) == MissionType.EXPLORATION:
                if system_id in aistate.needsEmergencyExploration:
                    aistate.needsEmergencyExploration.remove(system_id)
        return True


class OrderPause(AIFleetOrder):
    """Ensure Fleet at least temporarily halts movement at the target system."""
    ORDER_NAME = 'pause'
    TARGET_TYPE = TargetSystem

    def is_valid(self):
        if not super(OrderPause, self).is_valid():
            return False
        return bool(self.target.get_system().get_object())

    def issue_order(self):
        if not super(OrderPause, self).issue_order():
            return False
        # not executed until actually arrives at target system
        self.excuted = self.fleet.get_current_system_id() == self.target.get_system().id


class OrderResupply(AIFleetOrder):
    ORDER_NAME = 'resupply'
    TARGET_TYPE = TargetSystem

    def is_valid(self):
        if not super(OrderResupply, self).is_valid():
            return False
        return self.target.id in fo.getEmpire().fleetSupplyableSystemIDs

    def issue_order(self):
        if not super(OrderResupply, self).issue_order():
            return False
        fleet_id = self.fleet.id
        system_id = self.target.get_system().id
        fleet = self.fleet.get_object()
        aistate = get_aistate()
        if system_id == fleet.systemID:
            if aistate.get_fleet_role(fleet_id) == MissionType.EXPLORATION:
                if system_id in aistate.needsEmergencyExploration:
                    aistate.needsEmergencyExploration.remove(system_id)
            return True
        if system_id != fleet.nextSystemID:
            self.executed = False
            start_id = FleetUtilsAI.get_fleet_system(fleet)
            dest_id = MoveUtilsAI.get_safe_path_leg_to_dest(fleet_id, start_id, system_id)
            universe = fo.getUniverse()
            debug("fleet %d with order type(%s) sent to safe leg dest %s and ultimate dest %s" % (
                fleet_id, self.ORDER_NAME, universe.getSystem(dest_id), universe.getSystem(system_id)))
            fo.issueFleetMoveOrder(fleet_id, dest_id)
            debug("Order issued: %s fleet: %s target: %s" % (self.ORDER_NAME, self.fleet, self.target))
        return True


class OrderOutpost(AIFleetOrder):
    ORDER_NAME = 'outpost'
    TARGET_TYPE = TargetPlanet

    def is_valid(self):
        if not super(OrderOutpost, self).is_valid():
            return False
        planet = self.target.get_object()
        if not planet.unowned:
            # terminate early
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
            return False
        # we can't know yet if the order will actually execute; instead, rely on the fact that if the order does get
        # executed, then next turn it will be invalid
        self.executed = False
        planet = self.target.get_object()
        if not planet.unowned:
            return False
        fleet_id = self.fleet.id
        ship_id = FleetUtilsAI.get_ship_id_with_role(fleet_id, ShipRoleType.CIVILIAN_OUTPOST)
        if ship_id is None:
            ship_id = FleetUtilsAI.get_ship_id_with_role(fleet_id, ShipRoleType.BASE_OUTPOST)
        if fo.issueColonizeOrder(ship_id, self.target.id):
            debug("Order issued: %s fleet: %s target: %s" % (self.ORDER_NAME, self.fleet, self.target))
            return True
        else:
            self.order_issued = False
            warn("Order issuance failed: %s fleet: %s target: %s" % (self.ORDER_NAME, self.fleet, self.target))
            return False


class OrderColonize(AIFleetOrder):
    ORDER_NAME = 'colonize'
    TARGET_TYPE = TargetPlanet

    def issue_order(self):
        if not super(OrderColonize, self).issue_order():
            return False
        # we can't know yet if the order will actually execute; instead, rely on the fact that if the order does get
        # executed, then next turn it will be invalid
        self.executed = False

        fleet_id = self.fleet.id
        ship_id = FleetUtilsAI.get_ship_id_with_role(fleet_id, ShipRoleType.CIVILIAN_COLONISATION)
        if ship_id is None:
            ship_id = FleetUtilsAI.get_ship_id_with_role(fleet_id, ShipRoleType.BASE_COLONISATION)

        if fo.issueColonizeOrder(ship_id, self.target.id):
            debug("Order issued: %s fleet: %s target: %s" % (self.ORDER_NAME, self.fleet, self.target))
            return True
        else:
            self.order_issued = False
            warn("Order issuance failed: %s fleet: %s target: %s" % (self.ORDER_NAME, self.fleet, self.target))
            return False

    def is_valid(self):
        if not super(OrderColonize, self).is_valid():
            return False
        planet = self.target.get_object()
        if (planet.unowned or planet.ownedBy(fo.empireID())) and not planet.currentMeterValue(fo.meterType.population):
            return self.fleet.get_object().hasColonyShips
        # Otherwise, terminate early
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
    TARGET_TYPE = TargetSystem


class OrderInvade(AIFleetOrder):
    ORDER_NAME = 'invade'
    TARGET_TYPE = TargetPlanet

    def is_valid(self):
        if not super(OrderInvade, self).is_valid():
            return False
        planet = self.target.get_object()
        planet_population = planet.currentMeterValue(fo.meterType.population)
        if planet.unowned and not planet_population:
            debug("\t\t invasion order not valid due to target planet status-- owned: %s and population %.1f" % (
                not planet.unowned, planet_population))
            # terminate early
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
            not planet.initialMeterValue(fo.meterType.shield)
        ))

    def issue_order(self):
        if not super(OrderInvade, self).can_issue_order():
            return False

        universe = fo.getUniverse()
        planet_id = self.target.id
        planet = self.target.get_object()
        fleet = self.fleet.get_object()

        invasion_roles = (ShipRoleType.MILITARY_INVASION,
                          ShipRoleType.BASE_INVASION)

        debug("Issuing order: %s fleet: %s target: %s" % (self.ORDER_NAME, self.fleet, self.target))
        # will track if at least one invasion troops successfully deployed
        result = False
        aistate = get_aistate()
        for ship_id in fleet.shipIDs:
            ship = universe.getShip(ship_id)
            role = aistate.get_ship_role(ship.design.id)
            if role not in invasion_roles:
                continue

            debug("Ordering troop ship %d to invade %s" % (ship_id, planet))
            result = fo.issueInvadeOrder(ship_id, planet_id) or result
            if not result:
                shields = planet.currentMeterValue(fo.meterType.shield)
                planet_stealth = planet.currentMeterValue(fo.meterType.stealth)
                pop = planet.currentMeterValue(fo.meterType.population)
                warn("Invasion order failed!")
                debug(" -- planet has %.1f stealth, shields %.1f, %.1f population and "
                      "is owned by empire %d" % (planet_stealth, shields, pop, planet.owner))
                if 'needsEmergencyExploration' not in dir(aistate):
                    aistate.needsEmergencyExploration = []
                if fleet.systemID not in aistate.needsEmergencyExploration:
                    aistate.needsEmergencyExploration.append(fleet.systemID)
                    debug("Due to trouble invading, adding system %d to Emergency Exploration List" % fleet.systemID)
                self.executed = False
                # debug(universe.getPlanet(planet_id).dump())  # TODO: fix fo.UniverseObject.dump()
                break
        if result:
            debug("Successfully ordered troop ship(s) to invade %s" % planet)
            return True
        else:
            return False


class OrderMilitary(AIFleetOrder):
    ORDER_NAME = 'military'
    TARGET_TYPE = TargetSystem

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
            return False
        target_sys_id = self.target.id
        fleet = self.target.get_object()
        system_status = get_aistate().systemStatus.get(target_sys_id, {})
        total_threat = sum(system_status.get(threat, 0) for threat in ('fleetThreat', 'planetThreat', 'monsterThreat'))
        combat_trigger = system_status.get('fleetThreat', 0) or system_status.get('monsterThreat', 0)
        if not combat_trigger and system_status.get('planetThreat', 0):
            universe = fo.getUniverse()
            system = universe.getSystem(target_sys_id)
            for planet_id in system.planetIDs:
                planet = universe.getPlanet(planet_id)
                if planet.ownedBy(fo.empireID()):  # TODO: also exclude at-peace planets
                    continue
                if planet.unowned and not EspionageAI.colony_detectable_by_empire(planet_id, empire=fo.empireID()):
                    continue
                if sum([planet.currentMeterValue(meter_type) for meter_type in
                        [fo.meterType.defense, fo.meterType.shield, fo.meterType.construction]]):
                    combat_trigger = True
                    break
        if not all((
                    fleet,
                    fleet.systemID == target_sys_id,
                    system_status.get('currently_visible', False),
                    not (total_threat and combat_trigger)
        )):
            self.executed = False
        return True


class OrderRepair(AIFleetOrder):
    ORDER_NAME = 'repair'
    TARGET_TYPE = TargetSystem

    def is_valid(self):
        if not super(OrderRepair, self).is_valid():
            return False
        return self.target.id in fo.getEmpire().fleetSupplyableSystemIDs  # TODO: check for drydock still there/owned

    def issue_order(self):
        if not super(OrderRepair, self).issue_order():
            return False
        fleet_id = self.fleet.id
        system_id = self.target.get_system().id
        fleet = self.fleet.get_object()  # type: fo.fleet
        if system_id == fleet.systemID:
            aistate = get_aistate()
            if aistate.get_fleet_role(fleet_id) == MissionType.EXPLORATION:
                if system_id in aistate.needsEmergencyExploration:
                    aistate.needsEmergencyExploration.remove(system_id)
        elif system_id != fleet.nextSystemID:
            fo.issueAggressionOrder(fleet_id, False)
            start_id = FleetUtilsAI.get_fleet_system(fleet)
            dest_id = MoveUtilsAI.get_safe_path_leg_to_dest(fleet_id, start_id, system_id)
            universe = fo.getUniverse()
            debug("fleet %d with order type(%s) sent to safe leg dest %s and ultimate dest %s" % (
                fleet_id, self.ORDER_NAME, universe.getSystem(dest_id), universe.getSystem(system_id)))
            fo.issueFleetMoveOrder(fleet_id, dest_id)
            debug("Order issued: %s fleet: %s target: %s" % (self.ORDER_NAME, self.fleet, self.target))
        ships_cur_health, ships_max_health = FleetUtilsAI.get_current_and_max_structure(fleet_id)
        self.executed = (ships_cur_health == ships_max_health)
        return True
