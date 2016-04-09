import math
import traceback
import random
import freeOrionAIInterface as fo  # pylint: disable=import-error
import AIDependencies
import AIstate
import FleetUtilsAI
import FreeOrionAI as foAI
import PlanetUtilsAI
import PriorityAI
import ColonisationAI
import MilitaryAI
import ShipDesignAI
import time
import cProfile, pstats, StringIO

from EnumsAI import (PriorityType, EmpireProductionTypes, MissionType, get_priority_production_types,
                     FocusType, ShipRoleType, ShipDesignTypes)
from freeorion_tools import dict_from_map, ppstring, chat_human, tech_is_complete, print_error
from TechsListsAI import EXOBOT_TECH_NAME
from operator import itemgetter

best_military_design_rating_cache = {}  # indexed by turn, values are rating of the military design of the turn
design_cost_cache = {0: {(-1, -1): 0}}  # outer dict indexed by cur_turn (currently only one turn kept); inner dict indexed by (design_id, pid)

design_cache = {}  # dict of tuples (rating,pid,designID,cost) sorted by rating and indexed by priority type

_CHAT_DEBUG = False


def find_best_designs_this_turn():
    """Calculate the best designs for each ship class available at this turn."""
    pr = cProfile.Profile()
    pr.enable()
    start = time.clock()
    ShipDesignAI.Cache.update_for_new_turn()
    design_cache.clear()
    best_military_stats = ShipDesignAI.MilitaryShipDesigner().optimize_design()
    best_carrier_stats = ShipDesignAI.CarrierShipDesigner().optimize_design()
    best_stats = best_military_stats + best_carrier_stats
    if best_stats:
        best_stats.sort(key=itemgetter(0))
    design_cache[PriorityType.PRODUCTION_MILITARY] = best_stats
    design_cache[PriorityType.PRODUCTION_ORBITAL_INVASION] = ShipDesignAI.OrbitalTroopShipDesigner().optimize_design()
    design_cache[PriorityType.PRODUCTION_INVASION] = ShipDesignAI.StandardTroopShipDesigner().optimize_design()
    design_cache[PriorityType.PRODUCTION_COLONISATION] = ShipDesignAI.StandardColonisationShipDesigner().optimize_design()
    design_cache[PriorityType.PRODUCTION_ORBITAL_COLONISATION] = ShipDesignAI.OrbitalColonisationShipDesigner().optimize_design()
    design_cache[PriorityType.PRODUCTION_OUTPOST] = ShipDesignAI.StandardOutpostShipDesigner().optimize_design()
    design_cache[PriorityType.PRODUCTION_ORBITAL_OUTPOST] = ShipDesignAI.OrbitalOutpostShipDesigner().optimize_design()
    design_cache[PriorityType.PRODUCTION_ORBITAL_DEFENSE] = ShipDesignAI.OrbitalDefenseShipDesigner().optimize_design()
    design_cache[PriorityType.PRODUCTION_EXPLORATION] = ShipDesignAI.ScoutShipDesigner().optimize_design()
    ShipDesignAI.KrillSpawnerShipDesigner().optimize_design()  # just designing it, building+mission not supported yet
    end = time.clock()
    print "DEBUG INFORMATION: The design evaluations took %f s" % (end - start)
    print "-----"
    pr.disable()
    s = StringIO.StringIO()
    sort_by = 'cumulative'
    ps = pstats.Stats(pr, stream=s).sort_stats(sort_by)
    ps.print_stats()
    print s.getvalue()
    print "-----"
    if fo.currentTurn() % 10 == 0:
        ShipDesignAI.Cache.print_best_designs()


def get_design_cost(design, pid):  # TODO: Use new framework
    """Find and return the design_cost of the specified design on the specified planet.

    :param design:
    :type design: fo.shipDesign
    :param pid: planet id
    :type pid: int
    :return: cost of the design
    """
    cur_turn = fo.currentTurn()
    if cur_turn in design_cost_cache:
        cost_cache = design_cost_cache[cur_turn]
    else:
        design_cost_cache.clear()
        cost_cache = {}
        design_cost_cache[cur_turn] = cost_cache
    loc_invariant = design.costTimeLocationInvariant
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
    priority = PriorityType.PRODUCTION_MILITARY
    if priority in design_cache:
        if design_cache[priority] and design_cache[priority][0]:
            rating, pid, design_id, cost = design_cache[priority][0]
            best_military_design_rating_cache[current_turn] = rating
            return max(rating, 0.001)
        else:
            return 0.001
    else:
        return 0.001


def get_best_ship_info(priority, loc=None):
    """ Returns 3 item tuple: designID, design, buildLocList."""
    if loc is None:
        planet_ids = AIstate.popCtrIDs
    elif isinstance(loc, list):
        planet_ids = set(loc).intersection(AIstate.popCtrIDs)
    elif isinstance(loc, int) and loc in AIstate.popCtrIDs:
        planet_ids = [loc]
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


def get_best_ship_ratings(planet_ids):
    """
    Returns list of [partition, pid, designID, design] sublists, currently only for military ships.

    Since we haven't yet implemented a way to target military ship construction at/near particular locations
    where they are most in need, and also because our rating system is presumably useful-but-not-perfect, we want to
    distribute the construction across the Resource Group and across similarly rated designs, preferentially choosing
    the best rated design/loc combo, but if there are multiple design/loc combos with the same or similar ratings then
    we want some chance of choosing  those alternate designs/locations.

    The approach to this taken below is to treat the ratings akin to an energy to be used in a statistical mechanics
    type partition function. 'tally' will compute the normalization constant.
    So first go through and calculate the tally as well as convert each individual contribution to
    the running total up to that point, to facilitate later sampling.  Then those running totals are
    renormalized by the final tally, so that a later random number selector in the range [0,1) can be
    used to select the chosen design/loc.

    :param planet_ids: list of planets ids.
    :type planet_ids: list|set|tuple
    :rtype: list
    """
    priority = PriorityType.PRODUCTION_MILITARY
    planet_ids = set(planet_ids).intersection(ColonisationAI.empire_shipyards)

    if priority in design_cache:  # use new framework
        build_choices = design_cache[priority]
        loc_choices = [[item[0], item[1], item[2], fo.getShipDesign(item[2])]
                       for item in build_choices if item[1] in planet_ids]
        if not loc_choices:
            return []
        best_rating = loc_choices[0][0]
        tally = 0
        ret_val = []
        for rating, pid, design_id, design in loc_choices:
            if rating < 0.7 * best_rating:
                break
            p = math.exp(10 * (rating/best_rating - 1))
            tally += p
            ret_val.append([tally, pid, design_id, design])
        for item in ret_val:
            item[0] /= tally
        return ret_val
    else:
        return []


