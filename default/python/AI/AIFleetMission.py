import sys

import freeOrionAIInterface as fo  # pylint: disable=import-error

from fleet_orders import OrderMove, OrderOutpost, OrderColonize, OrderMilitary, OrderInvade, OrderDefend
import AIstate
import FleetUtilsAI
import FreeOrionAI as foAI
import MoveUtilsAI
import MilitaryAI
import InvasionAI
import CombatRatingsAI
from universe_object import UniverseObject, System, Fleet, Planet
from EnumsAI import MissionType
from AIDependencies import INVALID_ID

from common.configure_logging import convenience_function_references_for_logger
(debug, info, warn, error, fatal) = convenience_function_references_for_logger(__name__)

ORDERS_FOR_MISSION = {
    MissionType.EXPLORATION: OrderMove,
    MissionType.OUTPOST: OrderOutpost,
    MissionType.COLONISATION: OrderColonize,
    MissionType.INVASION: OrderInvade,
    MissionType.MILITARY: OrderMilitary,
    # SECURE is mostly same as MILITARY, but waits for system removal from all targeted system lists
    # (invasion, colonization, outpost, blockade) before clearing
    MissionType.SECURE: OrderMilitary,
    MissionType.ORBITAL_INVASION: OrderInvade,
    MissionType.ORBITAL_OUTPOST: OrderOutpost,
    MissionType.ORBITAL_DEFENSE: OrderDefend
}

COMBAT_MISSION_TYPES = (
    MissionType.MILITARY,
    MissionType.SECURE
)

MERGEABLE_MISSION_TYPES = (
    MissionType.MILITARY,
    MissionType.INVASION,
    MissionType.ORBITAL_INVASION,
    MissionType.SECURE,
    MissionType.ORBITAL_DEFENSE,
)

COMPATIBLE_ROLES_MAP = {
    MissionType.ORBITAL_DEFENSE: [MissionType.ORBITAL_DEFENSE],
    MissionType.MILITARY: [MissionType.MILITARY],
    MissionType.ORBITAL_INVASION: [MissionType.ORBITAL_INVASION],
    MissionType.INVASION: [MissionType.INVASION],
}


