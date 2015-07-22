import freeOrionAIInterface as fo  # pylint: disable=import-error
import FreeOrionAI as foAI
import AIstate
import universe_object
import AIFleetOrder
import ColonisationAI
import PlanetUtilsAI
from freeorion_tools import ppstring


def get_fleet_orders_from_system_targets(fleet_target, targets):  # TODO consider to change targets to single target
    """
    Return Move orders to fleet.

    :param fleet_target:
    :type fleet_target: universe_object.Fleet
    :param targets:
    :type targets: list
    :return: list of OrdersMove
    :rtype: list
    """
    result = []
    # TODO: use Graph Theory to construct move orders
    # TODO: add priority
    # determine system where fleet will be or where is if is going nowhere
    last_system_target = fleet_target.get_system()
    secure_targets = set(AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs + AIstate.invasionTargetedSystemIDs + AIstate.blockadeTargetedSystemIDs)
    # for every system which fleet wanted to visit, determine systems to visit and create move orders
    for target in targets:
        # determine systems required to visit(with possible return to supplied system)
        ensure_return = target.id not in secure_targets
        system_targets = can_travel_to_system(fleet_target.id, last_system_target, target, ensure_return=ensure_return)
        #print "making path with %d targets: "%len(system_targets) , ppstring(PlanetUtilsAI.sys_name_ids( [sysTarg.id for sysTarg in system_targets]))
        if system_targets:
            # for every system required to visit create move order
            for system_targer in system_targets:
                # remember last system which will be visited
                last_system_target = system_targer
                # create move order
                fleet_order = AIFleetOrder.OrderMove(fleet_target, system_targer)
                result.append(fleet_order)
        else:
            if last_system_target.id != target.id:
                print "fleetID: %s can't travel to target: %s" % (fleet_target.id, target)
    return result


def can_travel_to_system(fleet_id, from_system_target, to_system_target, ensure_return=False):
    """
    Return list systems to be visited.

    :param fleet_id:
    :param fleet_id: int
    :param from_system_target:
    :type from_system_target: universe_object.System
    :param to_system_target:
    :type to_system_target:  universe_object.System
    :param ensure_return:
    :type ensure_return: bool
    :return:
    :rtype: list
    """
    empire = fo.getEmpire()
    empire_id = empire.empireID
    fleet_supplyable_system_ids = set(empire.fleetSupplyableSystemIDs)
    # get current fuel and max fuel
    universe = fo.getUniverse()
    fleet = universe.getFleet(fleet_id)
    fuel = int(fleet.fuel)
    if fuel < 1.0 or from_system_target.id == to_system_target.id:
        return []
    if foAI.foAIstate.aggression <= fo.aggression.typical or True:  # TODO: sort out if shortestPath leaves off some intermediate destinations
        path_func = universe.leastJumpsPath
    else:
        path_func = universe.shortestPath
    start_sys_id = from_system_target.id
    target_sys_id = to_system_target.id
    if start_sys_id != -1 and target_sys_id != -1:
        short_path = list(path_func(start_sys_id, target_sys_id, empire_id))
    else:
        short_path = []
    legs = zip(short_path[:-1], short_path[1:])
    #suppliedStops = [ sid for sid in short_path if sid in fleet_supplyable_system_ids ]
    #unsupplied_stops = [sid for sid in short_path if sid not in suppliedStops ]
    unsupplied_stops = [sys_b for sys_a, sys_b in legs if ((sys_a not in fleet_supplyable_system_ids) and (sys_b not in fleet_supplyable_system_ids))]
    #print "getting path from %s to %s "%(ppstring(PlanetUtilsAI.sys_name_ids([ start_sys_id ])), ppstring(PlanetUtilsAI.sys_name_ids([ target_sys_id ])) ),
    #print " ::: found initial path %s having suppliedStops %s and unsupplied_stops %s ; tot fuel available is %.1f"%( ppstring(PlanetUtilsAI.sys_name_ids( short_path[:])), suppliedStops, unsupplied_stops, fuel)
    if False:
        if target_sys_id in fleet_supplyable_system_ids:
            print "target has FleetSupply"
        elif target_sys_id in ColonisationAI.annexable_ring1:
            print "target in Ring 1"
        elif target_sys_id in ColonisationAI.annexable_ring2:
            print "target in Ring 2, has enough aggression is ", foAI.foAIstate.aggression >= fo.aggression.typical
        elif target_sys_id in ColonisationAI.annexable_ring3:
            print "target in Ring 2, has enough aggression is ", foAI.foAIstate.aggression >= fo.aggression.aggressive
    if (not unsupplied_stops or not ensure_return or
                target_sys_id in fleet_supplyable_system_ids and len(unsupplied_stops) <= fuel or
                target_sys_id in ColonisationAI.annexable_ring1 and len(unsupplied_stops) < fuel or
                foAI.foAIstate.aggression >= fo.aggression.typical and target_sys_id in ColonisationAI.annexable_ring2 and len(unsupplied_stops) < fuel - 1 or
                foAI.foAIstate.aggression >= fo.aggression.aggressive and target_sys_id in ColonisationAI.annexable_ring3 and len(unsupplied_stops) < fuel - 2):
        return [universe_object.System(sid) for sid in short_path]
    else:
        #print " getting path from 'can_travel_to_system_and_return_to_resupply' ",
        return can_travel_to_system_and_return_to_resupply(fleet_id, from_system_target, to_system_target)


