import freeOrionAIInterface as fo
from EnumsAI import AITargetType, AIFleetOrderType
import AITarget
import AIFleetOrder

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
        systemAITargets = canTravelToSystemAndReturnToResupply(fleetAITarget.getTargetID(), lastSystemAITarget, aiTarget, empireID)
        if len(systemAITargets) > 0:
            # for every system required to visit create move order
            for systemAITarget in systemAITargets:
                # remember last system which will be visited
                lastSystemAITarget = systemAITarget
                # create move order
                aiFleetOrder = AIFleetOrder.AIFleetOrder(AIFleetOrderType.ORDER_MOVE, fleetAITarget, systemAITarget)
                result.append(aiFleetOrder)
        else:
            print "fleetID: " + str(fleetAITarget.getTargetID()) + " can't travel to target:" + str(aiTarget)
        
    return result

def canTravelToSystemAndReturnToResupply(fleetID, fromSystemAITarget, toSystemAITarget, empireID):
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
        
        # try to find path without going resupply first
        supplySystemID = getNearestSuppliedSystem(toSystemAITarget.getTargetID(), empireID)
        systemAITargets = __findPathWithFuelToSystemWithPossibleReturn(fromSystemAITarget, toSystemAITarget, empireID, systemAITargets, fleetSupplyableSystemIDs, maxFuel, fuel, supplySystemID)
        # resupply in system first is required to find path
        if not(fromSystemAITarget.getTargetID() in fleetSupplyableSystemIDs) and len(systemAITargets)==0:
            # add supply system to visit
            supplySystemID = getNearestSuppliedSystem(fromSystemAITarget.getTargetID(), empireID)
            fromSystemAITarget = AITarget.AITarget(AITargetType.TARGET_SYSTEM, supplySystemID)
            systemAITargets.append(fromSystemAITarget)
            # find path from supplied system to wanted system
            supplySystemID = getNearestSuppliedSystem(toSystemAITarget.getTargetID(), empireID)
            systemAITargets = __findPathWithFuelToSystemWithPossibleReturn(fromSystemAITarget, toSystemAITarget, empireID, systemAITargets, fleetSupplyableSystemIDs, maxFuel, maxFuel, supplySystemID)
        
    return systemAITargets

def getNearestSuppliedSystem(startSystemID, empireID):
    "returns systemID of nearest supplied system from starting system startSystemID"
    
    empire = fo.getEmpire()
    fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
    universe = fo.getUniverse()
    
    if startSystemID in fleetSupplyableSystemIDs:
        return startSystemID
    else:
        minJumps = 9999 # infinity
        supplySystemID = -1
        for systemID in fleetSupplyableSystemIDs:
            leastJumpsPath = universe.leastJumpsPath(startSystemID, systemID, empireID)
            if len(leastJumpsPath) < minJumps:
                minJumps = len(leastJumpsPath)
                supplySystemID = systemID
        
        return supplySystemID

def __findPathWithFuelToSystemWithPossibleReturn(fromSystemAITarget, toSystemAITarget, empireID, resultSystemAITargets, fleetSupplyableSystemIDs, maxFuel, fuel, supplySystemID):
    "returns system AITargets required to visit with fuel to nearest supplied system"

    result = True
    # try to find if there is possible path to wanted system from system
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
            if (not systemID == toSystemAITarget.getTargetID()) and (systemID in fleetSupplyableSystemIDs):
                resultSystemAITargets.append(AITarget.AITarget(AITargetType.TARGET_SYSTEM, systemID))
                
            if fuel < 0:
                result = False
                
        fromSystemID = systemID 
    
    # if there is path to wanted system, then also if there is path back to supplyable system
    if result == True:
        # jump from A to B means leastJumpsPath=[A,B], but minJumps=1
        minJumps = len(universe.leastJumpsPath(toSystemAITarget.getTargetID(), supplySystemID, empireID)) - 1

        if minJumps > fuel:
            # print "fleetID:" + str(fleetID) + " fuel:" + str(fuel) + " required: " + str(minJumps)
            result = False
        else:
            resultSystemAITargets.append(toSystemAITarget)
    
    if result == False:
        resultSystemAITargets = []
    
    return resultSystemAITargets

def getResupplyAIFleetOrder(fleetAITarget, currentSystemAITarget):
    "returns resupply AIFleetOrder to nearest supplied system"
    
    # find nearest supplied system
    empireID = fo.empireID()
    suppliedSystemID = getNearestSuppliedSystem(currentSystemAITarget.getTargetID(), empireID)
    suppliedSystemAITarget = AITarget.AITarget(AITargetType.TARGET_SYSTEM, suppliedSystemID)
    
    # create resupply AIFleetOrder
    aiFleetOrder = AIFleetOrder.AIFleetOrder(AIFleetOrderType.ORDER_RESUPPLY, fleetAITarget, suppliedSystemAITarget)
    
    return aiFleetOrder
