import freeOrionAIInterface as fo
import math
from collections.abc import Sequence
from logging import debug, error, warning
from typing import Union

import AIDependencies
import MoveUtilsAI
from AIDependencies import INVALID_ID
from aistate_interface import get_aistate
from CombatRatingsAI import get_fleet_rating, get_ship_combat_stats, rating_needed
from common.fo_typing import PlanetId
from common.print_utils import Sequence as SequenceField
from common.print_utils import Table, Text
from EnumsAI import MissionType, ShipRoleType
from freeorion_tools import assertion_fails, combine_ratings
from ShipDesignAI import get_ship_part
from target import TargetFleet, TargetPlanet, TargetSystem
from universe.system_network import get_neighbors, get_shortest_distance


def stats_meet_reqs(stats: dict, requirements: dict) -> bool:
    """
    Check if (fleet) stats meet requirements.

    :param stats: Stats (of fleet)
    :param requirements: Requirements
    :return: True if requirements are met.
    """
    for key in requirements:
        if key not in stats:  # skip requirements not related to stats
            if key != "target_system":  # expected not to be in stats
                warning("Requirement %s not in stats", key)
            continue
        if stats.get(key, 0) < requirements[key]:
            return False
    return True


def count_troops_in_fleet(fleet_id: int) -> float:
    """
    Get the number of troops in the fleet.

    :param fleet_id: fleet to be queried
    :return: total troopCapacity of the fleet
    """
    universe = fo.getUniverse()
    fleet = universe.getFleet(fleet_id)
    if not fleet:
        return 0.0
    fleet_troop_capacity = 0.0
    for ship_id in fleet.shipIDs:
        ship = universe.getShip(ship_id)
        if ship:
            fleet_troop_capacity += ship.troopCapacity
    return fleet_troop_capacity


def get_targeted_planet_ids(planet_ids: Sequence[PlanetId], mission_type: MissionType) -> list[PlanetId]:
    """Find the planets that are targets of the specified mission type.

    :param planet_ids: planets to be queried
    :param mission_type:
    :return: Subset of *planet_ids* targeted by *mission_type*
    """
    selected_fleet_missions = get_aistate().get_fleet_missions_with_any_mission_types([mission_type])
    targeted_planets = []
    for planet_id in planet_ids:
        # add planets that are target of a mission
        for fleet_mission in selected_fleet_missions:
            ai_target = TargetPlanet(planet_id)
            if fleet_mission.has_target(mission_type, ai_target):
                targeted_planets.append(planet_id)
    return targeted_planets


