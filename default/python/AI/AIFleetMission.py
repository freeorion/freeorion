import freeOrionAIInterface as fo
from logging import debug, warning

import AIstate
import CombatRatingsAI
import EspionageAI
import FleetUtilsAI
import InvasionAI
import MilitaryAI
import MoveUtilsAI
from AIDependencies import INVALID_ID
from aistate_interface import get_aistate
from common.fo_typing import FleetId
from EnumsAI import MissionType
from fleet_orders import (
    AIFleetOrder,
    OrderColonize,
    OrderDefend,
    OrderInvade,
    OrderMilitary,
    OrderMove,
    OrderOutpost,
    OrderPause,
)
from freeorion_tools import (
    assertion_fails,
    combine_ratings,
    get_fleet_position,
    get_partial_visibility_turn,
)
from target import Target, TargetFleet, TargetPlanet, TargetSystem
from universe.system_network import get_neighbors

ORDERS_FOR_MISSION = {
    MissionType.EXPLORATION: OrderPause,
    MissionType.OUTPOST: OrderOutpost,
    MissionType.COLONISATION: OrderColonize,
    MissionType.INVASION: OrderInvade,
    MissionType.MILITARY: OrderMilitary,
    # SECURE is mostly same as MILITARY, but waits for system removal from all targeted system lists
    # (invasion, colonization, outpost, blockade) before clearing
    MissionType.SECURE: OrderMilitary,
    MissionType.PROTECT_REGION: OrderPause,
    MissionType.ORBITAL_INVASION: OrderInvade,
    MissionType.ORBITAL_OUTPOST: OrderOutpost,
    MissionType.ORBITAL_DEFENSE: OrderDefend,
}

COMBAT_MISSION_TYPES = (
    MissionType.MILITARY,
    MissionType.SECURE,
    MissionType.PROTECT_REGION,
)

MERGEABLE_MISSION_TYPES = (
    MissionType.MILITARY,
    MissionType.INVASION,
    MissionType.PROTECT_REGION,
    MissionType.ORBITAL_INVASION,
    MissionType.SECURE,
    MissionType.ORBITAL_DEFENSE,
)


