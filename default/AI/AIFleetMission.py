import freeOrionAIInterface as fo # pylint: disable=import-error

import AIFleetOrder
import AIstate
import FleetUtilsAI
import EnumsAI
import FreeOrionAI as foAI
import MoveUtilsAI
import ProductionAI
import MilitaryAI
import InvasionAI
import PlanetUtilsAI
import math
from freeorion_tools import dict_from_map
from AITarget import AITarget
from EnumsAI import FLEET_MISSION_TYPES, AIFleetMissionType, AIFleetOrderType

ORDERS_FOR_MISSION = {
        AIFleetMissionType.FLEET_MISSION_EXPLORATION: AIFleetOrderType.ORDER_MOVE,
        AIFleetMissionType.FLEET_MISSION_OUTPOST: AIFleetOrderType.ORDER_OUTPOST,
        AIFleetMissionType.FLEET_MISSION_COLONISATION: AIFleetOrderType.ORDER_COLONISE,
        AIFleetMissionType.FLEET_MISSION_SPLIT_FLEET: AIFleetOrderType.ORDER_SPLIT_FLEET,  # not really supported in this fashion currently
        AIFleetMissionType.FLEET_MISSION_MERGE_FLEET: AIFleetOrderType.ORDER_MERGE_FLEET,  # not really supported in this fashion currently
        AIFleetMissionType.FLEET_MISSION_HIT_AND_RUN: AIFleetOrderType.ORDER_MILITARY,  # currently same as MILITARY
        AIFleetMissionType.FLEET_MISSION_ATTACK: AIFleetOrderType.ORDER_MILITARY,  # currently same as MILITARY
        AIFleetMissionType.FLEET_MISSION_DEFEND: AIFleetOrderType.ORDER_MILITARY,  # currently same as MILITARY
        AIFleetMissionType.FLEET_MISSION_LAST_STAND: AIFleetOrderType.ORDER_MILITARY,  # currently same as MILITARY
        AIFleetMissionType.FLEET_MISSION_INVASION: AIFleetOrderType.ORDER_INVADE,
        AIFleetMissionType.FLEET_MISSION_MILITARY: AIFleetOrderType.ORDER_MILITARY,
        AIFleetMissionType.FLEET_MISSION_SECURE: AIFleetOrderType.ORDER_MILITARY,  # mostly same as MILITARY, but waits for system removal from all targeted system lists (invasion, colonization, outpost, blockade) before clearing
        AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE: AIFleetOrderType.ORDER_DEFEND,
        AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION: AIFleetOrderType.ORDER_INVADE,
        AIFleetMissionType.FLEET_MISSION_ORBITAL_OUTPOST: AIFleetOrderType.ORDER_OUTPOST,
        AIFleetMissionType.FLEET_MISSION_ORBITAL_COLONISATION: AIFleetOrderType.ORDER_COLONISE,
        AIFleetMissionType.FLEET_MISSION_REPAIR: AIFleetOrderType.ORDER_REPAIR}

COMBAT_MISSION_TYPES = (AIFleetMissionType.FLEET_MISSION_MILITARY,
                       AIFleetMissionType.FLEET_MISSION_ATTACK,
                       AIFleetMissionType.FLEET_MISSION_DEFEND,
                       AIFleetMissionType.FLEET_MISSION_HIT_AND_RUN,
                       AIFleetMissionType.FLEET_MISSION_SECURE
                       )