# TODO: Avoid mutable arguments and use return values instead
# TODO: Use Dijkstra's algorithm instead of BFS to consider starlane length
def get_fleets_for_mission(  # noqa: C901
    target_stats: dict,
    min_stats: dict,
    cur_stats: dict,
    starting_system: int,
    fleet_pool_set: set[int],
    fleet_list: list[int],
    species: str = "",
    ensure_return: bool = False,
) -> list[int]:
    """Get fleets for a mission.

    Implements breadth-first search through systems starting at the **starting_sytem**.
    In each system, local fleets are checked if they are in the allowed **fleet_pool_set** and suitable for the mission.
    If so, they are added to the **fleet_list** and **cur_stats** is updated with the currently selected fleet summary.
    The search continues until the requirements defined in **target_stats** are met or there are no more systems/fleets.
    In that case, if the **min_stats** are covered, the **fleet_list** is returned anyway.
    Otherwise, an empty list is returned by the function, in which case the caller can make an evaluation of
    an emergency use of the found fleets in fleet_list; if not to be used they should be added back to the main pool.

    :param target_stats: stats the fleet should ideally meet
    :param min_stats: minimum stats the final fleet must meet to be accepted
    :param cur_stats: (**mutated**) stat summary of selected fleets
    :param starting_system: system_id where breadth-first-search is centered
    :param fleet_pool_set: (**mutated**) fleets allowed to be selected. Split fleed_ids are added, used ones removed.
    :param fleet_list: (**mutated**) fleets that are selected for the mission. Gets filled during the call.
    :param species: species for colonization mission
    :param bool ensure_return: If true, fleet must have sufficient fuel to return into supply after mission
    :return: List of selected fleet_ids or empty list if couldn't meet minimum requirements.
    """
    universe = fo.getUniverse()
    colonization_roles = (ShipRoleType.CIVILIAN_COLONISATION, ShipRoleType.BASE_COLONISATION)
    systems_enqueued = [starting_system]
    systems_visited = []
    # loop over systems in a breadth-first-search trying to find nearby suitable ships in fleet_pool_set
    aistate = get_aistate()
    while systems_enqueued and fleet_pool_set:
        this_system_id = systems_enqueued.pop(0)
        this_system_obj = TargetSystem(this_system_id)
        systems_visited.append(this_system_id)
        accessible_fleets = aistate.systemStatus.get(this_system_id, {}).get("myFleetsAccessible", [])
        fleets_here = [fid for fid in accessible_fleets if fid in fleet_pool_set]
        # loop over all fleets in the system, split them if possible and select suitable ships
        while fleets_here:
            fleet_id = fleets_here.pop(0)
            fleet = universe.getFleet(fleet_id)
            if not fleet:  # TODO should be checked before passed to the function
                fleet_pool_set.remove(fleet_id)
                continue
            # try splitting fleet
            if fleet.numShips > 1:
                debug("Splitting candidate fleet to get ships for mission.")
                new_fleets = split_fleet(fleet_id)
                fleet_pool_set.update(new_fleets)
                fleets_here.extend(new_fleets)

            if "target_system" in target_stats and not MoveUtilsAI.can_travel_to_system(
                fleet_id, this_system_obj, target_stats["target_system"], ensure_return=ensure_return
            ):
                continue

            # check species for colonization missions
            if species:
                for ship_id in fleet.shipIDs:
                    ship = universe.getShip(ship_id)
                    if (
                        ship
                        and aistate.get_ship_role(ship.design.id) in colonization_roles
                        and species == ship.speciesName
                    ):
                        break
                else:  # no suitable species found
                    continue
            # check troop capacity for invasion missions
            troop_capacity = 0
            if "troopCapacity" in target_stats:
                troop_capacity = count_troops_in_fleet(fleet_id)
                if troop_capacity <= 0:
                    continue

            # check if we need additional rating vs planets
            this_rating_vs_planets = 0
            if "ratingVsPlanets" in target_stats:
                this_rating_vs_planets = aistate.get_rating(fleet_id, against_planets=True)
                if this_rating_vs_planets <= 0 and cur_stats.get("rating", 0) >= target_stats.get("rating", 0):
                    # we already have enough general rating, so do not add any more warships useless against planets
                    continue

            # all checks passed, add ship to selected fleets and update the stats
            try:
                fleet_pool_set.remove(fleet_id)
            except KeyError:
                error("After having split a fleet, the original fleet apparently no longer exists.", exc_info=True)
                continue
            fleet_list.append(fleet_id)

            this_rating = aistate.get_rating(fleet_id)
            cur_stats["rating"] = combine_ratings(cur_stats.get("rating", 0), this_rating)
            if "ratingVsPlanets" in target_stats:
                cur_stats["ratingVsPlanets"] = combine_ratings(
                    cur_stats.get("ratingVsPlanets", 0), this_rating_vs_planets
                )
            if "troopCapacity" in target_stats:
                cur_stats["troopCapacity"] = cur_stats.get("troopCapacity", 0) + troop_capacity
            # if we already meet the requirements, we can stop looking for more ships
            if (sum(len(universe.getFleet(fid).shipIDs) for fid in fleet_list) >= 1) and stats_meet_reqs(
                cur_stats, target_stats
            ):
                return fleet_list

        # finished system without meeting requirements. Add neighboring systems to search queue.
        for neighbor_id in get_neighbors(this_system_id):
            if all(
                (
                    neighbor_id not in systems_visited,
                    neighbor_id not in systems_enqueued,
                    neighbor_id in aistate.exploredSystemIDs,
                )
            ):
                systems_enqueued.append(neighbor_id)
    # we ran out of systems or fleets to check but did not meet requirements yet.
    if stats_meet_reqs(cur_stats, min_stats) and any(universe.getFleet(fid).shipIDs for fid in fleet_list):
        return fleet_list
    else:
        return []


