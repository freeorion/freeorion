import freeOrionAIInterface as fo # pylint: disable=import-error
import FreeOrionAI as foAI
import AIstate
from EnumsAI import AITargetType, AIFleetOrderType
import AITarget
import AIFleetOrder
import ColonisationAI
import PlanetUtilsAI


def get_fleet_orders_from_system_targets(fleet_target, targets):
    result = []
    # TODO: use Graph Theory to construct move orders
    # TODO: add priority
    empire_id = fo.empireID()
    # determine system where fleet will be or where is if is going nowhere
    last_system_target = fleet_target.get_required_system_ai_targets()[0]
    secure_targets = set(AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs + AIstate.invasionTargetedSystemIDs + AIstate.blockadeTargetedSystemIDs)
    # for every system which fleet wanted to visit, determine systems to visit and create move orders
    for aiTarget in targets:
        # determine systems required to visit(with possible return to supplied system)
        #print "checking system targets"
        ensure_return = aiTarget.target_id not in  secure_targets
        system_targets = can_travel_to_system(fleet_target.target_id, last_system_target, aiTarget, empire_id, ensure_return=ensure_return)
        #print "making path with %d targets: "%len(system_targets) ,   PlanetUtilsAI.sys_name_ids( [sysTarg.target_id for sysTarg in system_targets])
        if len(system_targets) > 0:
            # for every system required to visit create move order
            for systemAITarget in system_targets:
                # remember last system which will be visited
                last_system_target = systemAITarget
                # create move order
                fleet_order = AIFleetOrder.AIFleetOrder(AIFleetOrderType.ORDER_MOVE, fleet_target, systemAITarget)
                result.append(fleet_order)
        else:
            start_sys_id = last_system_target.target_id
            target_sys_id = aiTarget.target_id
            if start_sys_id != target_sys_id:
                print "fleetID: " + str(fleet_target.target_id) + " can't travel to target:" + str(aiTarget)

    return result


def can_travel_to_system(fleet_id, from_system_target, to_system_target, empire_id, ensure_return=False):
    empire = fo.getEmpire()
    fleet_supplyable_system_ids = set(empire.fleetSupplyableSystemIDs)
    # get current fuel and max fuel
    universe = fo.getUniverse()
    fleet = universe.getFleet(fleet_id)
    fuel = int(fleet.fuel)
    if fuel < 1.0 or from_system_target.target_id == to_system_target.target_id:
        return []
    if foAI.foAIstate.aggression<=fo.aggression.typical or True: #TODO: sort out if shortestPath leaves off some intermediate destinations
        pathFunc=universe.leastJumpsPath
    else:
        pathFunc=universe.shortestPath
    start_sys_id = from_system_target.target_id
    target_sys_id = to_system_target.target_id
    if (start_sys_id != -1) and (target_sys_id != -1):
        short_path= list( pathFunc(start_sys_id, target_sys_id, empire_id) )
    else:
        short_path = []
    legs = zip( short_path[:-1],  short_path[1:])
    #suppliedStops = [ sid for sid in short_path if sid in fleet_supplyable_system_ids  ]
    #unsupplied_stops = [sid for sid in short_path if sid not in suppliedStops ]
    unsupplied_stops = [ sys_b for sys_a, sys_b in legs if ((sys_a not in fleet_supplyable_system_ids) and (sys_b not in fleet_supplyable_system_ids))]
    ret_path=[]
    #print "getting path from %s to %s "%(PlanetUtilsAI.sys_name_ids([  start_sys_id ]), PlanetUtilsAI.sys_name_ids([  target_sys_id ])  ),
    #print " ::: found initial path  %s having suppliedStops  %s and  unsupplied_stops  %s ; tot fuel available is %.1f"%( PlanetUtilsAI.sys_name_ids( short_path[:] ),  suppliedStops,  unsupplied_stops,  fuel)
    if False:
        if  target_sys_id in fleet_supplyable_system_ids:
            print "target has FleetSupply"
        elif target_sys_id in ColonisationAI.annexableRing1:
            print "target in Ring 1"
        elif  target_sys_id in ColonisationAI.annexableRing2:
            print "target in Ring 2,  has enough aggression is ",  foAI.foAIstate.aggression >=fo.aggression.typical
        elif target_sys_id in ColonisationAI.annexableRing3:
            print "target in Ring 2,  has enough aggression is ",   foAI.foAIstate.aggression >=fo.aggression.aggressive
    if  ( len( unsupplied_stops) == 0 or not ensure_return or
                target_sys_id in fleet_supplyable_system_ids and len( unsupplied_stops) <= fuel or
                target_sys_id in ColonisationAI.annexableRing1 and len( unsupplied_stops) < fuel or
                foAI.foAIstate.aggression >=fo.aggression.typical  and target_sys_id in ColonisationAI.annexableRing2 and len( unsupplied_stops) < fuel -1 or
                foAI.foAIstate.aggression >=fo.aggression.aggressive  and target_sys_id in ColonisationAI.annexableRing3 and len( unsupplied_stops) < fuel -2 ):
        return [ AITarget.AITarget(AITargetType.TARGET_SYSTEM, sid) for sid in short_path]
    else:
        #print " getting path from 'can_travel_to_system_and_return_to_resupply' ",
        return can_travel_to_system_and_return_to_resupply(fleet_id, from_system_target, to_system_target, empire_id,  verbose=True)