class AIFleetMission:
    """
    Stores information about AI mission. Every mission has fleetID and AI targets depending upon AI fleet mission type.
    :type orders: list[AIFleetOrder]
    :type target: target.Target | None
    """

    def __init__(self, fleet_id: FleetId):
        self.orders = []
        self.fleet = TargetFleet(fleet_id)
        self.type = None
        self.target = None

    def __setstate__(self, state):
        target_type = state.pop("target_type")
        if state["target"] is not None:
            object_map = {
                TargetPlanet.object_name: TargetPlanet,
                TargetSystem.object_name: TargetSystem,
                TargetFleet.object_name: TargetFleet,
            }
            state["target"] = object_map[target_type](state["target"])

        state["fleet"] = TargetFleet(state["fleet"])
        self.__dict__ = state

    def __getstate__(self):
        retval = dict(self.__dict__)
        # do only store the fleet id not the Fleet object
        retval["fleet"] = self.fleet.id

        # store target type and id rather than the object
        if self.target is not None:
            retval["target_type"] = self.target.object_name
            retval["target"] = self.target.id
        else:
            retval["target_type"] = None
            retval["target"] = None

        return retval

    def set_target(self, mission_type: MissionType, target: Target):
        """
        Set mission and target for this fleet.
        """
        if self.type == mission_type and self.target == target:
            return
        if self.type or self.target:
            debug(f"{self.fleet}: change mission assignment from {self.type}:{self.target} to {mission_type}:{target}")
        self.type = mission_type
        self.target = target

    def clear_target(self):
        """Clear target and mission for this fleet."""
        self.target = None
        self.type = None

    def has_target(self, mission_type: MissionType, target: Target) -> bool:
        """
        Check if fleet has specified mission_type and target.
        """
        return self.type == mission_type and self.target == target

    def clear_fleet_orders(self):
        """Clear this fleets orders but do not clear mission and target."""
        self.orders = []

    def _get_fleet_order_from_target(self, mission_type: MissionType, target: Target) -> AIFleetOrder:
        """
        Get a fleet order according to mission type and target.
        """
        fleet_target = TargetFleet(self.fleet.id)
        return ORDERS_FOR_MISSION[mission_type](fleet_target, target)

    def check_mergers(self, context: str = ""):
        """
        Merge local fleets with same mission into this fleet.

        :param context: Context of the function call for logging purposes
        """
        debug(f"Considering to merge {self}")
        if self.type not in MERGEABLE_MISSION_TYPES:
            debug("Mission type does not allow merging.")
            return

        if not self.target:
            debug("Mission has no valid target - do not merge.")
            return

        universe = fo.getUniverse()
        empire_id = fo.empireID()

        fleet_id = self.fleet.id
        main_fleet = universe.getFleet(fleet_id)
        main_fleet_system_id = main_fleet.systemID
        if main_fleet_system_id == INVALID_ID:
            debug("Can't merge: fleet in middle of starlane.")
            return

        # only merge PROTECT_REGION if there is any threat near target
        if self.type == MissionType.PROTECT_REGION:
            neighbor_systems = get_neighbors(self.target.id)
            if not any(MilitaryAI.get_system_local_threat(sys_id) for sys_id in neighbor_systems):
                debug("Not merging PROTECT_REGION fleet - no threat nearby.")
                return

        destroyed_list = set(universe.destroyedObjectIDs(empire_id))
        aistate = get_aistate()
        system_status = aistate.systemStatus[main_fleet_system_id]
        other_fleets_here = [
            fid
            for fid in system_status.get("myFleetsAccessible", [])
            if fid != fleet_id and fid not in destroyed_list and universe.getFleet(fid).ownedBy(empire_id)
        ]
        if not other_fleets_here:
            debug("No other fleets here")
            return

        for fid in other_fleets_here:
            fleet_mission = aistate.get_fleet_mission(fid)
            if fleet_mission.type != self.type or fleet_mission.target != self.target:
                debug("Local candidate %s does not have same mission." % fleet_mission)
                continue
            FleetUtilsAI.merge_fleet_a_into_b(fid, fleet_id, context=f"Order {context} of mission {self}")

    def _is_valid_fleet_mission_target(self, mission_type: MissionType, target: Target):  # noqa: C901
        if not target:
            return False
        if mission_type == MissionType.EXPLORATION:
            if isinstance(target, TargetSystem):
                empire = fo.getEmpire()
                if not empire.hasExploredSystem(target.id):
                    return True
        elif mission_type in [MissionType.OUTPOST, MissionType.ORBITAL_OUTPOST]:
            fleet = self.fleet.get_object()
            if not fleet.hasOutpostShips:
                return False
            if isinstance(target, TargetPlanet):
                planet = target.get_object()
                if planet.unowned:
                    return True
        elif mission_type == MissionType.COLONISATION:
            fleet = self.fleet.get_object()
            if not fleet.hasColonyShips:
                return False
            if isinstance(target, TargetPlanet):
                planet = target.get_object()
                population = planet.initialMeterValue(fo.meterType.population)
                if planet.unowned or (planet.owner == fleet.owner and population == 0):
                    return True
        elif mission_type in [MissionType.INVASION, MissionType.ORBITAL_INVASION]:
            fleet = self.fleet.get_object()
            if not fleet.hasTroopShips:
                return False
            if isinstance(target, TargetPlanet):
                planet = target.get_object()
                # TODO remove latter portion of next check in light of invasion retargeting, or else correct logic
                if not planet.unowned or planet.owner != fleet.owner:
                    return True
        elif mission_type in [
            MissionType.MILITARY,
            MissionType.SECURE,
            MissionType.ORBITAL_DEFENSE,
            MissionType.PROTECT_REGION,
        ]:
            if isinstance(target, TargetSystem):
                return True
        # TODO: implement other mission types
        return False

    def clean_invalid_targets(self):
        """clean invalid AITargets"""
        if not self._is_valid_fleet_mission_target(self.type, self.target):
            self.target = None
            self.type = None

    def _check_abort_mission(self, fleet_order: AIFleetOrder):  # noqa: C901
        """checks if current mission (targeting a planet) should be aborted"""
        planet_stealthed = False
        target_is_planet = fleet_order.target and isinstance(fleet_order.target, TargetPlanet)
        planet = None
        if target_is_planet:
            planet = fleet_order.target.get_object()
            # Check visibility prediction, but if somehow still have current visibility, don't
            # abort the mission yet
            if not EspionageAI.colony_detectable_by_empire(planet.id, empire=fo.empireID()):
                if get_partial_visibility_turn(planet.id) == fo.currentTurn():
                    debug(
                        "EspionageAI predicts planet id %d to be stealthed" % planet.id
                        + ", but somehow have current visibity anyway, so won't trigger mission abort"
                    )
                else:
                    debug("EspionageAI predicts we can no longer detect %s, will abort mission" % fleet_order.target)
                    planet_stealthed = True
        if target_is_planet and not planet_stealthed:
            # TBD use fleet_order.is_valid or add fleet_order.should_abort
            if isinstance(fleet_order, OrderColonize):
                if planet.initialMeterValue(fo.meterType.population) == 0 and (
                    planet.ownedBy(fo.empireID()) or planet.unowned
                ):
                    return False
            elif isinstance(fleet_order, OrderOutpost):
                if planet.unowned:
                    return False
            elif isinstance(fleet_order, OrderInvade):
                if planet.owner == fo.getEmpire().empireID:
                    debug(f"Abandoning invasion for {planet}, it already belongs to us")
                    return True
                if planet.currentMeterValue(fo.meterType.maxShield):
                    military_support_fleets = MilitaryAI.get_military_fleets_with_target_system(planet.systemID)
                    if not military_support_fleets:
                        # Maybe try again later...
                        debug(
                            f"Abandoning Invasion mission for {planet} because target has nonzero max shields "
                            f"and there is no military fleet assigned to secure the target system."
                        )
                        return True
                return False
            else:
                return False

        # canceling fleet orders
        debug("   %s" % fleet_order)
        debug("Fleet %d had a target planet that is no longer valid for this mission; aborting." % self.fleet.id)
        FleetUtilsAI.split_fleet(self.fleet.id)
        return True

    def _check_retarget_invasion(self):  # noqa: C901
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
        aistate = get_aistate()
        for pid in system.planetIDs:
            if pid in already_targeted or (pid in aistate.qualifyingTroopBaseTargets):
                continue
            planet = universe.getPlanet(pid)
            if planet.unowned or (planet.owner == empire_id):
                continue
            if (planet.initialMeterValue(fo.meterType.shield)) <= 0:
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

        debug("\t Splitting and retargetting fleet %d" % fleet_id)
        new_fleets = FleetUtilsAI.split_fleet(fleet_id)
        self.clear_target()  # TODO: clear from foAIstate
        self.clear_fleet_orders()
        troops_needed = max(0, target_troops - FleetUtilsAI.count_troops_in_fleet(fleet_id))
        min_stats = {"troopCapacity": troops_needed}
        target_stats = {"troopCapacity": troops_needed}
        found_fleets = []
        # TODO check if next statement does not mutate any global states and can be removed

        _ = FleetUtilsAI.get_fleets_for_mission(
            target_stats,
            min_stats,
            {},
            starting_system=fleet.systemID,  # noqa: F841
            fleet_pool_set=set(new_fleets),
            fleet_list=found_fleets,
        )
        for fid in found_fleets:
            FleetUtilsAI.merge_fleet_a_into_b(fid, fleet_id)
        target = TargetPlanet(target_id)
        self.set_target(MissionType.INVASION, target)
        self.generate_fleet_orders()

    def need_to_pause_movement(self, last_move_target_id: int, new_move_order: OrderMove) -> bool:
        """
        When a fleet has consecutive move orders, assesses whether something about the interim destination warrants
        forcing a stop (such as a military fleet choosing to engage with an enemy fleet about to enter the same system,
        or it may provide a good vantage point to view current status of next system in path). Assessments about whether
        the new destination is suitable to move to are (currently) separately made by OrderMove.can_issue_order()
        :param last_move_target_id:
        :param new_move_order:
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
        distance_to_next_system = (
            (fleet.x - current_dest_system.x) ** 2 + (fleet.y - current_dest_system.y) ** 2
        ) ** 0.5
        surplus_travel_distance = fleet.speed - distance_to_next_system
        # if need more than one turn to reach current destination, then don't add another jump yet
        if surplus_travel_distance < 0:
            return True
        # TODO: add assessments for other situations we'd prefer to pause, such as cited above re military fleets, and
        # for situations where high value fleets like colony fleets might deem it safest to stop and look around
        # before proceeding
        return False

    def issue_fleet_orders(self):  # noqa: C901
        """issues AIFleetOrders which can be issued in system and moves to next one if is possible"""
        # TODO: priority
        order_completed = True

        debug(
            "\nChecking orders for fleet %s (on turn %d), with mission type %s and target %s",
            self.fleet.get_object(),
            fo.currentTurn(),
            self.type or "No mission",
            self.target or "No Target",
        )
        if MissionType.INVASION == self.type:
            self._check_retarget_invasion()
        just_issued_move_order = False
        last_move_target_id = INVALID_ID
        for fleet_order in self.orders:
            if isinstance(fleet_order, (OrderColonize, OrderOutpost, OrderInvade)) and self._check_abort_mission(
                fleet_order
            ):
                self.clear_fleet_orders()
                self.clear_target()
                return
        aistate = get_aistate()
        for fleet_order in self.orders:
            if just_issued_move_order and self.fleet.get_object().systemID != last_move_target_id:
                # having just issued a move order, we will normally stop issuing orders this turn, except that if there
                # are consecutive move orders we will consider moving through the first destination rather than stopping
                # Without the below noinspection directive, PyCharm is concerned about the 2nd part of the test
                # noinspection PyTypeChecker
                if not isinstance(fleet_order, OrderMove) or self.need_to_pause_movement(
                    last_move_target_id, fleet_order
                ):
                    break
            debug("Checking order: %s" % fleet_order)
            self.check_mergers(context=str(fleet_order))
            if fleet_order.can_issue_order(verbose=False):
                # only move if all other orders completed
                if isinstance(fleet_order, OrderMove) and order_completed:
                    debug("Issuing fleet order %s" % fleet_order)
                    fleet_order.issue_order()
                    just_issued_move_order = True
                    last_move_target_id = fleet_order.target.id
                elif not isinstance(fleet_order, OrderMove):
                    debug("Issuing fleet order %s" % fleet_order)
                    fleet_order.issue_order()
                else:
                    debug("NOT issuing (even though can_issue) fleet order %s" % fleet_order)
                issued = "issued" if fleet_order.order_issued else "not issued"
                executed = "fully executed" if fleet_order.executed else "not fully executed"
                debug(f"Order {issued} issued and {executed}.")
                if not fleet_order.executed:
                    order_completed = False
            else:  # check that we're not held up by a Big Monster
                if fleet_order.order_issued:
                    # A previously issued order that wasn't instantly executed must have had cirumstances change so that
                    # the order can't currently be reissued (or perhaps simply a savegame has been reloaded on the same
                    # turn the order was issued).
                    if not fleet_order.executed:
                        order_completed = False
                    # Go on to the next order.
                    continue
                debug("CAN'T issue fleet order %s because:" % fleet_order)
                fleet_order.can_issue_order(verbose=True)
                if isinstance(fleet_order, OrderMove):
                    this_system_id = fleet_order.target.id
                    this_status = aistate.systemStatus.setdefault(this_system_id, {})
                    threat_threshold = fo.currentTurn() * MilitaryAI.cur_best_mil_ship_rating() / 4.0
                    if this_status.get("monsterThreat", 0) > threat_threshold:
                        # if this move order is not this mil fleet's final destination, and blocked by Big Monster,
                        # release and hope for more effective reassignment
                        if (
                            self.type not in (MissionType.MILITARY, MissionType.SECURE)
                            or fleet_order != self.orders[-1]
                        ):
                            debug(
                                "Aborting mission due to being blocked by Big Monster at system %d, threat %d"
                                % (this_system_id, aistate.systemStatus[this_system_id]["monsterThreat"])
                            )
                            debug("Full set of orders were:")
                            for this_order in self.orders:
                                debug(" - %s" % this_order)
                            self.clear_fleet_orders()
                            self.clear_target()
                            return
                break  # do not order the next order until this one is finished.
        else:  # went through entire order list
            if order_completed:
                debug("Final order is completed")
                orders = self.orders
                last_order = orders[-1] if orders else None
                universe = fo.getUniverse()

                if last_order and isinstance(last_order, OrderColonize):
                    planet = universe.getPlanet(last_order.target.id)
                    sys_partial_vis_turn = get_partial_visibility_turn(planet.systemID)
                    planet_partial_vis_turn = get_partial_visibility_turn(planet.id)
                    if planet_partial_vis_turn == sys_partial_vis_turn and not planet.initialMeterValue(
                        fo.meterType.population
                    ):
                        warning(
                            "Fleet %s has tentatively completed its "
                            "colonize mission but will wait to confirm population.",
                            self.fleet,
                        )
                        debug("    Order details are %s" % last_order)
                        debug(
                            f"    Order is valid: {last_order.is_valid()}; issued: {last_order.order_issued}; executed: {last_order.executed}"
                        )
                        if not last_order.is_valid():
                            source_target = last_order.fleet
                            target_target = last_order.target
                            debug(
                                f"        source target validity: {bool(source_target)}; target target validity: {bool(target_target)} "
                            )
                        return  # colonize order must not have completed yet
                clear_all = True
                last_sys_target = INVALID_ID
                secure_targets = set(
                    AIstate.colonyTargetedSystemIDs
                    + AIstate.outpostTargetedSystemIDs
                    + AIstate.invasionTargetedSystemIDs
                )
                if last_order and isinstance(last_order, OrderMilitary):
                    last_sys_target = last_order.target.id
                    # not doing this until decide a way to release from a SECURE mission
                    # if (MissionType.SECURE == self.type) or
                    if last_sys_target in secure_targets:  # consider a secure mission
                        if last_sys_target in AIstate.colonyTargetedSystemIDs:
                            secure_type = "Colony"
                        elif last_sys_target in AIstate.outpostTargetedSystemIDs:
                            secure_type = "Outpost"
                        elif last_sys_target in AIstate.invasionTargetedSystemIDs:
                            secure_type = "Invasion"
                        else:
                            secure_type = "Unidentified"
                        debug(
                            "Fleet %d has completed initial stage of its mission "
                            "to secure system %d (targeted for %s), "
                            "may release a portion of ships" % (self.fleet.id, last_sys_target, secure_type)
                        )
                        clear_all = False

                # for PROTECT_REGION missions, only release fleet if no more threat
                if self.type == MissionType.PROTECT_REGION:
                    # use military logic code below to determine if can release
                    # any or even all of the ships.
                    clear_all = False
                    last_sys_target = self.target.id
                    debug("Check if PROTECT_REGION mission with target %d is finished.", last_sys_target)

                # for SECURE missions, only release fleet if no settle or invade mission is ongoing
                if self.type == MissionType.SECURE and self.target.id in secure_targets:
                    debug(f"Secure mission for {self.target} not yet finished")
                    return

                fleet_id = self.fleet.id
                if clear_all:
                    if orders:
                        debug("Fleet %d has completed its mission; clearing all orders and targets." % self.fleet.id)
                        debug("Full set of orders were:")
                        for this_order in orders:
                            debug("\t\t %s" % this_order)
                        self.clear_fleet_orders()
                        self.clear_target()
                        if aistate.get_fleet_role(fleet_id) in (MissionType.MILITARY, MissionType.SECURE):
                            allocations = MilitaryAI.get_military_fleets(
                                mil_fleets_ids=[fleet_id], try_reset=False, thisround="Fleet %d Reassignment" % fleet_id
                            )
                            if allocations:
                                MilitaryAI.assign_military_fleets_to_systems(
                                    use_fleet_id_list=[fleet_id], allocations=allocations
                                )
                    else:  # no orders
                        debug("No Current Orders")
                else:
                    potential_threat = combine_ratings(
                        MilitaryAI.get_system_local_threat(last_sys_target),
                        MilitaryAI.get_system_neighbor_threat(last_sys_target),
                    )
                    threat_present = potential_threat > 0
                    debug("Fleet threat present? %s", threat_present)
                    target_system = universe.getSystem(last_sys_target)
                    if not threat_present and target_system:
                        for pid in target_system.planetIDs:
                            planet = universe.getPlanet(pid)
                            if (
                                planet
                                and planet.owner != fo.empireID()
                                and planet.currentMeterValue(fo.meterType.maxDefense) > 0
                            ):
                                debug("Found local planetary threat: %s", planet)
                                threat_present = True
                                break
                    if not threat_present:
                        debug("No current threat in target system; releasing a portion of ships.")
                        # at least first stage of current task is done;
                        # release extra ships for potential other deployments
                        new_fleets = FleetUtilsAI.split_fleet(self.fleet.id)
                        if self.type == MissionType.PROTECT_REGION:
                            self.clear_fleet_orders()
                            self.clear_target()
                            new_fleets.append(self.fleet.id)
                    else:
                        debug("Threat remains in target system; Considering to release some ships.")
                        new_fleets = []
                        fleet_portion_to_remain = self._portion_of_fleet_needed_here()
                        if fleet_portion_to_remain >= 1:
                            debug("Can not release fleet yet due to large threat.")
                        elif fleet_portion_to_remain > 0:
                            debug("Not all ships are needed here - considering releasing a few")
                            # TODO: Rate against specific enemy threat cause
                            fleet_remaining_rating = CombatRatingsAI.get_fleet_rating(fleet_id)
                            fleet_min_rating = fleet_portion_to_remain * fleet_remaining_rating
                            debug(
                                "Starting rating: %.1f, Target rating: %.1f", fleet_remaining_rating, fleet_min_rating
                            )
                            allowance = CombatRatingsAI.rating_needed(fleet_remaining_rating, fleet_min_rating)
                            debug("May release ships with total rating of %.1f", allowance)
                            for ship_id in self.fleet.get_object().shipIDs:
                                if len(self.fleet.get_object().shipIDs) == 1:
                                    break
                                ship_rating = CombatRatingsAI.get_ship_rating(ship_id)
                                debug("Considering to release ship %d with rating %.1f", ship_id, ship_rating)
                                if ship_rating > allowance:
                                    debug("Remaining rating insufficient. Not released.")
                                    continue
                                debug("Splitting from fleet.")
                                new_fleet_id = FleetUtilsAI.split_ship_from_fleet(fleet_id, ship_id)
                                if assertion_fails(new_fleet_id != INVALID_ID):
                                    break
                                new_fleets.append(new_fleet_id)
                                fleet_remaining_rating = CombatRatingsAI.rating_difference(
                                    fleet_remaining_rating, ship_rating
                                )
                                allowance = CombatRatingsAI.rating_difference(fleet_remaining_rating, fleet_min_rating)
                                debug(
                                    "Remaining fleet rating: %.1f - Allowance: %.1f", fleet_remaining_rating, allowance
                                )
                            if new_fleets:
                                aistate.get_fleet_role(fleet_id, force_new=True)
                                aistate.update_fleet_rating(fleet_id)
                                aistate.ensure_have_fleet_missions(new_fleets)
                        else:
                            debug("Planetary defenses are deemed sufficient. Release fleet.")
                            new_fleets = FleetUtilsAI.split_fleet(self.fleet.id)

                    new_military_fleets = []
                    for fleet_id in new_fleets:
                        if aistate.get_fleet_role(fleet_id) in COMBAT_MISSION_TYPES:
                            new_military_fleets.append(fleet_id)
                    allocations = []
                    if new_military_fleets:
                        allocations = MilitaryAI.get_military_fleets(
                            mil_fleets_ids=new_military_fleets,
                            try_reset=False,
                            thisround="Fleet Reassignment %s" % new_military_fleets,
                        )
                    if allocations:
                        MilitaryAI.assign_military_fleets_to_systems(
                            use_fleet_id_list=new_military_fleets, allocations=allocations
                        )

    def _portion_of_fleet_needed_here(self):
        """Calculate the portion of the fleet needed in target system considering enemy forces."""
        # TODO check rating against planets
        if assertion_fails(self.type in COMBAT_MISSION_TYPES, msg=str(self)):
            return 0
        if assertion_fails(self.target and self.target.id != INVALID_ID, msg=str(self)):
            return 0
        system_id = self.target.id
        aistate = get_aistate()
        local_defenses = MilitaryAI.get_my_defense_rating_in_system(system_id)
        potential_threat = combine_ratings(
            MilitaryAI.get_system_local_threat(system_id), MilitaryAI.get_system_neighbor_threat(system_id)
        )
        universe = fo.getUniverse()
        system = universe.getSystem(system_id)

        # tally planetary defenses
        total_defense = total_shields = 0
        for planet_id in system.planetIDs:
            planet = universe.getPlanet(planet_id)
            total_defense += planet.currentMeterValue(fo.meterType.defense)
            total_shields += planet.currentMeterValue(fo.meterType.shield)
        planetary_ratings = total_defense * (total_shields + total_defense)
        potential_threat += planetary_ratings  # TODO: rewrite to return min rating vs planets as well

        # consider safety factor just once here rather than everywhere below
        safety_factor = aistate.character.military_safety_factor()
        potential_threat *= safety_factor

        # TODO: Rate against specific threat here
        fleet_rating = CombatRatingsAI.get_fleet_rating(self.fleet.id)
        return CombatRatingsAI.rating_needed(potential_threat, local_defenses) / float(max(fleet_rating, 1.0))

    def generate_fleet_orders(self):
        """generates AIFleetOrders from fleets targets to accomplish"""
        universe = fo.getUniverse()
        fleet_id = self.fleet.id
        fleet = universe.getFleet(fleet_id)
        if (not fleet) or fleet.empty or (fleet_id in universe.destroyedObjectIDs(fo.empireID())):
            # fleet was probably merged into another or was destroyed
            get_aistate().delete_fleet_info(fleet_id)
            return

        # TODO: priority
        self.clear_fleet_orders()
        system_id = fleet.systemID
        # if fleet doesn't have any mission,
        # then repair if needed or resupply if is current location not in supplyable system
        empire = fo.getEmpire()
        fleet_supplyable_system_ids = empire.fleetSupplyableSystemIDs
        # if (not self.hasAnyAIMissionTypes()):
        if not self.target and (
            system_id
            not in set(
                AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs + AIstate.invasionTargetedSystemIDs
            )
        ):
            if self._need_repair():
                repair_fleet_order = MoveUtilsAI.get_repair_fleet_order(self.fleet)
                if repair_fleet_order and repair_fleet_order.is_valid():
                    self.orders.append(repair_fleet_order)
            cur_fighter_capacity, max_fighter_capacity = FleetUtilsAI.get_fighter_capacity_of_fleet(fleet_id)
            if (
                fleet.fuel < fleet.maxFuel
                or cur_fighter_capacity < max_fighter_capacity
                and get_fleet_position(self.fleet.id) not in fleet_supplyable_system_ids
            ):
                resupply_fleet_order = MoveUtilsAI.get_resupply_fleet_order(self.fleet)
                if resupply_fleet_order.is_valid():
                    self.orders.append(resupply_fleet_order)
            return  # no targets

        if self.target:
            # for some targets fleet has to visit systems and therefore fleet visit them

            system_to_visit = (
                self.target.get_system()
                if not self.type == MissionType.PROTECT_REGION
                else TargetSystem(self._get_target_for_protection_mission())
            )
            if not system_to_visit:
                return
            orders_to_visit_systems = MoveUtilsAI.create_move_orders_to_system(self.fleet, system_to_visit)
            # TODO: if fleet doesn't have enough fuel to get to final target, consider resetting Mission
            for fleet_order in orders_to_visit_systems:
                self.orders.append(fleet_order)

            # also generate appropriate final orders
            fleet_order = self._get_fleet_order_from_target(
                self.type, self.target if not self.type == MissionType.PROTECT_REGION else system_to_visit
            )
            self.orders.append(fleet_order)

    def _need_repair(self, repair_limit: float = 0.70) -> bool:
        """Check if fleet needs to be repaired.

        If the fleet is already at a system where it can be repaired, stay there until fully repaired.
        Otherwise, repair if fleet health is below specified *repair_limit*.
        For military fleets, there is a special evaluation called, cf. *MilitaryAI.avail_mil_needing_repair()*

        :param repair_limit: percentage of health below which the fleet is sent to repair
        :return: True if fleet needs repair
        """
        # TODO: More complex evaluation if fleet needs repair (consider self-repair, distance, threat, mission...)
        fleet_id = self.fleet.id
        # if we are already at a system where we can repair, make sure we use it...
        system = self.fleet.get_system()
        # TODO starlane obstruction is not considered in the next call
        nearest_dock = MoveUtilsAI.get_best_drydock_system_id(system.id, fleet_id)
        if nearest_dock == system.id:
            repair_limit = 0.99
        # if combat fleet, use military repair check
        if get_aistate().get_fleet_role(fleet_id) in COMBAT_MISSION_TYPES:
            return (
                fleet_id
                in MilitaryAI.avail_mil_needing_repair(
                    [fleet_id], on_mission=bool(self.orders), repair_limit=repair_limit
                )[0]
            )
        # TODO: Allow to split fleet to send only damaged ships to repair
        ships_cur_health, ships_max_health = FleetUtilsAI.get_current_and_max_structure(fleet_id)
        return ships_cur_health < repair_limit * ships_max_health

    def __eq__(self, other):
        return isinstance(other, self.__class__) and self.fleet == other.target

    def __hash__(self):
        return hash(self.fleet)

    def __str__(self):
        fleet = self.fleet.get_object()

        fleet_id = self.fleet.id
        return "%-25s [%-11s] ships: %2d; total rating: %4d; target: %s" % (
            fleet,
            "NONE" if self.type is None else self.type,
            (fleet and len(fleet.shipIDs)) or 0,
            CombatRatingsAI.get_fleet_rating(fleet_id),
            self.target or "no target",
        )

    def _get_target_for_protection_mission(self):
        """Get a target for a PROTECT_REGION mission.

        1) If primary target (system target of this mission) is under attack, move to primary target.
        2) If neighbors of primary target have local enemy forces weaker than this fleet, may move to attack
        3) If no neighboring fleets or strongest enemy force is too strong, move to defend primary target
        """
        # TODO: Also check fleet rating vs planets in decision making below not only vs fleets
        universe = fo.getUniverse()
        primary_objective = self.target.id
        # TODO: Rate against specific threats
        fleet_rating = CombatRatingsAI.get_fleet_rating(self.fleet.id)
        debug(
            "%s finding target for protection mission (primary target %s). Fleet Rating: %.1f",
            self.fleet,
            self.target,
            fleet_rating,
        )
        immediate_threat = MilitaryAI.get_system_local_threat(primary_objective)
        if immediate_threat:
            debug("  Immediate threat! Moving to primary mission target")
            return primary_objective
        else:
            debug("  No immediate threats.")
            # Try to eliminate neighbouring fleets
            neighbors = get_neighbors(primary_objective)
            threat_list = sorted(map(lambda x: (MilitaryAI.get_system_local_threat(x), x), neighbors), reverse=True)

            if not threat_list:
                debug("  No neighbors (?!). Moving to primary mission target")
                return primary_objective
            else:
                debug("  Neighboring threats:")
                for threat, sys_id in threat_list:
                    debug("    %s - %.1f", TargetSystem(sys_id), threat)
            top_threat, candidate_system = threat_list[0]
            if not top_threat:
                # TODO: Move into second ring but needs more careful evaluation
                # For now, consider staying at the current location if enemy
                # owns a planet here which we can bombard.
                current_system_id = self.fleet.get_current_system_id()
                if current_system_id in neighbors:
                    system = universe.getSystem(current_system_id)
                    if assertion_fails(system is not None):
                        return primary_objective
                    empire_id = fo.empireID()
                    for planet_id in system.planetIDs:
                        planet = universe.getPlanet(planet_id)
                        if planet and not planet.ownedBy(empire_id) and not planet.unowned:
                            debug("Currently no neighboring threats. " "Staying for bombardment of planet %s", planet)
                            return current_system_id

                # TODO consider attacking neighboring, non-military fleets
                # - needs more careful evaluation against neighboring threats
                # empire_id = fo.empireID()
                # for sys_id in neighbors:
                #     system = universe.getSystem(sys_id)
                #     if assertion_fails(system is not None):
                #         continue
                #     local_fleets = system.fleetIDs
                #     for fleet_id in local_fleets:
                #         fleet = universe.getFleet(fleet_id)
                #         if not fleet or fleet.ownedBy(empire_id):
                #             continue
                #         return sys_id

                debug("No neighboring threats. Moving to primary mission target")
                return primary_objective

            # TODO rate against threat in target system
            # TODO only engage if can reach in 1 turn or leaves sufficient defense behind
            safety_factor = get_aistate().character.military_safety_factor()
            if fleet_rating < safety_factor * top_threat:
                debug("  Neighboring threat is too powerful. Moving to primary mission target")
                return primary_objective  # do not engage!

            debug("  Engaging neighboring threat: %s", TargetSystem(candidate_system))
            return candidate_system