def split_fleet(fleet_id: int) -> list[int]:
    """Split a fleet into its ships.

    :param fleet_id: fleet to be split.
    :return: New fleets. Empty if couldn't split.
    """
    universe = fo.getUniverse()
    empire_id = fo.empireID()
    fleet = universe.getFleet(fleet_id)
    new_fleets = []

    if fleet is None:
        return []
    if not fleet.ownedBy(empire_id):
        return []

    if len(list(fleet.shipIDs)) <= 1:  # fleet with only one ship cannot be split
        return []
    ship_ids = list(fleet.shipIDs)
    aistate = get_aistate()
    for ship_id in ship_ids[1:]:
        new_fleet_id = split_ship_from_fleet(fleet_id, ship_id)
        new_fleets.append(new_fleet_id)

    aistate.get_fleet_role(fleet_id, force_new=True)
    aistate.update_fleet_rating(fleet_id)
    if new_fleets:
        aistate.ensure_have_fleet_missions(new_fleets)

    return new_fleets


def split_ship_from_fleet(fleet_id, ship_id) -> int:
    """Try to split a ship from the fleet, creating a new fleet.

    :return: ID of the newly created fleet or INVALID_ID if failed
    """
    universe = fo.getUniverse()
    fleet = universe.getFleet(fleet_id)
    if assertion_fails(fleet is not None):
        return INVALID_ID

    if assertion_fails(ship_id in fleet.shipIDs):
        return INVALID_ID

    if assertion_fails(fleet.numShips > 1, "Can't split last ship from fleet"):
        return INVALID_ID

    new_fleet_id = fo.issueNewFleetOrder("Fleet %4d" % ship_id, ship_id)
    if new_fleet_id != INVALID_ID:
        aistate = get_aistate()
        new_fleet = universe.getFleet(new_fleet_id)
        if not new_fleet:
            warning("Newly split fleet %d not available from universe" % new_fleet_id)
        debug("Successfully split ship %d from fleet %d into new fleet %d", ship_id, fleet_id, new_fleet_id)
        fo.issueRenameOrder(new_fleet_id, "Fleet %4d" % new_fleet_id)  # to ease review of debugging logs
        fo.issueAggressionOrder(new_fleet_id, True)
        aistate.update_fleet_rating(new_fleet_id)
        aistate.newlySplitFleets[new_fleet_id] = True
        # register the new fleets so AI logic is aware of them
        sys_status = aistate.systemStatus.setdefault(fleet.systemID, {})
        sys_status["myfleets"].append(new_fleet_id)
        sys_status["myFleetsAccessible"].append(new_fleet_id)
    else:
        if fleet.systemID == INVALID_ID:
            warning("Tried to split ship id (%d) from fleet %d when fleet is in starlane" % (ship_id, fleet_id))
        else:
            warning("Got no fleet ID back after trying to split ship id (%d) from fleet %d" % (ship_id, fleet_id))
    return new_fleet_id


def merge_fleet_a_into_b(fleet_a_id, fleet_b_id, leave_rating=0, need_rating=0, context=""):
    debug("Merging fleet %s into %s", TargetFleet(fleet_a_id), TargetFleet(fleet_b_id))
    universe = fo.getUniverse()
    fleet_a = universe.getFleet(fleet_a_id)
    fleet_b = universe.getFleet(fleet_b_id)
    if not fleet_a or not fleet_b:
        return 0
    system_id = fleet_a.systemID
    if fleet_b.systemID != system_id:
        return 0

    # TODO: Should this rate against specific enemy?
    remaining_rating = get_fleet_rating(fleet_a_id)
    transferred_rating = 0
    for ship_id in fleet_a.shipIDs:
        this_ship = universe.getShip(ship_id)
        if not this_ship:
            continue
        this_rating = get_ship_combat_stats(ship_id).get_rating()
        remaining_rating = rating_needed(remaining_rating, this_rating)
        if remaining_rating < leave_rating:  # merging this would leave old fleet under minimum rating, try other ships.
            continue
        transferred = fo.issueFleetTransferOrder(ship_id, fleet_b_id)
        if transferred:
            transferred_rating = combine_ratings(transferred_rating, this_rating)
        else:
            debug(
                "  *** transfer of ship %4d, formerly of fleet %4d, into fleet %4d failed; %s"
                % (ship_id, fleet_a_id, fleet_b_id, (" context is %s" % context) if context else "")
            )
        if need_rating != 0 and need_rating <= transferred_rating:
            break
    fleet_a = universe.getFleet(fleet_a_id)
    aistate = get_aistate()
    if not fleet_a or fleet_a.empty or fleet_a_id in universe.destroyedObjectIDs(fo.empireID()):
        aistate.delete_fleet_info(fleet_a_id)
    aistate.update_fleet_rating(fleet_b_id)


