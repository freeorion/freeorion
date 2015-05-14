import math
import traceback
import random
import freeOrionAIInterface as fo  # pylint: disable=import-error
import AIstate
import FleetUtilsAI
import FreeOrionAI as foAI
import PlanetUtilsAI
import PriorityAI
import ColonisationAI
import EnumsAI
import MilitaryAI
import ShipDesignAI
import time
import cProfile, pstats, StringIO
from freeorion_tools import dict_from_map, ppstring
from TechsListsAI import EXOBOT_TECH_NAME
from freeorion_tools import print_error

best_military_design_rating_cache = {}  # indexed by turn, values are rating of the military design of the turn
design_cost_cache = {0: {(-1, -1): 0}} #outer dict indexed by cur_turn (currently only one turn kept); inner dict indexed by (design_id, pid)
shipTypeMap = {EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_EXPLORATION: EnumsAI.AIShipDesignTypes.explorationShip,
               EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_OUTPOST: EnumsAI.AIShipDesignTypes.outpostShip,
               EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_ORBITAL_OUTPOST: EnumsAI.AIShipDesignTypes.outpostBase,
               EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION: EnumsAI.AIShipDesignTypes.colonyShip,
               EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_ORBITAL_COLONISATION: EnumsAI.AIShipDesignTypes.colonyBase,
               EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_INVASION: EnumsAI.AIShipDesignTypes.troopShip,
               EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY: EnumsAI.AIShipDesignTypes.attackShip,
               EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_ORBITAL_DEFENSE: EnumsAI.AIShipDesignTypes.defenseBase,
               EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_ORBITAL_INVASION: EnumsAI.AIShipDesignTypes.troopBase,
               }

design_cache = {}  # dict of tuples (rating,pid,designID,cost) sorted by rating and indexed by priority type


def find_best_designs_this_turn():
    """Calculate the best designs for each ship class available at this turn."""
    pr = cProfile.Profile()
    pr.enable()
    start = time.clock()
    ShipDesignAI.Cache.update_for_new_turn()
    design_cache.clear()
    design_cache[EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY] = ShipDesignAI.MilitaryShipDesigner().optimize_design()
    design_cache[EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_ORBITAL_INVASION] = ShipDesignAI.OrbitalTroopShipDesigner().optimize_design()
    design_cache[EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_INVASION] = ShipDesignAI.StandardTroopShipDesigner().optimize_design()
    design_cache[EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION] = ShipDesignAI.StandardColonisationShipDesigner().optimize_design()
    design_cache[EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_ORBITAL_COLONISATION] = ShipDesignAI.OrbitalColonisationShipDesigner().optimize_design()
    design_cache[EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_OUTPOST] = ShipDesignAI.StandardOutpostShipDesigner().optimize_design()
    design_cache[EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_ORBITAL_OUTPOST] = ShipDesignAI.OrbitalOutpostShipDesigner().optimize_design()
    design_cache[EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_ORBITAL_DEFENSE] = ShipDesignAI.OrbitalDefenseShipDesigner().optimize_design()
    design_cache[EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_EXPLORATION] = ShipDesignAI.ScoutShipDesigner().optimize_design()
    end = time.clock()
    print "DEBUG INFORMATION: The design evaluations took %f s" % (end-start)
    print "-----"
    pr.disable()
    s = StringIO.StringIO()
    sortby = 'cumulative'
    ps = pstats.Stats(pr, stream=s).sort_stats(sortby)
    ps.print_stats()
    print s.getvalue()
    print "-----"
    if fo.currentTurn() % 10 == 0:
        ShipDesignAI.Cache.print_best_designs()


def get_design_cost(design, pid):  # TODO: Use new framework
    """Find and return the design_cost of the specified design on the specified planet.

    :param design:
    :param pid: int: planet id
    :return: cost of the design
    """
    cur_turn = fo.currentTurn()
    if cur_turn in design_cost_cache:
        cost_cache = design_cost_cache[cur_turn]
    else:
        design_cost_cache.clear()
        cost_cache = {}
        design_cost_cache[cur_turn] = cost_cache
    loc_invariant = True  # TODO: check actual loc invariance of design cost
    if loc_invariant:
        loc = -1
    else:
        loc = pid
    return cost_cache.setdefault((design.id, loc), design.productionCost(fo.empireID(), pid))


def cur_best_military_design_rating():
    """Find and return the default combat rating of our best military design.

    :return: float: rating of the best military design
    """
    current_turn = fo.currentTurn()
    if current_turn in best_military_design_rating_cache:
        return best_military_design_rating_cache[current_turn]
    priority = EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY
    if priority in design_cache:
        if design_cache[priority] and design_cache[priority][0]:
            rating, pid, design_id, cost = design_cache[priority][0]
            pilots = fo.getUniverse().getPlanet(pid).speciesName
            ship_id = -1  # no existing ship
            design_rating = foAI.foAIstate.rate_psuedo_fleet(ship_info=[(ship_id, design_id, pilots)])['overall']
            best_military_design_rating_cache[current_turn] = design_rating
            return max(design_rating, 0.001)
        else:
            return 0.001
    else:
        return 0.001


def getBestShipInfo(priority,loc=None):
    """ Returns 3 item tuple: designID, design, buildLocList."""
    if loc is None:
        planet_ids = ColonisationAI.empire_shipyards
    elif isinstance(loc, list):
        planet_ids = set(loc).intersection(ColonisationAI.empire_shipyards)
    elif isinstance(loc, int):
        if loc in ColonisationAI.empire_shipyards:
            planet_ids = [loc]
        else:
            return None, None, None
    else:  # problem
        return None, None, None
    if priority in design_cache:
        best_designs = design_cache[priority]
        if not best_designs:
            return None, None, None

        for design_stats in best_designs:
            top_rating, pid, top_id, cost = design_stats
            if pid in planet_ids:
                break
        valid_locs = [item[1] for item in best_designs if item[0] == top_rating and item[2] == top_id]
        return top_id, fo.getShipDesign(top_id), valid_locs
    else:
        return None, None, None  # must be missing a Shipyard or other orbital (or missing tech)


def getBestShipRatings(loc=None):
    """returns list of [partition, pid, designID, design] sublists, currently only for military ships"""
    priority = EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY
    if loc is None:
        planet_ids = ColonisationAI.empire_shipyards
    elif isinstance(loc, list):
        planet_ids = set(loc).intersection(ColonisationAI.empire_shipyards)
    elif isinstance(loc, int):
        if loc in ColonisationAI.empire_shipyards:
            planet_ids = [loc]
        else:
            return []
    else:  # problem
        return []

    if priority in design_cache:  # use new framework
        build_choices = design_cache[priority]
        loc_choices = [[item[0], item[1], item[2], fo.getShipDesign(item[2])]
                       for item in build_choices if item[1] in planet_ids]
        if not loc_choices:
            return []
        best_rating = loc_choices[0][0]
        p_sum = 0
        ret_val = []
        for choice in loc_choices:
            if choice[0] < 0.7*best_rating:
                break
            p = math.exp(10*(choice[0]/best_rating - 1))
            p_sum += p
            ret_val.append([p_sum, choice[1], choice[2], choice[3]])
        for item in ret_val:
            item[0] /= p_sum
        return ret_val
    else:
        return []


