import freeOrionAIInterface as fo # pylint: disable=import-error

from fleet_orders import OrderMove, OrderOutpost, OrderColonize, OrderMilitary, OrderInvade, OrderDefend
import AIstate
import FleetUtilsAI
import EnumsAI
import FreeOrionAI as foAI
import MoveUtilsAI
import MilitaryAI
import InvasionAI
import PlanetUtilsAI
from universe_object import System, Fleet, Planet
from EnumsAI import AIFleetMissionType

ORDERS_FOR_MISSION = {
        AIFleetMissionType.FLEET_MISSION_EXPLORATION: OrderMove,
        AIFleetMissionType.FLEET_MISSION_OUTPOST: OrderOutpost,
        AIFleetMissionType.FLEET_MISSION_COLONISATION: OrderColonize,
        AIFleetMissionType.FLEET_MISSION_INVASION: OrderInvade,
        AIFleetMissionType.FLEET_MISSION_MILITARY: OrderMilitary,
        AIFleetMissionType.FLEET_MISSION_SECURE: OrderMilitary,  # mostly same as MILITARY, but waits for system removal from all targeted system lists (invasion, colonization, outpost, blockade) before clearing
        AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION: OrderInvade,
        AIFleetMissionType.FLEET_MISSION_ORBITAL_OUTPOST: OrderOutpost,
        AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE: OrderDefend
        }

COMBAT_MISSION_TYPES = (AIFleetMissionType.FLEET_MISSION_MILITARY,
                       AIFleetMissionType.FLEET_MISSION_SECURE
                       )