def fleet_has_ship_with_role(fleet_id, ship_role):
    """Returns True if a ship with shipRole is in the fleet."""
    universe = fo.getUniverse()
    fleet = universe.getFleet(fleet_id)

    if fleet is None:
        return False
    aistate = get_aistate()
    for ship_id in fleet.shipIDs:
        ship = universe.getShip(ship_id)
        if aistate.get_ship_role(ship.design.id) == ship_role:
            return True
    return False


def get_ship_id_with_role(fleet_id, ship_role, verbose=True):
    """Returns a ship with the specified role in the fleet."""

    if not fleet_has_ship_with_role(fleet_id, ship_role):
        if verbose:
            debug("No ship with role %s found." % ship_role)
        return None

    universe = fo.getUniverse()
    fleet = universe.getFleet(fleet_id)
    aistate = get_aistate()

    for ship_id in fleet.shipIDs:
        ship = universe.getShip(ship_id)
        if aistate.get_ship_role(ship.design.id) == ship_role:
            return ship_id


def get_empire_fleet_ids():
    """Returns all fleetIDs for current empire."""
    empire_id = fo.empireID()
    universe = fo.getUniverse()
    empire_fleet_ids = []
    destroyed_object_ids = universe.destroyedObjectIDs(empire_id)
    for fleet_id in set(list(universe.fleetIDs) + list(get_aistate().newlySplitFleets)):
        fleet = universe.getFleet(fleet_id)
        if fleet is None:
            continue
        if fleet.ownedBy(empire_id) and fleet_id not in destroyed_object_ids and not fleet.empty and fleet.shipIDs:
            empire_fleet_ids.append(fleet_id)
    return empire_fleet_ids


def get_empire_fleet_ids_by_role(fleet_role):
    """Returns a list with fleet_ids that have the specified role."""
    fleet_ids = get_empire_fleet_ids()
    fleet_ids_with_role = []
    aistate = get_aistate()
    for fleet_id in fleet_ids:
        if aistate.get_fleet_role(fleet_id) != fleet_role:
            continue
        fleet_ids_with_role.append(fleet_id)
    return fleet_ids_with_role


def extract_fleet_ids_without_mission_types(fleets_ids):
    """Extracts a list with fleetIDs that have no mission."""
    aistate = get_aistate()
    return [fleet_id for fleet_id in fleets_ids if not aistate.get_fleet_mission(fleet_id).type]


def assess_fleet_role(fleet_id):  # noqa: C901
    """
    Assesses ShipRoles represented in a fleet and
    returns a corresponding overall fleetRole (of type MissionType).
    """
    universe = fo.getUniverse()
    ship_roles = {}
    fleet = universe.getFleet(fleet_id)
    if not fleet:
        debug("couldn't get fleet with id " + str(fleet_id))
        return ShipRoleType.INVALID

    # count ship_roles
    aistate = get_aistate()
    for ship_id in fleet.shipIDs:
        ship = universe.getShip(ship_id)
        if ship.design:
            role = aistate.get_ship_role(ship.design.id)
        else:
            role = ShipRoleType.INVALID

        if role != ShipRoleType.INVALID:
            ship_roles[role] = ship_roles.get(role, 0) + 1
    # determine most common ship_role
    favourite_role = ShipRoleType.INVALID
    for ship_role in ship_roles:
        if ship_roles[ship_role] == max(ship_roles.values()):
            favourite_role = ship_role

    # assign fleet role
    if ShipRoleType.CIVILIAN_COLONISATION in ship_roles:
        selected_role = MissionType.COLONISATION
    elif ShipRoleType.BASE_COLONISATION in ship_roles:
        selected_role = MissionType.COLONISATION
    elif ShipRoleType.CIVILIAN_OUTPOST in ship_roles:
        selected_role = MissionType.OUTPOST
    elif ShipRoleType.BASE_OUTPOST in ship_roles:
        selected_role = MissionType.ORBITAL_OUTPOST
    elif ShipRoleType.BASE_INVASION in ship_roles:
        selected_role = MissionType.ORBITAL_INVASION
    elif ShipRoleType.BASE_DEFENSE in ship_roles:
        selected_role = MissionType.ORBITAL_DEFENSE
    elif ShipRoleType.MILITARY_INVASION in ship_roles:
        selected_role = MissionType.INVASION
    ####
    elif favourite_role == ShipRoleType.CIVILIAN_EXPLORATION:
        selected_role = MissionType.EXPLORATION
    elif favourite_role == ShipRoleType.MILITARY_ATTACK:
        selected_role = MissionType.MILITARY
    elif favourite_role == ShipRoleType.MILITARY:
        selected_role = MissionType.MILITARY
    else:
        selected_role = ShipRoleType.INVALID
    return selected_role


