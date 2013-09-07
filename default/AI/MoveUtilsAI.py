import freeOrionAIInterface as fo
import FreeOrionAI as foAI
from EnumsAI import AITargetType, AIFleetOrderType
import AITarget
import AIFleetOrder
from ColonisationAI import annexableSystemIDs,  annexableRing1,  annexableRing2,  annexableRing3
import PlanetUtilsAI

def getAIFleetOrdersFromSystemAITargets(fleetAITarget, aiTargets):
    result = []
    # TODO: use Graph Theory to construct move orders
    # TODO: add priority
    empireID = fo.empireID()
    # determine system where fleet will be or where is if is going nowhere
    lastSystemAITarget = fleetAITarget.getRequiredSystemAITargets()[0]
    # for every system which fleet wanted to visit, determine systems to visit and create move orders
    for aiTarget in aiTargets:
        # determine systems required to visit(with possible return to supplied system)
        #print "checking system targets"
        systemAITargets = canTravelToSystem(fleetAITarget.getTargetID(), lastSystemAITarget, aiTarget, empireID)
        #print "making path with %d targets: "%len(systemAITargets) ,   PlanetUtilsAI.sysNameIDs( [sysTarg. getTargetID() for sysTarg in systemAITargets])
        if len(systemAITargets) > 0:
            # for every system required to visit create move order
            for systemAITarget in systemAITargets:
                # remember last system which will be visited
                lastSystemAITarget = systemAITarget
                # create move order
                aiFleetOrder = AIFleetOrder.AIFleetOrder(AIFleetOrderType.ORDER_MOVE, fleetAITarget, systemAITarget)
                result.append(aiFleetOrder)
        else:
            startSysID = lastSystemAITarget.getTargetID()
            targetSysID = aiTarget.getTargetID()
            if startSysID != targetSysID:
                print "fleetID: " + str(fleetAITarget.getTargetID()) + " can't travel to target:" + str(aiTarget)

    return result

def  canTravelToSystem(fleetID, fromSystemAITarget, toSystemAITarget, empireID,  ensureReturn=False):
    empire = fo.getEmpire()
    fleetSupplyableSystemIDs = set(empire.fleetSupplyableSystemIDs)
    # get current fuel and max fuel
    universe = fo.getUniverse()
    fleet = universe.getFleet(fleetID)
    maxFuel = int(fleet.maxFuel)
    fuel = int(fleet.fuel)
    if fuel < 1.0 or fromSystemAITarget.getTargetID() == toSystemAITarget.getTargetID:
        return []
    if foAI.foAIstate.aggression<=fo.aggression.typical or True: #TODO: sort out if shortestPath leaves off some intermediate destinations
        pathFunc=universe.leastJumpsPath
    else:
        pathFunc=universe.shortestPath
    startSysID = fromSystemAITarget.getTargetID()
    targetSysID = toSystemAITarget.getTargetID()
    shortPath= list( pathFunc(startSysID, targetSysID, empireID) )
    suppliedStops = [ sid for sid in shortPath if sid in fleetSupplyableSystemIDs  ]
    unsuppliedStops = [sid for sid in shortPath if sid not in suppliedStops ]
    retPath=[]
    #print "getting path from %s to %s "%(PlanetUtilsAI.sysNameIDs([  startSysID ]), PlanetUtilsAI.sysNameIDs([  targetSysID ])  ),
    #print " ::: found initial path  %s having suppliedStops  %s and  unsuppliedStops  %s ; tot fuel available is %.1f"%( PlanetUtilsAI.sysNameIDs( shortPath[:] ),  suppliedStops,  unsuppliedStops,  fuel)
    if False:
        if  targetSysID in fleetSupplyableSystemIDs:
            print "target has FleetSupply"
        elif targetSysID in annexableRing1:
            print "target in Ring 1"
        elif  targetSysID in annexableRing2:
            print "target in Ring 2,  has enough aggression is ",  foAI.foAIstate.aggression >=fo.aggression.typical
        elif targetSysID in annexableRing3:
            print "target in Ring 2,  has enough aggression is ",   foAI.foAIstate.aggression >=fo.aggression.aggressive
    if  ( len( unsuppliedStops) == 0 or
                targetSysID in fleetSupplyableSystemIDs and len( unsuppliedStops) < fuel or
                targetSysID in annexableRing1 and len( unsuppliedStops) < fuel or
                foAI.foAIstate.aggression >=fo.aggression.typical  and targetSysID in annexableRing2 and len( unsuppliedStops) < fuel -1 or
                foAI.foAIstate.aggression >=fo.aggression.aggressive  and targetSysID in annexableRing3 and len( unsuppliedStops) < fuel -2 ):
        retPath =  [ AITarget.AITarget(AITargetType.TARGET_SYSTEM, sid) for sid in shortPath]
    else:
        #print " getting path from 'canTravelToSystemAndReturnToResupply' ",
        retPath =  canTravelToSystemAndReturnToResupply(fleetID, fromSystemAITarget, toSystemAITarget, empireID,  verbose=True)
    return retPath