def can_travel_to_system_and_return_to_resupply(fleet_id, from_system_target, to_system_target):
    """
    Filter systems where fleet can travel from starting system. # TODO rename function
    
    :param fleet_id:
    :type fleet_id: int
    :param from_system_target:
    :type from_system_target: universe_object.System
    :param to_system_target:
    :type to_system_target: universe_object.System
    :return:
    :rtype: list
    """
    system_targets = []
    if not from_system_target.id == to_system_target.id:
        # get supplyable systems
        empire = fo.getEmpire()
        fleet_supplyable_system_ids = empire.fleetSupplyableSystemIDs
        # get current fuel and max fuel
        universe = fo.getUniverse()
        fleet = universe.getFleet(fleet_id)
        max_fuel = int(fleet.maxFuel)
        fuel = int(fleet.fuel)
        #if verbose:
        # print "   fleet ID %d has %.1f fuel to get from %s to %s"%(fleetID, fuel, fromSystemAITarget, toSystemAITarget )

        # try to find path without going resupply first
        supply_system_target = get_nearest_supplied_system(to_system_target.id)
        system_targets = __find_path_with_fuel_to_system_with_possible_return(from_system_target, to_system_target, system_targets, fleet_supplyable_system_ids, max_fuel, fuel, supply_system_target)
        # resupply in system first is required to find path
        if not from_system_target.id in fleet_supplyable_system_ids and not system_targets:
            # add supply system to visit
            from_system_target = get_nearest_supplied_system(from_system_target.id)
            system_targets.append(from_system_target)
            # find path from supplied system to wanted system
            system_targets = __find_path_with_fuel_to_system_with_possible_return(from_system_target, to_system_target, system_targets, fleet_supplyable_system_ids, max_fuel, max_fuel, supply_system_target)
    return system_targets


def get_nearest_supplied_system(start_system_id):
    """ Return systemAITarget of nearest supplied system from starting system startSystemID."""
    empire = fo.getEmpire()
    fleet_supplyable_system_ids = empire.fleetSupplyableSystemIDs
    universe = fo.getUniverse()

    if start_system_id in fleet_supplyable_system_ids:
        return universe_object.System(start_system_id)
    else:
        min_jumps = 9999  # infinity
        supply_system_id = -1
        for system_id in fleet_supplyable_system_ids:
            if start_system_id != -1 and system_id != -1:
                least_jumps_len = universe.jumpDistance(start_system_id, system_id)
                if least_jumps_len < min_jumps:
                    min_jumps = least_jumps_len
                    supply_system_id = system_id
        return universe_object.System(supply_system_id)


def get_nearest_drydock_system_id(start_system_id):
    """ Return systemAITarget of nearest supplied system from starting system startSystemID."""
    drydock_system_ids = ColonisationAI.empire_dry_docks.keys()
    universe = fo.getUniverse()
    if start_system_id in drydock_system_ids:
        return start_system_id
    else:
        min_jumps = 9999  # infinity
        supply_system_id = -1
        for system_id in drydock_system_ids:
            if start_system_id != -1 and system_id != -1:
                least_jumps_len = universe.jumpDistance(start_system_id, system_id)
                if least_jumps_len < min_jumps:
                    min_jumps = least_jumps_len
                    supply_system_id = system_id
        return supply_system_id