def assess_ship_design_role(design):  # noqa: C901
    parts = [get_ship_part(partname) for partname in design.parts if partname and get_ship_part(partname)]

    if any(p.partClass == fo.shipPartClass.colony and p.capacity == 0 for p in parts):
        if design.speed > 0:
            return ShipRoleType.CIVILIAN_OUTPOST
        else:
            return ShipRoleType.BASE_OUTPOST

    if any(p.partClass == fo.shipPartClass.colony and p.capacity > 0 for p in parts):
        if design.speed > 0:
            return ShipRoleType.CIVILIAN_COLONISATION
        else:
            return ShipRoleType.BASE_COLONISATION

    if any(p.partClass == fo.shipPartClass.troops for p in parts):
        if design.speed > 0:
            return ShipRoleType.MILITARY_INVASION
        else:
            return ShipRoleType.BASE_INVASION

    if design.speed == 0:
        if not parts or parts[0].partClass in (
            fo.shipPartClass.shortRange,
            fo.shipPartClass.fighterBay,
            fo.shipPartClass.fighterHangar,
            fo.shipPartClass.shields,
            fo.shipPartClass.armour,
        ):
            return ShipRoleType.BASE_DEFENSE
        else:
            return ShipRoleType.INVALID

    if design.isArmed or design.hasFighters:
        return ShipRoleType.MILITARY
    if any(p.partClass == fo.shipPartClass.detection for p in parts):
        return ShipRoleType.CIVILIAN_EXPLORATION
    else:  # if no suitable role found, use as (bad) scout as it still has inherent detection
        warning("Defaulting ship role to 'exploration' for ship with parts: %s", design.parts)
        return ShipRoleType.CIVILIAN_EXPLORATION


