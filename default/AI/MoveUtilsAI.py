import freeOrionAIInterface as fo # pylint: disable=import-error
import FreeOrionAI as foAI
import AIstate
from EnumsAI import AITargetType, AIFleetOrderType
import AITarget
import AIFleetOrder
import ColonisationAI
import PlanetUtilsAI

def getAIFleetOrdersFromSystemAITargets(fleetAITarget, aiTargets):
    result = []
    # TODO: use Graph Theory to construct move orders
    # TODO: add priority
    empireID = fo.empireID()
    # determine system where fleet will be or where is if is going nowhere
    lastSystemAITarget = fleetAITarget.get_required_system_ai_targets()[0]
    secure_targets = set(AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs + AIstate.invasionTargetedSystemIDs + AIstate.blockadeTargetedSystemIDs)
    # for every system which fleet wanted to visit, determine systems to visit and create move orders
    for aiTarget in aiTargets:
        # determine systems required to visit(with possible return to supplied system)
        #print "checking system targets"
        ensure_return = aiTarget.target_id not in  secure_targets
        systemAITargets = canTravelToSystem(fleetAITarget.target_id, lastSystemAITarget, aiTarget, empireID, ensure_return=ensure_return)
        #print "making path with %d targets: "%len(systemAITargets) ,   PlanetUtilsAI.sysNameIDs( [sysTarg.target_id for sysTarg in systemAITargets])
        if len(systemAITargets) > 0:
            # for every system required to visit create move order
            for systemAITarget in systemAITargets:
                # remember last system which will be visited
                lastSystemAITarget = systemAITarget
                # create move order
                aiFleetOrder = AIFleetOrder.AIFleetOrder(AIFleetOrderType.ORDER_MOVE, fleetAITarget, systemAITarget)
                result.append(aiFleetOrder)
        else:
            startSysID = lastSystemAITarget.target_id
            targetSysID = aiTarget.target_id
            if startSysID != targetSysID:
                print "fleetID: " + str(fleetAITarget.target_id) + " can't travel to target:" + str(aiTarget)

    return result

def  canTravelToSystem(fleetID, fromSystemAITarget, toSystemAITarget, empireID,  ensure_return=False):
    empire = fo.getEmpire()
    fleetSupplyableSystemIDs = set(empire.fleetSupplyableSystemIDs)
    # get current fuel and max fuel
    universe = fo.getUniverse()
    fleet = universe.getFleet(fleetID)
    maxFuel = int(fleet.maxFuel)
    fuel = int(fleet.fuel)
    if fuel < 1.0 or fromSystemAITarget.target_id == toSystemAITarget.target_id:
        return []
    if foAI.foAIstate.aggression<=fo.aggression.typical or True: #TODO: sort out if shortestPath leaves off some intermediate destinations
        pathFunc=universe.leastJumpsPath
    else:
        pathFunc=universe.shortestPath
    startSysID = fromSystemAITarget.target_id
    targetSysID = toSystemAITarget.target_id
    if (startSysID != -1) and (targetSysID != -1):
        shortPath= list( pathFunc(startSysID, targetSysID, empireID) )
    else:
        shortPath = []
    legs = zip( shortPath[:-1],  shortPath[1:])
    #suppliedStops = [ sid for sid in shortPath if sid in fleetSupplyableSystemIDs  ]
    #unsuppliedStops = [sid for sid in shortPath if sid not in suppliedStops ]
    unsuppliedStops = [ sys_b for sys_a, sys_b in legs if ((sys_a not in fleetSupplyableSystemIDs) and (sys_b not in fleetSupplyableSystemIDs))]
    retPath=[]
    #print "getting path from %s to %s "%(PlanetUtilsAI.sysNameIDs([  startSysID ]), PlanetUtilsAI.sysNameIDs([  targetSysID ])  ),
    #print " ::: found initial path  %s having suppliedStops  %s and  unsuppliedStops  %s ; tot fuel available is %.1f"%( PlanetUtilsAI.sysNameIDs( shortPath[:] ),  suppliedStops,  unsuppliedStops,  fuel)
    if False:
        if  targetSysID in fleetSupplyableSystemIDs:
            print "target has FleetSupply"
        elif targetSysID in ColonisationAI.annexableRing1:
            print "target in Ring 1"
        elif  targetSysID in ColonisationAI.annexableRing2:
            print "target in Ring 2,  has enough aggression is ",  foAI.foAIstate.aggression >=fo.aggression.typical
        elif targetSysID in ColonisationAI.annexableRing3:
            print "target in Ring 2,  has enough aggression is ",   foAI.foAIstate.aggression >=fo.aggression.aggressive
    if  ( len( unsuppliedStops) == 0 or not ensure_return or
                targetSysID in fleetSupplyableSystemIDs and len( unsuppliedStops) <= fuel or
                targetSysID in ColonisationAI.annexableRing1 and len( unsuppliedStops) < fuel or
                foAI.foAIstate.aggression >=fo.aggression.typical  and targetSysID in ColonisationAI.annexableRing2 and len( unsuppliedStops) < fuel -1 or
                foAI.foAIstate.aggression >=fo.aggression.aggressive  and targetSysID in ColonisationAI.annexableRing3 and len( unsuppliedStops) < fuel -2 ):
        retPath =  [ AITarget.AITarget(AITargetType.TARGET_SYSTEM, sid) for sid in shortPath]
    else:
        #print " getting path from 'canTravelToSystemAndReturnToResupply' ",
        retPath =  canTravelToSystemAndReturnToResupply(fleetID, fromSystemAITarget, toSystemAITarget, empireID,  verbose=True)
    return retPath