def get_safe_path_leg_to_dest(fleet_id, start_id, dest_id):
    start_targ = universe_object.System(start_id)
    dest_targ = universe_object.System(dest_id)
    #TODO actually get a safe path
    this_path = can_travel_to_system(fleet_id, start_targ, dest_targ, ensure_return=False)
    path_ids = [targ.id for targ in this_path if targ.id != start_id] + [start_id]
    start_info = PlanetUtilsAI.sys_name_ids([start_id])
    dest_info = PlanetUtilsAI.sys_name_ids([dest_id])
    path_info = [PlanetUtilsAI.sys_name_ids([sys_id]) for sys_id in path_ids]
    print "Fleet %d requested safe path leg from %s to %s, found path %s" % (fleet_id, ppstring(start_info), ppstring(dest_info), ppstring(path_info))
    return path_ids[0]


def __find_path_with_fuel_to_system_with_possible_return(from_system_target, to_system_target, result_system_targets, fleet_supplyable_system_ids, max_fuel, fuel, supply_system_target):
    """
    Return systems required to visit with fuel to nearest supplied system.

    :param from_system_target:
    :type from_system_target: universe_object.System
    :param to_system_target:
    :type to_system_target: universe_object.System
    :param result_system_targets:
    :type result_system_targets: list
    :param fleet_supplyable_system_ids:
    :type fleet_supplyable_system_ids: list
    :param max_fuel:
    :type max_fuel: int
    :param fuel:
    :type fuel: int
    :param supply_system_target:
    :type supply_system_target: universe_object.System
    :return:
    :rtype list:
    """
    empire_id = fo.empireID()
    result = True
    # try to find if there is possible path to wanted system from system
    new_targets = result_system_targets[:]
    if from_system_target and to_system_target and supply_system_target:
        universe = fo.getUniverse()
        if from_system_target.id != -1 and to_system_target.id != -1:
            least_jumps_path = universe.leastJumpsPath(from_system_target.id, to_system_target.id, empire_id)
        else:
            least_jumps_path = []
            result = False
        from_system_id = from_system_target.id
        for system_id in least_jumps_path:
            if from_system_id != system_id:
                if from_system_id in fleet_supplyable_system_ids:
                    # from supplied system fleet can travel without fuel consumption and also in this system refuels
                    fuel = max_fuel
                else:
                    fuel -= 1

                # leastJumpPath can differ from shortestPath
                # TODO: use Graph Theory to optimize
                if True or (system_id != to_system_target.id and system_id in fleet_supplyable_system_ids):  # TODO: restructure
                    new_targets.append(universe_object.System(system_id))
                if fuel < 0:
                    result = False
            from_system_id = system_id
    else:
        result = False

    # if there is path to wanted system, then also if there is path back to supplyable system
    if result:
        # jump from A to B means least_jumps_path=[A,B], but min_jumps=1
        min_jumps = len(universe.leastJumpsPath(to_system_target.id, supply_system_target.id, empire_id)) - 1

        if min_jumps > fuel:
            # print "fleetID:" + str(fleetID) + " fuel:" + str(fuel) + " required: " + str(min_jumps)
            result = False
        #else:
            #resultSystemAITargets.append(toSystemAITarget)

    if not result:
        return []
    return new_targets


def get_resupply_fleet_order(fleet_target, current_system_target):
    """
    Return fleet_orders.OrderResupply to nearest supplied system.

    :param fleet_target: fleet that need to be resupplied
    :type fleet_target: universe_object.Fleet
    # TODO check if we can remove this id, because fleet already have it.
    :param current_system_target: current system of fleet
    :type current_system_target: universe_object.System
    :return: order to resupply
    :rtype fleet_orders.OrderResupply
    """
    # find nearest supplied system
    supplied_system_target = get_nearest_supplied_system(current_system_target.id)
    # create resupply AIFleetOrder
    return AIFleetOrder.OrderResupply(fleet_target, supplied_system_target)


def get_repair_fleet_order(fleet_target, current_sys_id):
    """
    Return fleet_orders.OrderRepair for fleet to proceed system with drydock.

    :param fleet_target: fleet that need to be repaired
    :type fleet_target: universe_object.Fleet
    # TODO check if we can remove this id, because fleet already have it.
    :param current_sys_id: current system id
    :type current_sys_id: int
    :return: order to repair
    :rtype fleet_orders.OrderRepair
    """
    # find nearest supplied system
    drydock_sys_id = get_nearest_drydock_system_id(current_sys_id)
    print "ordering fleet %d to %s for repair" % (fleet_target.id, ppstring(PlanetUtilsAI.sys_name_ids([drydock_sys_id])))
    # create resupply AIFleetOrder
    return AIFleetOrder.OrderRepair(fleet_target, universe_object.System(drydock_sys_id))