def generate_fleet_orders_for_fleet_missions():  # noqa: C901
    """Generates fleet orders from targets."""
    debug("Generating fleet orders")

    table = Table(Text("name"), SequenceField("values"), hide_header=True, table_name="Fleets by Role")

    # The following fleet lists are based on *Roles* -- Secure type missions are done by fleets with Military Roles
    table.add_row("Exploration", get_empire_fleet_ids_by_role(MissionType.EXPLORATION))
    table.add_row("Colonization", get_empire_fleet_ids_by_role(MissionType.COLONISATION))
    table.add_row("Outpost", get_empire_fleet_ids_by_role(MissionType.OUTPOST))
    table.add_row("Invasion", get_empire_fleet_ids_by_role(MissionType.INVASION))
    table.add_row("Military", get_empire_fleet_ids_by_role(MissionType.MILITARY))
    table.add_row("Orbital Defense", get_empire_fleet_ids_by_role(MissionType.ORBITAL_DEFENSE))
    table.add_row("Outpost Base", get_empire_fleet_ids_by_role(MissionType.ORBITAL_OUTPOST))
    table.add_row("Invasion Base", get_empire_fleet_ids_by_role(MissionType.ORBITAL_INVASION))
    table.add_row(
        "Securing",
        get_empire_fleet_ids_by_role(MissionType.SECURE),
        note="currently MissionType.MILITARY should be used instead of this role",
    )

    debug(table)

    aistate = get_aistate()
    if fo.currentTurn() < 50:
        debug("")
        debug("Explored systems:")
        _print_systems_and_supply(aistate.get_explored_system_ids())
        debug("Unexplored systems:")
        _print_systems_and_supply(aistate.get_unexplored_system_ids())
        debug("")

    exploration_fleet_missions = aistate.get_fleet_missions_with_any_mission_types([MissionType.EXPLORATION])
    if exploration_fleet_missions:
        debug("Exploration targets:")
        for explorationAIFleetMission in exploration_fleet_missions:
            debug(" - %s" % explorationAIFleetMission)
    else:
        debug("Exploration targets: None")

    colonisation_fleet_missions = aistate.get_fleet_missions_with_any_mission_types([MissionType.COLONISATION])
    if colonisation_fleet_missions:
        debug("Colonization targets: ")
    else:
        debug("Colonization targets: None")
    for colonisation_fleet_mission in colonisation_fleet_missions:
        debug("    %s" % colonisation_fleet_mission)

    outpost_fleet_missions = aistate.get_fleet_missions_with_any_mission_types([MissionType.OUTPOST])
    if outpost_fleet_missions:
        debug("Outpost targets: ")
    else:
        debug("Outpost targets: None")
    for outpost_fleet_mission in outpost_fleet_missions:
        debug("    %s" % outpost_fleet_mission)

    outpost_base_fleet_missions = aistate.get_fleet_missions_with_any_mission_types([MissionType.ORBITAL_OUTPOST])
    if outpost_base_fleet_missions:
        debug("Outpost Base targets (must have been interrupted by combat): ")
    else:
        debug("Outpost Base targets: None (as expected, due to expected timing of order submission and execution)")
    for outpost_fleet_mission in outpost_base_fleet_missions:
        debug("    %s" % outpost_fleet_mission)

    invasion_fleet_missions = aistate.get_fleet_missions_with_any_mission_types([MissionType.INVASION])
    if invasion_fleet_missions:
        debug("Invasion targets: ")
    else:
        debug("Invasion targets: None")
    for invasion_fleet_mission in invasion_fleet_missions:
        debug("    %s" % invasion_fleet_mission)

    troop_base_fleet_missions = aistate.get_fleet_missions_with_any_mission_types([MissionType.ORBITAL_INVASION])
    if troop_base_fleet_missions:
        debug("Invasion Base targets (must have been interrupted by combat): ")
    else:
        debug("Invasion Base targets: None (as expected, due to expected timing of order submission and execution)")
    for invasion_fleet_mission in troop_base_fleet_missions:
        debug("    %s" % invasion_fleet_mission)

    military_fleet_missions = aistate.get_fleet_missions_with_any_mission_types([MissionType.MILITARY])
    if military_fleet_missions:
        debug("General Military targets: ")
    else:
        debug("General Military targets: None")
    for military_fleet_mission in military_fleet_missions:
        debug("    %s" % military_fleet_mission)

    secure_fleet_missions = aistate.get_fleet_missions_with_any_mission_types([MissionType.SECURE])
    if secure_fleet_missions:
        debug("Secure targets: ")
    else:
        debug("Secure targets: None")
    for secure_fleet_mission in secure_fleet_missions:
        debug("    %s" % secure_fleet_mission)

    orb_defense_fleet_missions = aistate.get_fleet_missions_with_any_mission_types([MissionType.ORBITAL_DEFENSE])
    if orb_defense_fleet_missions:
        debug("Orbital Defense targets: ")
    else:
        debug("Orbital Defense targets: None")
    for orb_defence_fleet_mission in orb_defense_fleet_missions:
        debug("    %s" % orb_defence_fleet_mission)

    fleet_missions = list(aistate.get_all_fleet_missions())
    destroyed_objects = fo.getUniverse().destroyedObjectIDs(fo.empireID())

    # merge fleets where appropriate before generating fleet orders.
    # This allows us to consider the full fleet strength when determining
    # e.g. whether to engage or avoid an enemy.
    for mission in fleet_missions:
        fleet_id = mission.fleet.id
        fleet = mission.fleet.get_object()
        if not fleet or not fleet.shipIDs or fleet_id in destroyed_objects:
            continue
        mission.check_mergers()
    # get new set of fleet missions without fleets that are empty after merge
    fleet_missions = aistate.get_all_fleet_missions()
    for mission in fleet_missions:
        mission.generate_fleet_orders()