class AIFleetMission(object):
    """
    Stores information about AI mission. Every mission has fleetID and AI targets depending upon AI fleet mission type.
    """

    def __init__(self, fleet_id):
        self.orders = []
        self.fleet = Fleet(fleet_id)
        self.type = None
        self.target = None

    def __setstate__(self, state_dict):
        # New class
        if 'target_type' not in state_dict:
            self.__dict__.update(state_dict)  # update attributes
            return

        cache = {}  # old obj: new_obj

        from universe_object import Planet, Fleet, System
        TARGET_PLANET = 2
        TARGET_SYSTEM = 3
        TARGET_FLEET = 5

        def convert_target(obj):
            if obj in cache:
                return cache[obj]
            return cache.setdefault(obj, {TARGET_PLANET: Planet,
                                          TARGET_FLEET: Fleet,
                                          TARGET_SYSTEM: System
                                          }[obj.target_type](obj.target_id))

        ORDER_MOVE = 1
        ORDER_RESUPPLY = 2
        ORDER_SPLIT_FLEET = 3
        ORDER_OUTPOST = 5
        ORDER_COLONISE = 6
        ORDER_ATTACK = 7
        ORDER_DEFEND = 8
        ORDER_INVADE = 9
        ORDER_MILITARY = 10
        ORDER_REPAIR = 12

        from fleet_orders import (OrderMove, OrderResupply, OrderSplitFleet, OrderOutpost, OrderColonize,
                                  OrderAttack, OrderDefend, OrderInvade, OrderMilitary, OrderRepair)

        def convert_order(obj):
            if obj in cache:
               return cache[obj]
            types = {
                ORDER_MOVE: OrderMove,
                ORDER_RESUPPLY: OrderResupply,
                ORDER_SPLIT_FLEET: OrderSplitFleet,
                ORDER_OUTPOST: OrderOutpost,
                ORDER_COLONISE: OrderColonize,
                ORDER_ATTACK: OrderAttack,
                ORDER_DEFEND: OrderDefend,
                ORDER_INVADE: OrderInvade,
                ORDER_MILITARY: OrderMilitary,
                ORDER_REPAIR: OrderRepair}
            fleet = convert_target(obj.fleet)
            target = convert_target(obj.target)
            new_order = types[obj.order_type](fleet, target)
            new_order.executed = obj.executed
            new_order.order_issued = obj.execution_completed
            return new_order

        for k, v in state_dict.items():
            if k == 'orders':
                self.orders = [convert_order(old) for old in v]
            elif k == 'target':
                self.fleet = convert_target(v)
            elif k == '_mission_types':
                v = {k: v for k, v in v.items() if v}
                assert len(v) <= 1, 'olnly one mission allowed %s' % v
                if not v:
                    self.type = None
                    self.target = None
                    continue
                target_type, targets = v.popitem()
                assert len(targets) <= 1, 'only single target expected %s' % targets
                self.type = target_type
                self.target = convert_target(targets[0])
            elif k in ('target_type', 'target_id'):
                continue

    def add_target(self, mission_type, target):
        if self.type == mission_type and self.target == target:
            return
        if self.type or self.target:
            print "Change mission assignment from %s:%s to %s:%s" % (AIFleetMissionType.name(self.type),
                                                                     self.target,
                                                                     AIFleetMissionType.name(mission_type),
                                                                     target)
        self.type = mission_type
        self.target = target

    def clear_target(self):
        self.target = None
        self.type = None

    def has_target(self, mission_type, target):
        return self.type == mission_type and self.target == target

    def clear_fleet_orders(self):
        self.orders = []

    def _get_fleet_order_from_target(self, mission_type, target):
        fleet_target = Fleet(self.fleet.id)
        return ORDERS_FOR_MISSION[mission_type](fleet_target, target)

    def check_mergers(self, fleet_id=None, context=""):
        if fleet_id is None:
            fleet_id = self.fleet.id

        if self.type not in (AIFleetMissionType.FLEET_MISSION_MILITARY,
                            AIFleetMissionType.FLEET_MISSION_INVASION,
                            AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION,
                            AIFleetMissionType.FLEET_MISSION_SECURE,
                            AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE,
                            ):
            return
        universe = fo.getUniverse()
        empire_id = fo.empireID()
        main_fleet = universe.getFleet(fleet_id)
        system_id = main_fleet.systemID
        if system_id == -1:
            return  # can't merge fleets in middle of starlane
        system_status = foAI.foAIstate.systemStatus[system_id]
        destroyed_list = list(universe.destroyedObjectIDs(empire_id))
        other_fleets_here = [fid for fid in system_status.get('myFleetsAccessible', []) if fid != fleet_id and fid not in destroyed_list and universe.getFleet(fid).ownedBy(empire_id)]
        if not other_fleets_here:
            return  # nothing of record to merge with

        if not self.target:
            m_MT0_id = None
        else:
            m_MT0_id = self.target.id

        # TODO consider establishing an AI strategy & tactics planning document for discussing & planning
        # high level considerations for issues like fleet merger
        compatibileRolesMap = {EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE: [EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE],
                               EnumsAI.AIFleetMissionType.FLEET_MISSION_MILITARY: [EnumsAI.AIFleetMissionType.FLEET_MISSION_MILITARY],
                               EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION: [EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION],
                               EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION: [EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION],
                               }

        main_fleet_role = foAI.foAIstate.get_fleet_role(fleet_id)
        for fid in other_fleets_here:
            fleet_role = foAI.foAIstate.get_fleet_role(fid)
            if fleet_role not in compatibileRolesMap[main_fleet_role]:  # TODO: if fleetRoles such as LongRange start being used, adjust this
                continue  # will only considering subsuming fleets that have a compatible role
            fleet = universe.getFleet(fid)
            if not (fleet and (fleet.systemID == system_id)):
                continue
            if not (fleet.speed > 0 or main_fleet.speed == 0):  # TODO(Cjkjvfnby) Check this condition
                continue
            fleet_mission = foAI.foAIstate.get_fleet_mission(fid)
            do_merge = False
            need_left = 0
            if (main_fleet_role == AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE) or (fleet_role == AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE):
                if main_fleet_role == fleet_role:
                    do_merge = True
            elif (main_fleet_role == AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION) or (fleet_role == AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION):
                if main_fleet_role == fleet_role:
                    do_merge = False  # TODO: could allow merger if both orb invaders and both same target
            elif not fleet_mission and (main_fleet.speed > 0) and (fleet.speed > 0):
                do_merge = True
            else:
                if not self.target and (main_fleet.speed > 0 or fleet.speed == 0):
                    #print "\t\t\t ** Considering merging fleetA (id: %4d) into fleet (id %d) and former has no targets, will take it. FleetA mission was %s "%(fid, fleetID, fleet_mission)
                    do_merge = True
                else:
                    target = fleet_mission.target.id if fleet_mission.target else None
                    if target == m_MT0_id:
                        print "Military fleet %d has same target as %s fleet %d and will (at least temporarily) be merged into the latter" % (fid, EnumsAI.AIShipRoleType.name(fleet_role), fleet_id)
                        do_merge = True  # TODO: should probably ensure that fleetA has aggression on now
                    elif main_fleet.speed > 0:
                        neighbors = foAI.foAIstate.systemStatus.get(system_id, {}).get('neighbors', [])
                        if (target == system_id) and m_MT0_id in neighbors:  # consider 'borrowing' for work in neighbor system  # TODO check condition
                            if self.type in (AIFleetMissionType.FLEET_MISSION_MILITARY,
                                                      AIFleetMissionType.FLEET_MISSION_SECURE):
                                #continue
                                if self.type == AIFleetMissionType.FLEET_MISSION_SECURE:  # actually, currently this is probably the onle one of all four that should really be possibile in this situation
                                    need_left = 1.5 * sum([sysStat.get('fleetThreat', 0) for sysStat in
                                                           [foAI.foAIstate.systemStatus.get(neighbor, {}) for neighbor in
                                                                                                          [nid for nid in foAI.foAIstate.systemStatus.get(system_id, {}).get('neighbors', []) if nid != m_MT0_id]]])
                                    fBRating = foAI.foAIstate.get_rating(fid)
                                    if (need_left < fBRating.get('overall', 0)) and fBRating.get('nships', 0) > 1:
                                        do_merge = True
            if do_merge:
                FleetUtilsAI.merge_fleet_a_into_b(fid, fleet_id, need_left,
                                                  context="Order %s of mission %s" % (context, self))
        return

    def _is_valid_fleet_mission_target(self, mission_type, target):
        if not target:
            return False
        if mission_type == AIFleetMissionType.FLEET_MISSION_EXPLORATION:
            if isinstance(target, System):
                empire = fo.getEmpire()
                if not empire.hasExploredSystem(target.id):
                    return True
        elif mission_type in [AIFleetMissionType.FLEET_MISSION_OUTPOST, AIFleetMissionType.FLEET_MISSION_ORBITAL_OUTPOST]:
            fleet = self.fleet.get_object()
            if not fleet.hasOutpostShips:
                return False
            if isinstance(target, Planet):
                planet = target.get_object()
                if planet.unowned:
                    return True
        elif mission_type == AIFleetMissionType.FLEET_MISSION_COLONISATION:
            fleet = self.fleet.get_object()
            if not fleet.hasColonyShips:
                return False
            if isinstance(target, Planet):
                planet = target.get_object()
                population = planet.currentMeterValue(fo.meterType.population)
                if planet.unowned or (planet.owner == fleet.owner and population == 0):
                    return True
        elif mission_type in [AIFleetMissionType.FLEET_MISSION_INVASION, AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION]:
            fleet = self.fleet.get_object()
            if not fleet.hasTroopShips:
                return False
            if isinstance(target, Planet):
                planet = target.get_object()
                if not planet.unowned or planet.owner != fleet.owner:  # TODO remove latter portion of this check in light of invasion retargeting, or else correct logic
                    return True
        elif mission_type in [AIFleetMissionType.FLEET_MISSION_MILITARY, AIFleetMissionType.FLEET_MISSION_SECURE, AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE]:
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
                if planet.currentMeterValue(fo.meterType.population) == 0 and (planet.ownedBy(fo.empireID()) or planet.unowned):
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
        empire = fo.getEmpire()
        empire_id = fo.empireID()
        fleet_id = self.fleet.id
        fleet = universe.getFleet(fleet_id)
        if fleet.systemID == -1:
            # next_loc = fleet.nextSystemID
            return  # TODO: still check
        system = universe.getSystem(fleet.systemID)
        if not system:
            return
        orders = self.orders
        last_sys_target = -1
        if orders:
            last_sys_target = orders[-1].target.id
        if last_sys_target == fleet.systemID:
            return  # TODO: check for best local target
        open_targets = []
        already_targeted = InvasionAI.get_invasion_targeted_planet_ids(system.planetIDs, AIFleetMissionType.FLEET_MISSION_INVASION)
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
        target_id = -1
        best_score = -1
        target_troops = 0
        #
        fleet_supplyable_system_ids = empire.fleetSupplyableSystemIDs
        fleet_supplyable_planet_ids = PlanetUtilsAI.get_planets_in__systems_ids(fleet_supplyable_system_ids)
        for pid, rating in InvasionAI.assign_invasion_values(open_targets, AIFleetMissionType.FLEET_MISSION_INVASION, fleet_supplyable_planet_ids, empire).items():
            p_score, p_troops = rating
            if p_score > best_score:
                if p_troops >= troops_in_fleet:
                    continue
                best_score = p_score
                target_id = pid
                target_troops = p_troops
        if target_id == -1:
            return

        print "\t AIFleetMission._check_retarget_invasion: splitting and retargetting fleet %d" % fleet_id
        new_fleets = FleetUtilsAI.split_fleet(fleet_id)
        self.clear_target()  # TODO: clear from foAIstate
        self.clear_fleet_orders()
        # pods_needed = max(0, math.ceil((target_troops - 2 * (FleetUtilsAI.count_parts_fleetwide(fleet_id, ["GT_TROOP_POD"])) + 0.05) / 2.0))
        troops_needed = max(0, target_troops - FleetUtilsAI.count_troops_in_fleet(fleet_id))
        found_stats = {}
        min_stats = {'rating': 0, 'troopCapacity': troops_needed}
        target_stats = {'rating': 10, 'troopCapacity': troops_needed}
        found_fleets = []
        # TODO check if next statement does not mutate any global states and can be removed
        _ = FleetUtilsAI.get_fleets_for_mission(1, target_stats, min_stats, found_stats, "",
                                                systems_to_check=[fleet.systemID], systems_checked=[],
                                                fleet_pool_set=set(new_fleets), fleet_list=found_fleets,
                                                verbose=False)
        for fid in found_fleets:
            FleetUtilsAI.merge_fleet_a_into_b(fid, fleet_id)
        target = Planet(target_id)
        self.add_target(AIFleetMissionType.FLEET_MISSION_INVASION, target)
        self.generate_fleet_orders()

    def issue_fleet_orders(self):
        """issues AIFleetOrders which can be issued in system and moves to next one if is possible"""
        # TODO: priority
        order_completed = True
        print "--------------"
        print "Checking orders for fleet %d (on turn %d), with mission type %s" % (self.fleet.id, fo.currentTurn(), self.type and AIFleetMissionType.name(self.type) or 'No mission')
        print "\t Full Orders are:"
        for this_order in self.orders:
            print "\t\t| %s" % this_order
        print "\t\t------"
        if AIFleetMissionType.FLEET_MISSION_INVASION == self.type:
            self._check_retarget_invasion()
        for fleet_order in self.orders:
            print "\t| checking Order: %s" % fleet_order
            if isinstance(fleet_order, (OrderColonize, OrderOutpost, OrderInvade)):  # TODO: invasion?
                if self._check_abort_mission(fleet_order):
                    print "\t\t| Aborting fleet order %s" % fleet_order
                    return
            self.check_mergers(context=str(fleet_order))
            if fleet_order.can_issue_order(verbose=True):
                if isinstance(fleet_order, OrderMove) and order_completed:  # only move if all other orders completed
                    print "\t\t| issuing fleet order %s" % fleet_order
                    fleet_order.issue_order()
                elif not isinstance(fleet_order, OrderMove):
                    print "\t\t| issuing fleet order %s" % fleet_order
                    fleet_order.issue_order()
                else:
                    print "\t\t| NOT issuing (even though can_issue) fleet order %s" % fleet_order
                print "\t\t| order status-- order issued: %s" % fleet_order.order_issued
                if not fleet_order.order_issued:
                    order_completed = False
            else:  # check that we're not held up by a Big Monster
                if fleet_order.order_issued:
                    # It's unclear why we'd really get to this spot, but it has been observed to happen, perhaps due to
                    # game being reloaded after code changes.
                    # Go on to the next order.
                    continue
                print "\t\t| CAN'T issue fleet order %s" % fleet_order
                if isinstance(fleet_order, OrderMove):
                    this_system_id = fleet_order.target.id
                    this_status = foAI.foAIstate.systemStatus.setdefault(this_system_id, {})
                    if this_status.get('monsterThreat', 0) > fo.currentTurn() * MilitaryAI.cur_best_mil_ship_rating()/4.0:
                        if (self.type not in (AIFleetMissionType.FLEET_MISSION_MILITARY,
                                              AIFleetMissionType.FLEET_MISSION_SECURE) or
                            fleet_order != self.orders[-1]  # if this move order is not this mil fleet's final destination, and blocked by Big Monster, release and hope for more effective reassignment
                            ):
                            print "Aborting mission due to being blocked by Big Monster at system %d, threat %d"%(this_system_id, foAI.foAIstate.systemStatus[this_system_id]['monsterThreat'])
                            print "Full set of orders were:"
                            for this_order in self.orders:
                                print "\t\t %s" % this_order
                            self.clear_fleet_orders()
                            self.clear_target()
                            return
            # moving to another system stops issuing all orders in system where fleet is
            # move order is also the last order in system
            if isinstance(fleet_order, OrderMove):
                fleet = self.fleet.get_object()
                if fleet.systemID != fleet_order.target.id:
                    break
        else:  # went through entire order list
            if order_completed:
                print "\t| Final order is completed"
                orders = self.orders
                last_order = orders[-1] if orders else None
                universe = fo.getUniverse()

                if last_order and isinstance(last_order, OrderColonize):
                    planet = universe.getPlanet(last_order.target.id)
                    sys_partial_vis_turn = universe.getVisibilityTurnsMap(planet.systemID, fo.empireID()).get(fo.visibility.partial, -9999)
                    planet_partial_vis_turn = universe.getVisibilityTurnsMap(planet.id, fo.empireID()).get(fo.visibility.partial, -9999)
                    if planet_partial_vis_turn == sys_partial_vis_turn and not planet.currentMeterValue(fo.meterType.population):
                        print "Potential Error: Fleet %d has tentatively completed its colonize mission but will wait to confirm population." % self.fleet.id
                        print "    Order details are %s" % last_order
                        print "    Order is valid: %s ; is Executed : %s; is execution completed: %s " % (last_order.is_valid(), last_order.isExecuted(), last_order.isExecutionCompleted())
                        if not last_order.is_valid():
                            source_target = last_order.fleet
                            target_target = last_order.target
                            print "        source target validity: %s; target target validity: %s " % (bool(source_target), bool(target_target))
                        return  # colonize order must not have completed yet
                clearAll = True
                last_sys_target = -1
                if last_order and isinstance(last_order, OrderMilitary):
                    last_sys_target = last_order.target.id
                    # if (AIFleetMissionType.FLEET_MISSION_SECURE == self.type) or # not doing this until decide a way to release from a SECURE mission
                    secure_targets = set(AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs + AIstate.invasionTargetedSystemIDs + AIstate.blockadeTargetedSystemIDs)
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
                        print "Fleet %d has completed initial stage of its mission to secure system %d (targeted for %s), may release a portion of ships" % (self.fleet.id, last_sys_target, secure_type)
                        clearAll = False
                fleet_id = self.fleet.id
                if clearAll:
                    if orders:
                        print "Fleet %d has completed its mission; clearing all orders and targets." % self.fleet.id
                        print "Full set of orders were:"
                        for this_order in orders:
                            print "\t\t %s" % this_order
                        self.clear_fleet_orders()
                        self.clear_target()
                        if foAI.foAIstate.get_fleet_role(fleet_id) in (AIFleetMissionType.FLEET_MISSION_MILITARY,
                                                                       AIFleetMissionType.FLEET_MISSION_SECURE):
                            allocations = MilitaryAI.get_military_fleets(milFleetIDs=[fleet_id], tryReset=False, thisround="Fleet %d Reassignment" % fleet_id)
                            if allocations:
                                MilitaryAI.assign_military_fleets_to_systems(useFleetIDList=[fleet_id], allocations=allocations)
                    else:  # no orders
                        print "No Current Orders"
                else:
                    #TODO: evaluate releasing a smaller portion or none of the ships
                    system_status = foAI.foAIstate.systemStatus.setdefault(last_sys_target, {})
                    new_fleets = []
                    threat_present = (system_status.get('totalThreat', 0) != 0) or (system_status.get('neighborThreat', 0) != 0)
                    target_system = universe.getSystem(last_sys_target)
                    if not threat_present and target_system:
                        for pid in target_system.planetIDs:
                            planet = universe.getPlanet(pid)
                            if planet and planet.owner != fo.empireID() and planet.currentMeterValue(fo.meterType.maxDefense) > 0:
                                threat_present = True
                                break
                    if not threat_present:
                        print "No current threat in target system; releasing a portion of ships."
                        new_fleets = FleetUtilsAI.split_fleet(self.fleet.id)  # at least first stage of current task is done; release extra ships for potential other deployments
                    else:
                        print "Threat remains in target system; NOT releasing any ships."
                    new_military_fleets = []
                    for fleet_id in new_fleets:
                        if foAI.foAIstate.get_fleet_role(fleet_id) in COMBAT_MISSION_TYPES:
                            new_military_fleets.append(fleet_id)
                    allocations = []
                    if new_military_fleets:
                        allocations = MilitaryAI.get_military_fleets(milFleetIDs=new_military_fleets, tryReset=False, thisround="Fleet Reassignment %s" % new_military_fleets)
                    if allocations:
                        MilitaryAI.assign_military_fleets_to_systems(useFleetIDList=new_military_fleets, allocations=allocations)

    def generate_fleet_orders(self):
        """generates AIFleetOrders from fleets targets to accomplish"""
        universe = fo.getUniverse()
        fleet_id = self.fleet.id
        fleet = universe.getFleet(fleet_id)
        if (not fleet) or fleet.empty or (fleet_id in universe.destroyedObjectIDs(fo.empireID())):  # fleet was probably merged into another or was destroyed
            foAI.foAIstate.delete_fleet_info(fleet_id)
            return

        # TODO: priority
        self.clear_fleet_orders()
        system_id = fleet.systemID
        start_sys_id = [fleet.nextSystemID, system_id][system_id >= 0]
        # if fleet doesn't have any mission, then repair if needed or resupply if is current location not in supplyable system
        empire = fo.getEmpire()
        fleet_supplyable_system_ids = empire.fleetSupplyableSystemIDs
        #if (not self.hasAnyAIMissionTypes()):
        if not self.target and (system_id not in set(AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs +
                                                    AIstate.invasionTargetedSystemIDs + AIstate.blockadeTargetedSystemIDs)):
            if self._need_repair():
                repair_fleet_order = MoveUtilsAI.get_repair_fleet_order(self.fleet, start_sys_id)
                if repair_fleet_order.is_valid():
                    self.orders.append(repair_fleet_order)
            if fleet.fuel < fleet.maxFuel and self.get_location_target().id not in fleet_supplyable_system_ids:
                resupply_fleet_order = MoveUtilsAI.get_resupply_fleet_order(self.fleet, self.get_location_target())
                if resupply_fleet_order.is_valid():
                    self.orders.append(resupply_fleet_order)
            return  # no targets

        # for some targets fleet has to visit systems and therefore fleet visit them
        if self.target:
            system_targets_required_to_visit = [self.target.get_system()]
            orders_to_visit_systems = MoveUtilsAI.get_fleet_orders_from_system_targets(self.fleet, system_targets_required_to_visit)
            #TODO: if fleet doesn't have enough fuel to get to final target, consider resetting Mission
            for fleet_order in orders_to_visit_systems:
                self.orders.append(fleet_order)

        # if fleet is in some system = fleet.system_id >=0, then also generate system AIFleetOrders
        if system_id >= 0 and self.target:
            # system in where fleet is
            system_target = System(system_id)
            # if mission aiTarget has required system where fleet is, then generate fleet_order from this aiTarget
            # for all targets in all mission types get required systems to visit
            if system_target == self.target.get_system():
                # from target required to visit get fleet orders to accomplish target
                fleet_order = self._get_fleet_order_from_target(self.type, self.target)
                self.orders.append(fleet_order)

    def _need_repair(self):
        """
        Check if fleet need repair. Check if fleet HP is less of cutoff.
        TODO make more clever implementation.
        """
        repair_limit = 0.70

        universe = fo.getUniverse()
        fleet_id = self.fleet.id
        # if combat fleet, use military repair check
        if foAI.foAIstate.get_fleet_role(fleet_id) in COMBAT_MISSION_TYPES:
            return fleet_id in MilitaryAI.avail_mil_needing_repair([fleet_id], False, bool(self.orders))[0]
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
        if self.type:
            fleet_id = self.fleet.id
            fleet = self.fleet.get_object()
            return "%-20s [%10s mission]: %3d ships, total rating: %7d target: %s" % (fleet,
                                                                                 AIFleetMissionType.name(self.type),
                                                                                 (fleet and len(fleet.shipIDs)) or 0,
                                                                                 foAI.foAIstate.get_rating(fleet_id).get('overall', 0),
                                                                                 self.target)
        else:
            return 'Mission with out mission types'