def can_travel_to_system_and_return_to_resupply(fleet_id, from_system_target, to_system_target, empire_id,  verbose=False):
    """check if fleet can travel from starting system to wanted system"""
    system_targets = []
    if not from_system_target.target_id == to_system_target.target_id:
        # get supplyable systems
        empire = fo.getEmpire()
        fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
        # get current fuel and max fuel
        universe = fo.getUniverse()
        fleet = universe.getFleet(fleet_id)
        max_fuel = int(fleet.maxFuel)
        fuel = int(fleet.fuel)
        #if verbose:
        #    print "   fleet ID %d  has  %.1f fuel  to get from %s    to  %s"%(fleetID,  fuel,  fromSystemAITarget,  toSystemAITarget )

        # try to find path without going resupply first
        supply_system_target = get_nearest_supplied_system(to_system_target.target_id, empire_id)
        __find_path_with_fuel_to_system_with_possible_return(from_system_target, to_system_target, empire_id, system_targets, fleetSupplyableSystemIDs, max_fuel, fuel, supply_system_target)
        # resupply in system first is required to find path
        if not(from_system_target.target_id in fleetSupplyableSystemIDs) and len(system_targets) == 0:
            # add supply system to visit
            from_system_target = get_nearest_supplied_system(from_system_target.target_id, empire_id)
            system_targets.append(from_system_target)
            # find path from supplied system to wanted system
            __find_path_with_fuel_to_system_with_possible_return(from_system_target, to_system_target, empire_id, system_targets, fleetSupplyableSystemIDs, max_fuel, max_fuel, supply_system_target)

    return system_targets


def get_nearest_supplied_system(start_system_id, empire_id):
    """returns systemAITarget of nearest supplied system from starting system startSystemID"""
    empire = fo.getEmpire()
    fleet_supplyable_system_ids = empire.fleetSupplyableSystemIDs
    universe = fo.getUniverse()

    if start_system_id in fleet_supplyable_system_ids:
        return AITarget.AITarget(AITargetType.TARGET_SYSTEM, start_system_id)
    else:
        min_jumps = 9999  # infinity
        supply_system_id = -1
        for system_id in fleet_supplyable_system_ids:
            if (start_system_id != -1) and (system_id != -1):
                least_jumps_len = universe.jumpDistance(start_system_id, system_id)
                if least_jumps_len < min_jumps:
                    min_jumps = least_jumps_len
                    supply_system_id = system_id
        return AITarget.AITarget(AITargetType.TARGET_SYSTEM, supply_system_id)


def get_nearest_drydock_system_id(start_system_id):
    """returns systemAITarget of nearest supplied system from starting system startSystemID"""
    drydock_system_ids = ColonisationAI.empire_dry_docks.keys()
    universe = fo.getUniverse()

    if start_system_id in drydock_system_ids:
        return start_system_id
    else:
        min_jumps = 9999 # infinity
        supply_system_id = -1
        for system_id in drydock_system_ids:
            if (start_system_id != -1) and (system_id != -1):
                least_jumps_len = universe.jumpDistance(start_system_id, system_id)
                if least_jumps_len < min_jumps:
                    min_jumps = least_jumps_len
                    supply_system_id = system_id

        return supply_system_id