def issue_fleet_orders_for_fleet_missions():
    """Issues fleet orders."""
    debug("")
    universe = fo.getUniverse()
    aistate = get_aistate()
    fleet_missions = list(aistate.get_all_fleet_missions())
    thisround = 0
    while thisround < 3:
        thisround += 1
        debug("Issuing fleet orders round %d:" % thisround)
        for mission in fleet_missions:
            fleet_id = mission.fleet.id
            fleet = mission.fleet.get_object()
            # check that fleet was merged into another previously during this turn
            if not fleet or not fleet.shipIDs or fleet_id in universe.destroyedObjectIDs(fo.empireID()):
                continue
            mission.issue_fleet_orders()
        fleet_missions = aistate.misc.get("ReassignedFleetMissions", [])
        aistate.misc["ReassignedFleetMissions"] = []
    debug("")


def _print_systems_and_supply(system_ids):
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    fleet_supplyable_system_ids = empire.fleetSupplyableSystemIDs
    for system_id in system_ids:
        system = universe.getSystem(system_id)
        debug(
            "  {}{}".format(
                system if system else "  S_%s<>" % system_id,
                "supplied" if system_id in fleet_supplyable_system_ids else "",
            )
        )


def get_fighter_capacity_of_fleet(fleet_id: int) -> tuple[int, int]:
    """
    Return current and max fighter capacity.
    """
    universe = fo.getUniverse()
    fleet = universe.getFleet(fleet_id)
    cur_capacity = 0
    max_capacity = 0
    ships = (universe.getShip(ship_id) for ship_id in (fleet.shipIDs if fleet else []))
    for ship in ships:
        design = ship and ship.design
        design_parts = design.parts if design and design.hasFighters else []
        for partname in design_parts:
            part = get_ship_part(partname)
            if part and part.partClass == fo.shipPartClass.fighterHangar:
                cur_capacity += ship.currentPartMeterValue(fo.meterType.capacity, partname)
                max_capacity += ship.currentPartMeterValue(fo.meterType.maxCapacity, partname)
    return cur_capacity, max_capacity


def get_fleet_upkeep():
    # TODO: Use new upkeep calculation
    return 1 + AIDependencies.SHIP_UPKEEP * get_aistate().shipCount


def calculate_estimated_time_of_arrival(fleet_id, target_system_id):
    universe = fo.getUniverse()
    fleet = universe.getFleet(fleet_id)
    if not fleet or not fleet.speed:
        return 99999
    distance = get_shortest_distance(fleet_id, target_system_id)
    return math.ceil(float(distance) / fleet.speed)


def get_fleet_system(fleet: Union[TargetFleet, int]) -> int:
    """Return the current fleet location or the target system if currently on starlane."""
    if isinstance(fleet, int):
        fleet = fo.getUniverse().getFleet(fleet)
    return fleet.systemID if fleet.systemID != INVALID_ID else fleet.nextSystemID


def get_current_and_max_structure(fleet: int) -> tuple[float, float]:
    """Return a 2-tuple of the sums of structure and maxStructure meters of all ships in the fleet."""

    universe = fo.getUniverse()
    destroyed_ids = universe.destroyedObjectIDs(fo.empireID())
    fleet = universe.getFleet(fleet)
    if not fleet:
        return (0.0, 0.0)
    ships_cur_health = 0
    ships_max_health = 0
    for ship_id in fleet.shipIDs:
        # Check if we have see this ship get destroyed in a different fleet since the last time we saw the subject fleet
        # this may be redundant with the new fleet assignment check made below, but for its limited scope it may be more
        # reliable, in that it does not rely on any particular handling of post-destruction stats
        if ship_id in destroyed_ids:
            continue
        this_ship = universe.getShip(ship_id)
        # check that the ship has not been seen in a new fleet since this current fleet was last observed
        if not (this_ship and this_ship.fleetID == fleet.id):
            continue
        ships_cur_health += this_ship.initialMeterValue(fo.meterType.structure)
        ships_max_health += this_ship.initialMeterValue(fo.meterType.maxStructure)

    return ships_cur_health, ships_max_health