def generateProductionOrders():
    """generate production orders"""
    # first check ship designs
    # next check for buildings etc that could be placed on queue regardless of locally available PP
    # next loop over resource groups, adding buildings & ships
    universe = fo.getUniverse()
    capital_id = PlanetUtilsAI.get_capital()
    if capital_id is None or capital_id == -1:
        homeworld = None
        capital_system_id = None
    else:
        homeworld = universe.getPlanet(capital_id)
        capital_system_id = homeworld.systemID
    print "Production Queue Management:"
    empire = fo.getEmpire()
    production_queue = empire.productionQueue
    total_pp = empire.productionPoints
    # prodResPool = empire.getResourcePool(fo.resourceType.industry)
    # available_pp = dict_from_map(production_queue.available_pp(prodResPool))
    # allocated_pp = dict_from_map(production_queue.allocated_pp)
    # objectsWithWastedPP = production_queue.objectsWithWastedPP(prodResPool)
    current_turn = fo.currentTurn()
    print
    print "  Total Available Production Points: %s" % total_pp

    claimed_stars = foAI.foAIstate.misc.get('claimedStars', {})
    if claimed_stars == {}:
        for sType in AIstate.empireStars:
            claimed_stars[sType] = list(AIstate.empireStars[sType])
        for sys_id in set(AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs):
            t_sys = universe.getSystem(sys_id)
            if not t_sys:
                continue
            claimed_stars.setdefault(t_sys.starType, []).append(sys_id)

    if (current_turn in [1, 4]) and ((production_queue.totalSpent < total_pp) or (len(production_queue) <= 3)):
        best_design_id, best_design, build_choices = get_best_ship_info(PriorityType.PRODUCTION_EXPLORATION)
        if len(AIstate.opponentPlanetIDs) == 0:
            init_scouts = 3
        else:
            init_scouts = 0
        if best_design_id:
            for scout_count in range(init_scouts):
                fo.issueEnqueueShipProductionOrder(best_design_id, build_choices[0])
        fo.updateProductionQueue()

    building_expense = 0.0
    building_ratio = [0.4, 0.35, 0.30][fo.empireID() % 3]
    print "Buildings present on all owned planets:"
    for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
        planet = universe.getPlanet(pid)
        if planet:
            print "%30s: %s" % (planet.name, [universe.getObject(bldg).name for bldg in planet.buildingIDs])
    print

    if not homeworld:
        print "if no capital, no place to build, should get around to capturing or colonizing a new one"  # TODO
    else:
        print "Empire priority_id %d has current Capital %s:" % (empire.empireID, homeworld.name)
        print "Buildings present at empire Capital (priority_id, Name, Type, Tags, Specials, OwnedbyEmpire):"
        for bldg in homeworld.buildingIDs:
            this_building = universe.getObject(bldg)
            tags = ",".join(this_building.tags)
            specials = ",".join(this_building.specials)
            print "%8s | %20s | type:%20s | tags:%20s | specials: %20s | owner:%d " % (bldg, this_building.name, "_".join(this_building.buildingTypeName.split("_")[-2:])[:20], tags, specials, this_building.owner)
        print 
        capital_buildings = [universe.getObject(bldg).buildingTypeName for bldg in homeworld.buildingIDs]

        possible_building_type_ids = []
        for type_id in empire.availableBuildingTypes:
            try:
                if fo.getBuildingType(type_id).canBeProduced(empire.empireID, homeworld.id):
                    possible_building_type_ids.append(type_id)
            except:
                if fo.getBuildingType(type_id) is None:
                    print "For empire %d, 'available Building Type priority_id' %s returns None from fo.getBuildingType(type_id)" % (empire.empireID, type_id)
                else:
                    print "For empire %d, problem getting BuildingTypeID for 'available Building Type priority_id' %s" % (empire.empireID, type_id)
        if possible_building_type_ids:
            print "Possible building types to build:"
            for type_id in possible_building_type_ids:
                building_type = fo.getBuildingType(type_id)
                print "    %s cost: %s  time: %s" % (building_type.name,
                                                     building_type.productionCost(empire.empireID, homeworld.id),
                                                     building_type.productionTime(empire.empireID, homeworld.id))

            possible_building_types = [fo.getBuildingType(type_id) and fo.getBuildingType(type_id).name for type_id in possible_building_type_ids]  # makes sure is not None before getting name

            print
            print "Buildings already in Production Queue:"
            capital_queued_buildings = []
            queued_exobot_locs = []
            for element in [e for e in production_queue if (e.buildType == EmpireProductionTypes.BT_BUILDING)]:
                building_expense += element.allocation
                if element.locationID == homeworld.id:
                    capital_queued_buildings.append(element)
                if element.name == "BLD_COL_EXOBOT":
                    queued_exobot_locs.append(element.locationID)
            for bldg in capital_queued_buildings:
                print "    %s turns: %s PP: %s" % (bldg.name, bldg.turnsLeft, bldg.allocation)
            if not capital_queued_buildings:
                print "No capital queued buildings"
            print
            queued_building_names = [bldg.name for bldg in capital_queued_buildings]

            if (total_pp > 40 or ((current_turn > 40) and (ColonisationAI.empire_status.get('industrialists', 0) >= 20))) and ("BLD_INDUSTRY_CENTER" in possible_building_types) and ("BLD_INDUSTRY_CENTER" not in (capital_buildings+queued_building_names)) and (building_expense < building_ratio*total_pp):
                res = fo.issueEnqueueBuildingProductionOrder("BLD_INDUSTRY_CENTER", homeworld.id)
                print "Enqueueing BLD_INDUSTRY_CENTER, with result %d" % res
                if res:
                    cost, time = empire.productionCostAndTime(production_queue[production_queue.size - 1])
                    building_expense += cost / time

            if ("BLD_SHIPYARD_BASE" in possible_building_types) and ("BLD_SHIPYARD_BASE" not in (capital_buildings + queued_building_names)):
                try:
                    res = fo.issueEnqueueBuildingProductionOrder("BLD_SHIPYARD_BASE", homeworld.id)
                    print "Enqueueing BLD_SHIPYARD_BASE, with result %d" % res
                except:
                    print "Error: cant build shipyard at new capital, probably no population; we're hosed"
                    print "Error: exception triggered and caught: ", traceback.format_exc()

            for building_name in ["BLD_SHIPYARD_ORG_ORB_INC"]:
                if (building_name in possible_building_types) and (building_name not in (capital_buildings + queued_building_names)) and (building_expense < building_ratio * total_pp):
                    try:
                        res = fo.issueEnqueueBuildingProductionOrder(building_name, homeworld.id)
                        print "Enqueueing %s at capital, with result %d" % (building_name, res)
                        if res:
                            cost, time = empire.productionCostAndTime(production_queue[production_queue.size - 1])
                            building_expense += cost / time
                            res = fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                            print "Requeueing %s to front of build queue, with result %d" % (building_name, res)
                    except:
                        print "Error: exception triggered and caught: ", traceback.format_exc()

            num_queued_exobots = len(queued_exobot_locs)
            if empire.techResearched(EXOBOT_TECH_NAME) and num_queued_exobots < 2:
                potential_locs = []
                for pid, (score, this_spec) in foAI.foAIstate.colonisablePlanetIDs.items():
                    if this_spec == "SP_EXOBOT" and pid not in queued_exobot_locs:
                        candidate = universe.getPlanet(pid)
                        if candidate.systemID in empire.supplyUnobstructedSystems:
                            potential_locs.append((score, pid))
                if potential_locs:
                    candidate_id = sorted(potential_locs)[-1][-1]
                    res = fo.issueEnqueueBuildingProductionOrder("BLD_COL_EXOBOT", candidate_id)
                    print "Enqueueing BLD_COL_EXOBOT, with result %d" % res
                    # if res:
                    #    res=fo.issueRequeueProductionOrder(production_queue.size -1, 0) # move to front
                    #    print "Requeueing %s to front of build queue, with result %d"%("BLD_COL_EXOBOT", res)

            if ("BLD_IMPERIAL_PALACE" in possible_building_types) and ("BLD_IMPERIAL_PALACE" not in (capital_buildings + queued_building_names)):
                res = fo.issueEnqueueBuildingProductionOrder("BLD_IMPERIAL_PALACE", homeworld.id)
                print "Enqueueing BLD_IMPERIAL_PALACE at %s, with result %d" % (homeworld.name, res)
                if res:
                    res = fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                    print "Requeueing BLD_IMPERIAL_PALACE to front of build queue, with result %d"%res

            # ok, BLD_NEUTRONIUM_SYNTH is not currently unlockable, but just in case... ;-p
            if ("BLD_NEUTRONIUM_SYNTH" in possible_building_types) and ("BLD_NEUTRONIUM_SYNTH" not in (capital_buildings + queued_building_names)):
                res = fo.issueEnqueueBuildingProductionOrder("BLD_NEUTRONIUM_SYNTH", homeworld.id)
                print "Enqueueing BLD_NEUTRONIUM_SYNTH, with result %d"%res
                if res:
                    res = fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                    print "Requeueing BLD_NEUTRONIUM_SYNTH to front of build queue, with result %d" % res

    # TODO: add total_pp checks below, so don't overload queue
    best_pilot_facilities = ColonisationAI.facilities_by_species_grade.get(
        "WEAPONS_%.1f" % ColonisationAI.get_best_pilot_rating(), {})

    print "best_pilot_facilities: \n %s" % best_pilot_facilities

    max_defense_portion = [0.7, 0.4, 0.3, 0.2, 0.1, 0.0][foAI.foAIstate.aggression]
    aggression_index = max(1, foAI.foAIstate.aggression)
    if ((current_turn % aggression_index) == 0) and foAI.foAIstate.aggression < fo.aggression.maniacal:
        sys_orbital_defenses = {}
        queued_defenses = {}
        defense_allocation = 0.0
        target_orbitals = min(int(((current_turn + 4) / (8.0 * aggression_index**1.5))**0.8), fo.aggression.maniacal - aggression_index)
        print "Orbital Defense Check -- target Defense Orbitals: ", target_orbitals
        for element in production_queue:
            if (element.buildType == EmpireProductionTypes.BT_SHIP) and (foAI.foAIstate.get_ship_role(element.designID) == ShipRoleType.BASE_DEFENSE):
                planet = universe.getPlanet(element.locationID)
                if not planet:
                    print "Error: Problem getting Planet for build loc %s"%element.locationID
                    continue
                sys_id = planet.systemID
                queued_defenses[sys_id] = queued_defenses.get(sys_id, 0) + element.blocksize*element.remaining
                defense_allocation += element.allocation
        print "Queued Defenses:", [(ppstring(PlanetUtilsAI.sys_name_ids([sys_id])), num) for sys_id, num in queued_defenses.items()]
        for sys_id in ColonisationAI.empire_species_systems:
            if foAI.foAIstate.systemStatus.get(sys_id, {}).get('fleetThreat', 1) > 0:
                continue  # don't build orbital shields if enemy fleet present
            if defense_allocation > max_defense_portion * total_pp:
                break
            sys_orbital_defenses[sys_id] = 0
            fleets_here = foAI.foAIstate.systemStatus.get(sys_id, {}).get('myfleets', [])
            for fid in fleets_here:
                if foAI.foAIstate.get_fleet_role(fid) == MissionType.ORBITAL_DEFENSE:
                    print "Found %d existing Orbital Defenses in %s :" % (foAI.foAIstate.fleetStatus.get(fid, {}).get('nships', 0), ppstring(PlanetUtilsAI.sys_name_ids([sys_id])))
                    sys_orbital_defenses[sys_id] += foAI.foAIstate.fleetStatus.get(fid, {}).get('nships', 0)
            for pid in ColonisationAI.empire_species_systems.get(sys_id, {}).get('pids', []):
                sys_orbital_defenses[sys_id] += queued_defenses.get(pid, 0)
            if sys_orbital_defenses[sys_id] < target_orbitals:
                num_needed = target_orbitals - sys_orbital_defenses[sys_id]
                for pid in ColonisationAI.empire_species_systems.get(sys_id, {}).get('pids', []):
                    best_design_id, col_design, build_choices = get_best_ship_info(PriorityType.PRODUCTION_ORBITAL_DEFENSE, pid)
                    if not best_design_id:
                        print "no orbital defenses can be built at ", ppstring(PlanetUtilsAI.planet_name_ids([pid]))
                        continue
                    retval = fo.issueEnqueueShipProductionOrder(best_design_id, pid)
                    print "queueing %d Orbital Defenses at %s" % (num_needed, ppstring(PlanetUtilsAI.planet_name_ids([pid])))
                    if retval != 0:
                        if num_needed > 1:
                            fo.issueChangeProductionQuantityOrder(production_queue.size - 1, 1, num_needed)
                        cost, time = empire.productionCostAndTime(production_queue[production_queue.size - 1])
                        defense_allocation += production_queue[production_queue.size - 1].blocksize * cost/time
                        fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                        break

    building_type = fo.getBuildingType("BLD_SHIPYARD_BASE")
    queued_shipyard_locs = [element.locationID for element in production_queue if (element.name == "BLD_SHIPYARD_BASE")]
    system_colonies = {}
    colony_systems = {}
    for spec_name in ColonisationAI.empire_colonizers:
        if (len(ColonisationAI.empire_colonizers[spec_name]) == 0) and (spec_name in ColonisationAI.empire_species):  # not enough current shipyards for this species#TODO: also allow orbital incubators and/or asteroid ships
            for pid in ColonisationAI.empire_species.get(spec_name, []):  # SP_EXOBOT may not actually have a colony yet but be in empireColonizers
                if pid in queued_shipyard_locs:
                    break  # won't try building more than one shipyard at once, per colonizer
            else:  # no queued shipyards, get planets with target pop >=3, and queue a shipyard on the one with biggest current pop
                planets = zip(map(universe.getPlanet, ColonisationAI.empire_species[spec_name]), ColonisationAI.empire_species[spec_name])
                pops = sorted([(planet.currentMeterValue(fo.meterType.population), pid) for planet, pid in planets if (planet and planet.currentMeterValue(fo.meterType.targetPopulation) >= 3.0)])
                pids = [pid for pop, pid in pops if building_type.canBeProduced(empire.empireID, pid)]
                if pids:
                    build_loc = pids[-1]
                    res = fo.issueEnqueueBuildingProductionOrder("BLD_SHIPYARD_BASE", build_loc)
                    print "Enqueueing BLD_SHIPYARD_BASE at planet %d (%s) for colonizer species %s, with result %d" % (build_loc, universe.getPlanet(build_loc).name, spec_name, res)
                    if res:
                        queued_shipyard_locs.append(build_loc)
                        break  # only start at most one new shipyard per species per turn
        for pid in ColonisationAI.empire_species.get(spec_name, []):
            planet = universe.getPlanet(pid)
            if planet:
                system_colonies.setdefault(planet.systemID, {}).setdefault('pids', []).append(pid)
                colony_systems[pid] = planet.systemID

    acirema_systems = {}
    for pid in ColonisationAI.empire_species.get("SP_ACIREMA", []):
        acirema_systems.setdefault(universe.getPlanet(pid).systemID, []).append(pid)
        if (pid in queued_shipyard_locs) or not building_type.canBeProduced(empire.empireID, pid):
            continue  # but not 'break' because we want to build shipyards at *every* Acirema planet
        res = fo.issueEnqueueBuildingProductionOrder("BLD_SHIPYARD_BASE", pid)
        print "Enqueueing BLD_SHIPYARD_BASE at planet %d (%s) for Acirema, with result %d" % (pid, universe.getPlanet(pid).name, res)
        if res:
            queued_shipyard_locs.append(pid)
            res = fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
            print "Requeueing Acirema BLD_SHIPYARD_BASE to front of build queue, with result %d" % res

    top_pilot_systems = {}
    for pid, _ in ColonisationAI.pilot_ratings.items():
        if (_ <= ColonisationAI.get_medium_pilot_rating()) and (_ < ColonisationAI.GREAT_PILOT_RATING):
            continue
        top_pilot_systems.setdefault(universe.getPlanet(pid).systemID, []).append((pid, _))
        if (pid in queued_shipyard_locs) or not building_type.canBeProduced(empire.empireID, pid):
            continue  # but not 'break' because we want to build shipyards all top pilot planets
        res = fo.issueEnqueueBuildingProductionOrder("BLD_SHIPYARD_BASE", pid)
        print "Enqueueing BLD_SHIPYARD_BASE at planet %d (%s) for top pilot, with result %d" % (pid, universe.getPlanet(pid).name, res)
        if res:
            queued_shipyard_locs.append(pid)
            res = fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
            print "Requeueing BLD_SHIPYARD_BASE to front of build queue, with result %d" % res

    pop_ctrs = list(AIstate.popCtrIDs)
    red_popctrs = sorted([(ColonisationAI.pilot_ratings.get(pid, 0), pid) for pid in pop_ctrs
                          if colony_systems.get(pid, -1) in AIstate.empireStars.get(fo.starType.red, [])],
                         reverse=True)
    red_pilots = [pid for _, pid in red_popctrs if _ == ColonisationAI.get_best_pilot_rating()]
    blue_popctrs = sorted([(ColonisationAI.pilot_ratings.get(pid, 0), pid) for pid in pop_ctrs
                           if colony_systems.get(pid, -1) in AIstate.empireStars.get(fo.starType.blue, [])],
                          reverse=True)
    blue_pilots = [pid for _, pid in blue_popctrs if _ == ColonisationAI.get_best_pilot_rating()]
    bh_popctrs = sorted([(ColonisationAI.pilot_ratings.get(pid, 0), pid) for pid in pop_ctrs
                         if colony_systems.get(pid, -1) in AIstate.empireStars.get(fo.starType.blackHole, [])],
                        reverse=True)
    bh_pilots = [pid for _, pid in bh_popctrs if _ == ColonisationAI.get_best_pilot_rating()]
    enrgy_shipyard_locs = {}
    for building_name in ["BLD_SHIPYARD_ENRG_COMP"]:
        if empire.buildingTypeAvailable(building_name):
            queued_building_locs = [element.locationID for element in production_queue if (element.name == building_name)]
            building_type = fo.getBuildingType(building_name)
            for pid in bh_pilots + blue_pilots:
                if len(queued_building_locs) > 1:  # build a max of 2 at once
                    break
                this_planet = universe.getPlanet(pid)
                if not (this_planet and this_planet.speciesName in ColonisationAI.empire_ship_builders):  # TODO: also check that not already one for this spec in this sys
                    continue
                enrgy_shipyard_locs.setdefault(this_planet.systemID, []).append(pid)
                if pid not in queued_building_locs and building_type.canBeProduced(empire.empireID, pid):
                    res = fo.issueEnqueueBuildingProductionOrder(building_name, pid)
                    print "Enqueueing %s at planet %s, with result %d" % (building_name, universe.getPlanet(pid).name, res)
                    if _CHAT_DEBUG:
                        chat_human("Enqueueing %s at planet %s, with result %d" % (building_name, universe.getPlanet(pid), res))
                    if res:
                        queued_building_locs.append(pid)
                        cost, time = empire.productionCostAndTime(production_queue[production_queue.size - 1])
                        building_expense += cost / time  # production_queue[production_queue.size -1].blocksize *
                        res = fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                        print "Requeueing %s to front of build queue, with result %d" % (building_name, res)

    bld_name = "BLD_SHIPYARD_ENRG_SOLAR"
    queued_bld_locs = [element.locationID for element in production_queue if (element.name == bld_name)]
    if empire.buildingTypeAvailable(bld_name) and not queued_bld_locs:
        # TODO: check that production is not frozen at a queued location
        bld_type = fo.getBuildingType(bld_name)
        for pid in bh_pilots:
            this_planet = universe.getPlanet(pid)
            if not (this_planet and this_planet.speciesName in ColonisationAI.empire_ship_builders):  # TODO: also check that not already one for this spec in this sys
                continue
            if bld_type.canBeProduced(empire.empireID, pid):
                res = fo.issueEnqueueBuildingProductionOrder(bld_name, pid)
                print "Enqueueing %s at planet %s, with result %d" % (bld_name, universe.getPlanet(pid), res)
                if _CHAT_DEBUG:
                    chat_human("Enqueueing %s at planet %s, with result %d" % (bld_name, universe.getPlanet(pid), res))
                if res:
                    cost, time = empire.productionCostAndTime(production_queue[production_queue.size - 1])
                    building_expense += cost / time  # production_queue[production_queue.size -1].blocksize *
                    res = fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                    print "Requeueing %s to front of build queue, with result %d" % (bld_name, res)
                    break

    building_name = "BLD_SHIPYARD_BASE"
    if empire.buildingTypeAvailable(building_name) and (building_expense < building_ratio * total_pp) and (total_pp > 50 or current_turn > 80):
        building_type = fo.getBuildingType(building_name)
        for sys_id in enrgy_shipyard_locs:  # Todo ensure only one or 2 per sys
            for pid in enrgy_shipyard_locs[sys_id][:2]:
                if pid not in queued_shipyard_locs and building_type.canBeProduced(empire.empireID, pid):  # TODO: verify that canBeProduced() checks for prexistence of a barring building
                    res = fo.issueEnqueueBuildingProductionOrder(building_name, pid)
                    print "Enqueueing %s at planet %d (%s) , with result %d" % (building_name, pid, universe.getPlanet(pid).name, res)
                    if res:
                        queued_shipyard_locs.append(pid)
                        cost, time = empire.productionCostAndTime(production_queue[production_queue.size - 1])
                        building_expense += cost / time  # production_queue[production_queue.size -1].blocksize *
                        res = fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                        print "Requeueing %s to front of build queue, with result %d" % (building_name, res)
                        break  # only start one per turn

    for bld_name in ["BLD_SHIPYARD_ORG_ORB_INC"]:
        build_ship_facilities(bld_name, best_pilot_facilities)

    # gating by life cycle manipulation helps delay these until they are closer to being worthwhile
    if tech_is_complete(AIDependencies.GRO_LIFE_CYCLE) or empire.researchProgress(AIDependencies.GRO_LIFE_CYCLE) > 0:
        for bld_name in ["BLD_SHIPYARD_ORG_XENO_FAC", "BLD_SHIPYARD_ORG_CELL_GRO_CHAMB"]:
            build_ship_facilities(bld_name, best_pilot_facilities)

    shipyard_type = fo.getBuildingType("BLD_SHIPYARD_BASE")
    building_name = "BLD_SHIPYARD_AST"
    if empire.buildingTypeAvailable(building_name) and foAI.foAIstate.aggression > fo.aggression.beginner:
        queued_building_locs = [element.locationID for element in production_queue if (element.name == building_name)]
        if not queued_building_locs:
            building_type = fo.getBuildingType(building_name)
            asteroid_systems = {}
            asteroid_yards = {}
            shipyard_systems = {}
            builder_systems = {}
            for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
                planet = universe.getPlanet(pid)
                this_spec = planet.speciesName
                sys_id = planet.systemID
                if planet.size == fo.planetSize.asteroids and sys_id in ColonisationAI.empire_species_systems:
                    asteroid_systems.setdefault(sys_id, []).append(pid)
                    if (pid in queued_building_locs) or (building_name in [universe.getObject(bldg).buildingTypeName for bldg in planet.buildingIDs]):
                        asteroid_yards[sys_id] = pid  # shouldn't ever overwrite another, but ok if it did
                if this_spec in ColonisationAI.empire_ship_builders:
                    if pid in ColonisationAI.empire_ship_builders[this_spec]:
                        shipyard_systems.setdefault(sys_id, []).append(pid)
                    else:
                        builder_systems.setdefault(sys_id, []).append((planet.speciesName, pid))
            # check if we need to build another asteroid processor:
            # check if local shipyard to go with the asteroid processor
            yard_locs = []
            need_yard = {}
            top_pilot_locs = []
            for sys_id in set(asteroid_systems.keys()).difference(asteroid_yards.keys()):
                if sys_id in top_pilot_systems:
                    for pid, _ in top_pilot_systems[sys_id]:
                        if pid not in queued_shipyard_locs:  # will catch it later if shipyard already present
                            top_pilot_locs.append((_, pid, sys_id))
            top_pilot_locs.sort(reverse=True)
            for _, _, sys_id in top_pilot_locs:
                if sys_id not in yard_locs:
                    yard_locs.append(sys_id)  # prioritize asteroid yards for acirema and/or other top pilots
                    for pid, _ in top_pilot_systems[sys_id]:
                        if pid not in queued_shipyard_locs:  # will catch it later if shipyard already present
                            need_yard[sys_id] = pid
            if (not yard_locs) and len(asteroid_yards.values()) <= int(current_turn / 50):  # not yet building & not enough current locs, find a location to build one
                colonizer_loc_choices = []
                builder_loc_choices = []
                bld_systems = set(asteroid_systems.keys()).difference(asteroid_yards.keys())
                for sys_id in bld_systems.intersection(builder_systems.keys()):
                    for this_spec, pid in builder_systems[sys_id]:
                        if this_spec in ColonisationAI.empire_colonizers:
                            if pid in (ColonisationAI.empire_colonizers[this_spec] + queued_shipyard_locs):
                                colonizer_loc_choices.insert(0, sys_id)
                            else:
                                colonizer_loc_choices.append(sys_id)
                                need_yard[sys_id] = pid
                        else:
                            if pid in (ColonisationAI.empire_ship_builders.get(this_spec, []) + queued_shipyard_locs):
                                builder_loc_choices.insert(0, sys_id)
                            else:
                                builder_loc_choices.append(sys_id)
                                need_yard[sys_id] = pid
                yard_locs.extend((colonizer_loc_choices+builder_loc_choices)[:1])  # add at most one of these non top pilot locs
            new_yard_count = len(queued_building_locs)
            for sys_id in yard_locs:  # build at most 2 new asteroid yards at a time
                if new_yard_count >= 2:
                    break
                pid = asteroid_systems[sys_id][0]
                if sys_id in need_yard:
                    pid2 = need_yard[sys_id]
                    if shipyard_type.canBeProduced(empire.empireID, pid2):
                        res = fo.issueEnqueueBuildingProductionOrder("BLD_SHIPYARD_BASE", pid2)
                        print "Enqueueing %s at planet %d (%s) to go with Asteroid Processor , with result %d" % ("BLD_SHIPYARD_BASE", pid2, universe.getPlanet(pid2).name, res)
                        if res:
                            queued_shipyard_locs.append(pid2)
                            cost, time = empire.productionCostAndTime(production_queue[production_queue.size - 1])
                            building_expense += cost / time  # production_queue[production_queue.size -1].blocksize *
                            res = fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                            print "Requeueing %s to front of build queue, with result %d" % ("BLD_SHIPYARD_BASE", res)
                if pid not in queued_building_locs and building_type.canBeProduced(empire.empireID, pid):
                    res = fo.issueEnqueueBuildingProductionOrder(building_name, pid)
                    print "Enqueueing %s at planet %d (%s) , with result %d on turn %d" % (building_name, pid, universe.getPlanet(pid).name, res, current_turn)
                    if res:
                        new_yard_count += 1
                        queued_building_locs.append(pid)
                        cost, time = empire.productionCostAndTime(production_queue[production_queue.size - 1])
                        building_expense += cost / time  # production_queue[production_queue.size -1].blocksize *
                        res = fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                        print "Requeueing %s to front of build queue, with result %d" % (building_name, res)

    building_name = "BLD_GAS_GIANT_GEN"
    max_gggs = 1
    if empire.buildingTypeAvailable(building_name) and foAI.foAIstate.aggression > fo.aggression.beginner:
        queued_building_locs = [element.locationID for element in production_queue if (element.name == building_name)]
        building_type = fo.getBuildingType(building_name)
        for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):  # TODO: check to ensure that a resource center exists in system, or GGG would be wasted
            if pid not in queued_building_locs and building_type.canBeProduced(empire.empireID, pid):  # TODO: verify that canBeProduced() checks for preexistence of a barring building
                planet = universe.getPlanet(pid)
                if planet.systemID in ColonisationAI.empire_species_systems:
                    gg_list = []
                    can_use_gg = False
                    system = universe.getSystem(planet.systemID)
                    for opid in system.planetIDs:
                        other_planet = universe.getPlanet(opid)
                        if other_planet.size == fo.planetSize.gasGiant:
                            gg_list.append(opid)
                        if opid != pid and other_planet.owner == empire.empireID and (FocusType.FOCUS_INDUSTRY in list(other_planet.availableFoci) + [other_planet.focus]):
                            can_use_gg = True
                    if pid in sorted(gg_list)[:max_gggs] and can_use_gg:
                        res = fo.issueEnqueueBuildingProductionOrder(building_name, pid)
                        if res:
                            queued_building_locs.append(pid)
                            cost, time = empire.productionCostAndTime(production_queue[production_queue.size - 1])
                            building_expense += cost / time  # production_queue[production_queue.size -1].blocksize *
                            res = fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                            print "Requeueing %s to front of build queue, with result %d" % (building_name, res)
                        print "Enqueueing %s at planet %d (%s) , with result %d" % (building_name, pid, universe.getPlanet(pid).name, res)

    building_name = "BLD_SOL_ORB_GEN"
    if empire.buildingTypeAvailable(building_name) and foAI.foAIstate.aggression > fo.aggression.turtle:
        already_got_one = 99
        for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
            planet = universe.getPlanet(pid)
            if planet and building_name in [bld.buildingTypeName for bld in map(universe.getObject, planet.buildingIDs)]:
                system = universe.getSystem(planet.systemID)
                if system and system.starType < already_got_one:
                    already_got_one = system.starType
        best_type = fo.starType.white
        best_locs = AIstate.empireStars.get(fo.starType.blue, []) + AIstate.empireStars.get(fo.starType.white, [])
        if not best_locs:
            best_type = fo.starType.orange
            best_locs = AIstate.empireStars.get(fo.starType.yellow, []) + AIstate.empireStars.get(fo.starType.orange, [])
        if (not best_locs) or (already_got_one < 99 and already_got_one <= best_type):
            pass  # could consider building at a red star if have a lot of PP but somehow no better stars
        else:
            use_new_loc = True
            queued_building_locs = [element.locationID for element in production_queue if (element.name == building_name)]
            if queued_building_locs:
                queued_star_types = {}
                for loc in queued_building_locs:
                    planet = universe.getPlanet(loc)
                    if not planet:
                        continue
                    system = universe.getSystem(planet.systemID)
                    queued_star_types.setdefault(system.starType, []).append(loc)
                if queued_star_types:
                    best_queued = sorted(queued_star_types.keys())[0]
                    if best_queued > best_type:  # i.e., best_queued is yellow, best_type available is blue or white
                        pass  # should probably evaluate cancelling the existing one under construction
                    else:
                        use_new_loc = False
            if use_new_loc:  # (of course, may be only loc, not really new)
                if not homeworld:
                    use_sys = best_locs[0]  # as good as any
                else:
                    distance_map = {}
                    for sys_id in best_locs:  # want to build close to capital for defense
                        if sys_id == -1:
                            continue
                        try:
                            distance_map[sys_id] = universe.jumpDistance(homeworld.systemID, sys_id)
                        except:
                            pass
                    use_sys = ([(-1, -1)] + sorted([(dist, sys_id) for sys_id, dist in distance_map.items()]))[:2][-1][-1]  # kinda messy, but ensures a value
                if use_sys != -1:
                    try:
                        use_loc = AIstate.colonizedSystems[use_sys][0]
                        res = fo.issueEnqueueBuildingProductionOrder(building_name, use_loc)
                        print "Enqueueing %s at planet %d (%s) , with result %d" % (building_name, use_loc, universe.getPlanet(use_loc).name, res)
                        if res:
                            cost, time = empire.productionCostAndTime(production_queue[production_queue.size - 1])
                            building_expense += cost / time  # production_queue[production_queue.size -1].blocksize *
                            res = fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                            print "Requeueing %s to front of build queue, with result %d"%(building_name, res)
                    except:
                        print "problem queueing BLD_SOL_ORB_GEN at planet", use_loc, "of system ", use_sys
                        pass

    building_name = "BLD_ART_BLACK_HOLE"
    if (
        empire.buildingTypeAvailable(building_name) and
        foAI.foAIstate.aggression > fo.aggression.typical and
        len(AIstate.empireStars.get(fo.starType.red, [])) > 0
    ):
        already_got_one = False
        for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
            planet = universe.getPlanet(pid)
            if planet and building_name in [bld.buildingTypeName for bld in map(universe.getObject, planet.buildingIDs)]:
                already_got_one = True  # has been built, needs one turn to activate
        queued_building_locs = [element.locationID for element in production_queue if (element.name == building_name)]  # TODO: check that queued locs or already built one are at red stars
        if not bh_pilots and len(queued_building_locs) == 0 and (red_pilots or not already_got_one):
            use_loc = None
            nominal_home = homeworld or universe.getPlanet(
                (red_pilots + AIstate.colonizedSystems[AIstate.empireStars[fo.starType.red][0]])[0])
            distance_map = {}
            for sys_id in AIstate.empireStars.get(fo.starType.red, []):
                if sys_id == -1:
                    continue
                try:
                    distance_map[sys_id] = universe.jumpDistance(nominal_home.systemID, sys_id)
                except:
                    pass
            red_sys_list = sorted([(dist, sys_id) for sys_id, dist in distance_map.items()])
            for dist, sys_id in red_sys_list:
                for loc in AIstate.colonizedSystems[sys_id]:
                    planet = universe.getPlanet(loc)
                    if planet and planet.speciesName not in ["", None]:
                        species = fo.getSpecies(planet.speciesName)
                        if species and "PHOTOTROPHIC" in list(species.tags):
                            break
                else:
                    use_loc = list(set(red_pilots).intersection(AIstate.colonizedSystems[sys_id]) or
                               AIstate.colonizedSystems[sys_id])[0]
                if use_loc is not None:
                    break
            if use_loc is not None:
                planet_used = universe.getPlanet(use_loc)
                try:
                    res = fo.issueEnqueueBuildingProductionOrder(building_name, use_loc)
                    print "Enqueueing %s at planet %s , with result %d" % (building_name, planet_used, res)
                    if res:
                        if _CHAT_DEBUG:
                            chat_human("Enqueueing %s at planet %s , with result %d" % (building_name, planet_used, res))
                        res = fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                        print "Requeueing %s to front of build queue, with result %d" % (building_name, res)
                except:
                    print "problem queueing %s at planet %s" % (building_name, planet_used)

    building_name = "BLD_BLACK_HOLE_POW_GEN"
    if empire.buildingTypeAvailable(building_name) and foAI.foAIstate.aggression > fo.aggression.cautious:
        already_got_one = False
        for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
            planet = universe.getPlanet(pid)
            if planet and building_name in [bld.buildingTypeName for bld in map(universe.getObject, planet.buildingIDs)]:
                already_got_one = True
        queued_building_locs = [element.locationID for element in production_queue if (element.name == building_name)]
        if (len(AIstate.empireStars.get(fo.starType.blackHole, [])) > 0) and len(queued_building_locs) == 0 and not already_got_one:
            if not homeworld:
                use_sys = AIstate.empireStars.get(fo.starType.blackHole, [])[0]
            else:
                distance_map = {}
                for sys_id in AIstate.empireStars.get(fo.starType.blackHole, []):
                    if sys_id == -1:
                        continue
                    try:
                        distance_map[sys_id] = universe.jumpDistance(homeworld.systemID, sys_id)
                    except:
                        pass
                use_sys = ([(-1, -1)] + sorted([(dist, sys_id) for sys_id, dist in distance_map.items()]))[:2][-1][-1]  # kinda messy, but ensures a value
            if use_sys != -1:
                try:
                    use_loc = AIstate.colonizedSystems[use_sys][0]
                    res = fo.issueEnqueueBuildingProductionOrder(building_name, use_loc)
                    print "Enqueueing %s at planet %d (%s) , with result %d" % (building_name, use_loc, universe.getPlanet(use_loc).name, res)
                    if res:
                        res = fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                        print "Requeueing %s to front of build queue, with result %d" % (building_name, res)
                except:
                    print "problem queueing BLD_BLACK_HOLE_POW_GEN at planet", use_loc, "of system ", use_sys
                    pass

    building_name = "BLD_ENCLAVE_VOID"
    if empire.buildingTypeAvailable(building_name):
        already_got_one = False
        for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
            planet = universe.getPlanet(pid)
            if planet and building_name in [bld.buildingTypeName for bld in map(universe.getObject, planet.buildingIDs)]:
                already_got_one = True
        queued_locs = [element.locationID for element in production_queue if (element.name == building_name)]
        if len(queued_locs) == 0 and homeworld and not already_got_one:
            try:
                res = fo.issueEnqueueBuildingProductionOrder(building_name, capital_id)
                print "Enqueueing %s at planet %d (%s) , with result %d" % (building_name, capital_id, universe.getPlanet(capital_id).name, res)
                if res:
                    res = fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                    print "Requeueing %s to front of build queue, with result %d" % (building_name, res)
            except:
                pass

    building_name = "BLD_GENOME_BANK"
    if empire.buildingTypeAvailable(building_name):
        already_got_one = False
        for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
            planet = universe.getPlanet(pid)
            if planet and building_name in [bld.buildingTypeName for bld in map(universe.getObject, planet.buildingIDs)]:
                already_got_one = True
        queued_locs = [element.locationID for element in production_queue if (element.name == building_name)]
        if len(queued_locs) == 0 and homeworld and not already_got_one:
            try:
                res = fo.issueEnqueueBuildingProductionOrder(building_name, capital_id)
                print "Enqueueing %s at planet %d (%s) , with result %d" % (building_name, capital_id, universe.getPlanet(capital_id).name, res)
                if res:
                    res = fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                    print "Requeueing %s to front of build queue, with result %d" % (building_name, res)
            except:
                pass

    building_name = "BLD_NEUTRONIUM_EXTRACTOR"
    already_got_extractor = False
    if (
        empire.buildingTypeAvailable(building_name) and
        [element.locationID for element in production_queue if (element.name == building_name)] == [] and
        AIstate.empireStars.get(fo.starType.neutron, [])
    ):
        # building_type = fo.getBuildingType(building_name)
        for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
            planet = universe.getPlanet(pid)
            if planet:
                if (
                    (planet.systemID in AIstate.empireStars.get(fo.starType.neutron, []) and
                        (building_name in [bld.buildingTypeName for bld in map(universe.getObject, planet.buildingIDs)])) or
                    "BLD_NEUTRONIUM_SYNTH" in [bld.buildingTypeName for bld in map(universe.getObject, planet.buildingIDs)]
                ):
                    already_got_extractor = True
        if not already_got_extractor:
            if not homeworld:
                use_sys = AIstate.empireStars.get(fo.starType.neutron, [])[0]
            else:
                distance_map = {}
                for sys_id in AIstate.empireStars.get(fo.starType.neutron, []):
                    if sys_id == -1:
                        continue
                    try:
                        distance_map[sys_id] = universe.jumpDistance(homeworld.systemID, sys_id)
                    except Exception as e:
                        print_error(e, location="ProductionAI.generateProductionOrders")
                print ([-1] + sorted([(dist, sys_id) for sys_id, dist in distance_map.items()]))
                use_sys = ([(-1, -1)] + sorted([(dist, sys_id) for sys_id, dist in distance_map.items()]))[:2][-1][-1]  # kinda messy, but ensures a value
            if use_sys != -1:
                try:
                    use_loc = AIstate.colonizedSystems[use_sys][0]
                    res = fo.issueEnqueueBuildingProductionOrder(building_name, use_loc)
                    print "Enqueueing %s at planet %d (%s) , with result %d" % (building_name, use_loc, universe.getPlanet(use_loc).name, res)
                    if res:
                        res = fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                        print "Requeueing %s to front of build queue, with result %d" % (building_name, res)
                except:
                    print "problem queueing BLD_NEUTRONIUM_EXTRACTOR at planet", use_loc, "of system ", use_sys
                    pass

    bld_name = "BLD_SHIPYARD_CON_GEOINT"
    build_ship_facilities(bld_name, best_pilot_facilities)

    # with current stats the AI considers Titanic Hull superior to Scattered Asteroid, so don't bother building for now
    # TODO: uncomment once dynamic assessment of prospective designs is enabled & indicates building is worthwhile
    bld_name = "BLD_SHIPYARD_AST_REF"
    build_ship_facilities(bld_name, best_pilot_facilities)

    bld_name = "BLD_NEUTRONIUM_FORGE"
    priority_facilities = ["BLD_SHIPYARD_ENRG_SOLAR",
                           "BLD_SHIPYARD_CON_GEOINT",
                           "BLD_SHIPYARD_AST_REF",
                           "BLD_SHIPYARD_ENRG_COMP"]
    # not a problem if locs appear multiple times here
    # TODO: also cover good troopship locations
    top_locs = list(loc for facil in priority_facilities for loc in best_pilot_facilities.get(facil, []))
    build_ship_facilities(bld_name, best_pilot_facilities, top_locs)

    colony_ship_map = {}
    for fid in FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.COLONISATION):
        fleet = universe.getFleet(fid)
        if not fleet:
            continue
        for shipID in fleet.shipIDs:
            ship = universe.getShip(shipID)
            if ship and (foAI.foAIstate.get_ship_role(ship.design.id) == ShipRoleType.CIVILIAN_COLONISATION):
                colony_ship_map.setdefault(ship.speciesName, []).append(1)

    building_name = "BLD_CONC_CAMP"
    verbose_camp = False
    building_type = fo.getBuildingType(building_name)
    for pid in AIstate.popCtrIDs:
        planet = universe.getPlanet(pid)
        if not planet:
            continue
        can_build_camp = building_type.canBeProduced(empire.empireID, pid) and empire.buildingTypeAvailable(building_name)
        t_pop = planet.currentMeterValue(fo.meterType.targetPopulation)
        c_pop = planet.currentMeterValue(fo.meterType.population)
        t_ind = planet.currentMeterValue(fo.meterType.targetIndustry)
        c_ind = planet.currentMeterValue(fo.meterType.industry)
        pop_disqualified = (c_pop <= 32) or (c_pop < 0.9*t_pop)
        built_camp = False
        this_spec = planet.speciesName
        safety_margin_met = ((this_spec in ColonisationAI.empire_colonizers and (len(ColonisationAI.empire_species.get(this_spec, []) + colony_ship_map.get(this_spec, [])) >= 2)) or (c_pop >= 50))
        if pop_disqualified or not safety_margin_met:  # check even if not aggressive, etc, just in case acquired planet with a ConcCamp on it
            if can_build_camp:
                if pop_disqualified:
                    if verbose_camp:
                        print "Conc Camp disqualified at %s due to low pop: current %.1f target: %.1f" % (planet.name, c_pop, t_pop)
                else:
                    if verbose_camp:
                        print "Conc Camp disqualified at %s due to safety margin; species %s, colonizing planets %s, with %d colony ships" % (planet.name, planet.speciesName, ColonisationAI.empire_species.get(planet.speciesName, []), len(colony_ship_map.get(planet.speciesName, [])))
            for bldg in planet.buildingIDs:
                if universe.getObject(bldg).buildingTypeName == building_name:
                    res = fo.issueScrapOrder(bldg)
                    print "Tried scrapping %s at planet %s, got result %d" % (building_name, planet.name, res)
        elif foAI.foAIstate.aggression > fo.aggression.typical and can_build_camp and (t_pop >= 36):
            if (planet.focus == FocusType.FOCUS_GROWTH) or ("COMPUTRONIUM_SPECIAL" in planet.specials) or (pid == capital_id):
                continue
                # pass  # now that focus setting takes these into account, probably works ok to have conc camp, but let's not push it
            queued_building_locs = [element.locationID for element in production_queue if (element.name == building_name)]
            if c_pop < 0.95 * t_pop:
                if verbose_camp:
                    print "Conc Camp disqualified at %s due to pop: current %.1f target: %.1f" % (planet.name, c_pop, t_pop)
            else:
                if pid not in queued_building_locs:
                    if planet.focus in [FocusType.FOCUS_INDUSTRY]:
                        if c_ind >= t_ind + c_pop:
                            continue
                    else:
                        old_focus = planet.focus
                        fo.issueChangeFocusOrder(pid, FocusType.FOCUS_INDUSTRY)
                        universe.updateMeterEstimates([pid])
                        t_ind = planet.currentMeterValue(fo.meterType.targetIndustry)
                        if c_ind >= t_ind + c_pop:
                            fo.issueChangeFocusOrder(pid, old_focus)
                            universe.updateMeterEstimates([pid])
                            continue
                    res = fo.issueEnqueueBuildingProductionOrder(building_name, pid)
                    built_camp = res
                    print "Enqueueing %s at planet %d (%s) , with result %d" % (building_name, pid, universe.getPlanet(pid).name, res)
                    if res:
                        queued_building_locs.append(pid)
                        fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                    else:
                        # TODO: enable location condition reporting a la mapwnd BuildDesignatorWnd
                        print "Error enqueing Conc Camp at %s despite building_type.canBeProduced(empire.empireID, pid) reporting " % planet.name, can_build_camp
        if verbose_camp:
            print "conc camp status at %s : checkedCamp: %s, built_camp: %s" % (planet.name, can_build_camp, built_camp)

    building_name = "BLD_SCANNING_FACILITY"
    if empire.buildingTypeAvailable(building_name):
        queued_locs = [element.locationID for element in production_queue if (element.name == building_name)]
        scanner_locs = {}
        for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
            planet = universe.getPlanet(pid)
            if planet:
                if (pid in queued_locs) or (building_name in [bld.buildingTypeName for bld in map(universe.getObject, planet.buildingIDs)]):
                    scanner_locs[planet.systemID] = True
        max_scanner_builds = max(1, int(empire.productionPoints / 30))
        for sys_id in AIstate.colonizedSystems:
            if len(queued_locs) >= max_scanner_builds:
                break
            if sys_id in scanner_locs:
                continue
            need_scanner = False
            for nSys in dict_from_map(universe.getSystemNeighborsMap(sys_id, empire.empireID)):
                if universe.getVisibility(nSys, empire.empireID) < fo.visibility.partial:
                    need_scanner = True
                    break
            if not need_scanner:
                continue
            build_locs = []
            for pid in AIstate.colonizedSystems[sys_id]:
                planet = universe.getPlanet(pid)
                if not planet:
                    continue
                build_locs.append((planet.currentMeterValue(fo.meterType.maxTroops), pid))
            if not build_locs:
                continue
            for troops, loc in sorted(build_locs):
                planet = universe.getPlanet(loc)
                res = fo.issueEnqueueBuildingProductionOrder(building_name, loc)
                print "Enqueueing %s at planet %d (%s) , with result %d" % (building_name, loc, planet.name, res)
                if res:
                    res = fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                    print "Requeueing %s to front of build queue, with result %d" % (building_name, res)
                    queued_locs.append(planet.systemID)
                    break

    building_name = "BLD_SHIPYARD_ORBITAL_DRYDOCK"
    if empire.buildingTypeAvailable(building_name):
        queued_locs = [element.locationID for element in production_queue if (element.name == building_name)]
        queued_sys = set()
        for pid in queued_locs:
            dd_planet = universe.getPlanet(pid)
            if dd_planet:
                queued_sys.add(dd_planet.systemID)
        cur_drydoc_sys = set(ColonisationAI.empire_dry_docks.keys()).union(queued_sys)
        covered_drydoc_locs = set()
        for start_set, dest_set in [(cur_drydoc_sys, covered_drydoc_locs),
                                    (covered_drydoc_locs, covered_drydoc_locs)]:  # coverage of neighbors up to 2 jumps away from a drydock
            for dd_sys_id in start_set.copy():
                dest_set.add(dd_sys_id)
                dd_neighbors = dict_from_map(universe.getSystemNeighborsMap(dd_sys_id, empire.empireID))
                dest_set.update(dd_neighbors.keys())
            
        max_dock_builds = int(0.8 + empire.productionPoints/120.0)
        print "Considering building %s, found current and queued systems %s" % (building_name, ppstring(PlanetUtilsAI.sys_name_ids(cur_drydoc_sys.union(queued_sys))))
        for sys_id, pids_dict in ColonisationAI.empire_species_systems.items():  # TODO: sort/prioritize in some fashion
            pids = pids_dict.get('pids', [])
            local_top_pilots = dict(top_pilot_systems.get(sys_id, []))
            local_drydocks = ColonisationAI.empire_dry_docks.get(sys_id, [])
            if len(queued_locs) >= max_dock_builds:
                print "Drydock enqueing halted with %d of max %d" % (len(queued_locs), max_dock_builds)
                break
            if (sys_id in covered_drydoc_locs) and not local_top_pilots:
                continue
            else:
                pass
            for _, pid in sorted([(local_top_pilots.get(pid, 0), pid) for pid in pids], reverse=True):
                if pid not in ColonisationAI.empire_shipyards:
                    continue
                if pid in local_drydocks or pid in queued_locs:
                    break
                planet = universe.getPlanet(pid)
                res = fo.issueEnqueueBuildingProductionOrder(building_name, pid)
                print "Enqueueing %s at planet %d (%s) , with result %d" % (building_name, pid, planet.name, res)
                if res:
                    queued_locs.append(planet.systemID)
                    covered_drydoc_locs.add(planet.systemID)
                    dd_neighbors = dict_from_map(universe.getSystemNeighborsMap(planet.systemID, empire.empireID))
                    covered_drydoc_locs.update(dd_neighbors.keys())
                    if max_dock_builds >= 2:
                        res = fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                        print "Requeueing %s to front of build queue, with result %d" % (building_name, res)
                    break
                else:
                    print "Error failed enqueueing %s at planet %d (%s) , with result %d" % (building_name, pid, planet.name, res)

    queued_clny_bld_locs = [element.locationID for element in production_queue if element.name.startswith('BLD_COL_')]
    colony_bldg_entries = ([entry for entry in foAI.foAIstate.colonisablePlanetIDs.items() if entry[1][0] > 60 and
                           entry[0] not in queued_clny_bld_locs and entry[0] in ColonisationAI.empire_outpost_ids]
                           [:PriorityAI.allottedColonyTargets+2])
    for entry in colony_bldg_entries:
        pid = entry[0]
        building_name = "BLD_COL_" + entry[1][1][3:]
        planet = universe.getPlanet(pid)
        building_type = fo.getBuildingType(building_name)
        if not (building_type and building_type.canBeEnqueued(empire.empireID, pid)):
            continue
        res = fo.issueEnqueueBuildingProductionOrder(building_name, pid)
        print "Enqueueing %s at planet %d (%s) , with result %d" % (building_name, pid, planet.name, res)
        if res:
            res = fo.issueRequeueProductionOrder(production_queue.size - 1, 2)  # move to near front
            print "Requeueing %s to front of build queue, with result %d" % (building_name, res)
            break
        else:
            print "Error failed enqueueing %s at planet %d (%s) , with result %d" % (building_name, pid, planet.name, res)

    building_name = "BLD_EVACUATION"
    for pid in AIstate.popCtrIDs:
        planet = universe.getPlanet(pid)
        if not planet:
            continue
        for bldg in planet.buildingIDs:
            if universe.getObject(bldg).buildingTypeName == building_name:
                res = fo.issueScrapOrder(bldg)
                print "Tried scrapping %s at planet %s, got result %d" % (building_name, planet.name, res)

    total_pp_spent = fo.getEmpire().productionQueue.totalSpent
    print "  Total Production Points Spent: %s" % total_pp_spent

    wasted_pp = max(0, total_pp - total_pp_spent)
    print "  Wasted Production Points: %s" % wasted_pp  # TODO: add resource group analysis
    avail_pp = total_pp - total_pp_spent - 0.0001

    print
    if False:
        print "Possible ship designs to build:"
        if homeworld:
            for ship_design_id in empire.availableShipDesigns:
                design = fo.getShipDesign(ship_design_id)
                print "    %s cost: %s  time: %s" % (design.name,
                                                     design.productionCost(empire.empireID, homeworld.id),
                                                     design.productionTime(empire.empireID, homeworld.id))
    print
    print "Projects already in Production Queue:"
    production_queue = empire.productionQueue
    print "production summary: %s" % [elem.name for elem in production_queue]
    queued_colony_ships = {}
    queued_outpost_ships = 0
    queued_troop_ships = 0

    # TODO: blocked items might not need dequeuing, but rather for supply lines to be un-blockaded
    dequeue_list = []
    fo.updateProductionQueue()
    can_prioritize_troops = False
    for queue_index in range(len(production_queue)):
        element = production_queue[queue_index]
        block_str = "%d x " % element.blocksize  # ["a single ", "in blocks of %d "%element.blocksize][element.blocksize>1]
        print "    %s%s  requiring %s  more turns; alloc: %.2f PP with cum. progress of %.1f  being built at %s" % (
            block_str, element.name, element.turnsLeft, element.allocation,
            element.progress, universe.getObject(element.locationID).name)
        if element.turnsLeft == -1:
            if element.locationID not in AIstate.popCtrIDs + AIstate.outpostIDs:
                # dequeue_list.append(queue_index) #TODO add assessment of recapture -- invasion target etc.
                print "element %s will never be completed as stands and location %d no longer owned; could consider deleting from queue" % (element.name, element.locationID)  # TODO:
            else:
                print "element %s is projected to never be completed as currently stands, but will remain on queue " % element.name
        elif element.buildType == EmpireProductionTypes.BT_SHIP:
            this_role = foAI.foAIstate.get_ship_role(element.designID)
            if this_role == ShipRoleType.CIVILIAN_COLONISATION:
                this_spec = universe.getPlanet(element.locationID).speciesName
                queued_colony_ships[this_spec] = queued_colony_ships.get(this_spec, 0) + element.remaining * element.blocksize
            elif this_role == ShipRoleType.CIVILIAN_OUTPOST:
                queued_outpost_ships += element.remaining * element.blocksize
            elif this_role == ShipRoleType.BASE_OUTPOST:
                queued_outpost_ships += element.remaining * element.blocksize
            elif this_role == ShipRoleType.MILITARY_INVASION:
                queued_troop_ships += element.remaining * element.blocksize
            elif (this_role == ShipRoleType.CIVILIAN_EXPLORATION) and (queue_index <= 1):
                if len(AIstate.opponentPlanetIDs) > 0:
                    can_prioritize_troops = True
    if queued_colony_ships:
        print "\nFound colony ships in build queue: %s" % queued_colony_ships
    if queued_outpost_ships:
        print "\nFound outpost ships and bases in build queue: %s" % queued_outpost_ships

    for queue_index in dequeue_list[::-1]:
        fo.issueDequeueProductionOrder(queue_index)

    all_military_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.MILITARY)
    total_military_ships = sum([foAI.foAIstate.fleetStatus.get(fid, {}).get('nships', 0) for fid in all_military_fleet_ids])
    all_troop_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.INVASION)
    total_troop_ships = sum([foAI.foAIstate.fleetStatus.get(fid, {}).get('nships', 0) for fid in all_troop_fleet_ids])
    avail_troop_fleet_ids = list(FleetUtilsAI.extract_fleet_ids_without_mission_types(all_troop_fleet_ids))
    total_available_troops = sum([foAI.foAIstate.fleetStatus.get(fid, {}).get('nships', 0) for fid in avail_troop_fleet_ids])
    print "Trooper Status turn %d: %d total, with %d unassigned. %d queued, compared to %d total Military Attack Ships" % (current_turn, total_troop_ships,
                                                                                                                           total_available_troops, queued_troop_ships, total_military_ships)
    if (
        capital_id is not None and
        (current_turn >= 40 or can_prioritize_troops) and
        foAI.foAIstate.systemStatus.get(capital_system_id, {}).get('fleetThreat', 0) == 0 and
        foAI.foAIstate.systemStatus.get(capital_system_id, {}).get('neighborThreat', 0) == 0
    ):
        best_design_id, best_design, build_choices = get_best_ship_info(PriorityType.PRODUCTION_INVASION)
        if build_choices is not None and len(build_choices) > 0:
            loc = random.choice(build_choices)
            prod_time = best_design.productionTime(empire.empireID, loc)
            prod_cost = best_design.productionCost(empire.empireID, loc)
            troopers_needed = max(0, int(min(0.99 + (current_turn/20.0 - total_available_troops)/max(2, prod_time - 1), total_military_ships/3 - total_troop_ships)))
            ship_number = troopers_needed
            per_turn_cost = (float(prod_cost) / prod_time)
            if troopers_needed > 0 and total_pp > 3*per_turn_cost*queued_troop_ships and foAI.foAIstate.aggression >= fo.aggression.typical:
                retval = fo.issueEnqueueShipProductionOrder(best_design_id, loc)
                if retval != 0:
                    print "forcing %d new ship(s) to production queue: %s; per turn production cost %.1f"%(ship_number, best_design.name, ship_number*per_turn_cost)
                    print
                    if ship_number > 1:
                        fo.issueChangeProductionQuantityOrder(production_queue.size - 1, 1, ship_number)
                    avail_pp -= ship_number * per_turn_cost
                    fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                    fo.updateProductionQueue()
        print

    print
    # get the highest production priorities
    production_priorities = {}
    for priority_type in get_priority_production_types():
        production_priorities[priority_type] = int(max(0, (foAI.foAIstate.get_priority(priority_type)) ** 0.5))

    sorted_priorities = production_priorities.items()
    sorted_priorities.sort(lambda x, y: cmp(x[1], y[1]), reverse=True)

    top_score = -1

    num_colony_fleets = len(FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.COLONISATION))  # counting existing colony fleets each as one ship
    total_colony_fleets = sum(queued_colony_ships.values()) + num_colony_fleets
    num_outpost_fleets = len(FleetUtilsAI.get_empire_fleet_ids_by_role(MissionType.OUTPOST))  # counting existing outpost fleets each as one ship
    total_outpost_fleets = queued_outpost_ships + num_outpost_fleets

    max_colony_fleets = PriorityAI.allottedColonyTargets
    max_outpost_fleets = max_colony_fleets

    _, _, colony_build_choices = get_best_ship_info(PriorityType.PRODUCTION_COLONISATION)
    military_emergency = PriorityAI.unmetThreat > (2.0 * MilitaryAI.totMilRating)

    print "Production Queue Priorities:"
    filtered_priorities = {}
    for priority_id, score in sorted_priorities:
        if military_emergency:
            if priority_id == PriorityType.PRODUCTION_EXPLORATION:
                score /= 10.0
            elif priority_id != PriorityType.PRODUCTION_MILITARY:
                score /= 2.0
        if top_score < score:
            top_score = score  # don't really need top_score nor sorting with current handling
        print " Score: %4d -- %s " % (score, priority_id)
        if priority_id != PriorityType.PRODUCTION_BUILDINGS:
            if (priority_id == PriorityType.PRODUCTION_COLONISATION) and (total_colony_fleets < max_colony_fleets) and (colony_build_choices is not None) and len(colony_build_choices) > 0:
                filtered_priorities[priority_id] = score
            elif (priority_id == PriorityType.PRODUCTION_OUTPOST) and (total_outpost_fleets < max_outpost_fleets):
                filtered_priorities[priority_id] = score
            elif priority_id not in [PriorityType.PRODUCTION_OUTPOST, PriorityType.PRODUCTION_COLONISATION]:
                filtered_priorities[priority_id] = score
    if filtered_priorities == {}:
        print "No non-building-production priorities with nonzero score, setting to default: Military"
        filtered_priorities[PriorityType.PRODUCTION_MILITARY] = 1
    if top_score <= 100:
        scaling_power = 1.0
    else:
        scaling_power = math.log(100) / math.log(top_score)
    for pty in filtered_priorities:
        filtered_priorities[pty] **= scaling_power

    available_pp = dict([(tuple(el.key()), el.data()) for el in empire.planetsWithAvailablePP])  # keys are sets of ints; data is doubles
    allocated_pp = dict([(tuple(el.key()), el.data()) for el in empire.planetsWithAllocatedPP])  # keys are sets of ints; data is doubles
    planets_with_wasted_pp = set([tuple(pidset) for pidset in empire.planetsWithWastedPP])
    print "avail_pp ( <systems> : pp ):"
    for planet_set in available_pp:
        print "\t%s\t%.2f" % (ppstring(PlanetUtilsAI.sys_name_ids(set(PlanetUtilsAI.get_systems(planet_set)))), available_pp[planet_set])
    print "\nallocated_pp ( <systems> : pp ):"
    for planet_set in allocated_pp:
        print "\t%s\t%.2f" % (ppstring(PlanetUtilsAI.sys_name_ids(set(PlanetUtilsAI.get_systems(planet_set)))), allocated_pp[planet_set])

    print "\n\nBuilding Ships in system groups with remaining PP:"
    for planet_set in planets_with_wasted_pp:
        total_pp = available_pp.get(planet_set, 0)
        avail_pp = total_pp - allocated_pp.get(planet_set, 0)
        if avail_pp <= 0.01:
            continue
        print "%.2f PP remaining in system group: %s" % (avail_pp, ppstring(PlanetUtilsAI.sys_name_ids(set(PlanetUtilsAI.get_systems(planet_set)))))
        print "\t owned planets in this group are:"
        print "\t %s" % (ppstring(PlanetUtilsAI.planet_name_ids(planet_set)))
        best_design_id, best_design, build_choices = get_best_ship_info(PriorityType.PRODUCTION_COLONISATION, list(planet_set))
        species_map = {}
        for loc in (build_choices or []):
            this_spec = universe.getPlanet(loc).speciesName
            species_map.setdefault(this_spec, []).append(loc)
        colony_build_choices = []
        for pid, (score, this_spec) in foAI.foAIstate.colonisablePlanetIDs.items():
            colony_build_choices.extend(int(math.ceil(score))*[pid2 for pid2 in species_map.get(this_spec, []) if pid2 in planet_set])

        local_priorities = {}
        local_priorities.update(filtered_priorities)
        best_ships = {}
        mil_build_choices = get_best_ship_ratings(planet_set)
        for priority in list(local_priorities):
            if priority == PriorityType.PRODUCTION_MILITARY:
                if not mil_build_choices:
                    del local_priorities[priority]
                    continue
                _, pid, best_design_id, best_design = mil_build_choices[0]
                build_choices = [pid]
                # score = ColonisationAI.pilotRatings.get(pid, 0)
                # if bestScore < ColonisationAI.curMidPilotRating:
            else:
                best_design_id, best_design, build_choices = get_best_ship_info(priority, list(planet_set))
            if best_design is None:
                del local_priorities[priority]  # must be missing a shipyard -- TODO build a shipyard if necessary
                continue
            best_ships[priority] = [best_design_id, best_design, build_choices]
            print "best_ships[%s] = %s \t locs are %s from %s" % (priority, best_design.name, build_choices, planet_set)

        if len(local_priorities) == 0:
            print "Alert!! need shipyards in systemSet ", ppstring(PlanetUtilsAI.sys_name_ids(set(PlanetUtilsAI.get_systems(sorted(planet_set)))))
        priority_choices = []
        for priority in local_priorities:
            priority_choices.extend(int(local_priorities[priority]) * [priority])

        loop_count = 0
        while (avail_pp > 0) and (loop_count < max(100, current_turn)) and (priority_choices != []):  # make sure don't get stuck in some nonbreaking loop like if all shipyards captured
            loop_count += 1
            print "Beginning build enqueue loop %d; %.1f PP available" % (loop_count, avail_pp)
            this_priority = random.choice(priority_choices)
            print "selected priority: ", this_priority
            making_colony_ship = False
            making_outpost_ship = False
            if this_priority == PriorityType.PRODUCTION_COLONISATION:
                if total_colony_fleets >= max_colony_fleets:
                    print "Already sufficient colony ships in queue, trying next priority choice"
                    print
                    for i in range(len(priority_choices) - 1, -1, -1):
                        if priority_choices[i] == PriorityType.PRODUCTION_COLONISATION:
                            del priority_choices[i]
                    continue
                elif colony_build_choices is None or len(colony_build_choices) == 0:
                    for i in range(len(priority_choices) - 1, -1, -1):
                        if priority_choices[i] == PriorityType.PRODUCTION_COLONISATION:
                            del priority_choices[i]
                    continue
                else:
                    making_colony_ship = True
            if this_priority == PriorityType.PRODUCTION_OUTPOST:
                if total_outpost_fleets >= max_outpost_fleets:
                    print "Already sufficient outpost ships in queue, trying next priority choice"
                    print
                    for i in range(len(priority_choices) - 1, -1, -1):
                        if priority_choices[i] == PriorityType.PRODUCTION_OUTPOST:
                            del priority_choices[i]
                    continue
                else:
                    making_outpost_ship = True
            best_design_id, best_design, build_choices = best_ships[this_priority]
            if making_colony_ship:
                loc = random.choice(colony_build_choices)
                best_design_id, best_design, build_choices = get_best_ship_info(PriorityType.PRODUCTION_COLONISATION, loc)
            elif this_priority == PriorityType.PRODUCTION_MILITARY:
                selector = random.random()
                choice = mil_build_choices[0]  # mil_build_choices can't be empty due to earlier check
                for choice in mil_build_choices:
                    if choice[0] >= selector:
                        break
                loc, best_design_id, best_design = choice[1:4]
                if best_design is None:
                    print "Error: problem with mil_build_choices; with selector (%s) chose loc (%s), best_design_id (%s), best_design (None) from mil_build_choices: %s" % (selector, loc, best_design_id, mil_build_choices)
                    continue
            else:
                loc = random.choice(build_choices)

            ship_number = 1
            per_turn_cost = (float(best_design.productionCost(empire.empireID, loc)) / best_design.productionTime(empire.empireID, loc))
            if this_priority == PriorityType.PRODUCTION_MILITARY:
                this_rating = ColonisationAI.pilot_ratings.get(loc, 0)
                rating_ratio = float(this_rating) / ColonisationAI.get_best_pilot_rating()
                if rating_ratio < 0.1:
                    loc_planet = universe.getPlanet(loc)
                    if loc_planet:
                        pname = loc_planet.name
                        this_rating = ColonisationAI.rate_planetary_piloting(loc)
                        rating_ratio = float(this_rating) / ColonisationAI.get_best_pilot_rating()
                        qualifier = ["", "suboptimal"][rating_ratio < 1.0]
                        print "Building mil ship at loc %d (%s) with %s pilot Rating: %.1f; ratio to empire best is %.1f" % (loc, pname, qualifier, this_rating, rating_ratio)
                while total_pp > 40 * per_turn_cost:
                    ship_number *= 2
                    per_turn_cost *= 2
            retval = fo.issueEnqueueShipProductionOrder(best_design_id, loc)
            if retval != 0:
                prioritized = False
                print "adding %d new ship(s) at location %s to production queue: %s; per turn production cost %.1f" % (ship_number, ppstring(PlanetUtilsAI.planet_name_ids([loc])), best_design.name, per_turn_cost)
                print
                if ship_number > 1:
                    fo.issueChangeProductionQuantityOrder(production_queue.size - 1, 1, ship_number)
                avail_pp -= per_turn_cost
                if making_colony_ship:
                    total_colony_fleets += ship_number
                    if total_pp > 4 * per_turn_cost:
                        fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                    continue
                if making_outpost_ship:
                    total_outpost_fleets += ship_number
                    if total_pp > 4 * per_turn_cost:
                        fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                    continue
                if total_pp > 10 * per_turn_cost:
                    leading_block_pp = 0
                    for elem in [production_queue[elemi] for elemi in range(0, min(4, production_queue.size))]:
                        cost, time = empire.productionCostAndTime(elem)
                        leading_block_pp += elem.blocksize * cost / time
                    if leading_block_pp > 0.5 * total_pp or (military_emergency and this_priority == PriorityType.PRODUCTION_MILITARY):
                        prioritized = True
                        fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
                if (not prioritized) and (priority == PriorityType.PRODUCTION_INVASION):
                    fo.issueRequeueProductionOrder(production_queue.size - 1, 0)  # move to front
        print
    fo.updateProductionQueue()