def canTravelToSystemAndReturnToResupply(fleetID, fromSystemAITarget, toSystemAITarget, empireID,  verbose=False):
    "check if fleet can travel from starting system to wanted system"

    systemAITargets = []
    if not fromSystemAITarget.target_id == toSystemAITarget.target_id:
        # get supplyable systems
        empire = fo.getEmpire()
        fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
        # get current fuel and max fuel
        universe = fo.getUniverse()
        fleet = universe.getFleet(fleetID)
        maxFuel = int(fleet.maxFuel)
        fuel = int(fleet.fuel)
        #if verbose:
        #    print "   fleet ID %d  has  %.1f fuel  to get from %s    to  %s"%(fleetID,  fuel,  fromSystemAITarget,  toSystemAITarget )

        # try to find path without going resupply first
        supplySystemAITarget = getNearestSuppliedSystem(toSystemAITarget.target_id, empireID)
        __findPathWithFuelToSystemWithPossibleReturn(fromSystemAITarget, toSystemAITarget, empireID, systemAITargets, fleetSupplyableSystemIDs, maxFuel, fuel, supplySystemAITarget)
        # resupply in system first is required to find path
        if not(fromSystemAITarget.target_id in fleetSupplyableSystemIDs) and len(systemAITargets) == 0:
            # add supply system to visit
            fromSystemAITarget = getNearestSuppliedSystem(fromSystemAITarget.target_id, empireID)
            systemAITargets.append(fromSystemAITarget)
            # find path from supplied system to wanted system
            __findPathWithFuelToSystemWithPossibleReturn(fromSystemAITarget, toSystemAITarget, empireID, systemAITargets, fleetSupplyableSystemIDs, maxFuel, maxFuel, supplySystemAITarget)

    return systemAITargets

def getNearestSuppliedSystem(startSystemID, empireID):
    "returns systemAITarget of nearest supplied system from starting system startSystemID"

    empire = fo.getEmpire()
    fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
    universe = fo.getUniverse()

    if startSystemID in fleetSupplyableSystemIDs:
        return AITarget.AITarget(AITargetType.TARGET_SYSTEM, startSystemID)
    else:
        minJumps = 9999 # infinity
        supplySystemID = -1
        for systemID in fleetSupplyableSystemIDs:
            if (startSystemID != -1) and (systemID != -1):
                leastJumpsLen = universe.jumpDistance(startSystemID, systemID)
                if leastJumpsLen < minJumps:
                    minJumps = leastJumpsLen
                    supplySystemID = systemID

        return AITarget.AITarget(AITargetType.TARGET_SYSTEM, supplySystemID)