class AIFleetMission(object):
    """
    Stores information about AI mission. Every mission has fleetID and AI targets depending upon AI fleet mission type.
    """

    def __init__(self, fleet_id):
        self.orders = []
        self.mission_type = EnumsAI.AIMissionType.FLEET_MISSION
        self.target = AITarget(EnumsAI.TargetType.TARGET_FLEET, fleet_id)
        self._mission_types = {}
        for mt in FLEET_MISSION_TYPES:
            self._mission_types[mt] = []
        self.target_id = self.target.target_id
        self.target_type = self.target.target_type

    def __setstate__(self, state_dict):
        self.__dict__.update(state_dict)  # update attributes
        #print "Fleet mission unpickle: passed %s"%state_dict
        for attrib, default in [('orders', state_dict.get('_AIFleetMission__aiFleetOrders', [])),
                            ('mission_type', EnumsAI.AIMissionType.FLEET_MISSION),
                            ('_mission_types', state_dict.get('_AIAbstractMission__aiMissionTypes', {})),
                            ('target_type', EnumsAI.TargetType.TARGET_FLEET)]:
            if attrib not in state_dict:
                #print "Fleet mission unpickle: setting %s to %s"%(attrib, default)
                self.__dict__[attrib] = default
        if 'target' not in state_dict:
            old_target = state_dict.get('_AIAbstractMission__aiTarget', None)
            target_id = old_target.target_id if (old_target is not None) else -1 #TODO consider a harder fail
            self.__dict__['target'] = old_target
            self.__dict__['target_id'] = target_id
            #print "Fleet mission unpickle: setting %s to %s"%('target', old_target)
            #print "Fleet mission unpickle: setting %s to %s"%('target_id', target_id)

    def add_target(self, mission_type, target):
        targets = self.get_targets(mission_type)
        if not target in targets:
            targets.append(target)

    def _remove_target(self, mission_type, target):
        targets = self.get_targets(mission_type)
        if target in targets:
            targets.remove(target)

    def clear_targets(self, mission_type):
        if mission_type == -1:
            targets = []
            for mission_type in self.get_mission_types():
                targets.extend(self.get_targets(mission_type))
        else:
            targets = self.get_targets(mission_type)
        for target in targets:
            self._remove_target(mission_type, target)

    def get_targets(self, mission_type):
        return self._mission_types.get(mission_type, [])

    def has_target(self, mission_type, target):
        return target in self.get_targets(mission_type)

    def get_mission_types(self):
        return [mission_type for mission_type in FLEET_MISSION_TYPES if self.get_targets(mission_type)]

    def clear_fleet_orders(self):
        self.orders = []

    def _get_fleet_order_from_target(self, mission_type, target):
        fleet_targets = AITarget(EnumsAI.TargetType.TARGET_FLEET, self.target_id)
        order_type = ORDERS_FOR_MISSION.get(mission_type, AIFleetOrderType.ORDER_INVALID)
        return AIFleetOrder.AIFleetOrder(order_type, fleet_targets, target)

    def check_mergers(self, fleet_id=None, context=""):
        if fleet_id is None:
            fleet_id = self.target_id

        mission_types = self.get_mission_types()  # normally, currently, should only be one
        if len(mission_types) != 1:
            return  # if this fleet has multiple mission types, will not subsume other fleets
        mission_type = mission_types[0]
        if mission_type not in (AIFleetMissionType.FLEET_MISSION_ATTACK,
                                AIFleetMissionType.FLEET_MISSION_DEFEND,
                                AIFleetMissionType.FLEET_MISSION_LAST_STAND,
                                AIFleetMissionType.FLEET_MISSION_MILITARY,
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
        targets = self.get_targets(mission_type)
        if not targets:
            pass
            #return  #let's let invasion fleets with no target get merged
            m_MT0_id = None
        else:
            m_MT0_id = targets[0].target_id
        if len(targets) > 1:
            pass
            print "\tConsidering merging fleets into fleet %d, but it has multiple targets: %s" % (fleet_id, str(targets))
        # TODO (Cjkjvfnby) check why we cant merge fleet with other missions to each other
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
                fleet_mission_type = fleet_mission.get_mission_types()[0] if fleet_mission.get_mission_types() else AIFleetMissionType.FLEET_MISSION_INVALID
                fleet_targets = fleet_mission.get_targets(fleet_mission_type)
                if len(fleet_targets) > 1:
                    pass
                elif not fleet_targets and (main_fleet.speed > 0 or fleet.speed == 0):
                    #print "\t\t\t ** Considering merging fleetA (id: %4d) into fleet (id %d) and former has no targets, will take it. FleetA mission was %s "%(fid, fleetID, fleet_mission)
                    do_merge = True
                else:
                    target = fleet_targets[0].target_id
                    if target == m_MT0_id:
                        print "Military fleet %d has same target as %s fleet %d and will (at least temporarily) be merged into the latter" % (fid, EnumsAI.AIShipRoleType.name(fleet_role), fleet_id)
                        do_merge = True  # TODO: should probably ensure that fleetA has aggression on now
                    elif main_fleet.speed > 0:
                        neighbors = foAI.foAIstate.systemStatus.get(system_id, {}).get('neighbors', [])
                        if (target == system_id) and m_MT0_id in neighbors:  # consider 'borrowing' for work in neighbor system  # TODO check condition
                            if fleet_mission_type in (AIFleetMissionType.FLEET_MISSION_ATTACK,
                                                      AIFleetMissionType.FLEET_MISSION_DEFEND,
                                                      AIFleetMissionType.FLEET_MISSION_MILITARY,
                                                      AIFleetMissionType.FLEET_MISSION_SECURE):
                                #continue
                                if fleet_mission_type in (AIFleetMissionType.FLEET_MISSION_DEFEND,
                                                          AIFleetMissionType.FLEET_MISSION_SECURE,  # actually, currently this is probably the onle one of all four that should really be possibile in this situation
                                                          ):
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
        if not target.valid:
            return False
        if mission_type == AIFleetMissionType.FLEET_MISSION_EXPLORATION:
            if target.target_type == EnumsAI.TargetType.TARGET_SYSTEM:
                empire = fo.getEmpire()
                if not empire.hasExploredSystem(target.target_id):
                    return True
        elif mission_type in [AIFleetMissionType.FLEET_MISSION_OUTPOST, AIFleetMissionType.FLEET_MISSION_ORBITAL_OUTPOST]:
            universe = fo.getUniverse()
            fleet = universe.getFleet(self.target_id)
            if not fleet.hasOutpostShips:
                return False
            if target.target_type == EnumsAI.TargetType.TARGET_PLANET:
                planet = universe.getPlanet(target.target_id)
                if planet.unowned:
                    return True
        elif mission_type in [ AIFleetMissionType.FLEET_MISSION_COLONISATION, AIFleetMissionType.FLEET_MISSION_ORBITAL_COLONISATION]:
            universe = fo.getUniverse()
            fleet = universe.getFleet(self.target_id)
            if not fleet.hasColonyShips:
                return False
            if target.target_type == EnumsAI.TargetType.TARGET_PLANET:
                planet = universe.getPlanet(target.target_id)
                population = planet.currentMeterValue(fo.meterType.population)
                if planet.unowned or (planet.owner == fleet.owner and population == 0):
                    return True
        elif mission_type in [AIFleetMissionType.FLEET_MISSION_INVASION, AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION]:
            universe = fo.getUniverse()
            fleet = universe.getFleet(self.target_id)
            if not fleet.hasTroopShips:
                return False
            if target.target_type == EnumsAI.TargetType.TARGET_PLANET:
                planet = universe.getPlanet(target.target_id)
                if not planet.unowned or planet.owner != fleet.owner:  # TODO remove latter portion of this check in light of invasion retargeting, or else correct logic
                    return True
        elif mission_type in [AIFleetMissionType.FLEET_MISSION_MILITARY, AIFleetMissionType.FLEET_MISSION_SECURE, AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE]:
            universe = fo.getUniverse()
            if target.target_type == EnumsAI.TargetType.TARGET_SYSTEM:
                return True
        # TODO: implement other mission types
        return False

    def clean_invalid_targets(self):
        """clean invalid AITargets"""
        for mission_type in self.get_mission_types():
            for target in self.get_targets(mission_type):
                if not self._is_valid_fleet_mission_target(mission_type, target):
                    self._remove_target(mission_type, target)

    def _check_abort_mission(self, fleet_order):
        """ checks if current mission (targeting a planet) should be aborted"""
        planet = fo.getUniverse().getPlanet(fleet_order.target.target_id)
        if planet:
            order_type = fleet_order.order_type
            if order_type == AIFleetOrderType.ORDER_COLONISE:
                if planet.currentMeterValue(fo.meterType.population) == 0 and (planet.ownedBy(fo.empireID()) or planet.unowned):
                    return False
            elif order_type == AIFleetOrderType.ORDER_OUTPOST:
                if planet.unowned:
                    return False
            elif order_type == AIFleetOrderType.ORDER_INVADE: #TODO add substantive abort check
                return False
            else:
                return False

        # canceling fleet orders
        print "   %s" % fleet_order
        print "Fleet %d had a target planet that is no longer valid for this mission; aborting." % self.target_id
        self.clear_fleet_orders()
        self.clear_targets(([-1] + self.get_mission_types()[:1])[-1])
        FleetUtilsAI.split_fleet(self.target_id)
        return True

    def _check_retarget_invasion(self):
        """checks if an invasion mission should be retargeted"""
        universe = fo.getUniverse()
        empire = fo.getEmpire()
        empire_id = fo.empireID()
        fleet_id = self.target_id
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
            last_sys_target = orders[-1].target.target_id
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
        self.clear_targets(-1)  # TODO: clear from foAIstate
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
        target = AITarget(EnumsAI.TargetType.TARGET_PLANET, target_id)
        self.add_target(AIFleetMissionType.FLEET_MISSION_INVASION, target)
        self.generate_fleet_orders()

    def issue_fleet_orders(self):
        """issues AIFleetOrders which can be issued in system and moves to next one if is possible"""
        # TODO: priority
        order_completed = True
        print "--------------"
        print "Checking orders for fleet %d (on turn %d), with mission types %s" % (self.target_id, fo.currentTurn(), [AIFleetMissionType.name(mt) for mt in self.get_mission_types()])
        print "\t Full Orders are:"
        for this_order in self.orders:
            print "\t\t| %s" % this_order
        print "/t/t------"
        if AIFleetMissionType.FLEET_MISSION_INVASION in self.get_mission_types():
            self._check_retarget_invasion()
        for fleet_order in self.orders:
            print "\t| checking Order: %s" % fleet_order
            order_type = fleet_order.order_type
            if order_type in [AIFleetOrderType.ORDER_COLONISE,
                              AIFleetOrderType.ORDER_OUTPOST,
                              AIFleetOrderType.ORDER_INVADE]:  # TODO: invasion?
                if self._check_abort_mission(fleet_order):
                    print "\t\t| Aborting fleet order %s" % fleet_order
                    return
            self.check_mergers(context=str(fleet_order))
            if fleet_order.can_issue_order(verbose=True):
                if order_type == AIFleetOrderType.ORDER_MOVE and order_completed:  # only move if all other orders completed
                    print "\t\t| issuing fleet order %s" % fleet_order
                    fleet_order.issue_order()
                elif order_type not in [AIFleetOrderType.ORDER_MOVE, AIFleetOrderType.ORDER_DEFEND]:
                    print "\t\t| issuing fleet order %s" % fleet_order
                    fleet_order.issue_order()
                else:
                    print "\t\t| NOT issuing (even though can_issue) fleet order %s" % fleet_order
                print "\t\t| order status-- execution completed: %s" % fleet_order.execution_completed
                if not fleet_order.execution_completed:
                    order_completed = False
            else:  # check that we're not held up by a Big Monster
                print "\t\t| CAN'T issue fleet order %s" % fleet_order
                if order_type == AIFleetOrderType.ORDER_MOVE:
                    this_system_id = fleet_order.target.target_id
                    this_status = foAI.foAIstate.systemStatus.setdefault(this_system_id, {})
                    if this_status.get('monsterThreat', 0) > fo.currentTurn() * ProductionAI.cur_best_mil_ship_rating()/4.0:
                        first_mission = self.get_mission_types()[0] if self.get_mission_types() else AIFleetMissionType.FLEET_MISSION_INVALID
                        if (first_mission not in (AIFleetMissionType.FLEET_MISSION_ATTACK,
                                                   AIFleetMissionType.FLEET_MISSION_MILITARY,
                                                   AIFleetMissionType.FLEET_MISSION_HIT_AND_RUN,
                                                   AIFleetMissionType.FLEET_MISSION_SECURE,
                                                   ) or
                            fleet_order != self.orders[-1]  # if this move order is not this mil fleet's final destination, and blocked by Big Monster, release and hope for more effective reassignment
                            ):
                            print "Aborting mission due to being blocked by Big Monster at system %d, threat %d"%(this_system_id, foAI.foAIstate.systemStatus[this_system_id]['monsterThreat'])
                            print "Full set of orders were:"
                            for this_order in self.orders:
                                print "\t\t %s" % this_order
                            self.clear_fleet_orders()
                            self.clear_targets(([-1] + self.get_mission_types()[:1])[-1])
                            return
            # moving to another system stops issuing all orders in system where fleet is
            # move order is also the last order in system
            if order_type == AIFleetOrderType.ORDER_MOVE:
                fleet = fo.getUniverse().getFleet(self.target_id)
                if fleet.systemID != fleet_order.target.target_id:
                    break
        else:  # went through entire order list
            if order_completed:
                print "\t| Final order is completed"
                orders = self.orders
                last_order = orders[-1] if orders else None
                universe = fo.getUniverse()

                if last_order and last_order.order_type == AIFleetOrderType.ORDER_COLONISE:
                    planet = universe.getPlanet(last_order.target.target_id)
                    sys_partial_vis_turn = universe.getVisibilityTurnsMap(planet.systemID, fo.empireID()).get(fo.visibility.partial, -9999)
                    planet_partial_vis_turn = universe.getVisibilityTurnsMap(planet.id, fo.empireID()).get(fo.visibility.partial, -9999)
                    if planet_partial_vis_turn == sys_partial_vis_turn and not planet.currentMeterValue(fo.meterType.population):
                        print "Potential Error: Fleet %d has tentatively completed its colonize mission but will wait to confirm population." % self.target_id
                        print "    Order details are %s" % last_order
                        print "    Order is valid: %s ; is Executed : %s; is execution completed: %s " % (last_order.is_valid(), last_order.isExecuted(), last_order.isExecutionCompleted())
                        if not last_order.is_valid():
                            source_target = last_order.fleet
                            target_target = last_order.target
                            print "        source target validity: %s; target target validity: %s " % (source_target.valid, target_target.valid)
                            if EnumsAI.TargetType.TARGET_SHIP == source_target.target_type:
                                ship_id = source_target.target_id
                                ship = universe.getShip(ship_id)
                                if not ship:
                                    print "Ship id %d not a valid ship id" % ship_id
                                print "        source target Ship (%d), species %s, can%s colonize" % (ship_id, ship.speciesName, ["not", ""][ship.canColonize])
                        return  # colonize order must not have completed yet
                clearAll = True
                last_sys_target = -1
                if last_order and last_order.order_type == AIFleetOrderType.ORDER_MILITARY:
                    last_sys_target = last_order.target.target_id
                    # if (AIFleetMissionType.FLEET_MISSION_SECURE in self.get_mission_types()) or # not doing this until decide a way to release from a SECURE mission
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
                        print "Fleet %d has completed initial stage of its mission to secure system %d (targeted for %s), may release a portion of ships" % (self.target_id, last_sys_target, secure_type)
                        clearAll = False
                fleet_id = self.target_id
                if clearAll:
                    if orders:
                        print "Fleet %d has completed its mission; clearing all orders and targets." % self.target_id
                        print "Full set of orders were:"
                        for this_order in orders:
                            print "\t\t %s" % this_order
                        self.clear_fleet_orders()
                        self.clear_targets(([-1] + self.get_mission_types()[:1])[-1])
                        if foAI.foAIstate.get_fleet_role(fleet_id) in (AIFleetMissionType.FLEET_MISSION_MILITARY,
                                                                       AIFleetMissionType.FLEET_MISSION_ATTACK,
                                                                       AIFleetMissionType.FLEET_MISSION_DEFEND,
                                                                       AIFleetMissionType.FLEET_MISSION_HIT_AND_RUN,
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
                        new_fleets = FleetUtilsAI.split_fleet(self.target_id)  # at least first stage of current task is done; release extra ships for potential other deployments
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
        fleet_id = self.target_id
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
        ntargets = 0
        for mission_type in self.get_mission_types():
            ntargets += len(self.get_targets(mission_type))
        if not ntargets and (system_id not in set(AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs +
                                                    AIstate.invasionTargetedSystemIDs + AIstate.blockadeTargetedSystemIDs)):
            if self._need_repair():
                repair_fleet_order = MoveUtilsAI.get_repair_fleet_order(self.target, start_sys_id)
                if repair_fleet_order.is_valid():
                    self.orders.append(repair_fleet_order)
            if self.get_location_target().target_id not in fleet_supplyable_system_ids:
                resupply_fleet_order = MoveUtilsAI.get_resupply_fleet_order(self.target, self.get_location_target())
                if resupply_fleet_order.is_valid():
                    self.orders.append(resupply_fleet_order)
            return  # no targets

        # for some targets fleet has to visit systems and therefore fleet visit them
        system_targets_required_to_visit = []
        for mission_type in self.get_mission_types():
            for aiTarget in self.get_targets(mission_type):
                system_targets_required_to_visit.extend(aiTarget.get_required_system_ai_targets())

        orders_to_visit_systems = MoveUtilsAI.get_fleet_orders_from_system_targets(self.target, system_targets_required_to_visit)
        #TODO: if fleet doesn't have enough fuel to get to final target, consider resetting Mission
        #print "----------------------------------------"
        #print "*+*+ fleet %d : has fleet action system targets: %s"%(fleet_id, [str(obj) for obj in system_targets_required_to_visit])
        #print "----------"
        #print "*+*+ fleet %d: has movement orders: %s"%(fleet_id, [str(obj) for obj in orders_to_visit_systems])

        for fleet_order in orders_to_visit_systems:
            self.orders.append(fleet_order)

        # if fleet is in some system = fleet.system_id >=0, then also generate system AIFleetOrders
        if system_id >= 0:
            # system in where fleet is
            system_target = AITarget(EnumsAI.TargetType.TARGET_SYSTEM, system_id)
            # if mission aiTarget has required system where fleet is, then generate fleet_order from this aiTarget
            # for all targets in all mission types get required systems to visit
            for mission_type in self.get_mission_types():
                tragets = self.get_targets(mission_type)
                for target in tragets:
                    if system_target in target.get_required_system_ai_targets():
                        # from target required to visit get fleet orders to accomplish target
                        fleet_order = self._get_fleet_order_from_target(mission_type, target)
                        self.orders.append(fleet_order)

    def _need_repair(self):
        """
        Check if fleet need repair. Check if fleet HP is less of cutoff.
        TODO make more clever implementation.
        """
        repair_limit = 0.70

        universe = fo.getUniverse()
        fleet_id = self.target_id
        # if combat fleet, use military repair check
        if foAI.foAIstate.get_fleet_role(fleet_id) in COMBAT_MISSION_TYPES:
            return fleet_id in MilitaryAI.avail_mil_needing_repair([fleet_id], False, True)[0]
        fleet = universe.getFleet(fleet_id)
        ships_cur_health = 0
        ships_max_health = 0

        for ship_id in fleet.shipIDs:
            this_ship = universe.getShip(ship_id)
            ships_cur_health += this_ship.currentMeterValue(fo.meterType.structure)
            ships_max_health += this_ship.currentMeterValue(fo.meterType.maxStructure)
        return ships_cur_health >= repair_limit * ships_max_health

    def get_location_target(self):
        """system AITarget where fleet is or will be"""
        # TODO add parameter turn
        fleet = fo.getUniverse().getFleet(self.target_id)
        system_id = fleet.systemID
        if system_id >= 0:
            return AITarget(EnumsAI.TargetType.TARGET_SYSTEM, system_id)
        else:  # in starlane, so return next system
            return AITarget(EnumsAI.TargetType.TARGET_SYSTEM, fleet.nextSystemID)

    def __eq__(self, other):
        return isinstance(other, self.__class__) and self.target == other.target

    def __str__(self):
        mission_strings = []
        for aiFleetMissionType in self.get_mission_types():
            universe = fo.getUniverse()
            fleet_id = self.target_id
            fleet = universe.getFleet(fleet_id)
            targets_string = "fleet %4d (%14s) [ %10s mission ] : %3d ships , total Rating:%7d " % (fleet_id, (fleet and fleet.name) or "Fleet Invalid",
                                                                                                 AIFleetMissionType.name(aiFleetMissionType), (fleet and len(fleet.shipIDs)) or 0, foAI.foAIstate.get_rating(fleet_id).get('overall', 0))
            targets = self.get_targets(aiFleetMissionType)
            for target in targets:
                targets_string += str(target)
            mission_strings.append(targets_string)
        return "\n".join(mission_strings)