def canTravelToSystemAndReturnToResupply(fleetID, fromSystemAITarget, toSystemAITarget, empireID,  verbose=False):
    "check if fleet can travel from starting system to wanted system"

    systemAITargets = []
    if not fromSystemAITarget.getTargetID() == toSystemAITarget.getTargetID:
        # get supplyable systems
        empire = fo.getEmpire()
        fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
        # get current fuel and max fuel
        universe = fo.getUniverse()
        fleet = universe.getFleet(fleetID)
        maxFuel = int(fleet.maxFuel)
        fuel = int(fleet.fuel)
        if verbose:
            print "   fleet ID %d  has  %.1f fuel  to get from %s    to  %s"%(fleetID,  fuel,  fromSystemAITarget,  toSystemAITarget )

        # try to find path without going resupply first
        supplySystemAITarget = getNearestSuppliedSystem(toSystemAITarget.getTargetID(), empireID)
        __findPathWithFuelToSystemWithPossibleReturn(fromSystemAITarget, toSystemAITarget, empireID, systemAITargets, fleetSupplyableSystemIDs, maxFuel, fuel, supplySystemAITarget)
        # resupply in system first is required to find path
        if not(fromSystemAITarget.getTargetID() in fleetSupplyableSystemIDs) and len(systemAITargets) == 0:
            # add supply system to visit
            fromSystemAITarget = getNearestSuppliedSystem(fromSystemAITarget.getTargetID(), empireID)
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
            leastJumpsPath = universe.leastJumpsPath(startSystemID, systemID, empireID)
            if len(leastJumpsPath) < minJumps:
                minJumps = len(leastJumpsPath)
                supplySystemID = systemID

        return AITarget.AITarget(AITargetType.TARGET_SYSTEM, supplySystemID)

def __findPathWithFuelToSystemWithPossibleReturn(fromSystemAITarget, toSystemAITarget, empireID, resultSystemAITargets, fleetSupplyableSystemIDs, maxFuel, fuel, supplySystemAITarget):
    "returns system AITargets required to visit with fuel to nearest supplied system"

    result = True
    # try to find if there is possible path to wanted system from system
    newTargets = resultSystemAITargets[:]
    if fromSystemAITarget.isValid() and toSystemAITarget.isValid() and supplySystemAITarget.isValid():
        universe = fo.getUniverse()
        leastJumpsPath = universe.leastJumpsPath(fromSystemAITarget.getTargetID(), toSystemAITarget.getTargetID(), empireID)
        fromSystemID = fromSystemAITarget.getTargetID()
        for systemID in leastJumpsPath:
            if not fromSystemID == systemID:
                if fromSystemID in fleetSupplyableSystemIDs:
                    # from supplied system fleet can travel without fuel consumption and also in this system refuels
                    fuel = maxFuel
                else:
                    fuel = fuel - 1

                # leastJumpPath can differ from shortestPath
                # TODO: use Graph Theory to optimize
                if True or ((not systemID == toSystemAITarget.getTargetID()) and (systemID in fleetSupplyableSystemIDs)):#TODO:   restructure
                    newTargets.append(AITarget.AITarget(AITargetType.TARGET_SYSTEM, systemID))

                if fuel < 0:
                    result = False

            fromSystemID = systemID
    else:
        result = False

    # if there is path to wanted system, then also if there is path back to supplyable system
    if result == True:
        # jump from A to B means leastJumpsPath=[A,B], but minJumps=1
        minJumps = len(universe.leastJumpsPath(toSystemAITarget.getTargetID(), supplySystemAITarget.getTargetID(), empireID)) - 1

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
    suppliedSystemAITarget = getNearestSuppliedSystem(currentSystemAITarget.getTargetID(), empireID)

    # create resupply AIFleetOrder
    aiFleetOrder = AIFleetOrder.AIFleetOrder(AIFleetOrderType.ORDER_RESUPPLY, fleetAITarget, suppliedSystemAITarget)

    return aiFleetOrder