def getNearestDrydockSystemID(startSystemID):
    "returns systemAITarget of nearest supplied system from starting system startSystemID"

    drydock_system_ids = ColonisationAI.empire_dry_docks.keys()
    universe = fo.getUniverse()

    if startSystemID in drydock_system_ids:
        return startSystemID
    else:
        minJumps = 9999 # infinity
        supplySystemID = -1
        for systemID in drydock_system_ids:
            if (startSystemID != -1) and (systemID != -1):
                leastJumpsLen = universe.jumpDistance(startSystemID, systemID)
                if leastJumpsLen < minJumps:
                    minJumps = leastJumpsLen
                    supplySystemID = systemID

        return supplySystemID

def get_safe_path_leg_to_dest(fleet_id,  start_id,  dest_id):
    start_targ = AITarget.AITarget(AITargetType.TARGET_SYSTEM, start_id)
    dest_targ = AITarget.AITarget(AITargetType.TARGET_SYSTEM, dest_id)
    #TODO actually get a safe path
    this_path = canTravelToSystem(fleet_id, start_targ, dest_targ, fo.empireID(),  ensure_return=False)
    path_ids = [ targ.target_id for targ in this_path if targ.target_id != start_id] + [start_id]
    start_info = PlanetUtilsAI.sysNameIDs([start_id])
    dest_info = PlanetUtilsAI.sysNameIDs([dest_id])
    path_info = [ PlanetUtilsAI.sysNameIDs([sys_id]) for sys_id in path_ids]
    print "Fleet %d requested safe path leg from %s to %s,  found path %s"%(fleet_id,  start_info, dest_info ,  path_info)
    return path_ids[0]

def __findPathWithFuelToSystemWithPossibleReturn(fromSystemAITarget, toSystemAITarget, empireID, resultSystemAITargets, fleetSupplyableSystemIDs, maxFuel, fuel, supplySystemAITarget):
    "returns system AITargets required to visit with fuel to nearest supplied system"

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
                    fuel = fuel - 1

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
    if result == True:
        # jump from A to B means leastJumpsPath=[A,B], but minJumps=1
        minJumps = len(universe.leastJumpsPath(toSystemAITarget.target_id, supplySystemAITarget.target_id, empireID)) - 1

        if minJumps > fuel:
            # print "fleetID:" + str(fleetID) + " fuel:" + str(fuel) + " required: " + str(minJumps)
            result = False
        #else:
            #resultSystemAITargets.append(toSystemAITarget)

    if result == False:
        return []
    resultSystemAITargets[:] = newTargets
    return resultSystemAITargets

def getResupplyAIFleetOrder(fleetAITarget, currentSystemAITarget):
    "returns resupply AIFleetOrder to nearest supplied system"

    # find nearest supplied system
    empireID = fo.empireID()
    suppliedSystemAITarget = getNearestSuppliedSystem(currentSystemAITarget.target_id, empireID)

    # create resupply AIFleetOrder
    aiFleetOrder = AIFleetOrder.AIFleetOrder(AIFleetOrderType.ORDER_RESUPPLY, fleetAITarget, suppliedSystemAITarget)

    return aiFleetOrder

def getRepairAIFleetOrder(fleetAITarget, current_sys_id):
    "returns repair AIFleetOrder to [nearest safe] drydock"
    # find nearest supplied system
    empireID = fo.empireID()
    drydock_sys_id = getNearestDrydockSystemID(current_sys_id)
    drydockSystemAITarget = AITarget.AITarget(AITargetType.TARGET_SYSTEM, drydock_sys_id)
    print "ordering fleet %d to %s for repair"%(fleetAITarget.target_id,  PlanetUtilsAI.sysNameIDs([drydock_sys_id]))
    # create resupply AIFleetOrder
    aiFleetOrder = AIFleetOrder.AIFleetOrder(AIFleetOrderType.ORDER_REPAIR, fleetAITarget, drydockSystemAITarget)
    return aiFleetOrder