def generateProductionOrders():
    """generate production orders"""
    # first check ship designs
    # next check for buildings etc that could be placed on queue regardless of locally available PP
    # next loop over resource groups, adding buildings & ships
    empire = fo.getEmpire()
    universe = fo.getUniverse()
    capitolID = PlanetUtilsAI.get_capital()
    if capitolID is None or capitolID == -1:
        homeworld=None
        capitolSysID=None
    else:
        homeworld = universe.getPlanet(capitolID)
        capitolSysID = homeworld.systemID
    print "Production Queue Management:"
    empire = fo.getEmpire()
    productionQueue = empire.productionQueue
    totalPP = empire.productionPoints
    #prodResPool = empire.getResourcePool(fo.resourceType.industry)
    #availablePP = dict_from_map(productionQueue.availablePP(prodResPool))
    #allocatedPP = dict_from_map(productionQueue.allocatedPP)
    #objectsWithWastedPP = productionQueue.objectsWithWastedPP(prodResPool)
    currentTurn = fo.currentTurn()
    print
    print "  Total Available Production Points: " + str(totalPP)

    if empire.productionPoints <100:
        backupFactor = 0.0
    else:
        backupFactor = min(1.0, (empire.productionPoints/200.0)**2 )

    claimedStars= foAI.foAIstate.misc.get('claimedStars', {} )
    if claimedStars == {}:
        for sType in AIstate.empireStars:
            claimedStars[sType] = list( AIstate.empireStars[sType] )
        for sysID in set( AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs):
            tSys = universe.getSystem(sysID)
            if not tSys: continue
            claimedStars.setdefault( tSys.starType, []).append(sysID)

    if (currentTurn in [1, 4]) and ((productionQueue.totalSpent < totalPP) or (len(productionQueue) <=3)):
        bestDesignID, bestDesign, buildChoices = getBestShipInfo(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_EXPLORATION)
        if len(AIstate.opponentPlanetIDs) == 0:
            init_scouts = 3
        else:
            init_scouts = 0
        if bestDesignID:
            for scout_count in range(init_scouts):
                retval = fo.issueEnqueueShipProductionOrder(bestDesignID, buildChoices[0])
        fo.updateProductionQueue()

    movedCapital=False
    bldgExpense=0.0
    bldgRatio = [ 0.4, 0.35, 0.30 ][fo.empireID()%3]
    print "Buildings present on all owned planets:"
    for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
        planet = universe.getPlanet(pid)
        if planet:
            print "%30s: %s"%(planet.name, [universe.getObject(bldg).name for bldg in planet.buildingIDs])
    print

    if not homeworld:
        print "if no capitol, no place to build, should get around to capturing or colonizing a new one"#TODO
    else:
        print "Empire ID %d has current Capital %s:"%(empire.empireID, homeworld.name )
        print "Buildings present at empire Capital (ID, Name, Type, Tags, Specials, OwnedbyEmpire):"
        for bldg in homeworld.buildingIDs:
            thisObj=universe.getObject(bldg)
            tags=",".join( thisObj.tags)
            specials=",".join(thisObj.specials)
            print "%8s | %20s | type:%20s | tags:%20s | specials: %20s | owner:%d "%(bldg, thisObj.name, "_".join(thisObj.buildingTypeName.split("_")[-2:])[:20], tags, specials, thisObj.owner )
        print 
        capitalBldgs = [universe.getObject(bldg).buildingTypeName for bldg in homeworld.buildingIDs]

        #possibleBuildingTypeIDs = [bldTID for bldTID in empire.availableBuildingTypes if fo.getBuildingType(bldTID).canBeProduced(empire.empireID, homeworld.id)]
        possibleBuildingTypeIDs = []
        for bldTID in empire.availableBuildingTypes:
            try:
                if fo.getBuildingType(bldTID).canBeProduced(empire.empireID, homeworld.id):
                    possibleBuildingTypeIDs.append(bldTID)
            except:
                if fo.getBuildingType(bldTID) is None:
                    print "For empire %d, 'available Building Type ID' %s returns Nonetype from fo.getBuildingType(bldTID)"%(empire.empireID, bldTID)
                else:
                    print "For empire %d, problem getting BuildingTypeID for 'available Building Type ID' %s"%(empire.empireID, bldTID)
        if possibleBuildingTypeIDs:
            print "Possible building types to build:"
            for buildingTypeID in possibleBuildingTypeIDs:
                buildingType = fo.getBuildingType(buildingTypeID)
                #print "buildingType object:", buildingType
                #print "dir(buildingType): ", dir(buildingType)
                print "    " + str(buildingType.name) + " cost: " +str(buildingType.productionCost(empire.empireID, homeworld.id)) + " time: " + str(buildingType.productionTime(empire.empireID, homeworld.id))

            possibleBuildingTypes = [fo.getBuildingType(buildingTypeID) and fo.getBuildingType(buildingTypeID).name for buildingTypeID in possibleBuildingTypeIDs ] #makes sure is not None before getting name

            print
            print "Buildings already in Production Queue:"
            capitolQueuedBldgs=[]
            queued_exobot_locs = []
            for element in [e for e in productionQueue if (e.buildType == EnumsAI.AIEmpireProductionTypes.BT_BUILDING)]:
                bldgExpense += element.allocation
                if element.locationID==homeworld.id:
                    capitolQueuedBldgs.append ( element )
                if element.name == "BLD_COL_EXOBOT":
                    queued_exobot_locs.append(element.locationID)
            for bldg in capitolQueuedBldgs:
                print "    " + bldg.name + " turns:" + str(bldg.turnsLeft) + " PP:" + str(bldg.allocation)
            if not capitolQueuedBldgs: print "None"
            print
            queuedBldgNames=[ bldg.name for bldg in capitolQueuedBldgs ]

            if ( totalPP >40 or ((currentTurn > 40 ) and (ColonisationAI.empire_status.get('industrialists', 0) >= 20 ))) and ("BLD_INDUSTRY_CENTER" in possibleBuildingTypes) and ("BLD_INDUSTRY_CENTER" not in (capitalBldgs+queuedBldgNames)) and (bldgExpense<bldgRatio*totalPP):
                res=fo.issueEnqueueBuildingProductionOrder("BLD_INDUSTRY_CENTER", homeworld.id)
                print "Enqueueing BLD_INDUSTRY_CENTER, with result %d"%res
                if res:
                    cost, time = empire.productionCostAndTime( productionQueue[productionQueue.size -1] )
                    bldgExpense += cost/time

            if ("BLD_SHIPYARD_BASE" in possibleBuildingTypes) and ("BLD_SHIPYARD_BASE" not in (capitalBldgs+queuedBldgNames)):
                try:
                    res=fo.issueEnqueueBuildingProductionOrder("BLD_SHIPYARD_BASE", homeworld.id)
                    print "Enqueueing BLD_SHIPYARD_BASE, with result %d"%res
                except:
                    print "Error: cant build shipyard at new capital, probably no population; we're hosed"
                    print "Error: exception triggered and caught: ", traceback.format_exc()

            for bldName in [ "BLD_SHIPYARD_ORG_ORB_INC" ]:
                if (bldName in possibleBuildingTypes) and (bldName not in (capitalBldgs+queuedBldgNames)) and (bldgExpense<bldgRatio*totalPP):
                    try:
                        res=fo.issueEnqueueBuildingProductionOrder(bldName, homeworld.id)
                        print "Enqueueing %s at capitol, with result %d"%(bldName, res)
                        if res:
                            cost, time = empire.productionCostAndTime( productionQueue[productionQueue.size -1] )
                            bldgExpense += cost/time
                            res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                            print "Requeueing %s to front of build queue, with result %d"%(bldName, res)
                    except:
                        print "Error: exception triggered and caught: ", traceback.format_exc()

            for bldName in [ "BLD_SHIPYARD_ORG_XENO_FAC", "BLD_SHIPYARD_ORG_CELL_GRO_CHAMB"   ]:
                if ( totalPP >30 or currentTurn > 30 ) and (bldName in possibleBuildingTypes) and (bldName not in (capitalBldgs+queuedBldgNames)) and (bldgExpense<bldgRatio*totalPP):
                    try:
                        res=fo.issueEnqueueBuildingProductionOrder(bldName, homeworld.id)
                        print "Enqueueing %s at capitol, with result %d"%(bldName, res)
                        if res:
                            cost, time = empire.productionCostAndTime( productionQueue[productionQueue.size -1] )
                            bldgExpense += cost/time
                            res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                            print "Requeueing %s to front of build queue, with result %d"%(bldName, res)
                    except:
                        print "Error: exception triggered and caught: ", traceback.format_exc()

            numExobotShips=0 #TODO: do real calc here
            num_queued_exobots = len(queued_exobot_locs)
            if empire.techResearched(EXOBOT_TECH_NAME) and num_queued_exobots < 2:
                potential_locs = []
                for pid, (score, this_spec) in foAI.foAIstate.colonisablePlanetIDs.items():
                    if this_spec == "SP_EXOBOT" and pid not in queued_exobot_locs:
                        candidate = universe.getPlanet(pid)
                        if candidate.systemID in empire.supplyUnobstructedSystems:
                            potential_locs.append( (score, pid) )
                if potential_locs:
                    candidate_id = sorted(potential_locs)[-1][-1]
                    res=fo.issueEnqueueBuildingProductionOrder("BLD_COL_EXOBOT", candidate_id)
                    print "Enqueueing BLD_COL_EXOBOT, with result %d"%res
                    #if res:
                    #    res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                    #    print "Requeueing %s to front of build queue, with result %d"%("BLD_COL_EXOBOT", res)

            if ("BLD_IMPERIAL_PALACE" in possibleBuildingTypes) and ("BLD_IMPERIAL_PALACE" not in (capitalBldgs+queuedBldgNames)):
                res=fo.issueEnqueueBuildingProductionOrder("BLD_IMPERIAL_PALACE", homeworld.id)
                print "Enqueueing BLD_IMPERIAL_PALACE at %s, with result %d"%(homeworld.name, res)
                if res:
                    res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                    print "Requeueing BLD_IMPERIAL_PALACE to front of build queue, with result %d"%res


            # ok, BLD_NEUTRONIUM_SYNTH is not currently unlockable, but just in case... ;-p
            if ("BLD_NEUTRONIUM_SYNTH" in possibleBuildingTypes) and ("BLD_NEUTRONIUM_SYNTH" not in (capitalBldgs+queuedBldgNames)):
                res=fo.issueEnqueueBuildingProductionOrder("BLD_NEUTRONIUM_SYNTH", homeworld.id)
                print "Enqueueing BLD_NEUTRONIUM_SYNTH, with result %d"%res
                if res:
                    res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                    print "Requeueing BLD_NEUTRONIUM_SYNTH to front of build queue, with result %d"%res