def build_ship_facilities(bld_name, best_pilot_facilities, top_locs=None):
    if top_locs is None:
        top_locs = []
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    total_pp = empire.productionPoints
    min_aggr, prereq_bldg, this_cost, time = AIDependencies.SHIP_FACILITIES.get(bld_name, (None, '', -1, -1))
    if min_aggr is None or min_aggr > foAI.foAIstate.aggression:
        return
    bld_type = fo.getBuildingType(bld_name)
    if not empire.buildingTypeAvailable(bld_name):
        return
    queued_bld_locs = [element.locationID for element in empire.productionQueue if element.name == bld_name]
    if bld_name in AIDependencies.SYSTEM_SHIP_FACILITIES:
        current_locs = ColonisationAI.system_facilities.get(bld_name, {}).get('systems', set())
        current_coverage = current_locs.union(universe.getPlanet(planet_id).systemID for planet_id in queued_bld_locs)
        open_systems = set(universe.getPlanet(pid).systemID
                           for pid in best_pilot_facilities.get("BLD_SHIPYARD_BASE", [])).difference(current_coverage)
        try_systems = open_systems.intersection(ColonisationAI.system_facilities.get(
            prereq_bldg, {}).get('systems', [])) if prereq_bldg else open_systems
        try_locs = set(pid for sys_id in try_systems for pid in AIstate.colonizedSystems.get(sys_id, []))
    else:
        current_locs = best_pilot_facilities.get(bld_name, [])
        try_locs = set(best_pilot_facilities.get(prereq_bldg, [])).difference(
            queued_bld_locs, current_locs)
    print "Considering constructing a %s, have %d already built and %d queued" % (
        bld_name, len(current_locs), len(queued_bld_locs))
    max_under_construction = max(1, (time * total_pp) // (5 * this_cost))
    max_total = max(1, (time * total_pp) // (2 * this_cost))
    print "Allowances: max total: %d, max under construction: %d" % (max_total, max_under_construction)
    if len(current_locs) >= max_total:
        return
    valid_locs = (list(loc for loc in try_locs.intersection(top_locs) if bld_type.canBeProduced(empire.empireID, loc)) +
                  list(loc for loc in try_locs.difference(top_locs) if bld_type.canBeProduced(empire.empireID, loc)))
    print "Have %d potential locations: %s" % (len(valid_locs), map(universe.getPlanet, valid_locs))
    # TODO: rank by defense ability, etc.
    num_queued = len(queued_bld_locs)
    already_covered = []  # just those covered on this turn
    while valid_locs:
        if num_queued >= max_under_construction:
            break
        pid = valid_locs.pop()
        if pid in already_covered:
            continue
        res = fo.issueEnqueueBuildingProductionOrder(bld_name, pid)
        print "Enqueueing %s at planet %s , with result %d" % (bld_name, universe.getPlanet(pid), res)
        if res:
            num_queued += 1
            already_covered.extend(AIstate.colonizedSystems[universe.getPlanet(pid).systemID])