def get_safe_path_leg_to_dest(fleet_id,  start_id,  dest_id):
    start_targ = AITarget.AITarget(AITargetType.TARGET_SYSTEM, start_id)
    dest_targ = AITarget.AITarget(AITargetType.TARGET_SYSTEM, dest_id)
    #TODO actually get a safe path
    this_path = can_travel_to_system(fleet_id, start_targ, dest_targ, fo.empireID(),  ensure_return=False)
    path_ids = [ targ.target_id for targ in this_path if targ.target_id != start_id] + [start_id]
    start_info = PlanetUtilsAI.sys_name_ids([start_id])
    dest_info = PlanetUtilsAI.sys_name_ids([dest_id])
    path_info = [ PlanetUtilsAI.sys_name_ids([sys_id]) for sys_id in path_ids]
    print "Fleet %d requested safe path leg from %s to %s,  found path %s"%(fleet_id,  start_info, dest_info ,  path_info)
    return path_ids[0]


def __find_path_with_fuel_to_system_with_possible_return(fromSystemAITarget, toSystemAITarget, empireID, resultSystemAITargets, fleetSupplyableSystemIDs, maxFuel, fuel, supplySystemAITarget):
    """returns system AITargets required to visit with fuel to nearest supplied system"""

    result = True
    # try to find if there is possible path to wanted system from system
    newTargets = resultSystemAITargets[:]
    if fromSystemAITarget.valid and toSystemAITarget.valid and supplySystemAITarget.valid:
        universe = fo.getUniverse()
        if (fromSystemAITarget.target_id != -1) and (toSystemAITarget.target_id != -1):
            leastJumpsPath = universe.leastJumpsPath(fromSystemAITarget.target_id, toSystemAITarget.target_id, empireID)
        else:
            leastJumpsPath = []
            result = False
        fromSystemID = fromSystemAITarget.target_id
        for systemID in leastJumpsPath:
            if not fromSystemID == systemID:
                if fromSystemID in fleetSupplyableSystemIDs:
                    # from supplied system fleet can travel without fuel consumption and also in this system refuels
                    fuel = maxFuel
                else:
                    fuel -= 1

                # leastJumpPath can differ from shortestPath
                # TODO: use Graph Theory to optimize
                if True or ((not systemID == toSystemAITarget.target_id) and (systemID in fleetSupplyableSystemIDs)):#TODO:   restructure
                    newTargets.append(AITarget.AITarget(AITargetType.TARGET_SYSTEM, systemID))

                if fuel < 0:
                    result = False

            fromSystemID = systemID
    else:
        result = False

    # if there is path to wanted system, then also if there is path back to supplyable system
    if result:
        # jump from A to B means leastJumpsPath=[A,B], but minJumps=1
        minJumps = len(universe.leastJumpsPath(toSystemAITarget.target_id, supplySystemAITarget.target_id, empireID)) - 1

        if minJumps > fuel:
            # print "fleetID:" + str(fleetID) + " fuel:" + str(fuel) + " required: " + str(minJumps)
            result = False
        #else:
            #resultSystemAITargets.append(toSystemAITarget)

    if not result:
        return []
    resultSystemAITargets[:] = newTargets
    return resultSystemAITargets


def get_resupply_fleet_order(fleetAITarget, currentSystemAITarget):
    """returns resupply AIFleetOrder to nearest supplied system"""

    # find nearest supplied system
    empireID = fo.empireID()
    suppliedSystemAITarget = get_nearest_supplied_system(currentSystemAITarget.target_id, empireID)

    # create resupply AIFleetOrder
    aiFleetOrder = AIFleetOrder.AIFleetOrder(AIFleetOrderType.ORDER_RESUPPLY, fleetAITarget, suppliedSystemAITarget)

    return aiFleetOrder


def get_repair_fleet_order(fleetAITarget, current_sys_id):
    """returns repair AIFleetOrder to [nearest safe] drydock"""
    # find nearest supplied system
    empireID = fo.empireID()
    drydock_sys_id = get_nearest_drydock_system_id(current_sys_id)
    drydockSystemAITarget = AITarget.AITarget(AITargetType.TARGET_SYSTEM, drydock_sys_id)
    print "ordering fleet %d to %s for repair"%(fleetAITarget.target_id,  PlanetUtilsAI.sys_name_ids([drydock_sys_id]))
    # create resupply AIFleetOrder
    aiFleetOrder = AIFleetOrder.AIFleetOrder(AIFleetOrderType.ORDER_REPAIR, fleetAITarget, drydockSystemAITarget)
    return aiFleetOrder