#TODO: add totalPP checks below, so don't overload queue

    maxDefensePortion = [0.7, 0.4, 0.3, 0.2, 0.1, 0.0 ][ foAI.foAIstate.aggression]
    aggrIndex=max(1, foAI.foAIstate.aggression)
    if ( (currentTurn % aggrIndex)==0) and foAI.foAIstate.aggression < fo.aggression.maniacal:
        sysOrbitalDefenses={}
        queuedDefenses={}
        defenseAllocation=0.0
        targetOrbitals= min( int( ((currentTurn+4)/( 8.0* aggrIndex **1.5))**0.8) , fo.aggression.maniacal - aggrIndex )
        print "Orbital Defense Check -- target Defense Orbitals: ", targetOrbitals
        for element in productionQueue:
            if ( element.buildType == EnumsAI.AIEmpireProductionTypes.BT_SHIP) and (foAI.foAIstate.get_ship_role(element.designID) == EnumsAI.AIShipRoleType.SHIP_ROLE_BASE_DEFENSE):
                bldPlanet = universe.getPlanet(element.locationID)
                if not bldPlanet:
                    print "Error: Problem getting Planet for build loc %s"%element.locationID
                    continue
                sysID = bldPlanet.systemID
                queuedDefenses[sysID] = queuedDefenses.get( sysID, 0) + element.blocksize*element.remaining
                defenseAllocation += element.allocation
        print "Queued Defenses:", [( ppstring(PlanetUtilsAI.sys_name_ids([sysID])), num) for sysID, num in queuedDefenses.items()]
        for sysID in ColonisationAI.empire_species_systems:
            if foAI.foAIstate.systemStatus.get(sysID, {}).get('fleetThreat', 1) > 0:
                continue#don't build orbital shields if enemy fleet present
            if defenseAllocation > maxDefensePortion * totalPP:
                break
            #print "checking ", ppstring(PlanetUtilsAI.sys_name_ids([sysID]))
            sysOrbitalDefenses[sysID]=0
            fleetsHere = foAI.foAIstate.systemStatus.get(sysID, {}).get('myfleets', [])
            for fid in fleetsHere:
                if foAI.foAIstate.get_fleet_role(fid)==EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE:
                    print "Found %d existing Orbital Defenses in %s :"%(foAI.foAIstate.fleetStatus.get(fid, {}).get('nships', 0), ppstring(PlanetUtilsAI.sys_name_ids([sysID])))
                    sysOrbitalDefenses[sysID] += foAI.foAIstate.fleetStatus.get(fid, {}).get('nships', 0)
            for pid in ColonisationAI.empire_species_systems.get(sysID, {}).get('pids', []):
                sysOrbitalDefenses[sysID] += queuedDefenses.get(pid, 0)
            if sysOrbitalDefenses[sysID] < targetOrbitals:
                numNeeded = targetOrbitals - sysOrbitalDefenses[sysID]
                for pid in ColonisationAI.empire_species_systems.get(sysID, {}).get('pids', []):
                    bestDesignID, colDesign, buildChoices = getBestShipInfo(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_ORBITAL_DEFENSE, pid)
                    if not bestDesignID:
                        print "no orbital defenses can be built at ", ppstring(PlanetUtilsAI.planet_name_ids([pid]))
                        continue
                    #print "selecting ", ppstring(PlanetUtilsAI.planet_name_ids([pid])), " to build Orbital Defenses"
                    retval = fo.issueEnqueueShipProductionOrder(bestDesignID, pid)
                    print "queueing %d Orbital Defenses at %s"%(numNeeded, ppstring(PlanetUtilsAI.planet_name_ids([pid])))
                    if retval !=0:
                        if numNeeded > 1 :
                            fo.issueChangeProductionQuantityOrder(productionQueue.size -1, 1, numNeeded )
                        cost, time = empire.productionCostAndTime( productionQueue[productionQueue.size -1] )
                        defenseAllocation += productionQueue[productionQueue.size -1].blocksize * cost/time
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                        break


    bldType = fo.getBuildingType("BLD_SHIPYARD_BASE")
    queuedShipyardLocs = [element.locationID for element in productionQueue if (element.name=="BLD_SHIPYARD_BASE") ]
    systemColonies={}
    colonySystems={}
    for specName in ColonisationAI.empire_colonizers:
        if (len( ColonisationAI.empire_colonizers[specName])<=int(currentTurn/100)) and (specName in ColonisationAI.empire_species): #not enough current shipyards for this species#TODO: also allow orbital incubators and/or asteroid ships
            for pID in ColonisationAI.empire_species.get(specName, []): #SP_EXOBOT may not actually have a colony yet but be in empireColonizers
                if pID in queuedShipyardLocs:
                    break #won't try building more than one shipyard at once, per colonizer
            else:  #no queued shipyards, get planets with target pop >=3, and queue a shipyard on the one with biggest current pop
                planetList = zip( map( universe.getPlanet, ColonisationAI.empire_species[specName]), ColonisationAI.empire_species[specName] )
                pops = sorted( [ (planet.currentMeterValue(fo.meterType.population), pID) for planet, pID in planetList if ( planet and planet.currentMeterValue(fo.meterType.targetPopulation)>=3.0)] )
                pids = [pid for pop, pid in pops if bldType.canBeProduced(empire.empireID, pid)]
                if pids:
                    buildLoc= pids[-1]
                    res=fo.issueEnqueueBuildingProductionOrder("BLD_SHIPYARD_BASE", buildLoc)
                    print "Enqueueing BLD_SHIPYARD_BASE at planet %d (%s) for colonizer species %s, with result %d"%(buildLoc, universe.getPlanet(buildLoc).name, specName, res)
                    if res:
                        queuedShipyardLocs.append(buildLoc)
                        break # only start at most one new shipyard per species per turn
        for pid in ColonisationAI.empire_species.get(specName, []):
            planet=universe.getPlanet(pid)
            if planet:
                systemColonies.setdefault(planet.systemID, {}).setdefault('pids', []).append(pid)
                colonySystems[pid]=planet.systemID

    aciremaSystems={}
    for pid in ColonisationAI.empire_species.get("SP_ACIREMA", []):
        aciremaSystems.setdefault( universe.getPlanet(pid).systemID, [] ).append(pid)
        if (pid in queuedShipyardLocs ) or not bldType.canBeProduced(empire.empireID, pid) :
            continue #but not 'break' because we want to build shipyards at *every* Acirema planet
        res=fo.issueEnqueueBuildingProductionOrder("BLD_SHIPYARD_BASE", pid)
        print "Enqueueing BLD_SHIPYARD_BASE at planet %d (%s) for Acirema, with result %d"%(pid, universe.getPlanet(pid).name, res)
        if res:
            queuedShipyardLocs.append(pid)
            res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
            print "Requeueing Acirema BLD_SHIPYARD_BASE to front of build queue, with result %d"% res

    top_pilot_systems={}
    for pid, rating in ColonisationAI.pilot_ratings.items() :
        if (rating <= ColonisationAI.curMidPilotRating) and (rating < ColonisationAI.GREAT_PILOT_RATING):
            continue
        top_pilot_systems.setdefault( universe.getPlanet(pid).systemID, [] ).append( (pid, rating) )
        if (pid in queuedShipyardLocs ) or not bldType.canBeProduced(empire.empireID, pid) :
            continue #but not 'break' because we want to build shipyards all top pilot planets
        res=fo.issueEnqueueBuildingProductionOrder("BLD_SHIPYARD_BASE", pid)
        print "Enqueueing BLD_SHIPYARD_BASE at planet %d (%s) for top pilot, with result %d"%(pid, universe.getPlanet(pid).name, res)
        if res:
            queuedShipyardLocs.append(pid)
            res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
            print "Requeueing BLD_SHIPYARD_BASE to front of build queue, with result %d"% res

    popCtrs = list(AIstate.popCtrIDs)
    enrgyShipyardLocs={}
    for bldName in [ "BLD_SHIPYARD_ENRG_COMP"  ]:
        if empire.buildingTypeAvailable(bldName):
            queuedBldLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
            bldType = fo.getBuildingType(bldName)
            blue_popctrs = [ (ColonisationAI.pilot_ratings.get(pid, 0), pid) for pid in popCtrs if colonySystems.get(pid, -1) in AIstate.empireStars.get(fo.starType.blue, [])]
            bh_popctrs = [ (ColonisationAI.pilot_ratings.get(pid, 0), pid) for pid in popCtrs if colonySystems.get(pid, -1) in AIstate.empireStars.get(fo.starType.blackHole, [])]
            for pilot_rating, pid in sorted(blue_popctrs, reverse = True) + sorted(bh_popctrs, reverse = True):
                if len(queuedBldLocs)>1: #build a max of 2 at once
                    break
                this_planet = universe.getPlanet(pid)
                if not (this_planet and this_planet.speciesName in ColonisationAI.empire_ship_builders): #TODO: also check that not already one for this spec in this sys
                    continue
                enrgyShipyardLocs.setdefault(this_planet.systemID, []).append( (pilot_rating, pid) )
                if pid not in queuedBldLocs and bldType.canBeProduced(empire.empireID, pid):
                    res=fo.issueEnqueueBuildingProductionOrder(bldName, pid)
                    print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName, pid, universe.getPlanet(pid).name, res)
                    if res:
                        queuedBldLocs.append(pid)
                        cost, time = empire.productionCostAndTime( productionQueue[productionQueue.size -1] )
                        bldgExpense += cost/time  # productionQueue[productionQueue.size -1].blocksize *
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                        print "Requeueing %s to front of build queue, with result %d"%(bldName, res)

    queuedShipyards=[]
    bldName = "BLD_SHIPYARD_BASE"
    if empire.buildingTypeAvailable(bldName) and (bldgExpense<bldgRatio*totalPP) and ( totalPP >50 or currentTurn > 80 ):
        bldType = fo.getBuildingType(bldName)
        for sys_id in enrgyShipyardLocs: #Todo ensure only one or 2 per sys
            for pilot_rating, pid in sorted(enrgyShipyardLocs[sys_id], reverse=True)[:2]:
                if pid not in queuedShipyardLocs and bldType.canBeProduced(empire.empireID, pid):#TODO: verify that canBeProduced() checks for prexistence of a barring building
                    res=fo.issueEnqueueBuildingProductionOrder(bldName, pid)
                    print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName, pid, universe.getPlanet(pid).name, res)
                    if res:
                        queuedShipyardLocs.append(pid)
                        cost, time = empire.productionCostAndTime( productionQueue[productionQueue.size -1] )
                        bldgExpense += cost/time  # productionQueue[productionQueue.size -1].blocksize *
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                        print "Requeueing %s to front of build queue, with result %d"%(bldName, res)
                        break #only start one per turn

    for bldName in [ "BLD_SHIPYARD_ORG_ORB_INC" , "BLD_SHIPYARD_ORG_XENO_FAC" ]:
        if empire.buildingTypeAvailable(bldName) and (bldgExpense<bldgRatio*totalPP) and ( totalPP >40 or currentTurn > 40 ):
            queuedBldLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
            bldType = fo.getBuildingType(bldName)
            for pid in popCtrs:
                if len(queuedBldLocs)>1+int(totalPP/200.0) : # limit build at once
                    break
                if pid not in queuedBldLocs and bldType.canBeProduced(empire.empireID, pid):#TODO: verify that canBeProduced() checks for prexistence of a barring building
                    res=fo.issueEnqueueBuildingProductionOrder(bldName, pid)
                    print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName, pid, universe.getPlanet(pid).name, res)
                    if res:
                        queuedBldLocs.append(pid)
                        cost, time = empire.productionCostAndTime( productionQueue[productionQueue.size -1] )
                        bldgExpense += cost/time  # productionQueue[productionQueue.size -1].blocksize *
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                        print "Requeueing %s to front of build queue, with result %d"%(bldName, res)

    for bldName in [ "BLD_SHIPYARD_ORG_CELL_GRO_CHAMB" ]:
        if empire.buildingTypeAvailable(bldName) and (bldgExpense<bldgRatio*totalPP) and ( totalPP >50 or currentTurn > 80 ):
            queuedBldLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
            bldType = fo.getBuildingType(bldName)
            for pid in popCtrs:
                if len(queuedBldLocs)>1: #build a max of 2 at once
                    break
                if pid not in queuedBldLocs and bldType.canBeProduced(empire.empireID, pid):#TODO: verify that canBeProduced() checks for prexistence of a barring building
                    res=fo.issueEnqueueBuildingProductionOrder(bldName, pid)
                    print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName, pid, universe.getPlanet(pid).name, res)
                    if res:
                        queuedBldLocs.append(pid)
                        cost, time = empire.productionCostAndTime( productionQueue[productionQueue.size -1] )
                        bldgExpense += cost/time  # productionQueue[productionQueue.size -1].blocksize *
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                        print "Requeueing %s to front of build queue, with result %d"%(bldName, res)

    ShipYardType = fo.getBuildingType("BLD_SHIPYARD_BASE")
    bldName = "BLD_SHIPYARD_AST"
    if empire.buildingTypeAvailable(bldName) and foAI.foAIstate.aggression > fo.aggression.beginner:
        queuedBldLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
        if not queuedBldLocs:
            bldType = fo.getBuildingType(bldName)
            asteroidSystems = {}
            asteroidYards = {}
            queuedSystems=set()
            shipyardSystems={}
            builderSystems={}
            for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
                planet=universe.getPlanet(pid)
                thisSpec = planet.speciesName
                sysID = planet.systemID
                if planet.size == fo.planetSize.asteroids and sysID in ColonisationAI.empire_species_systems:
                    asteroidSystems.setdefault(sysID, []).append(pid)
                    if (pid in queuedBldLocs) or (bldName in [universe.getObject(bldg).buildingTypeName for bldg in planet.buildingIDs]):
                        asteroidYards[sysID]=pid #shouldn't ever overwrite another, but ok if it did
                if thisSpec in ColonisationAI.empire_ship_builders:
                    if pid in ColonisationAI.empire_ship_builders[thisSpec]:
                        shipyardSystems.setdefault(sysID, []).append(pid)
                    else:
                        builderSystems.setdefault(sysID, []).append((planet.speciesName, pid))
            #check if we need to build another asteroid processor:
            # check if local shipyard to go with the asteroid processor
            yardLocs = []
            needYard ={}
            topPilotLocs = []
            for sysID in set(asteroidSystems.keys()).difference(asteroidYards.keys()):
                if sysID in top_pilot_systems:
                    for pid, rating in top_pilot_systems[sysID]:
                        if pid not in queuedShipyardLocs: #will catch it later if shipyard already present
                            topPilotLocs.append( (rating, pid, sysID) )
            topPilotLocs.sort(reverse=True)
            for rating, pid, sysID in topPilotLocs:
                if sysID not in yardLocs:
                    yardLocs.append(sysID ) #prioritize asteroid yards for acirema and/or other top pilots
                    for pid, rating in top_pilot_systems[sysID]:
                        if pid not in queuedShipyardLocs: #will catch it later if shipyard already present
                            needYard[sysID] = pid
            if ( not yardLocs ) and len(asteroidYards.values())<= int(currentTurn/50): # not yet building & not enough current locs, find a location to build one
                queuedYardSystems = set(PlanetUtilsAI.get_systems(queuedShipyardLocs))
                colonizerLocChoices=[]
                builderLocChoices=[]
                bldSystems = set(asteroidSystems.keys()).difference(asteroidYards.keys())
                for sysID in bldSystems.intersection(builderSystems.keys()):
                    for thisSpec, pid in builderSystems[sysID]:
                        if thisSpec in ColonisationAI.empire_colonizers:
                            if pid in (ColonisationAI.empire_colonizers[thisSpec]+queuedShipyardLocs):
                                colonizerLocChoices.insert(0, sysID)
                            else:
                                colonizerLocChoices.append(sysID)
                                needYard[sysID] = pid
                        else:
                            if pid in (ColonisationAI.empire_ship_builders.get(thisSpec, [])+queuedShipyardLocs):
                                builderLocChoices.insert(0, sysID)
                            else:
                                builderLocChoices.append(sysID)
                                needYard[sysID] = pid
                yardLocs.extend( (colonizerLocChoices+builderLocChoices)[:1] ) #add at most one of these non top pilot locs
            newYardCount=len(queuedBldLocs)
            for sysID in yardLocs : #build at most 2 new asteroid yards at a time
                if newYardCount >= 2:
                    break
                pid = asteroidSystems[sysID][0]
                if sysID in needYard:
                    pid2 = needYard[sysID]
                    if ShipYardType.canBeProduced(empire.empireID, pid2):
                        res=fo.issueEnqueueBuildingProductionOrder("BLD_SHIPYARD_BASE", pid2)
                        print "Enqueueing %s at planet %d (%s) to go with Aseroid Processor , with result %d"%("BLD_SHIPYARD_BASE", pid2, universe.getPlanet(pid2).name, res)
                        if res:
                            queuedShipyardLocs.append(pid2)
                            cost, time = empire.productionCostAndTime( productionQueue[productionQueue.size -1] )
                            bldgExpense += cost/time  # productionQueue[productionQueue.size -1].blocksize *
                            res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                            print "Requeueing %s to front of build queue, with result %d"%("BLD_SHIPYARD_BASE", res)
                if pid not in queuedBldLocs and bldType.canBeProduced(empire.empireID, pid):
                    res=fo.issueEnqueueBuildingProductionOrder(bldName, pid)
                    print "Enqueueing %s at planet %d (%s) , with result %d on turn %d"%(bldName, pid, universe.getPlanet(pid).name, res, currentTurn)
                    if res:
                        newYardCount += 1
                        queuedBldLocs.append(pid)
                        cost, time = empire.productionCostAndTime( productionQueue[productionQueue.size -1] )
                        bldgExpense += cost/time  # productionQueue[productionQueue.size -1].blocksize *
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                        print "Requeueing %s to front of build queue, with result %d"%(bldName, res)

    bldName = "BLD_GAS_GIANT_GEN"
    maxGGGs=1
    if empire.buildingTypeAvailable(bldName) and foAI.foAIstate.aggression > fo.aggression.beginner:
        queuedBldLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
        bldType = fo.getBuildingType(bldName)
        for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):#TODO: check to ensure that a resource center exists in system, or GGG would be wasted
            if pid not in queuedBldLocs and bldType.canBeProduced(empire.empireID, pid):#TODO: verify that canBeProduced() checks for prexistence of a barring building
                thisPlanet=universe.getPlanet(pid)
                if thisPlanet.systemID in ColonisationAI.empire_species_systems:
                    GGList=[]
                    canUseGGG=False
                    system=universe.getSystem(thisPlanet.systemID)
                    for opid in system.planetIDs:
                        otherPlanet=universe.getPlanet(opid)
                        if otherPlanet.size== fo.planetSize.gasGiant:
                            GGList.append(opid)
                        if opid!=pid and otherPlanet.owner==empire.empireID and (EnumsAI.AIFocusType.FOCUS_INDUSTRY in list(otherPlanet.availableFoci)+[otherPlanet.focus]):
                            canUseGGG=True
                    if pid in sorted(GGList)[:maxGGGs] and canUseGGG:
                        res=fo.issueEnqueueBuildingProductionOrder(bldName, pid)
                        if res:
                            queuedBldLocs.append(pid)
                            cost, time = empire.productionCostAndTime( productionQueue[productionQueue.size -1] )
                            bldgExpense += cost/time  # productionQueue[productionQueue.size -1].blocksize *
                            res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                            print "Requeueing %s to front of build queue, with result %d"%(bldName, res)
                        print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName, pid, universe.getPlanet(pid).name, res)

    bldName = "BLD_SOL_ORB_GEN"
    if empire.buildingTypeAvailable(bldName) and foAI.foAIstate.aggression > fo.aggression.turtle:
        bldType = fo.getBuildingType(bldName)
        alreadyGotOne=99
        for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
            planet=universe.getPlanet(pid)
            if planet and bldName in [bld.buildingTypeName for bld in map( universe.getObject, planet.buildingIDs)]:
                system = universe.getSystem(planet.systemID)
                if system and system.starType < alreadyGotOne:
                    alreadyGotOne = system.starType
        bestType=fo.starType.white
        best_locs = AIstate.empireStars.get(fo.starType.blue, []) + AIstate.empireStars.get(fo.starType.white, [])
        if not best_locs:
            bestType=fo.starType.orange
            best_locs = AIstate.empireStars.get(fo.starType.yellow, []) + AIstate.empireStars.get(fo.starType.orange, [])
        if ( not best_locs) or ( alreadyGotOne<99 and alreadyGotOne <= bestType ):
            pass # could consider building at a red star if have a lot of PP but somehow no better stars
        else:
            useNewLoc=True
            queuedBldLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
            if queuedBldLocs:
                queuedStarTypes = {}
                for loc in queuedBldLocs:
                    planet = universe.getPlanet(loc)
                    if not planet: continue
                    system = universe.getSystem(planet.systemID)
                    queuedStarTypes.setdefault(system.starType, []).append(loc)
                if queuedStarTypes:
                    bestQueued = sorted(queuedStarTypes.keys())[0]
                    if bestQueued > bestType:  # i.e., bestQueued is yellow, bestType available is blue or white
                        pass #should probably evaluate cancelling the existing one under construction
                    else:
                        useNewLoc=False
            if useNewLoc:  # (of course, may be only loc, not really new)
                if not homeworld:
                    useSys=best_locs[0]#as good as any
                else:
                    distanceMap={}
                    for sysID in best_locs: #want to build close to capitol for defense
                        if sysID == -1:
                            continue
                        try:
                            distanceMap[sysID] = universe.jumpDistance(homeworld.systemID, sysID)
                        except:
                            pass
                    useSys = ([(-1, -1)] + sorted( [ (dist, sysID) for sysID, dist in distanceMap.items() ] ))[:2][-1][-1]  # kinda messy, but ensures a value
                if useSys!= -1:
                    try:
                        useLoc = AIstate.colonizedSystems[useSys][0]
                        res=fo.issueEnqueueBuildingProductionOrder(bldName, useLoc)
                        print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName, useLoc, universe.getPlanet(useLoc).name, res)
                        if res:
                            cost, time = empire.productionCostAndTime( productionQueue[productionQueue.size -1] )
                            bldgExpense += cost/time  # productionQueue[productionQueue.size -1].blocksize *
                            res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                            print "Requeueing %s to front of build queue, with result %d"%(bldName, res)
                    except:
                        print "problem queueing BLD_SOL_ORB_GEN at planet", useLoc, "of system ", useSys
                        pass

    bldName = "BLD_ART_BLACK_HOLE"
    if ( ( empire.buildingTypeAvailable(bldName) ) and (foAI.foAIstate.aggression > fo.aggression.typical) and
                        (len( claimedStars.get(fo.starType.blackHole, [])) == 0 ) and (len( AIstate.empireStars.get(fo.starType.red, [])) > 0) ):
        bldType = fo.getBuildingType(bldName)
        alreadyGotOne=False
        for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
            planet=universe.getPlanet(pid)
            if planet and bldName in [bld.buildingTypeName for bld in map( universe.getObject, planet.buildingIDs)]:
                alreadyGotOne = True
        queuedBldLocs = [element.locationID for element in productionQueue if (element.name==bldName) ] #TODO: check that queued locs or already built one are at red stars
        if len (queuedBldLocs)==0 and not alreadyGotOne:  #
            if not homeworld:
                useSys= AIstate.empireStars[fo.starType.red][0]
            else:
                distanceMap={}
                for sysID in AIstate.empireStars.get(fo.starType.red, []):
                    if sysID == -1:
                        continue
                    try:
                        distanceMap[sysID] = universe.jumpDistance(homeworld.systemID, sysID)
                    except:
                        pass
                redSysList = sorted( [ (dist, sysID) for sysID, dist in distanceMap.items() ] )
                useLoc=None
                for dist, sysID in redSysList:
                    for loc in AIstate.colonizedSystems[sysID]:
                        planet=universe.getPlanet(loc)
                        if planet and planet.speciesName not in [ "", None ]:
                            species= fo.getSpecies(planet.speciesName)
                            if species and "PHOTOTROPHIC" in list(species.tags):
                                break
                    else:
                        if len(AIstate.colonizedSystems[sysID]) >0:
                            useLoc = AIstate.colonizedSystems[sysID][0]
                    if useLoc is not None:
                        break
                if useLoc is not None:
                    try:
                        res=fo.issueEnqueueBuildingProductionOrder(bldName, useLoc)
                        print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName, useLoc, universe.getPlanet(useLoc).name, res)
                        if res:
                            res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                            print "Requeueing %s to front of build queue, with result %d"%(bldName, res)
                    except:
                        print "problem queueing %s at planet"%bldName, useLoc, "of system ", useSys

    bldName = "BLD_BLACK_HOLE_POW_GEN"
    if empire.buildingTypeAvailable(bldName) and foAI.foAIstate.aggression > fo.aggression.cautious:
        bldType = fo.getBuildingType(bldName)
        alreadyGotOne=False
        for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
            planet=universe.getPlanet(pid)
            if planet and bldName in [bld.buildingTypeName for bld in map( universe.getObject, planet.buildingIDs)]:
                alreadyGotOne = True
        queuedBldLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
        if (len( AIstate.empireStars.get(fo.starType.blackHole, [])) > 0) and len (queuedBldLocs)==0 and not alreadyGotOne:  #
            if not homeworld:
                useSys= AIstate.empireStars.get(fo.starType.blackHole, [])[0]
            else:
                distanceMap={}
                for sysID in AIstate.empireStars.get(fo.starType.blackHole, []):
                    if sysID == -1:
                        continue
                    try:
                        distanceMap[sysID] = universe.jumpDistance(homeworld.systemID, sysID)
                    except:
                        pass
                useSys = ([(-1, -1)] + sorted( [ (dist, sysID) for sysID, dist in distanceMap.items() ] ))[:2][-1][-1]  # kinda messy, but ensures a value
            if useSys!= -1:
                try:
                    useLoc = AIstate.colonizedSystems[useSys][0]
                    res=fo.issueEnqueueBuildingProductionOrder(bldName, useLoc)
                    print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName, useLoc, universe.getPlanet(useLoc).name, res)
                    if res:
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                        print "Requeueing %s to front of build queue, with result %d"%(bldName, res)
                except:
                    print "problem queueing BLD_BLACK_HOLE_POW_GEN at planet", useLoc, "of system ", useSys
                    pass

    bldName = "BLD_ENCLAVE_VOID"
    if empire.buildingTypeAvailable(bldName):
        bldType = fo.getBuildingType(bldName)
        alreadyGotOne=False
        for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
            planet=universe.getPlanet(pid)
            if planet and bldName in [bld.buildingTypeName for bld in map( universe.getObject, planet.buildingIDs)]:
                alreadyGotOne = True
        queuedLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
        if len (queuedLocs)==0 and homeworld and not alreadyGotOne:  #
            try:
                res=fo.issueEnqueueBuildingProductionOrder(bldName, capitolID)
                print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName, capitolID, universe.getPlanet(capitolID).name, res)
                if res:
                    res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                    print "Requeueing %s to front of build queue, with result %d"%(bldName, res)
            except:
                pass

    bldName = "BLD_GENOME_BANK"
    if empire.buildingTypeAvailable(bldName):
        bldType = fo.getBuildingType(bldName)
        alreadyGotOne=False
        for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
            planet=universe.getPlanet(pid)
            if planet and bldName in [bld.buildingTypeName for bld in map( universe.getObject, planet.buildingIDs)]:
                alreadyGotOne = True
        queuedLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
        if len (queuedLocs)==0 and homeworld and not alreadyGotOne:  #
            try:
                res=fo.issueEnqueueBuildingProductionOrder(bldName, capitolID)
                print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName, capitolID, universe.getPlanet(capitolID).name, res)
                if res:
                    res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                    print "Requeueing %s to front of build queue, with result %d"%(bldName, res)
            except:
                pass

    bldName = "BLD_NEUTRONIUM_EXTRACTOR"
    alreadyGotExtractor=False
    if empire.buildingTypeAvailable(bldName) and ( [element.locationID for element in productionQueue if (element.name==bldName) ]==[]):
        #bldType = fo.getBuildingType(bldName)
        for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
            planet=universe.getPlanet(pid)
            if ( planet and (( planet.systemID in AIstate.empireStars.get(fo.starType.neutron, []) and
                                                                                                 (bldName in [bld.buildingTypeName for bld in map( universe.getObject, planet.buildingIDs)]) ) or
                                                                                                 ("BLD_NEUTRONIUM_SYNTH" in [bld.buildingTypeName for bld in map( universe.getObject, planet.buildingIDs)])
                                                                                                 )):
                alreadyGotExtractor = True
        if not alreadyGotExtractor:
            if not homeworld:
                useSys= AIstate.empireStars.get(fo.starType.neutron, [])[0]
            else:
                distanceMap={}
                for sysID in AIstate.empireStars.get(fo.starType.neutron, []):
                    if sysID == -1:
                        continue
                    try:
                        distanceMap[sysID] = universe.jumpDistance(homeworld.systemID, sysID, empire.empireID)
                    except:
                        pass
                print ([-1] + sorted( [ (dist, sysID) for sysID, dist in distanceMap.items() ] ))
                useSys = ([(-1, -1)] + sorted( [ (dist, sysID) for sysID, dist in distanceMap.items() ] ))[:2][-1][-1]  # kinda messy, but ensures a value
            if useSys!= -1:
                try:
                    useLoc = AIstate.colonizedSystems[useSys][0]
                    res=fo.issueEnqueueBuildingProductionOrder(bldName, useLoc)
                    print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName, useLoc, universe.getPlanet(useLoc).name, res)
                    if res:
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                        print "Requeueing %s to front of build queue, with result %d"%(bldName, res)
                except:
                    print "problem queueing BLD_NEUTRONIUM_EXTRACTOR at planet", useLoc, "of system ", useSys
                    pass

    bldName = "BLD_NEUTRONIUM_FORGE"
    if empire.buildingTypeAvailable(bldName) and foAI.foAIstate.aggression > fo.aggression.beginner:
        print "considering building a ", bldName
        if not alreadyGotExtractor:
            print "Apparently have no Neutronium_Extractors nor Sythesizers"
        else:
            queuedBldLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
            bldType = fo.getBuildingType(bldName)
            if len(queuedBldLocs) < 2 :  #don't build too many at once
                if homeworld:
                    tryLocs= [capitolID] + list(AIstate.popCtrIDs)
                else:
                    tryLocs= AIstate.popCtrIDs
                print "Possible locs for %s are: "%bldName, ppstring(PlanetUtilsAI.planet_name_ids(tryLocs))
                for pid in tryLocs:
                    if pid not in queuedBldLocs:
                        planet=universe.getPlanet(pid)
                        if bldType.canBeProduced(empire.empireID, pid):#TODO: verify that canBeProduced() checks for prexistence of a barring building
                            if bldName not in [bld.buildingTypeName for bld in map( universe.getObject, planet.buildingIDs)]:
                                if "BLD_SHIPYARD_BASE"  in [bld.buildingTypeName for bld in map( universe.getObject, planet.buildingIDs)]:
                                    res=fo.issueEnqueueBuildingProductionOrder(bldName, pid)
                                    print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName, pid, universe.getPlanet(pid).name, res)
                                    if res:
                                        if productionQueue.size > 5:
                                            res=fo.issueRequeueProductionOrder(productionQueue.size -1, 3) # move to front
                                            print "Requeueing %s to front of build queue, with result %d"%(bldName, res)
                                        break #only initiate max of one new build per turn

    colonyShipMap={}
    for fid in FleetUtilsAI.get_empire_fleet_ids_by_role( EnumsAI.AIFleetMissionType.FLEET_MISSION_COLONISATION):
        fleet = universe.getFleet(fid)
        if not fleet: continue
        for shipID in fleet.shipIDs:
            thisShip=universe.getShip(shipID)
            if thisShip and (foAI.foAIstate.get_ship_role(thisShip.design.id) ==EnumsAI.AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION):
                colonyShipMap.setdefault( thisShip.speciesName, []).append(1)

    bldName = "BLD_CONC_CAMP"
    verboseCamp = False
    bldType = fo.getBuildingType(bldName)
    for pid in AIstate.popCtrIDs:
        planet=universe.getPlanet(pid)
        if not planet:
            continue
        canBuildCamp = bldType.canBeProduced(empire.empireID, pid)
        tPop = planet.currentMeterValue(fo.meterType.targetPopulation)
        tInd=planet.currentMeterValue(fo.meterType.targetIndustry)
        cInd=planet.currentMeterValue(fo.meterType.industry)
        cPop = planet.currentMeterValue(fo.meterType.population)
        popDisqualified = (cPop <= 32) or (cPop < 0.9*tPop)
        checkedCamp=False
        builtCamp=False
        thisSpec = planet.speciesName
        safetyMarginMet = ( (thisSpec in ColonisationAI.empire_colonizers and ( len(ColonisationAI.empire_species.get(thisSpec, [])+colonyShipMap.get(thisSpec, []))>=2 )) or (cPop >= 50 ))
        if popDisqualified or not safetyMarginMet :  #check even if not aggressive, etc, just in case acquired planet with a ConcCamp on it
            if canBuildCamp:
                if popDisqualified:
                    if verboseCamp:
                        print "Conc Camp disqualified at %s due to low pop: current %.1f target: %.1f"%(planet.name, cPop, tPop)
                else:
                    if verboseCamp:
                        print "Conc Camp disqualified at %s due to safety margin; species %s, colonizing planets %s, with %d colony ships"%(planet.name, planet.speciesName, ColonisationAI.empire_species.get(planet.speciesName, []), len(colonyShipMap.get(planet.speciesName, [])))
            for bldg in planet.buildingIDs:
                if universe.getObject(bldg).buildingTypeName == bldName:
                    res=fo.issueScrapOrder( bldg)
                    print "Tried scrapping %s at planet %s, got result %d"%(bldName, planet.name, res)
        elif foAI.foAIstate.aggression>fo.aggression.typical and canBuildCamp and (tPop >= 36):
            checkedCamp=True
            if (planet.focus== EnumsAI.AIFocusType.FOCUS_GROWTH) or ("COMPUTRONIUM_SPECIAL" in planet.specials) or (pid == capitolID):
                continue
                #pass  # now that focus setting takes these into account, probably works ok to have conc camp, but let's not push it
            queuedBldLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
            if cPop <0.95*tPop:#
                if verboseCamp:
                    print "Conc Camp disqualified at %s due to pop: current %.1f target: %.1f"%(planet.name, cPop, tPop)
            else:
                if pid not in queuedBldLocs:
                    if planet.focus in [ EnumsAI.AIFocusType.FOCUS_INDUSTRY ]:
                        if cInd >= tInd+cPop:
                            continue
                    else:
                        oldFocus=planet.focus
                        fo.issueChangeFocusOrder(pid, EnumsAI.AIFocusType.FOCUS_INDUSTRY)
                        universe.updateMeterEstimates([pid])
                        tInd=planet.currentMeterValue(fo.meterType.targetIndustry)
                        if cInd >= tInd+cPop:
                            fo.issueChangeFocusOrder(pid, oldFocus)
                            universe.updateMeterEstimates([pid])
                            continue
                    res=fo.issueEnqueueBuildingProductionOrder(bldName, pid)
                    builtCamp = res
                    print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName, pid, universe.getPlanet(pid).name, res)
                    if res:
                        queuedBldLocs.append(pid)
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                    else:
                        #TODO: enable location condition reporting a la mapwnd BuildDesignatorWnd
                        print "Error enqueing Conc Camp at %s despite bldType.canBeProduced(empire.empireID, pid) reporting "%planet.name, canBuildCamp
        if verboseCamp:
            print "conc camp status at %s : checkedCamp: %s, builtCamp: %s"%( planet.name, canBuildCamp, builtCamp)

    bldName = "BLD_SCANNING_FACILITY"
    if empire.buildingTypeAvailable(bldName):
        bldType = fo.getBuildingType(bldName)
        queuedLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
        scannerLocs={}
        for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
            planet=universe.getPlanet(pid)
            if planet:
                if ( pid in queuedLocs ) or ( bldName in [bld.buildingTypeName for bld in map( universe.getObject, planet.buildingIDs)] ):
                    scannerLocs[planet.systemID] = True
        maxScannerBuilds = max(1, int(empire.productionPoints/30))
        for sysID in AIstate.colonizedSystems:
            if len(queuedLocs)>= maxScannerBuilds:
                break
            if sysID in scannerLocs:
                continue
            needScanner=False
            for nSys in dict_from_map(universe.getSystemNeighborsMap(sysID, empire.empireID)):
                if universe.getVisibility(nSys, empire.empireID) < fo.visibility.partial:
                    needScanner=True
                    break
            if not needScanner:
                continue
            buildLocs=[]
            for pid in AIstate.colonizedSystems[sysID]:
                planet=universe.getPlanet(pid)
                if not planet: continue
                buildLocs.append( (planet.currentMeterValue(fo.meterType.maxTroops), pid) )
            if not buildLocs: continue
            for troops, loc in sorted( buildLocs ):
                planet = universe.getPlanet(loc)
                res=fo.issueEnqueueBuildingProductionOrder(bldName, loc)
                print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName, loc, planet.name, res)
                if res:
                    res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                    print "Requeueing %s to front of build queue, with result %d"%(bldName, res)
                    queuedLocs.append( planet.systemID )
                    break

    bldName = "BLD_SHIPYARD_ORBITAL_DRYDOCK"
    if empire.buildingTypeAvailable(bldName):
        bldType = fo.getBuildingType(bldName)
        queued_locs = [element.locationID for element in productionQueue if (element.name==bldName) ]
        queued_sys = set()
        for pid in queued_locs:
            dd_planet = universe.getPlanet(pid)
            if dd_planet:
                queued_sys.add(dd_planet.systemID)
        cur_drydoc_sys = set(ColonisationAI.empire_dry_docks.keys()).union(queued_sys)
        covered_drydoc_locs = set()
        for start_set, dest_set in [ (cur_drydoc_sys, covered_drydoc_locs),
                                               (covered_drydoc_locs, covered_drydoc_locs) ]:  #coverage of neighbors up to 2 jumps away from a drydock
            for dd_sys_id in start_set.copy():
                dest_set.add(dd_sys_id)
                dd_neighbors = dict_from_map(universe.getSystemNeighborsMap(dd_sys_id, empire.empireID))
                dest_set.update( dd_neighbors.keys() )
            
        max_dock_builds = int(0.8 + empire.productionPoints/120.0)
        print "Considering building %s, found current and queued systems %s"%(bldName, ppstring(PlanetUtilsAI.sys_name_ids(cur_drydoc_sys.union(queued_sys))))
        #print "Empire shipyards found at %s"%(ppstring(PlanetUtilsAI.planet_name_ids(ColonisationAI.empireShipyards)))
        for sys_id, pids_dict in ColonisationAI.empire_species_systems.items(): #TODO: sort/prioritize in some fashion
            pids = pids_dict.get('pids', [])
            local_top_pilots = dict(top_pilot_systems.get(sys_id, []))
            local_drydocks = ColonisationAI.empire_dry_docks.get(sys_id, [])
            if len(queued_locs)>= max_dock_builds:
                print "Drydock enqueing halted with %d of max %d"%( len(queued_locs) , max_dock_builds )
                break
            if (sys_id in covered_drydoc_locs) and not local_top_pilots:
                continue
            else:
                #print "Considering %s at %s, with planet choices %s"%(bldName, ppstring(PlanetUtilsAI.sys_name_ids([sys_id]), pids))
                pass
            for _, pid in sorted([(local_top_pilots.get(pid, 0), pid) for pid in  pids], reverse = True):
                #print "checking planet '%s'"%pid
                if pid not in ColonisationAI.empire_shipyards:
                    #print "Planet %s not in empireShipyards"%(ppstring(PlanetUtilsAI.planet_name_ids([pid])))
                    continue
                if pid in local_drydocks or pid in queued_locs:
                    break
                planet = universe.getPlanet(pid)
                res=fo.issueEnqueueBuildingProductionOrder(bldName, pid)
                print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName, pid, planet.name, res)
                if res:
                    queued_locs.append( planet.systemID )
                    covered_drydoc_locs.add( planet.systemID )
                    dd_neighbors = dict_from_map(universe.getSystemNeighborsMap(planet.systemID, empire.empireID))
                    covered_drydoc_locs.update( dd_neighbors.keys() )
                    if max_dock_builds >= 2:
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                        print "Requeueing %s to front of build queue, with result %d"%(bldName, res)
                    break
                else:
                    print "Error failed enqueueing %s at planet %d (%s) , with result %d"%(bldName, pid, planet.name, res)

    bldName = "BLD_EVACUATION"
    bldType = fo.getBuildingType(bldName)
    for pid in AIstate.popCtrIDs:
        planet=universe.getPlanet(pid)
        if not planet:
            continue
        for bldg in planet.buildingIDs:
            if universe.getObject(bldg).buildingTypeName == bldName:
                res=fo.issueScrapOrder( bldg)
                print "Tried scrapping %s at planet %s, got result %d"%(bldName, planet.name, res)

    totalPPSpent = fo.getEmpire().productionQueue.totalSpent
    print "  Total Production Points Spent: " + str(totalPPSpent)

    wastedPP = max(0, totalPP - totalPPSpent)
    print "  Wasted Production Points: " + str(wastedPP)#TODO: add resource group analysis
    availPP = totalPP - totalPPSpent - 0.0001

    print
    if False:
        print "Possible ship designs to build:"
        if homeworld:
            for shipDesignID in empire.availableShipDesigns:
                shipDesign = fo.getShipDesign(shipDesignID)
                print "    " + str(shipDesign.name(True)) + " cost:" + str(shipDesign.productionCost(empire.empireID, homeworld.id) )+ " time:" + str(shipDesign.productionTime(empire.empireID, homeworld.id))

    print
    print "Projects already in Production Queue:"
    productionQueue = empire.productionQueue
    print "production summary: %s"%[elem.name for elem in productionQueue]
    queuedColonyShips={}
    queuedOutpostShips = 0
    queuedTroopShips=0

    #TODO: blocked items might not need dequeuing, but rather for supply lines to be un-blockaded
    dequeueList=[]
    fo.updateProductionQueue()
    can_prioritize_troops = False
    for queue_index in range( len(productionQueue)):
        element=productionQueue[queue_index]
        blockStr = "%d x "%element.blocksize  #["a single ", "in blocks of %d "%element.blocksize][element.blocksize>1]
        print "    " + blockStr + element.name+" requiring " + str(element.turnsLeft) + " more turns; alloc: %.2f PP"%element.allocation + " with cum. progress of %.1f"%element.progress + " being built at " + universe.getObject(element.locationID).name
        if element.turnsLeft == -1:
            if element.locationID not in AIstate.popCtrIDs+AIstate.outpostIDs:
                #dequeueList.append(queue_index) #TODO add assessment of recapture -- invasion target etc.
                #print "element %s will never be completed as stands and location %d no longer owned; deleting from queue "%(element.name, element.locationID)
                print "element %s will never be completed as stands and location %d no longer owned; could consider deleting from queue "%(element.name, element.locationID) #TODO:
            else:
                print "element %s is projected to never be completed as currently stands, but will remain on queue "%element.name
        elif element.buildType == EnumsAI.AIEmpireProductionTypes.BT_SHIP:
            this_role = foAI.foAIstate.get_ship_role(element.designID)
            if this_role == EnumsAI.AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION:
                thisSpec=universe.getPlanet(element.locationID).speciesName
                queuedColonyShips[thisSpec] = queuedColonyShips.get(thisSpec, 0) + element.remaining*element.blocksize
            elif this_role == EnumsAI.AIShipRoleType.SHIP_ROLE_CIVILIAN_OUTPOST:
                queuedOutpostShips+= element.remaining*element.blocksize
            elif this_role == EnumsAI.AIShipRoleType.SHIP_ROLE_BASE_OUTPOST:
                queuedOutpostShips+= element.remaining*element.blocksize
            elif this_role == EnumsAI.AIShipRoleType.SHIP_ROLE_MILITARY_INVASION:
                queuedTroopShips+= element.remaining*element.blocksize
            elif (this_role == EnumsAI.AIShipRoleType.SHIP_ROLE_CIVILIAN_EXPLORATION) and (queue_index <= 1):
                if len(AIstate.opponentPlanetIDs) > 0:
                    can_prioritize_troops = True
    if queuedColonyShips:
        print "\nFound colony ships in build queue: %s"%queuedColonyShips
    if queuedOutpostShips:
        print "\nFound outpost ships and bases in build queue: %s"% queuedOutpostShips

    for queue_index in dequeueList[::-1]:
        fo.issueDequeueProductionOrder(queue_index)

    allMilitaryFleetIDs = FleetUtilsAI.get_empire_fleet_ids_by_role(EnumsAI.AIFleetMissionType.FLEET_MISSION_MILITARY )
    nMilitaryTot = sum( [ foAI.foAIstate.fleetStatus.get(fid, {}).get('nships', 0) for fid in allMilitaryFleetIDs ] )
    allTroopFleetIDs = FleetUtilsAI.get_empire_fleet_ids_by_role(EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION )
    nTroopTot = sum( [ foAI.foAIstate.fleetStatus.get(fid, {}).get('nships', 0) for fid in allTroopFleetIDs ] )
    availTroopFleetIDs = list( FleetUtilsAI.extract_fleet_ids_without_mission_types(allTroopFleetIDs))
    nAvailTroopTot = sum( [ foAI.foAIstate.fleetStatus.get(fid, {}).get('nships', 0) for fid in availTroopFleetIDs ] )
    print "Trooper Status turn %d: %d total, with %d unassigned. %d queued, compared to %d total Military Attack Ships"%(currentTurn, nTroopTot,
                                                                                                                   nAvailTroopTot, queuedTroopShips, nMilitaryTot)
    if ( capitolID is not None and ((currentTurn>=40) or can_prioritize_troops) and foAI.foAIstate.systemStatus.get(capitolSysID, {}).get('fleetThreat', 0)==0 and
                                                                                           foAI.foAIstate.systemStatus.get(capitolSysID, {}).get('neighborThreat', 0)==0):
        bestDesignID, bestDesign, buildChoices = getBestShipInfo( EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_INVASION)
        if buildChoices is not None and len(buildChoices)>0:
            loc = random.choice(buildChoices)
            prodTime = bestDesign.productionTime(empire.empireID, loc)
            prodCost=bestDesign.productionCost(empire.empireID, loc)
            troopersNeededForcing = max(0, int( min(0.99+ (currentTurn/20.0 - nAvailTroopTot)/max(2, prodTime-1), nMilitaryTot/3 -nTroopTot)))
            numShips=troopersNeededForcing
            perTurnCost = (float(prodCost) / prodTime)
            if troopersNeededForcing>0 and totalPP > 3*perTurnCost*queuedTroopShips and foAI.foAIstate.aggression >= fo.aggression.typical:
                retval = fo.issueEnqueueShipProductionOrder(bestDesignID, loc)
                if retval !=0:
                    print "forcing %d new ship(s) to production queue: %s; per turn production cost %.1f"%(numShips, bestDesign.name(True), numShips*perTurnCost)
                    print
                    if numShips>1:
                        fo.issueChangeProductionQuantityOrder(productionQueue.size -1, 1, numShips)
                    availPP -= numShips*perTurnCost
                    res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                    fo.updateProductionQueue()
        print

    print
    # get the highest production priorities
    productionPriorities = {}
    for priorityType in EnumsAI.get_priority_production_types():
        productionPriorities[priorityType] = int(max(0, ( foAI.foAIstate.get_priority(priorityType) )**0.5))

    sortedPriorities = productionPriorities.items()
    sortedPriorities.sort(lambda x,y: cmp(x[1], y[1]), reverse=True)

    topPriority = -1
    topscore = -1
    numTotalFleets = len(foAI.foAIstate.fleetStatus)
    numColonyTargs=len(foAI.foAIstate.colonisablePlanetIDs )
    numColonyFleets=len( FleetUtilsAI.get_empire_fleet_ids_by_role( EnumsAI.AIFleetMissionType.FLEET_MISSION_COLONISATION) )# counting existing colony fleets each as one ship
    totColonyFleets = sum(queuedColonyShips.values()) + numColonyFleets
    numOutpostTargs=len(foAI.foAIstate.colonisableOutpostIDs )
    numOutpostFleets=len( FleetUtilsAI.get_empire_fleet_ids_by_role( EnumsAI.AIFleetMissionType.FLEET_MISSION_OUTPOST) )# counting existing outpost fleets each as one ship
    totOutpostFleets = queuedOutpostShips + numOutpostFleets

    #maxColonyFleets = max( min( numColonyTargs+1+currentTurn/10 , numTotalFleets/4 ), 3+int(3*len(empireColonizers)))
    #maxOutpostFleets = min(numOutpostTargs+1+currentTurn/10, numTotalFleets/4 )
    maxColonyFleets = PriorityAI.allottedColonyTargets
    maxOutpostFleets = maxColonyFleets


    _, _, colonyBuildChoices = getBestShipInfo(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION)
    militaryEmergency = PriorityAI.unmetThreat > (2.0 * MilitaryAI.totMilRating )

    print "Production Queue Priorities:"
    filteredPriorities = {}
    for ID, score in sortedPriorities:
        if militaryEmergency:
            if ID == EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_EXPLORATION:
                score /= 10.0
            elif ID != EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY:
                score /= 2.0
        if topscore < score:
            topPriority = ID
            topscore = score #don't really need topscore nor sorting with current handling
        print " Score: %4d -- %s "%(score, EnumsAI.AIPriorityNames[ID] )
        if ID != EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_BUILDINGS:
            if ( ID == EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION ) and ( totColonyFleets < maxColonyFleets) and (colonyBuildChoices is not None) and len(colonyBuildChoices) >0:
                filteredPriorities[ID]= score
            elif ( ID == EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_OUTPOST ) and ( totOutpostFleets < maxOutpostFleets ):
                filteredPriorities[ID]= score
            elif ID not in [EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_OUTPOST , EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION ]:
                filteredPriorities[ID]= score
    if filteredPriorities == {}:
        print "No non-building-production priorities with nonzero score, setting to default: Military"
        filteredPriorities [EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY ] = 1
    if topscore <= 100:
        scalingPower = 1.0
    else:
        scalingPower = math.log(100)/math.log(topscore)
    for pty in filteredPriorities:
        filteredPriorities[pty] **= scalingPower


    availablePP = dict( [ (tuple(el.key()), el.data() ) for el in empire.planetsWithAvailablePP ])  #keys are sets of ints; data is doubles
    allocatedPP = dict( [ (tuple(el.key()), el.data() ) for el in empire.planetsWithAllocatedPP ])  #keys are sets of ints; data is doubles
    planetsWithWastedPP = set( [tuple(pidset) for pidset in empire.planetsWithWastedPP ] )
    print "availPP ( <systems> : pp ):"
    for pSet in availablePP:
        print "\t%s\t%.2f"%( ppstring(PlanetUtilsAI.sys_name_ids(set(PlanetUtilsAI.get_systems( pSet)))), availablePP[pSet])
    print "\nallocatedPP ( <systems> : pp ):"
    for pSet in allocatedPP:
        print "\t%s\t%.2f"%( ppstring(PlanetUtilsAI.sys_name_ids(set(PlanetUtilsAI.get_systems( pSet)))), allocatedPP[pSet])

    if False: #log ship design assessment
        getBestShipRatings( list(AIstate.popCtrIDs))
    print "\n\nBuilding Ships in system groups with remaining PP:"
    for pSet in planetsWithWastedPP:
        totalPP = availablePP.get(pSet, 0)
        availPP = totalPP - allocatedPP.get(pSet, 0)
        if availPP <=0.01:
            continue
        print "%.2f PP remaining in system group: %s"%(availPP, ppstring(PlanetUtilsAI.sys_name_ids(set(PlanetUtilsAI.get_systems( pSet)))))
        print "\t owned planets in this group are:"
        print "\t %s"%( ppstring(PlanetUtilsAI.planet_name_ids(pSet) ))
        bestDesignID, bestDesign, buildChoices = getBestShipInfo(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION, list(pSet))
        speciesMap = {}
        for loc in (buildChoices or []):
            this_spec = universe.getPlanet(loc).speciesName
            speciesMap.setdefault(this_spec, []).append( loc)
        colonyBuildChoices=[]
        for pid, (score, this_spec) in foAI.foAIstate.colonisablePlanetIDs.items():
            colonyBuildChoices.extend( int(math.ceil(score))*[pid2 for pid2 in speciesMap.get(this_spec, []) if pid2 in pSet] )

        localPriorities = {}
        localPriorities.update( filteredPriorities )
        bestShips={}
        milBuildChoices = getBestShipRatings(list(pSet))
        for priority in list(localPriorities):
            if priority == EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY:
                if not milBuildChoices:
                    del localPriorities[priority]
                    continue
                top = milBuildChoices[0]
                bestDesignID, bestDesign, buildChoices = top[2], top[3], [top[1]]
                #score = ColonisationAI.pilotRatings.get(pid, 0)
                #if bestScore < ColonisationAI.curMidPilotRating:
            else:
                bestDesignID, bestDesign, buildChoices = getBestShipInfo(priority, list(pSet))
            if bestDesign is None:
                del localPriorities[priority] #must be missing a shipyard -- TODO build a shipyard if necessary
                continue
            bestShips[priority] = [bestDesignID, bestDesign, buildChoices ]
            print "bestShips[%s] = %s \t locs are %s from %s"%( EnumsAI.AIPriorityNames[priority], bestDesign.name(False) , buildChoices, pSet)

        if len(localPriorities)==0:
            print "Alert!! need shipyards in systemSet ", ppstring(PlanetUtilsAI.sys_name_ids(set(PlanetUtilsAI.get_systems( sorted(pSet)))))
        priorityChoices=[]
        for priority in localPriorities:
            priorityChoices.extend( int(localPriorities[priority]) * [priority] )

        loopCount = 0
        while (availPP > 0) and (loopCount < max(100, currentTurn)) and (priorityChoices != [] ): #make sure don't get stuck in some nonbreaking loop like if all shipyards captured
            loopCount +=1
            print "Beginning build enqueue loop %d; %.1f PP available"%(loopCount, availPP)
            thisPriority = random.choice( priorityChoices )
            print "selected priority: ", EnumsAI.AIPriorityNames[thisPriority]
            makingColonyShip=False
            makingOutpostShip=False
            if thisPriority == EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION:
                if totColonyFleets >= maxColonyFleets:
                    print "Already sufficient colony ships in queue, trying next priority choice"
                    print
                    for i in range( len(priorityChoices)-1, -1, -1):
                        if priorityChoices[i]==EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION:
                            del priorityChoices[i]
                    continue
                elif colonyBuildChoices is None or len(colonyBuildChoices)==0:
                    for i in range( len(priorityChoices)-1, -1, -1):
                        if priorityChoices[i]==EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION:
                            del priorityChoices[i]
                    continue
                else:
                    makingColonyShip=True
            if thisPriority == EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_OUTPOST:
                if totOutpostFleets >= maxOutpostFleets:
                    print "Already sufficient outpost ships in queue, trying next priority choice"
                    print
                    for i in range( len(priorityChoices)-1, -1, -1):
                        if priorityChoices[i]==EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_OUTPOST:
                            del priorityChoices[i]
                    continue
                else:
                    makingOutpostShip=True
            bestDesignID, bestDesign, buildChoices = bestShips[thisPriority]
            if makingColonyShip:
                loc = random.choice(colonyBuildChoices)
                bestDesignID, bestDesign, buildChoices = getBestShipInfo(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION, loc)
            elif thisPriority == EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY:
                selector = random.random()
                choice = milBuildChoices[0] #milBuildChoices can't be empty due to earlier check
                for choice in milBuildChoices:
                    if choice[0] >= selector:
                        break
                loc, bestDesignID, bestDesign = choice[1:4]
                if bestDesign is None:
                    print "Error: problem with milBuildChoices; with selector (%s) chose loc (%s), bestDesignID (%s), bestDesign (None) from milBuildChoices: %s" % (selector, loc, bestDesignID, milBuildChoices)
                    continue
                #print "Mil ship choices ", loc, bestDesignID, " from ", choice
            else:
                loc = random.choice(buildChoices)

            numShips=1
            perTurnCost = (float(bestDesign.productionCost(empire.empireID, loc)) / bestDesign.productionTime(empire.empireID, loc))
            if thisPriority == EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY:
                thisRating = ColonisationAI.pilot_ratings.get(loc, 0)
                ratingRatio = float(thisRating) / ColonisationAI.cur_best_pilot_rating
                pname = ""
                if ratingRatio < 0.1:
                    locPlanet = universe.getPlanet(loc)
                    if locPlanet:
                        pname = locPlanet.name
                        thisRating = ColonisationAI.rate_planetary_piloting(loc)
                        ratingRatio = float(thisRating) / ColonisationAI.cur_best_pilot_rating
                        qualifier = ["", "suboptimal"][ratingRatio < 1.0]
                        print "Building mil ship at loc %d (%s) with %s pilot Rating: %.1f; ratio to empire best is %.1f"%(loc, pname, qualifier, thisRating, ratingRatio)
                while totalPP > 40*perTurnCost:
                    numShips *= 2
                    perTurnCost *= 2
            retval = fo.issueEnqueueShipProductionOrder(bestDesignID, loc)
            if retval !=0:
                prioritized = False
                print "adding %d new ship(s) at location %s to production queue: %s; per turn production cost %.1f"%(numShips, ppstring(PlanetUtilsAI.planet_name_ids([loc])), bestDesign.name(True), perTurnCost)
                print
                if numShips>1:
                    fo.issueChangeProductionQuantityOrder(productionQueue.size -1, 1, numShips)
                availPP -= perTurnCost
                if makingColonyShip:
                    totColonyFleets +=numShips
                    if totalPP > 4* perTurnCost:
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                    continue
                if makingOutpostShip:
                    totOutpostFleets +=numShips
                    if totalPP > 4* perTurnCost:
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                    continue
                if totalPP > 10* perTurnCost :
                    leadingBlockPP = 0
                    for elem in [productionQueue[elemi] for elemi in range(0, min(4, productionQueue.size))]:
                        cost, time = empire.productionCostAndTime( elem )
                        leadingBlockPP += elem.blocksize *cost/time
                    if leadingBlockPP > 0.5* totalPP or (militaryEmergency and thisPriority==EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY ):
                        prioritized = True
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
                if (not prioritized) and (priority == EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_INVASION):
                    res=fo.issueRequeueProductionOrder(productionQueue.size -1, 0) # move to front
        print
    fo.updateProductionQueue()


def getAvailableBuildLocations(shipDesignID):
    """returns locations where shipDesign can be built"""
    result = []
    shipDesign = fo.getShipDesign(shipDesignID)
    empire = fo.getEmpire()
    empireID = empire.empireID
    capitolID = PlanetUtilsAI.get_capital()
    shipyards = set()
    for yardlist in ColonisationAI.empire_ship_builders.values():
        shipyards.update(yardlist)
    shipyards.discard(capitolID)
    for planetID in [capitolID] + list(shipyards):  # gets capitol at front of list
        if shipDesign.productionLocationForEmpire(empireID, planetID):
            result.append(planetID)
    return result


def spentPP():
    """calculate PPs spent this turn so far"""
    return fo.getEmpire().productionQueue.totalSpent