class AIFleetMission(object):
    """
    Stores information about AI mission. Every mission has fleetID and AI targets depending upon AI fleet mission type.
    :type target: UniverseObject | None
    """

    def __init__(self, fleet_id):
        self.orders = []
        self.fleet = Fleet(fleet_id)
        self.type = None
        self.target = None

    def set_target(self, mission_type, target):
        """
        Set mission and target for this fleet.

        :type mission_type: MissionType
        :type target: UniverseObject
        """
        if self.type == mission_type and self.target == target:
            return
        if self.type or self.target:
            print "Change mission assignment from %s:%s to %s:%s" % (self.type, self.target, mission_type, target)
        self.type = mission_type
        self.target = target

    def clear_target(self):
        """Clear target and mission for this fleet."""
        self.target = None
        self.type = None

    def has_target(self, mission_type, target):
        """
        Check if fleet has specified mission_type and target.

        :type mission_type: MissionType
        :type target: UniverseObject
        :rtype: bool
        """
        return self.type == mission_type and self.target == target

    def clear_fleet_orders(self):
        """Clear this fleets orders but do not clear mission and target."""
        self.orders = []

    def _get_fleet_order_from_target(self, mission_type, target):
        """
        Get a fleet order according to mission type and target.

        :type mission_type: MissionType
        :type target: UniverseObject
        :rtype: AIFleetOrder
        """
        fleet_target = Fleet(self.fleet.id)
        return ORDERS_FOR_MISSION[mission_type](fleet_target, target)

    def check_mergers(self, context=""):
        """
        If possible and reasonable, merge this fleet with others.

        :param context: Context of the function call for logging purposes
        :type context: str
        """
        if self.type not in MERGEABLE_MISSION_TYPES:
            return
        universe = fo.getUniverse()
        empire_id = fo.empireID()
        fleet_id = self.fleet.id
        main_fleet = universe.getFleet(fleet_id)
        system_id = main_fleet.systemID
        if system_id == INVALID_ID:
            return  # can't merge fleets in middle of starlane
        system_status = foAI.foAIstate.systemStatus[system_id]
        destroyed_list = list(universe.destroyedObjectIDs(empire_id))
        other_fleets_here = [fid for fid in system_status.get('myFleetsAccessible', []) if fid != fleet_id and
                             fid not in destroyed_list and universe.getFleet(fid).ownedBy(empire_id)]
        if not other_fleets_here:
            return

        target_id = self.target.id if self.target else None
        main_fleet_role = foAI.foAIstate.get_fleet_role(fleet_id)
        for fid in other_fleets_here:
            fleet_role = foAI.foAIstate.get_fleet_role(fid)
            if fleet_role not in COMPATIBLE_ROLES_MAP[main_fleet_role]:
                continue
            fleet = universe.getFleet(fid)

            if not fleet or fleet.systemID != system_id or len(fleet.shipIDs) == 0:
                continue
            if not (fleet.speed > 0 or main_fleet.speed == 0):  # TODO(Cjkjvfnby) Check this condition
                continue
            fleet_mission = foAI.foAIstate.get_fleet_mission(fid)
            do_merge = False
            need_left = 0
            if (main_fleet_role == MissionType.ORBITAL_DEFENSE) or (fleet_role == MissionType.ORBITAL_DEFENSE):
                if main_fleet_role == fleet_role:
                    do_merge = True
            elif (main_fleet_role == MissionType.ORBITAL_INVASION) or (fleet_role == MissionType.ORBITAL_INVASION):
                if main_fleet_role == fleet_role:
                    do_merge = False  # TODO: could allow merger if both orb invaders and both same target
            elif not fleet_mission and (main_fleet.speed > 0) and (fleet.speed > 0):
                do_merge = True
            else:
                if not self.target and (main_fleet.speed > 0 or fleet.speed == 0):
                    do_merge = True
                else:
                    target = fleet_mission.target.id if fleet_mission.target else None
                    if target == target_id:
                        print "Military fleet %d has same target as %s fleet %d. Merging." % (fid, fleet_role, fleet_id)
                        do_merge = True  # TODO: should probably ensure that fleetA has aggression on now
                    elif main_fleet.speed > 0:
                        neighbors = foAI.foAIstate.systemStatus.get(system_id, {}).get('neighbors', [])
                        if target == system_id and target_id in neighbors and self.type == MissionType.SECURE:
                            # consider 'borrowing' for work in neighbor system  # TODO check condition
                            need_left = 1.5 * sum(foAI.foAIstate.systemStatus.get(nid, {}).get('fleetThreat', 0)
                                                  for nid in neighbors if nid != target_id)
                            fleet_rating = CombatRatingsAI.get_fleet_rating(fid)
                            if need_left < fleet_rating:
                                do_merge = True
            if do_merge:
                FleetUtilsAI.merge_fleet_a_into_b(fid, fleet_id, need_left,
                                                  context="Order %s of mission %s" % (context, self))
        return

    def _is_valid_fleet_mission_target(self, mission_type, target):
        if not target:
            return False
        if mission_type == MissionType.EXPLORATION:
            if isinstance(target, System):
                empire = fo.getEmpire()
                if not empire.hasExploredSystem(target.id):
                    return True
        elif mission_type in [MissionType.OUTPOST, MissionType.ORBITAL_OUTPOST]:
            fleet = self.fleet.get_object()
            if not fleet.hasOutpostShips:
                return False
            if isinstance(target, Planet):
                planet = target.get_object()
                if planet.unowned:
                    return True
        elif mission_type == MissionType.COLONISATION:
            fleet = self.fleet.get_object()
            if not fleet.hasColonyShips:
                return False
            if isinstance(target, Planet):
                planet = target.get_object()
                population = planet.currentMeterValue(fo.meterType.population)
                if planet.unowned or (planet.owner == fleet.owner and population == 0):
                    return True
        elif mission_type in [MissionType.INVASION, MissionType.ORBITAL_INVASION]:
            fleet = self.fleet.get_object()
            if not fleet.hasTroopShips:
                return False
            if isinstance(target, Planet):
                planet = target.get_object()
                # TODO remove latter portion of next check in light of invasion retargeting, or else correct logic
                if not planet.unowned or planet.owner != fleet.owner:
                    return True
        elif mission_type in [MissionType.MILITARY, MissionType.SECURE, MissionType.ORBITAL_DEFENSE]:
            if isinstance(target, System):
                return True
        # TODO: implement other mission types
        return False

    def clean_invalid_targets(self):
        """clean invalid AITargets"""
        if not self._is_valid_fleet_mission_target(self.type, self.target):
            self.target = None
            self.type = None

    def _check_abort_mission(self, fleet_order):
        """ checks if current mission (targeting a planet) should be aborted"""
        if fleet_order.target and isinstance(fleet_order.target, Planet):
            planet = fleet_order.target.get_object()
            if isinstance(fleet_order, OrderColonize):
                if (planet.currentMeterValue(fo.meterType.population) == 0 and
                        (planet.ownedBy(fo.empireID()) or planet.unowned)):
                    return False
            elif isinstance(fleet_order, OrderOutpost):
                if planet.unowned:
                    return False
            elif isinstance(fleet_order, OrderInvade):  # TODO add substantive abort check
                return False
            else:
                return False

        # canceling fleet orders
        print "   %s" % fleet_order
        print "Fleet %d had a target planet that is no longer valid for this mission; aborting." % self.fleet.id
        self.clear_fleet_orders()
        self.clear_target()
        FleetUtilsAI.split_fleet(self.fleet.id)
        return True

    def _check_retarget_invasion(self):
        """checks if an invasion mission should be retargeted"""
        universe = fo.getUniverse()
        empire_id = fo.empireID()
        fleet_id = self.fleet.id
        fleet = universe.getFleet(fleet_id)
        if fleet.systemID == INVALID_ID:
            # next_loc = fleet.nextSystemID
            return  # TODO: still check
        system = universe.getSystem(fleet.systemID)
        if not system:
            return
        orders = self.orders
        last_sys_target = INVALID_ID
        if orders:
            last_sys_target = orders[-1].target.id
        if last_sys_target == fleet.systemID:
            return  # TODO: check for best local target
        open_targets = []
        already_targeted = InvasionAI.get_invasion_targeted_planet_ids(system.planetIDs, MissionType.INVASION)
        for pid in system.planetIDs:
            if pid in already_targeted or (pid in foAI.foAIstate.qualifyingTroopBaseTargets):
                continue
            planet = universe.getPlanet(pid)
            if planet.unowned or (planet.owner == empire_id):
                continue
            if (planet.currentMeterValue(fo.meterType.shield)) <= 0:
                open_targets.append(pid)
        if not open_targets:
            return
        troops_in_fleet = FleetUtilsAI.count_troops_in_fleet(fleet_id)
        target_id = INVALID_ID
        best_score = -1
        target_troops = 0
        #
        for pid, rating in InvasionAI.assign_invasion_values(open_targets).items():
            p_score, p_troops = rating
            if p_score > best_score:
                if p_troops >= troops_in_fleet:
                    continue
                best_score = p_score
                target_id = pid
                target_troops = p_troops
        if target_id == INVALID_ID:
            return

        print "\t AIFleetMission._check_retarget_invasion: splitting and retargetting fleet %d" % fleet_id
        new_fleets = FleetUtilsAI.split_fleet(fleet_id)
        self.clear_target()  # TODO: clear from foAIstate
        self.clear_fleet_orders()
        troops_needed = max(0, target_troops - FleetUtilsAI.count_troops_in_fleet(fleet_id))
        min_stats = {'rating': 0, 'troopCapacity': troops_needed}
        target_stats = {'rating': 10, 'troopCapacity': troops_needed}
        found_fleets = []
        # TODO check if next statement does not mutate any global states and can be removed
        _ = FleetUtilsAI.get_fleets_for_mission(target_stats, min_stats, {}, starting_system=fleet.systemID,
                                                fleet_pool_set=set(new_fleets), fleet_list=found_fleets)
        for fid in found_fleets:
            FleetUtilsAI.merge_fleet_a_into_b(fid, fleet_id)
        target = Planet(target_id)
        self.set_target(MissionType.INVASION, target)
        self.generate_fleet_orders()

    def need_to_pause_movement(self, last_move_target_id, new_move_order):
        """
        When a fleet has consecutive move orders, assesses whether something about the interim destination warrants
        forcing a stop (such as a military fleet choosing to engage with an enemy fleet about to enter the same system,
        or it may provide a good vantage point to view current status of next system in path). Assessments about whether
        the new destination is suitable to move to are (currently) separately made by OrderMove.can_issue_order()
        :param last_move_target_id:
        :type last_move_target_id: int
        :param new_move_order:
        :type new_move_order: OrderMove
        :rtype: bool
        """
        fleet = self.fleet.get_object()
        # don't try skipping over more than one System
        if fleet.nextSystemID != last_move_target_id:
            return True
        universe = fo.getUniverse()
        current_dest_system = universe.getSystem(fleet.nextSystemID)
        if not current_dest_system:
            # shouldn't really happen, but just to be safe
            return True
        distance_to_next_system = ((fleet.x - current_dest_system.x)**2 + (fleet.y - current_dest_system.y)**2)**0.5
        surplus_travel_distance = fleet.speed - distance_to_next_system
        # if need more than one turn to reach current destination, then don't add another jump yet
        if surplus_travel_distance < 0:
            return True
        # TODO: add assessments for other situations we'd prefer to pause, such as cited above re military fleets, and
        # for situations where high value fleets like colony fleets might deem it safest to stop and look around
        # before proceeding
        return False

    def issue_fleet_orders(self):
        """issues AIFleetOrders which can be issued in system and moves to next one if is possible"""
        # TODO: priority
        order_completed = True
        print
        print "Checking orders for fleet %s (on turn %d), with mission type %s" % (
            self.fleet.get_object(), fo.currentTurn(), self.type or 'No mission')
        if MissionType.INVASION == self.type:
            self._check_retarget_invasion()
        just_issued_move_order = False
        last_move_target_id = INVALID_ID
        for fleet_order in self.orders:
            if just_issued_move_order and self.fleet.get_object().systemID != last_move_target_id:
                # having just issued a move order, we will normally stop issuing orders this turn, except that if there
                # are consecutive move orders we will consider moving through the first destination rather than stopping
                if (not isinstance(fleet_order, OrderMove) or
                        self.need_to_pause_movement(last_move_target_id, fleet_order)):
                    break
            print "Checking order: %s" % fleet_order
            if isinstance(fleet_order, (OrderColonize, OrderOutpost, OrderInvade)):  # TODO: invasion?
                if self._check_abort_mission(fleet_order):
                    print "Aborting fleet order %s" % fleet_order
                    return
            self.check_mergers(context=str(fleet_order))
            if fleet_order.can_issue_order(verbose=False):
                if isinstance(fleet_order, OrderMove) and order_completed:  # only move if all other orders completed
                    print "Issuing fleet order %s" % fleet_order
                    fleet_order.issue_order()
                    just_issued_move_order = True
                    last_move_target_id = fleet_order.target.id
                elif not isinstance(fleet_order, OrderMove):
                    print "Issuing fleet order %s" % fleet_order
                    fleet_order.issue_order()
                else:
                    print "NOT issuing (even though can_issue) fleet order %s" % fleet_order
                print "Order issued: %s" % fleet_order.order_issued
                if not fleet_order.order_issued:
                    order_completed = False
            else:  # check that we're not held up by a Big Monster
                if fleet_order.order_issued:
                    # It's unclear why we'd really get to this spot, but it has been observed to happen, perhaps due to
                    # game being reloaded after code changes.
                    # Go on to the next order.
                    continue
                print "CAN'T issue fleet order %s" % fleet_order
                if isinstance(fleet_order, OrderMove):
                    this_system_id = fleet_order.target.id
                    this_status = foAI.foAIstate.systemStatus.setdefault(this_system_id, {})
                    threat_threshold = fo.currentTurn() * MilitaryAI.cur_best_mil_ship_rating() / 4.0
                    if this_status.get('monsterThreat', 0) > threat_threshold:
                        # if this move order is not this mil fleet's final destination, and blocked by Big Monster,
                        # release and hope for more effective reassignment
                        if (self.type not in (MissionType.MILITARY, MissionType.SECURE) or
                                fleet_order != self.orders[-1]):
                            print "Aborting mission due to being blocked by Big Monster at system %d, threat %d" % (
                                this_system_id, foAI.foAIstate.systemStatus[this_system_id]['monsterThreat'])
                            print "Full set of orders were:"
                            for this_order in self.orders:
                                print " - %s" % this_order
                            self.clear_fleet_orders()
                            self.clear_target()
                            return
                break  # do not order the next order until this one is finished.
        else:  # went through entire order list
            if order_completed:
                print "Final order is completed"
                orders = self.orders
                last_order = orders[-1] if orders else None
                universe = fo.getUniverse()

                if last_order and isinstance(last_order, OrderColonize):
                    planet = universe.getPlanet(last_order.target.id)
                    sys_partial_vis_turn = universe.getVisibilityTurnsMap(planet.systemID, fo.empireID()).get(
                        fo.visibility.partial, -9999)
                    planet_partial_vis_turn = universe.getVisibilityTurnsMap(planet.id, fo.empireID()).get(
                        fo.visibility.partial, -9999)
                    if (planet_partial_vis_turn == sys_partial_vis_turn and
                            not planet.currentMeterValue(fo.meterType.population)):
                        print >> sys.stderr, ("Fleet %d has tentatively completed its "
                                              "colonize mission but will wait to confirm population.") % self.fleet.id
                        print "    Order details are %s" % last_order
                        print "    Order is valid: %s; issued: %s; executed: %s" % (
                            last_order.is_valid(), last_order.order_issued, last_order.executed)
                        if not last_order.is_valid():
                            source_target = last_order.fleet
                            target_target = last_order.target
                            print "        source target validity: %s; target target validity: %s " % (
                                bool(source_target), bool(target_target))
                        return  # colonize order must not have completed yet
                clear_all = True
                last_sys_target = INVALID_ID
                if last_order and isinstance(last_order, OrderMilitary):
                    last_sys_target = last_order.target.id
                    # not doing this until decide a way to release from a SECURE mission
                    # if (MissionType.SECURE == self.type) or
                    secure_targets = set(AIstate.colonyTargetedSystemIDs +
                                         AIstate.outpostTargetedSystemIDs +
                                         AIstate.invasionTargetedSystemIDs +
                                         AIstate.blockadeTargetedSystemIDs)
                    if last_sys_target in secure_targets:  # consider a secure mission
                        if last_sys_target in AIstate.colonyTargetedSystemIDs:
                            secure_type = "Colony"
                        elif last_sys_target in AIstate.outpostTargetedSystemIDs:
                            secure_type = "Outpost"
                        elif last_sys_target in AIstate.invasionTargetedSystemIDs:
                            secure_type = "Invasion"
                        elif last_sys_target in AIstate.blockadeTargetedSystemIDs:
                            secure_type = "Blockade"
                        else:
                            secure_type = "Unidentified"
                        print ("Fleet %d has completed initial stage of its mission "
                               "to secure system %d (targeted for %s), "
                               "may release a portion of ships" % (self.fleet.id, last_sys_target, secure_type))
                        clear_all = False
                fleet_id = self.fleet.id
                if clear_all:
                    if orders:
                        print "Fleet %d has completed its mission; clearing all orders and targets." % self.fleet.id
                        print "Full set of orders were:"
                        for this_order in orders:
                            print "\t\t %s" % this_order
                        self.clear_fleet_orders()
                        self.clear_target()
                        if foAI.foAIstate.get_fleet_role(fleet_id) in (MissionType.MILITARY,
                                                                       MissionType.SECURE):
                            allocations = MilitaryAI.get_military_fleets(mil_fleets_ids=[fleet_id],
                                                                         try_reset=False,
                                                                         thisround="Fleet %d Reassignment" % fleet_id)
                            if allocations:
                                MilitaryAI.assign_military_fleets_to_systems(use_fleet_id_list=[fleet_id],
                                                                             allocations=allocations)
                    else:  # no orders
                        print "No Current Orders"
                else:
                    # TODO: evaluate releasing a smaller portion or none of the ships
                    system_status = foAI.foAIstate.systemStatus.setdefault(last_sys_target, {})
                    new_fleets = []
                    threat_present = system_status.get('totalThreat', 0) + system_status.get('neighborThreat', 0) > 0
                    target_system = universe.getSystem(last_sys_target)
                    if not threat_present and target_system:
                        for pid in target_system.planetIDs:
                            planet = universe.getPlanet(pid)
                            if (planet and
                                    planet.owner != fo.empireID() and
                                    planet.currentMeterValue(fo.meterType.maxDefense) > 0):
                                threat_present = True
                                break
                    if not threat_present:
                        print "No current threat in target system; releasing a portion of ships."
                        # at least first stage of current task is done;
                        # release extra ships for potential other deployments
                        new_fleets = FleetUtilsAI.split_fleet(self.fleet.id)
                    else:
                        print "Threat remains in target system; NOT releasing any ships."
                    new_military_fleets = []
                    for fleet_id in new_fleets:
                        if foAI.foAIstate.get_fleet_role(fleet_id) in COMBAT_MISSION_TYPES:
                            new_military_fleets.append(fleet_id)
                    allocations = []
                    if new_military_fleets:
                        allocations = MilitaryAI.get_military_fleets(
                            mil_fleets_ids=new_military_fleets,
                            try_reset=False,
                            thisround="Fleet Reassignment %s" % new_military_fleets
                        )
                    if allocations:
                        MilitaryAI.assign_military_fleets_to_systems(use_fleet_id_list=new_military_fleets,
                                                                     allocations=allocations)

    def generate_fleet_orders(self):
        """generates AIFleetOrders from fleets targets to accomplish"""
        universe = fo.getUniverse()
        fleet_id = self.fleet.id
        fleet = universe.getFleet(fleet_id)
        if (not fleet) or fleet.empty or (fleet_id in universe.destroyedObjectIDs(fo.empireID())):
            # fleet was probably merged into another or was destroyed
            foAI.foAIstate.delete_fleet_info(fleet_id)
            return

        # TODO: priority
        self.clear_fleet_orders()
        system_id = fleet.systemID
        start_sys_id = [fleet.nextSystemID, system_id][system_id >= 0]
        # if fleet doesn't have any mission,
        # then repair if needed or resupply if is current location not in supplyable system
        empire = fo.getEmpire()
        fleet_supplyable_system_ids = empire.fleetSupplyableSystemIDs
        # if (not self.hasAnyAIMissionTypes()):
        if not self.target and (system_id not in set(AIstate.colonyTargetedSystemIDs +
                                                     AIstate.outpostTargetedSystemIDs +
                                                     AIstate.invasionTargetedSystemIDs +
                                                     AIstate.blockadeTargetedSystemIDs)):
            if self._need_repair():
                repair_fleet_order = MoveUtilsAI.get_repair_fleet_order(self.fleet, start_sys_id)
                if repair_fleet_order and repair_fleet_order.is_valid():
                    self.orders.append(repair_fleet_order)
            cur_fighter_capacity, max_fighter_capacity = FleetUtilsAI.get_fighter_capacity_of_fleet(fleet_id)
            if (fleet.fuel < fleet.maxFuel or cur_fighter_capacity < max_fighter_capacity
                    and self.get_location_target().id not in fleet_supplyable_system_ids):
                resupply_fleet_order = MoveUtilsAI.get_resupply_fleet_order(self.fleet, self.get_location_target())
                if resupply_fleet_order.is_valid():
                    self.orders.append(resupply_fleet_order)
            return  # no targets

        if self.target:
            # for some targets fleet has to visit systems and therefore fleet visit them
            system_to_visit = self.target.get_system()
            orders_to_visit_systems = MoveUtilsAI.create_move_orders_to_system(self.fleet, system_to_visit)
            # TODO: if fleet doesn't have enough fuel to get to final target, consider resetting Mission
            for fleet_order in orders_to_visit_systems:
                self.orders.append(fleet_order)

            # also generate appropriate final orders
            fleet_order = self._get_fleet_order_from_target(self.type, self.target)
            self.orders.append(fleet_order)

    def _need_repair(self, repair_limit=0.70):
        """Check if fleet needs to be repaired.

         If the fleet is already at a system where it can be repaired, stay there until fully repaired.
         Otherwise, repair if fleet health is below specified *repair_limit*.
         For military fleets, there is a special evaluation called, cf. *MilitaryAI.avail_mil_needing_repair()*

         :param repair_limit: percentage of health below which the fleet is sent to repair
         :type repair_limit: float
         :return: True if fleet needs repair
         :rtype: bool
        """
        # TODO: More complex evaluation if fleet needs repair (consider self-repair, distance, threat, mission...)
        universe = fo.getUniverse()
        fleet_id = self.fleet.id
        # if we are already at a system where we can repair, make sure we use it...
        system = self.fleet.get_system()
        # TODO starlane obstruction is not considered in the next call
        nearest_dock = MoveUtilsAI.get_best_drydock_system_id(system.id, fleet_id)
        if nearest_dock == system.id:
            repair_limit = 0.99
        # if combat fleet, use military repair check
        if foAI.foAIstate.get_fleet_role(fleet_id) in COMBAT_MISSION_TYPES:
            return fleet_id in MilitaryAI.avail_mil_needing_repair([fleet_id], on_mission=bool(self.orders),
                                                                   repair_limit=repair_limit)[0]
        # TODO: Allow to split fleet to send only damaged ships to repair
        fleet = universe.getFleet(fleet_id)
        ships_cur_health = 0
        ships_max_health = 0
        for ship_id in fleet.shipIDs:
            this_ship = universe.getShip(ship_id)
            ships_cur_health += this_ship.currentMeterValue(fo.meterType.structure)
            ships_max_health += this_ship.currentMeterValue(fo.meterType.maxStructure)
        return ships_cur_health < repair_limit * ships_max_health

    def get_location_target(self):
        """system AITarget where fleet is or will be"""
        # TODO add parameter turn
        fleet = fo.getUniverse().getFleet(self.fleet.id)
        system_id = fleet.systemID
        if system_id >= 0:
            return System(system_id)
        else:  # in starlane, so return next system
            return System(fleet.nextSystemID)

    def __eq__(self, other):
        return isinstance(other, self.__class__) and self.fleet == other.target

    def __str__(self):
        fleet = self.fleet.get_object()

        fleet_id = self.fleet.id
        return "%-25s [%-11s] ships: %2d; total rating: %4d; target: %s" % (fleet,
                                                                            "NONE" if self.type is None else self.type,
                                                                            (fleet and len(fleet.shipIDs)) or 0,
                                                                            CombatRatingsAI.get_fleet_rating(fleet_id),
                                                                            self.target or 'no target')
