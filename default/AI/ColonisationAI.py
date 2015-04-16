# get these declared first to help avoid import circularities
import freeOrionAIInterface as fo  # pylint: disable=import-error

import AIDependencies
import AITarget
import AIstate
import FleetUtilsAI
import FreeOrionAI as foAI
import PlanetUtilsAI
import ProductionAI
import TechsListsAI
from EnumsAI import AIFleetMissionType, AIExplorableSystemType, TargetType, AIFocusType
import EnumsAI
from freeorion_tools import dict_from_map, tech_is_complete, get_ai_tag_grade
from freeorion_debug import Timer

colonization_timer = Timer('getColonyFleets()')

empire_species = {}
empire_species_by_planet = {}
empire_species_systems = {}  # TODO: as currently used, is duplicative with combo of foAI.foAIstate.popCtrSystemIDs and foAI.foAIstate.colonizedSystems
empire_colonizers = {}
empire_ship_builders = {}
empire_shipyards = {}
empire_dry_docks = {}
available_growth_specials = {}
empire_planets_with_growth_specials = {}
active_growth_specials = {}
empire_metabolisms = {}
annexable_system_ids = set()
annexable_ring1 = set()
annexable_ring2 = set()
annexable_ring3 = set()
annexable_planet_ids = set()
systems_by_supply_tier = {}
system_supply = {}
cur_best_mil_ship_rating = 20
all_colony_opportunities = {}
gotRuins = False
got_ast = False
got_gg = False
cur_best_pilot_rating = 1e-8
curMidPilotRating = 1e-8
pilot_ratings = {}
colony_status = {}
pop_map = {}
empire_status = {'industrialists': 0, 'researchers': 0}
unowned_empty_planet_ids = set()
empire_outpost_ids = set()
empire_ast_outpost_ids = set()
claimed_stars = {}

ENVIRONS = {str(fo.planetEnvironment.uninhabitable): 0, str(fo.planetEnvironment.hostile): 1,
            str(fo.planetEnvironment.poor): 2, str(fo.planetEnvironment.adequate): 3, str(fo.planetEnvironment.good): 4}
PHOTO_MAP = {fo.starType.blue: 3, fo.starType.white: 1.5, fo.starType.red: -1, fo.starType.neutron: -1,
             fo.starType.blackHole: -10, fo.starType.noStar: -10}
# mods per environ uninhab hostile poor adequate good
POP_SIZE_MOD_MAP = {
    "env": [0, -4, -2, 0, 3],
    "subHab": [0, 1, 1, 1, 1],
    "symBio": [0, 0, 1, 1, 1],
    "xenoGen": [0, 1, 2, 2, 0],
    "xenoHyb": [0, 2, 1, 0, 0],
    "cyborg": [0, 2, 0, 0, 0],
    "ndim": [0, 2, 2, 2, 2],
    "orbit": [0, 1, 1, 1, 1],
    "gaia": [0, 3, 3, 3, 3],
}

NEST_VAL_MAP = {
    "SNOWFLAKE_NEST_SPECIAL": 15,
    "KRAKEN_NEST_SPECIAL": 40,
    "JUGGERNAUT_NEST_SPECIAL": 80,
}

AVG_PILOT_RATING = 2.0
GOOD_PILOT_RATING = 4.0
GREAT_PILOT_RATING = 6.0
ULT_PILOT_RATING = 12.0


def galaxy_is_sparse():
    setup_data = fo.getGalaxySetupData()
    avg_empire_systems = setup_data.size / len(fo.allEmpireIDs())
    return ((setup_data.monsterFrequency <= fo.galaxySetupOption.low) and
            ((avg_empire_systems >= 40) or
             ((avg_empire_systems >= 35) and (setup_data.shape != fo.galaxyShape.elliptical))))


def rate_piloting_tag(tag_list):
    grades = {'NO': 1e-8, 'BAD': 0.75, 'GOOD': GOOD_PILOT_RATING, 'GREAT': GREAT_PILOT_RATING,
              'ULTIMATE': ULT_PILOT_RATING}
    return grades.get(get_ai_tag_grade(tag_list, "WEAPONS"), 1.0)


def rate_planetary_piloting(pid):
    universe = fo.getUniverse()
    planet = universe.getPlanet(pid)
    if not planet or not planet.speciesName:
        return 0.0
    this_spec = fo.getSpecies(planet.speciesName)
    if not this_spec:
        return 0.0
    return rate_piloting_tag(this_spec.tags)


def check_supply():
    # get suppliable systems and planets
    colonization_timer.start('Getting Empire Supply Info')
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empire_id = empire.empireID
    fleet_suppliable_system_ids = empire.fleetSupplyableSystemIDs
    fleet_suppliable_planet_ids = PlanetUtilsAI.get_planets_in__systems_ids(fleet_suppliable_system_ids)
    print
    print "    fleet_suppliable_system_ids: %s" % list(fleet_suppliable_system_ids)
    print "    fleet_suppliable_planet_ids: %s" % fleet_suppliable_planet_ids
    print

    print "-------\nEmpire Obstructed Starlanes:"
    print list(empire.obstructedStarlanes())
    colonization_timer.start('Determining Annexable Systems')

    annexable_system_ids.clear()  # TODO: distinguish colony-annexable systems and outpost-annexable systems
    annexable_ring1.clear()
    annexable_ring2.clear()
    annexable_ring3.clear()
    annexable_planet_ids.clear()
    systems_by_supply_tier.clear()
    system_supply.clear()
    supply_distance = 0
    for tech in AIDependencies.supply_range_techs:
        if tech_is_complete(tech):
            supply_distance += AIDependencies.supply_range_techs[tech]
    foAI.foAIstate.misc['supply_tech'] = supply_distance
    supply_distance += 4  # 2 for up to great supply species, and 2 for possible tiny planets
    # if foAI.foAIstate.aggression >= fo.aggression.aggressive:
    # supply_distance += 1
    for sys_id in empire.fleetSupplyableSystemIDs:
        annexable_system_ids.add(sys_id)  # add fleet suppliable system
        for nID in universe.getImmediateNeighbors(sys_id, empire_id):
            annexable_system_ids.add(nID)  # add immediate neighbor of fleet suppliable system
    if supply_distance > 1:
        for sys_id in list(annexable_system_ids):
            for nID in universe.getImmediateNeighbors(sys_id, empire_id):
                annexable_ring1.add(nID)
        annexable_ring1.difference_update(annexable_system_ids)
        annexable_system_ids.update(annexable_ring1)
    if supply_distance > 2:
        for sys_id in list(annexable_ring1):
            for nID in universe.getImmediateNeighbors(sys_id, empire_id):
                annexable_ring2.add(nID)
        annexable_ring2.difference_update(annexable_system_ids)
        annexable_system_ids.update(annexable_ring2)
    if supply_distance > 3:
        for sys_id in list(annexable_ring2):
            for nID in universe.getImmediateNeighbors(sys_id, empire_id):
                annexable_ring3.add(nID)
        annexable_ring3.difference_update(annexable_system_ids)
        annexable_system_ids.update(annexable_ring3)
    annexable_planet_ids.update(PlanetUtilsAI.get_planets_in__systems_ids(annexable_system_ids))
    print "First Ring of annexable systems:", PlanetUtilsAI.sys_name_ids(annexable_ring1)
    print "Second Ring of annexable systems:", PlanetUtilsAI.sys_name_ids(annexable_ring2)
    print "Third Ring of annexable systems:", PlanetUtilsAI.sys_name_ids(annexable_ring3)
    # print "standard supply calc took ", supp_timing[0][-1]-supp_timing[0][-2]
    print
    new_supply_map = empire.supplyProjections(-3, False)
    print "New Supply Calc:"
    print "Known Systems:", list(universe.systemIDs)
    print "Base Supply:", dict_from_map(empire.systemSupplyRanges)
    for el in new_supply_map:
        # print PlanetUtilsAI.sys_name_ids([el.key()]), ' -- ', el.data()
        systems_by_supply_tier.setdefault(min(0, el.data()), []).append(el.key())
        system_supply[el.key()] = el.data()
    print "New Supply connected systems: ", PlanetUtilsAI.sys_name_ids(systems_by_supply_tier.get(0, []))
    print "New First Ring of annexable systems: ", PlanetUtilsAI.sys_name_ids(systems_by_supply_tier.get(-1, []))
    print "New Second Ring of annexable systems: ", PlanetUtilsAI.sys_name_ids(systems_by_supply_tier.get(-2, []))
    print "New Third Ring of annexable systems: ", PlanetUtilsAI.sys_name_ids(systems_by_supply_tier.get(-3, []))
    # print "new supply calc took ", new_time-supp_timing[0][-1]
    annexable_system_ids.clear()  # TODO: distinguish colony-annexable systems and outpost-annexable systems
    annexable_ring1.clear()
    annexable_ring2.clear()
    annexable_ring3.clear()
    annexable_ring1.update(systems_by_supply_tier.get(-1, []))
    annexable_ring2.update(systems_by_supply_tier.get(-2, []))
    annexable_ring3.update(systems_by_supply_tier.get(-3, []))
    # annexableSystemIDs.update(systems_by_supply_tier.get(0, []), annexableRing1, annexableRing2, annexableRing3)
    for jumps in range(0, -1 - supply_distance, -1):
        annexable_system_ids.update(systems_by_supply_tier.get(jumps, []))
    colonization_timer.stop()
    return fleet_suppliable_planet_ids


def survey_universe():
    global gotRuins, got_ast, got_gg, cur_best_pilot_rating, curMidPilotRating
    univ_stats = {}
    fleet_suppliable_planet_ids = check_supply()
    colonization_timer.start("Categorizing Visible Planets")
    univ_stats['fleetSupplyablePlanetIDs'] = fleet_suppliable_planet_ids
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empire_id = empire.empireID
    current_turn = fo.currentTurn()

    # get outpost and colonization planets
    explored_system_ids = foAI.foAIstate.get_explorable_systems(AIExplorableSystemType.EXPLORABLE_SYSTEM_EXPLORED)
    un_ex_sys_ids = list(foAI.foAIstate.get_explorable_systems(AIExplorableSystemType.EXPLORABLE_SYSTEM_UNEXPLORED))
    un_ex_systems = map(universe.getSystem, un_ex_sys_ids)
    print "Unexplored Systems: %s " % [(sysID, (sys and sys.name) or "name unknown") for sysID, sys in
                                       zip(un_ex_sys_ids, un_ex_systems)]
    print "Explored SystemIDs: " + str(list(explored_system_ids))

    explored_planet_ids = PlanetUtilsAI.get_planets_in__systems_ids(explored_system_ids)
    print "Explored PlanetIDs: %s" % explored_planet_ids
    print

    # visibleSystemIDs = foAI.foAIstate.visInteriorSystemIDs.keys() + foAI.foAIstate. visBorderSystemIDs.keys()
    # visiblePlanetIDs = PlanetUtilsAI.get_planets_in__systems_ids(visibleSystemIDs)
    # print "VisiblePlanets: %s "%[ (pid, (universe.getPlanet(pid) and universe.getPlanet(pid).name) or "unknown") for pid in visiblePlanetIDs]
    # accessibleSystemIDs = [sysID for sysID in visibleSystemIDs if universe.systemsConnected(sysID, homeSystemID, empireID) ]
    # acessiblePlanetIDs = PlanetUtilsAI.get_planets_in__systems_ids(accessibleSystemIDs)

    # set up / reset various variables; the 'if' is purely for code folding convenience
    if True:
        claimed_stars.clear()
        colony_status['colonies_under_attack'] = []
        colony_status['colonies_under_threat'] = []
        pop_map.clear()
        empire_status.clear()
        empire_status.update({'industrialists': 0, 'researchers': 0})
        AIstate.empireStars.clear()

        # empire_owned_planet_ids = PlanetUtilsAI.get_owned_planets_by_empire(universe.planetIDs, empireID)
        # print "Empire Owned PlanetIDs: " + str(empire_owned_planet_ids)
        ##allOwnedPlanetIDs = PlanetUtilsAI.get_all_owned_planet_ids(explored_planet_ids) #working with Explored systems not all 'visible' because might not have a path to the latter
        # allOwnedPlanetIDs = PlanetUtilsAI.get_all_owned_planet_ids(annexablePlanetIDs) #
        #print "All annexable Owned or Populated PlanetIDs: " + str(set(allOwnedPlanetIDs)-set(empire_owned_planet_ids))
        ##unowned_empty_planet_ids = list(set(explored_planet_ids) -set(allOwnedPlanetIDs))
        #unowned_empty_planet_ids = list(set(annexablePlanetIDs) -set(allOwnedPlanetIDs))
        #print "UnOwned annexable PlanetIDs: ", ", ".join(PlanetUtilsAI.planet_name_ids(unowned_empty_planet_ids))
        empire_owned_planet_ids = []
        empire_pop_ctrs = set()
        empire_outpost_ids.clear()
        empire_ast_outpost_ids.clear()

        old_pop_ctrs = []
        for specn in empire_species:
            old_pop_ctrs.extend(empire_species[specn])
        old_emp_spec = dict(empire_species)
        empire_species.clear()
        empire_species_by_planet.clear()
        old_emp_col = {}
        old_emp_col.update(empire_colonizers)
        empire_colonizers.clear()
        empire_ship_builders.clear()
        empire_shipyards.clear()
        empire_dry_docks.clear()
        empire_metabolisms.clear()
        available_growth_specials.clear()
        active_growth_specials.clear()
        empire_planets_with_growth_specials.clear()
        if tech_is_complete(TechsListsAI.EXOBOT_TECH_NAME):
            empire_colonizers["SP_EXOBOT"] = []  # get it into colonizer list even if no colony yet
        empire_species_systems.clear()
        AIstate.popCtrIDs[:] = []
        AIstate.popCtrSystemIDs[:] = []
        AIstate.outpostIDs[:] = []
        AIstate.outpostSystemIDs[:] = []
        AIstate.colonizedSystems.clear()
        pilot_ratings.clear()
        unowned_empty_planet_ids.clear()
    # var setup done

    for sys_id in universe.systemIDs:
        sys = universe.getSystem(sys_id)
        if not sys:
            continue
        empire_has_colony_in_sys = False
        empire_has_pop_ctr_in_sys = False
        local_ast = False
        local_gg = False
        empire_has_qualifying_planet = False
        for pid in sys.planetIDs:
            planet = universe.getPlanet(pid)
            if not planet:
                continue
            if pid in foAI.foAIstate.colonisablePlanetIDs:
                empire_has_qualifying_planet = True
            if planet.size == fo.planetSize.asteroids:
                local_ast = True
            elif planet.size == fo.planetSize.gasGiant:
                local_gg = True
            spec_name = planet.speciesName
            this_spec = fo.getSpecies(spec_name)
            owner_id = planet.owner
            planet_population = planet.currentMeterValue(fo.meterType.population)
            buildings_here = [universe.getObject(bldg).buildingTypeName for bldg in planet.buildingIDs]
            if owner_id == empire_id:
                empire_has_colony_in_sys = True
                empire_owned_planet_ids.append(pid)
                AIstate.colonizedSystems.setdefault(sys_id, []).append(
                    pid)  # track these to plan Solar Generators and Singularity Generators, etc.
                pop_map[pid] = planet_population
                if planet_population <= 0.0:
                    empire_outpost_ids.add(pid)
                    AIstate.outpostIDs.append(pid)
                    if planet.type == fo.planetType.asteroids:
                        empire_ast_outpost_ids.add(pid)
                else:
                    empire_pop_ctrs.add(pid)
                    empire_has_qualifying_planet = True
                    AIstate.popCtrIDs.append(pid)
                    empire_species_systems.setdefault(sys_id, {}).setdefault('pids', []).append(pid)
                    empire_has_pop_ctr_in_sys = True
                    empire_species_by_planet[pid] = spec_name
                    empire_species.setdefault(spec_name, []).append(pid)
                    for metab in [tag for tag in this_spec.tags if tag in AIDependencies.metabolismBoostMap]:
                        empire_metabolisms[metab] = empire_metabolisms.get(metab, 0.0) + planet.size
                    if this_spec.canProduceShips:
                        pilot_val = rate_piloting_tag(list(this_spec.tags))
                        if spec_name == "SP_ACIREMA":
                            pilot_val += 1
                        pilot_ratings[pid] = pilot_val
                        yard_here = []
                        if "BLD_SHIPYARD_BASE" in buildings_here:
                            empire_ship_builders.setdefault(spec_name, []).append(pid)
                            empire_shipyards[pid] = pilot_val
                            yard_here = [pid]
                        if this_spec.canColonize:
                            empire_colonizers.setdefault(spec_name, []).extend(yard_here)
                if planet.focus == EnumsAI.AIFocusType.FOCUS_INDUSTRY:
                    empire_status['industrialists'] += planet_population
                elif planet.focus == EnumsAI.AIFocusType.FOCUS_RESEARCH:
                    empire_status['researchers'] += planet_population
                if "ANCIENT_RUINS_SPECIAL" in planet.specials:
                    gotRuins = True
                for special in planet.specials:
                    if special in AIDependencies.metabolismBoosts:
                        empire_planets_with_growth_specials.setdefault(pid, []).append(special)
                        available_growth_specials.setdefault(special, []).append(pid)
                        if planet.focus == AIFocusType.FOCUS_GROWTH:
                            active_growth_specials.setdefault(special, []).append(pid)
                if "BLD_SHIPYARD_ORBITAL_DRYDOCK" in buildings_here:
                    empire_dry_docks.setdefault(planet.systemID, []).append(pid)
            elif owner_id == -1:
                if spec_name == "":
                    unowned_empty_planet_ids.add(pid)
            else:
                partial_vis_turn = universe.getVisibilityTurnsMap(pid, empire_id).get(fo.visibility.partial, -9999)
                if partial_vis_turn >= current_turn - 1:  # only interested in immediately recent data
                    foAI.foAIstate.misc.setdefault('enemies_sighted', {}).setdefault(current_turn, []).append(pid)

        if empire_has_qualifying_planet:
            if local_ast:
                got_ast = True
            elif local_gg:
                got_gg = True

        if empire_has_colony_in_sys:
            if empire_has_pop_ctr_in_sys:
                AIstate.popCtrSystemIDs.append(sys_id)
            else:
                AIstate.outpostSystemIDs.append(sys_id)
            AIstate.empireStars.setdefault(sys.starType, []).append(sys_id)
            sys_status = foAI.foAIstate.systemStatus.setdefault(sys_id, {})
            if sys_status.get('fleetThreat', 0) > 0:
                colony_status['colonies_under_attack'].append(sys_id)
            if sys_status.get('neighborThreat', 0) > 0:
                colony_status['colonies_under_threat'].append(sys_id)

    if empire_species != old_emp_spec:
        print "Old empire species: %s ; new empire species: %s" % (old_emp_spec, empire_species)
    if empire_colonizers != old_emp_col:
        print "Old empire colonizers: %s ; new empire colonizers: %s" % (old_emp_col, empire_colonizers)

    print "\n" + "Empire species roster:"
    for spec_name in empire_species:
        this_spec = fo.getSpecies(spec_name)
        print "%s on planets %s; can%s colonize from %d shipyards; has tags %s" % (
            spec_name, empire_species[spec_name], ["not", ""][spec_name in empire_colonizers],
            len(empire_ship_builders.get(spec_name, [])), list(this_spec.tags))
    print ""
    if len(pilot_ratings) != 0:
        rating_list = sorted(pilot_ratings.values(), reverse=True)
        cur_best_pilot_rating = rating_list[0]
        if len(pilot_ratings) == 1:
            curMidPilotRating = rating_list[0]
        else:
            curMidPilotRating = rating_list[1 + int(len(rating_list) / 5)]
    else:
        cur_best_pilot_rating = 1e-8
        curMidPilotRating = 1e-8

        # the idea behind this was to note systems that the empire has claimed-- either has a current colony or has targetted
        # for making/invading a colony
        # claimedStars = {}
        # for sType in AIstate.empireStars:
        #claimedStars[sType] = list( AIstate.empireStars[sType] )
        #for sysID in set( AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs):
        #tSys = universe.getSystem(sysID)
        #if not tSys: continue
        #claimedStars.setdefault( tSys.starType, []).append(sysID)
    # foAI.foAIstate.misc['claimedStars'] = claimedStars
    colonization_timer.stop()
    return univ_stats


def get_colony_fleets():
    """examines known planets, collects various colonization data, to be later used to send colony fleets"""
    global cur_best_mil_ship_rating
    colonization_timer.start("Getting best milship rating")

    cur_best_mil_ship_rating = ProductionAI.cur_best_mil_ship_rating()
    colonization_timer.start('Getting avail colony fleets')

    all_colony_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(AIFleetMissionType.FLEET_MISSION_COLONISATION)
    AIstate.colonyFleetIDs[:] = FleetUtilsAI.extract_fleet_ids_without_mission_types(all_colony_fleet_ids)

    # get suppliable systems and planets
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    #
    ## survey universe -----------------------------------------------------------
    #
    univ_stats = survey_universe()

    fleet_supplyable_planet_ids = univ_stats['fleetSupplyablePlanetIDs']
    colonization_timer.start('Identify Existing colony/outpost targets')

    # export colony targeted systems for other AI modules
    colony_targeted_planet_ids = get_colony_targeted_planet_ids(universe.planetIDs,
                                                                AIFleetMissionType.FLEET_MISSION_COLONISATION)
    all_colony_targeted_system_ids = PlanetUtilsAI.get_systems(colony_targeted_planet_ids)
    AIstate.colonyTargetedSystemIDs = all_colony_targeted_system_ids
    print
    print "Colony Targeted SystemIDs: %s" % AIstate.colonyTargetedSystemIDs
    print "Colony Targeted PlanetIDs: %s" % colony_targeted_planet_ids

    colony_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(AIFleetMissionType.FLEET_MISSION_COLONISATION)
    if not colony_fleet_ids:
        print "Available Colony Fleets: 0"
    else:
        print "Colony FleetIDs: " % FleetUtilsAI.get_empire_fleet_ids_by_role(
            AIFleetMissionType.FLEET_MISSION_COLONISATION)

    num_colony_fleets = len(FleetUtilsAI.extract_fleet_ids_without_mission_types(colony_fleet_ids))
    print "Colony Fleets Without Missions: %s" % num_colony_fleets
    outpost_targeted_planet_ids = get_outpost_targeted_planet_ids(universe.planetIDs,
                                                                  AIFleetMissionType.FLEET_MISSION_OUTPOST)
    outpost_targeted_planet_ids.extend(
        get_outpost_targeted_planet_ids(universe.planetIDs, AIFleetMissionType.FLEET_MISSION_ORBITAL_OUTPOST))
    all_outpost_targeted_system_ids = PlanetUtilsAI.get_systems(outpost_targeted_planet_ids)

    # export outpost targeted systems for other AI modules
    AIstate.outpostTargetedSystemIDs = all_outpost_targeted_system_ids
    print
    print "Outpost Targeted SystemIDs: " + str(AIstate.outpostTargetedSystemIDs)
    print "Outpost Targeted PlanetIDs: " + str(outpost_targeted_planet_ids)

    outpost_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(AIFleetMissionType.FLEET_MISSION_OUTPOST)
    if not outpost_fleet_ids:
        print "Available Outpost Fleets: 0"
    else:
        print "Outpost FleetIDs: %s" % FleetUtilsAI.get_empire_fleet_ids_by_role(
            AIFleetMissionType.FLEET_MISSION_OUTPOST)

    num_outpost_fleets = len(FleetUtilsAI.extract_fleet_ids_without_mission_types(outpost_fleet_ids))
    print "Outpost Fleets Without Missions: %s" % num_outpost_fleets
    colonization_timer.start('Identify colony base targets')

    available_pp = dict([(tuple(el.key()), el.data()) for el in
                         empire.planetsWithAvailablePP])  # keys are sets of ints; data is doubles
    avail_pp_by_sys = {}
    for p_set in available_pp:
        avail_pp_by_sys.update([(sys_id, available_pp[p_set]) for sys_id in set(PlanetUtilsAI.get_systems(p_set))])
    colony_cost = AIDependencies.COLONY_POD_COST * (1 + AIDependencies.COLONY_POD_UPKEEP * len(list(AIstate.popCtrIDs)))
    outpost_cost = AIDependencies.OUTPOST_POD_COST * (
        1 + AIDependencies.COLONY_POD_UPKEEP * len(list(AIstate.popCtrIDs)))
    production_queue = empire.productionQueue
    queued_outpost_bases = []
    queued_colony_bases = []
    for queue_index in range(0, len(production_queue)):
        element = production_queue[queue_index]
        if element.buildType == EnumsAI.AIEmpireProductionTypes.BT_SHIP:
            if foAI.foAIstate.get_ship_role(element.designID) in [EnumsAI.AIShipRoleType.SHIP_ROLE_BASE_OUTPOST]:
                build_planet = universe.getPlanet(element.locationID)
                queued_outpost_bases.append(build_planet.systemID)
            elif foAI.foAIstate.get_ship_role(element.designID) in [EnumsAI.AIShipRoleType.SHIP_ROLE_BASE_COLONISATION]:
                build_planet = universe.getPlanet(element.locationID)
                queued_colony_bases.append(build_planet.systemID)

    evaluated_colony_planet_ids = list(unowned_empty_planet_ids.union(AIstate.outpostIDs) - set(
        colony_targeted_planet_ids))  # places for possible colonyBase

    # foAI.foAIstate.qualifyingOutpostBaseTargets.clear() #don't want to lose the info by clearing, but #TODO: should double check if still own colonizer planet
    # foAI.foAIstate.qualifyingColonyBaseTargets.clear()
    cost_ratio = 120  # TODO: temp ratio; reest to 12 *; consider different ratio
    for pid in evaluated_colony_planet_ids:  # TODO: reorganize
        planet = universe.getPlanet(pid)
        if not planet:
            continue
        sys_id = planet.systemID
        for pid2 in empire_species_systems.get(sys_id, {}).get('pids', []):
            planet2 = universe.getPlanet(pid2)
            if not (planet2 and planet2.speciesName in empire_colonizers):
                continue
            if pid not in foAI.foAIstate.qualifyingOutpostBaseTargets:
                if planet.unowned:
                    foAI.foAIstate.qualifyingOutpostBaseTargets.setdefault(pid, [pid2, -1])
            if ((pid not in foAI.foAIstate.qualifyingColonyBaseTargets) and
                    (colony_cost < cost_ratio * avail_pp_by_sys.get(sys_id, 0)) and
                    (planet.unowned or pid in empire_outpost_ids)):
                # TODO: enable actual building, remove from outpostbases, check other local colonizers for better score
                foAI.foAIstate.qualifyingColonyBaseTargets.setdefault(pid, [pid2, -1])

    # print "Evaluated Colony PlanetIDs: " + str(evaluated_colony_planet_ids)
    colonization_timer.start('Initiate outpost base construction')

    reserved_outpost_base_targets = foAI.foAIstate.qualifyingOutpostBaseTargets.keys()
    max_queued_outpost_bases = max(1, int(2 * empire.productionPoints / outpost_cost))
    print "considering possible outpost bases at %s" % reserved_outpost_base_targets
    if tech_is_complete(AIDependencies.OUTPOSTING_TECH):
        for pid in (set(reserved_outpost_base_targets) - set(outpost_targeted_planet_ids)):
            if len(queued_outpost_bases) >= max_queued_outpost_bases:
                print "too many queued outpost bases to build any more now"
                break
            if pid not in unowned_empty_planet_ids:
                continue
            if foAI.foAIstate.qualifyingOutpostBaseTargets[pid][1] != -1:
                continue  # already building for here
            loc = foAI.foAIstate.qualifyingOutpostBaseTargets[pid][0]
            this_score = evaluate_planet(pid, EnumsAI.AIFleetMissionType.FLEET_MISSION_OUTPOST, None, empire, [])
            planet = universe.getPlanet(pid)
            if this_score == 0:
                # print "Potential outpost base (rejected) for %s to be built at planet id(%d); outpost score %.1f" % ( ((planet and planet.name) or "unknown"), loc, this_score)
                continue
            print "Potential outpost base for %s to be built at planet id(%d); outpost score %.1f" % (
                ((planet and planet.name) or "unknown"), loc, this_score)
            if this_score < 100:
                print "Potential outpost base (rejected) for %s to be built at planet id(%d); outpost score %.1f" % (
                    ((planet and planet.name) or "unknown"), loc, this_score)
                continue
            best_ship, col_design, build_choices = ProductionAI.getBestShipInfo(
                EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_ORBITAL_OUTPOST, loc)
            if not best_ship:
                print "Error: no outpost base can be built at ", PlanetUtilsAI.planet_name_ids([loc])
                continue
            # print "selecting ", PlanetUtilsAI.planet_name_ids([pid]), " to build Orbital Defenses"
            retval = fo.issueEnqueueShipProductionOrder(best_ship, loc)
            print "Enqueueing Outpost Base at %s for %s" % (
                PlanetUtilsAI.planet_name_ids([loc]), PlanetUtilsAI.planet_name_ids([pid]))
            if retval:
                foAI.foAIstate.qualifyingOutpostBaseTargets[pid][1] = loc
                queued_outpost_bases.append((planet and planet.systemID) or -1)
                # res=fo.issueRequeueProductionOrder(production_queue.size -1, 0) # TODO: evaluate move to front
    colonization_timer.start('Evaluate Primary Colony Opportunities')

    evaluated_outpost_planet_ids = list(
        unowned_empty_planet_ids - set(outpost_targeted_planet_ids) - set(colony_targeted_planet_ids) - set(
            reserved_outpost_base_targets))

    # print "Evaluated Outpost PlanetIDs: " + str(evaluated_outpost_planet_ids)

    evaluated_colony_planets = assign_colonisation_values(evaluated_colony_planet_ids,
                                                          AIFleetMissionType.FLEET_MISSION_COLONISATION, None, empire)
    colonization_timer.stop('Evaluate %d Primary Colony Opportunities' % (len(evaluated_outpost_planet_ids)))
    colonization_timer.start('Evaluate All Colony Opportunities')
    all_colony_opportunities.clear()
    all_colony_opportunities.update(
        assign_colonisation_values(evaluated_colony_planet_ids, AIFleetMissionType.FLEET_MISSION_COLONISATION, None,
                                   empire, [], True))
    colonization_timer.start('Evaluate Outpost Opportunities')

    sorted_planets = evaluated_colony_planets.items()
    sorted_planets.sort(lambda x, y: cmp(x[1], y[1]), reverse=True)

    print
    print "Settleable Colony Planets (score,species) | ID | Name | Specials:"
    for planet_id, (score, spec) in sorted_planets:
        if score > 0.5:
            print "   %15s | %5s | %s | %s " % (
                (score, spec), planet_id, universe.getPlanet(planet_id).name,
                list(universe.getPlanet(planet_id).specials))
    print

    sorted_planets = [(planet_id, score) for planet_id, score in sorted_planets if score[0] > 0]
    # export planets for other AI modules
    foAI.foAIstate.colonisablePlanetIDs.clear()
    foAI.foAIstate.colonisablePlanetIDs.update(sorted_planets)

    # get outpost fleets
    all_outpost_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(AIFleetMissionType.FLEET_MISSION_OUTPOST)
    AIstate.outpostFleetIDs = FleetUtilsAI.extract_fleet_ids_without_mission_types(all_outpost_fleet_ids)

    evaluated_outpost_planets = assign_colonisation_values(evaluated_outpost_planet_ids,
                                                           AIFleetMissionType.FLEET_MISSION_OUTPOST, None, empire)
    colonization_timer.stop()

    sorted_outposts = evaluated_outpost_planets.items()
    sorted_outposts.sort(lambda x, y: cmp(x[1], y[1]), reverse=True)

    print "Settleable Outpost PlanetIDs:"
    for planet_id, score in sorted_outposts:
        if score > 0.5:
            print "   %5s | %5s | %s | %s " % (
                score, planet_id, universe.getPlanet(planet_id).name, list(universe.getPlanet(planet_id).specials))
    print

    sorted_outposts = [(planet_id, score) for planet_id, score in sorted_outposts if score[0] > 0]
    # export outposts for other AI modules
    foAI.foAIstate.colonisableOutpostIDs.clear()
    foAI.foAIstate.colonisableOutpostIDs.update(sorted_outposts)
    colonization_timer.end()


def get_colony_targeted_planet_ids(planet_ids, mission_type):
    """return list being settled with colony planets"""
    colony_fleet_missions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([mission_type])
    colony_targeted_planets = []
    for planet_id in planet_ids:
        # add planets that are target of a mission
        for colony_fleet_mission in colony_fleet_missions:
            ai_target = AITarget.AITarget(TargetType.TARGET_PLANET, planet_id)
            if colony_fleet_mission.has_target(mission_type, ai_target):
                colony_targeted_planets.append(planet_id)
    return colony_targeted_planets


def get_outpost_targeted_planet_ids(planet_ids, mission_type):
    """return list being settled with outposts planets"""
    outpost_fleet_missions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([mission_type])
    outpost_targeted_planets = []
    for planetID in planet_ids:
        # add planets that are target of a mission
        for outpost_fleet_mission in outpost_fleet_missions:
            ai_target = AITarget.AITarget(TargetType.TARGET_PLANET, planetID)
            if outpost_fleet_mission.has_target(mission_type, ai_target):
                outpost_targeted_planets.append(planetID)
    return outpost_targeted_planets


def assign_colonisation_values(planet_ids, mission_type, species, empire, detail=None,
                               return_all=False):  # TODO: clean up suppliable versus annexable
    """creates a dictionary that takes planetIDs as key and their colonisation score as value"""
    if detail is None:
        detail = []
    orig_detail = detail
    planet_values = {}
    if mission_type == AIFleetMissionType.FLEET_MISSION_OUTPOST:
        # print "\n=========\nAssigning Outpost Values\n========="
        try_species = [""]
    elif species is not None:
        # print "\n=========\nAssigning Colony Values\n========="
        if isinstance(species, str):
            try_species = [species]
        elif isinstance(species, list):
            try_species = species
        else:
            try_species = [species.name]
    else:
        # print "\n=========\nAssigning Colony Values\n========="
        try_species = list(empire_colonizers)
    for planet_id in planet_ids:
        pv = []
        for spec_name in try_species:
            detail = orig_detail[:]
            pv.append((evaluate_planet(planet_id, mission_type, spec_name, empire, detail), spec_name, list(detail)))
        all_sorted = sorted(pv, reverse=True)
        best = all_sorted[:1]
        if best:
            if return_all:
                planet_values[planet_id] = all_sorted
            else:
                planet_values[planet_id] = best[0][:2]
                # print best[0][2]
    return planet_values


def next_turn_pop_change(cur_pop, target_pop):
    """Population change calc taken from PopCenter.cpp."""
    if target_pop > cur_pop:
        pop_change = cur_pop * (target_pop + 1 - cur_pop) / 100
        return min(pop_change, target_pop - cur_pop)
    else:
        pop_change = -(cur_pop - target_pop) / 10
        return max(pop_change, target_pop - cur_pop)


def project_ind_val(init_pop, max_pop_size, init_industry, max_ind_factor, flat_industry, discount_multiplier):
    """returns a discouted value for a projected industry stream over time with changing population"""
    discount_factor = 0.95
    if discount_multiplier > 1.0:
        discount_factor = 1.0 - 1.0 / discount_multiplier
    cur_pop = float(init_pop)
    cur_ind = float(init_industry)
    ind_val = 0
    val_factor = 1.0
    for turn in range(50):
        cur_pop += next_turn_pop_change(cur_pop, max_pop_size)
        cur_ind = min(cur_ind + 1, max(0, cur_ind - 1, flat_industry + cur_pop * max_ind_factor))
        val_factor *= discount_factor
        ind_val += val_factor * cur_ind
    return ind_val


def evaluate_planet(planet_id, mission_type, spec_name, empire, detail=None):
    """returns the colonisation value of a planet"""
    if detail is None:
        detail = []
    retval = 0
    discount_multiplier = [30.0, 40.0][fo.empireID() % 2]
    species = fo.getSpecies(spec_name or "")  # in case None is passed as specName
    pilot_val = 0
    if species and species.canProduceShips:
        pilot_val = rate_piloting_tag(species.tags)
        if pilot_val > cur_best_pilot_rating:
            pilot_val *= 2
        if pilot_val > 2:
            retval += discount_multiplier * 5 * pilot_val
            detail.append("Pilot Val %.1f" % (discount_multiplier * 5 * pilot_val))

    if detail:
        detail = []
    priority_scaling = 1.0
    max_gggs = 1  # if goes above 1 need some other adjustments below
    if empire.productionPoints < 100:
        backup_factor = 0.0
    else:
        backup_factor = min(1.0, (empire.productionPoints / 200.0) ** 2)

    universe = fo.getUniverse()
    capital_id = PlanetUtilsAI.get_capital()
    homeworld = universe.getPlanet(capital_id)
    planet = universe.getPlanet(planet_id)

    if (spec_name != planet.speciesName and planet.speciesName and
                mission_type != AIFleetMissionType.FLEET_MISSION_INVASION):
        return 0

    planet_size = planet.size
    this_sysid = planet.systemID
    if homeworld:
        home_system_id = homeworld.systemID
        eval_system_id = this_sysid
        if home_system_id != -1 and eval_system_id != -1:
            least_jumps = universe.jumpDistance(home_system_id, eval_system_id)
            if least_jumps == -1:  # indicates no known path
                return 0.0
                # distanceFactor = 1.001 / (least_jumps + 1)

    if not claimed_stars:
        for s_type in AIstate.empireStars:
            claimed_stars[s_type] = list(AIstate.empireStars[s_type])
        for sys_id in set(AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs):
            t_sys = universe.getSystem(sys_id)
            if not t_sys:
                continue
            claimed_stars.setdefault(t_sys.starType, []).append(sys_id)

    empire_research_list = [element.tech for element in empire.researchQueue]
    if planet is None:
        vis_map = universe.getVisibilityTurnsMap(planet_id, empire.empireID)
        print "Planet %d object not available; visMap: %s" % (planet_id, vis_map)
        return 0
    detail.append("%s : " % planet.name)
    # only count existing presence if not target planet
    have_existing_presence = AIstate.colonizedSystems.get(this_sysid, []) not in [[], [planet_id]]
    system = universe.getSystem(this_sysid)
    sys_status = foAI.foAIstate.systemStatus.get(this_sysid, {})

    sys_supply = system_supply.get(this_sysid, -99)
    planet_supply = AIDependencies.supply_by_size.get(int(planet_size), 0)
    planet_build_names = [universe.getObject(bldg).buildingTypeName for bldg in planet.buildingIDs]
    for bld_type in set(planet_build_names).intersection(AIDependencies.building_supply):
        planet_supply += sum(
            [AIDependencies.building_supply.get(bld_type, {}).get(int(psize), 0) for psize in [-1, planet.size]])
    planet_supply += foAI.foAIstate.misc.get('supply_tech', 0)

    myrating = sys_status.get('myFleetRating', 0)
    fleet_threat_ratio = (sys_status.get('fleetThreat', 0) - myrating) / float(cur_best_mil_ship_rating)
    monster_threat_ratio = sys_status.get('monsterThreat', 0) / float(cur_best_mil_ship_rating)
    neighbor_threat_ratio = ((sys_status.get('neighborThreat', 0)) / float(cur_best_mil_ship_rating)) + \
                            min(0, fleet_threat_ratio)  # last portion gives credit for inner extra defenses
    myrating = sys_status.get('my_neighbor_rating', 0)
    jump2_threat_ratio = ((sys_status.get('jump2_threat', 0) - myrating) / float(cur_best_mil_ship_rating)) + \
                         min(0, neighbor_threat_ratio)  # last portion gives credit for inner extra defenses

    thrt_factor = 1.0
    ship_limit = 2 * (2 ** (fo.currentTurn() / 40.0))
    threat_tally = fleet_threat_ratio + neighbor_threat_ratio + monster_threat_ratio
    if have_existing_presence:
        threat_tally += 0.3 * jump2_threat_ratio
        threat_tally *= 0.8
    else:
        threat_tally += 0.6 * jump2_threat_ratio
    if threat_tally > ship_limit:
        thrt_factor = 0.1
    elif fleet_threat_ratio + neighbor_threat_ratio + monster_threat_ratio > 0.6 * ship_limit:
        thrt_factor = 0.4
    elif fleet_threat_ratio + neighbor_threat_ratio + monster_threat_ratio > 0.2 * ship_limit:
        thrt_factor = 0.8

    sys_partial_vis_turn = universe.getVisibilityTurnsMap(this_sysid, empire.empireID).get(fo.visibility.partial, -9999)
    planet_partial_vis_turn = universe.getVisibilityTurnsMap(planet_id, empire.empireID).get(fo.visibility.partial,
                                                                                             -9999)

    if planet_partial_vis_turn < sys_partial_vis_turn:
        # last time we had partial vis of the system, the planet was stealthed to us
        print "Colonization AI couldn't get current info on planet id %d (was stealthed at last sighting)" % planet_id
        return 0  # TODO: track detection strength, order new scouting when it goes up

    tag_list = []
    star_bonus = 0
    colony_star_bonus = 0
    research_bonus = 0
    growth_val = 0
    fixed_ind = 0
    fixed_res = 0
    if species:
        tag_list = list(species.tags)
    star_pop_mod = 0
    if system:
        already_got_this_one = this_sysid in (AIstate.popCtrSystemIDs + AIstate.outpostSystemIDs)
        if "PHOTOTROPHIC" in tag_list:
            star_pop_mod = PHOTO_MAP.get(system.starType, 0)
            detail.append("PHOTOTROPHIC popMod %.1f" % star_pop_mod)
        if tech_is_complete("PRO_SOL_ORB_GEN") or "PRO_SOL_ORB_GEN" in empire_research_list[:5]:
            if system.starType in [fo.starType.blue, fo.starType.white]:
                if not claimed_stars.get(fo.starType.blue, []) + claimed_stars.get(fo.starType.white, []):
                    star_bonus += 20 * discount_multiplier
                    detail.append("PRO_SOL_ORB_GEN BW %.1f" % (20 * discount_multiplier))
                elif not already_got_this_one:
                    # still has extra value as an alternate location for solar generators
                    star_bonus += 10 * discount_multiplier * backup_factor
                    detail.append(
                        "PRO_SOL_ORB_GEN BW Backup Location %.1f" % (10 * discount_multiplier * backup_factor))
                elif fo.currentTurn() > 100:  # lock up this whole system
                    pass
                    # starBonus += 5  # TODO: how much?
                    # detail.append("PRO_SOL_ORB_GEN BW LockingDownSystem %.1f"%5)
            if system.starType in [fo.starType.yellow, fo.starType.orange]:
                if not (claimed_stars.get(fo.starType.blue, []) + claimed_stars.get(fo.starType.white, []) +
                        claimed_stars.get(fo.starType.yellow, []) + claimed_stars.get(fo.starType.orange, [])):
                    star_bonus += 10 * discount_multiplier
                    detail.append("PRO_SOL_ORB_GEN YO %.1f" % (10 * discount_multiplier))
                else:
                    pass
                    # starBonus +=2  # still has extra value as an alternate location for solar generators
                    # detail.append("PRO_SOL_ORB_GEN YO Backup %.1f" % 2)
        if system.starType in [fo.starType.blackHole] and fo.currentTurn() > 100:
            if not already_got_this_one:
                # whether have tech yet or not, assign some base value
                star_bonus += 10 * discount_multiplier * backup_factor
                detail.append("Black Hole %.1f" % (10 * discount_multiplier * backup_factor))
            else:
                star_bonus += 5 * discount_multiplier * backup_factor
                detail.append("Black Hole Backup %.1f" % (5 * discount_multiplier * backup_factor))
        if tech_is_complete(AIDependencies.PRO_SINGULAR_GEN) or \
                            AIDependencies.PRO_SINGULAR_GEN in empire_research_list[:8]:
            if system.starType in [fo.starType.blackHole]:
                if not claimed_stars.get(fo.starType.blackHole, []):
                    star_bonus += 200 * discount_multiplier  # pretty rare planets, good for generator
                    detail.append("PRO_SINGULAR_GEN %.1f" % (200 * discount_multiplier))
                elif this_sysid not in claimed_stars.get(fo.starType.blackHole, []):
                    # still has extra value as an alternate location for generators & for blocking enemies generators
                    star_bonus += 100 * discount_multiplier * backup_factor
                    detail.append("PRO_SINGULAR_GEN Backup %.1f" % (100 * discount_multiplier * backup_factor))
            elif system.starType in [fo.starType.red] and not claimed_stars.get(fo.starType.blackHole, []):
                rfactor = (1.0 + len(claimed_stars.get(fo.starType.red, []))) ** -2
                star_bonus += 40 * discount_multiplier * backup_factor * rfactor  # can be used for artif'l black hole
                detail.append("Red Star for Art Black Hole %.1f" % (20 * discount_multiplier * backup_factor * rfactor))
        if tech_is_complete("PRO_NEUTRONIUM_EXTRACTION") or "PRO_NEUTRONIUM_EXTRACTION" in empire_research_list[:8]:
            if system.starType in [fo.starType.neutron]:
                if not claimed_stars.get(fo.starType.neutron, []):
                    star_bonus += 80 * discount_multiplier  # pretty rare planets, good for armor
                    detail.append("PRO_NEUTRONIUM_EXTRACTION %.1f" % (80 * discount_multiplier))
                else:
                    # still has extra value as an alternate location for generators & for bnlocking enemies generators
                    star_bonus += 20 * discount_multiplier * backup_factor
                    detail.append("PRO_NEUTRONIUM_EXTRACTION Backup %.1f" % (20 * discount_multiplier * backup_factor))
        if tech_is_complete("SHP_ENRG_BOUND_MAN") or "SHP_ENRG_BOUND_MAN" in empire_research_list[:6]:
            if system.starType in [fo.starType.blackHole, fo.starType.blue]:
                init_val = 100 * discount_multiplier * (pilot_val or 1)
                if not claimed_stars.get(fo.starType.blackHole, []) + claimed_stars.get(fo.starType.blue, []):
                    colony_star_bonus += init_val  # pretty rare planets, good for energy shipyards
                    detail.append("SHP_ENRG_BOUND_MAN %.1f" % init_val)
                elif this_sysid not in (claimed_stars.get(fo.starType.blackHole, []) +
                                        claimed_stars.get(fo.starType.blue, [])):
                    # still has extra value as an alternate location for energy shipyard
                    colony_star_bonus += 0.5 * init_val * backup_factor
                    detail.append("SHP_ENRG_BOUND_MAN Backup %.1f" % (0.5 * init_val * backup_factor))
    retval += star_bonus

    planet_specials = list(planet.specials)
    if "ECCENTRIC_ORBIT_SPECIAL" in planet.specials:
        fixed_res += discount_multiplier * 6
        detail.append("ECCENTRIC_ORBIT_SPECIAL %.1f" % (discount_multiplier * 6))

    if (mission_type == AIFleetMissionType.FLEET_MISSION_OUTPOST or
            (mission_type == AIFleetMissionType.FLEET_MISSION_INVASION and not spec_name)):

        if "ANCIENT_RUINS_SPECIAL" in planet.specials:  # TODO: add value for depleted ancient ruins
            retval += discount_multiplier * 30
            detail.append("Undepleted Ruins %.1f" % discount_multiplier * 30)

        for special in planet_specials:
            if "_NEST_" in special:
                nest_val = NEST_VAL_MAP.get(special,
                                            5) * discount_multiplier * backup_factor  # get an outpost on the nest quick
                retval += nest_val
                detail.append("%s %.1f" % (special, nest_val))
        if planet.size == fo.planetSize.asteroids:
            ast_val = 0
            if tech_is_complete("PRO_MICROGRAV_MAN"):
                per_ast = 5
            else:
                per_ast = 2.5
            if system:
                for pid in system.planetIDs:
                    other_planet = universe.getPlanet(pid)
                    if other_planet.size == fo.planetSize.asteroids:
                        if pid == planet_id:
                            continue
                        elif pid < planet_id and planet.unowned:
                            ast_val = 0
                            break
                    elif other_planet.speciesName:  # and otherPlanet.owner==empire.empireID
                        ast_val += per_ast * discount_multiplier
                retval += ast_val
                if ast_val > 0:
                    detail.append("AsteroidMining %.1f" % ast_val)
            ast_val = 0
            if tech_is_complete("SHP_ASTEROID_HULLS"):
                per_ast = 20
            elif tech_is_complete("CON_ORBITAL_CON"):
                per_ast = 10
            else:
                per_ast = 5
            if system:
                for pid in system.planetIDs:
                    other_planet = universe.getPlanet(pid)
                    if other_planet.size == fo.planetSize.asteroids:
                        if pid == planet_id:
                            continue
                        elif pid < planet_id and planet.unowned:
                            ast_val = 0
                            break
                    elif other_planet.speciesName:  # and otherPlanet.owner==empire.empireID
                        other_species = fo.getSpecies(other_planet.speciesName)
                        if other_species and other_species.canProduceShips:
                            ast_val += per_ast * discount_multiplier
                retval += ast_val
                if ast_val > 0:
                    detail.append("AsteroidShipBuilding %.1f" % ast_val)
        if planet.size == fo.planetSize.gasGiant and tech_is_complete("PRO_ORBITAL_GEN"):
            per_gg = 20
        elif planet.size == fo.planetSize.gasGiant and tech_is_complete("CON_ORBITAL_CON"):
            per_gg = 10
        else:
            per_gg = 5
        if system:
            gg_list = []
            orb_gen_val = 0
            gg_detail = []
            for pid in system.planetIDs:
                other_planet = universe.getPlanet(pid)
                if other_planet.size == fo.planetSize.gasGiant:
                    gg_list.append(pid)
                if pid != planet_id and other_planet.owner == empire.empireID and AIFocusType.FOCUS_INDUSTRY in list(
                        other_planet.availableFoci) + [other_planet.focus]:
                    orb_gen_val += per_gg * discount_multiplier
                    gg_detail.append("GGG for %s %.1f" % (other_planet.name, discount_multiplier * per_gg))
            if planet_id in sorted(gg_list)[:max_gggs]:
                retval += orb_gen_val
                detail.extend(gg_detail)
            else:
                detail.append("Won't GGG")
        if thrt_factor < 1.0:
            retval *= thrt_factor
            detail.append("threat reducing value by %3d %%" % (100 * (1 - thrt_factor)))
        if have_existing_presence:
            detail.append("preexisting system colony")
            retval *= 1.5
        if sys_supply < 0:
            if sys_supply + planet_supply >= 0:
                retval += 30 * (planet_supply - max(-3, sys_supply))
            else:
                retval += 30 * (planet_supply + sys_supply)  # a penalty
        elif planet_supply > sys_supply and (sys_supply < 2):  # TODO: check min neighbor supply
            retval += 25 * (planet_supply - sys_supply)
        return int(retval)
    else:  # colonization mission
        if not species:
            return 0
        supplyval = 0
        if "ANCIENT_RUINS_SPECIAL" in planet.specials:
            retval += discount_multiplier * 50
            detail.append("Undepleted Ruins %.1f" % discount_multiplier * 50)
        pop_tag_mod = 1.0
        ind_tag_mod = 1.0
        res_tag_mod = 1.0
        supply_tag_mod = 0.0
        ai_tags = ""
        for tag in [tag1 for tag1 in tag_list if "AI_TAG" in tag1]:
            tag_parts = tag.split('_')
            tag_type = tag_parts[3]
            ai_tags += " AI_TAG: %s " % tag
            grade = {'NO': 0.0, 'BAD': 0.5, 'GOOD': 1.5, 'GREAT': 2.0, 'ULTIMATE': 4.0}.get(tag_parts[2], 1.0)
            if tag_type == "POPULATION":
                pop_tag_mod = {'BAD': 0.75, 'GOOD': 1.25}.get(tag_parts[2], 1.0)
            elif tag_type == "INDUSTRY":
                ind_tag_mod = grade
            elif tag_type == "RESEARCH":
                res_tag_mod = grade
            elif tag_type == "SUPPLY":
                supply_tag_mod = {'BAD': 0, 'AVERAGE': 1, 'GOOD': 1, 'GREAT': 2, 'ULTIMATE': 3}.get(tag_parts[2], 0)

        planet_supply += supply_tag_mod
        if sys_supply <= 0:
            if sys_supply + planet_supply >= 0:
                supplyval = 100 * (planet_supply - max(-3, sys_supply))
            else:
                supplyval = 200 * (planet_supply + sys_supply)  # a penalty
        elif planet_supply > sys_supply == 1:  # TODO: check min neighbor supply
            supplyval = 50 * (planet_supply - sys_supply)

        # if AITags != "":
        # print "Species %s has AITags %s"%(specName, AITags)

        retval += fixed_res
        retval += colony_star_bonus
        asteroid_bonus = 0
        gas_giant_bonus = 0
        flat_industry = 0
        mining_bonus = 0
        per_ggg = 10
        planet_size = planet.size

        got_asteroids = False
        got_owned_asteroids = False
        local_gg = False
        got_owned_gg = False
        ast_shipyard_name = ""
        if system and AIFocusType.FOCUS_INDUSTRY in species.foci:
            for pid in [temp_id for temp_id in system.planetIDs if temp_id != planet_id]:
                p2 = universe.getPlanet(pid)
                if p2:
                    if p2.size == fo.planetSize.asteroids:
                        got_asteroids = True
                        ast_shipyard_name = p2.name
                        if p2.owner != -1:
                            got_owned_asteroids = True
                    if p2.size == fo.planetSize.gasGiant:
                        local_gg = True
                        if p2.owner != -1:
                            got_owned_gg = True
        if got_asteroids:
            if tech_is_complete("PRO_MICROGRAV_MAN") or "PRO_MICROGRAV_MAN" in empire_research_list[:10]:
                if got_owned_asteroids:  # can be quickly captured
                    flat_industry += 5  # will go into detailed industry projection
                    detail.append("Asteroid mining ~ %.1f" % (5 * discount_multiplier))
                else:  # uncertain when can be outposted
                    asteroid_bonus = 2.5 * discount_multiplier  # give partial value
                    detail.append("Asteroid mining %.1f" % (5 * discount_multiplier))
            if tech_is_complete("SHP_ASTEROID_HULLS") or "SHP_ASTEROID_HULLS" in empire_research_list[:11]:
                if species and species.canProduceShips:
                    asteroid_bonus += 30 * discount_multiplier * pilot_val
                    detail.append(
                        "Asteroid ShipBuilding from %s %.1f" % (
                            ast_shipyard_name, discount_multiplier * 20 * pilot_val))
        if local_gg:
            if tech_is_complete("PRO_ORBITAL_GEN") or "PRO_ORBITAL_GEN" in empire_research_list[:5]:
                if got_owned_gg:
                    flat_industry += per_ggg  # will go into detailed industry projection
                    detail.append("GGG ~ %.1f" % (per_ggg * discount_multiplier))
                else:
                    gas_giant_bonus = 0.5 * per_ggg * discount_multiplier
                    detail.append("GGG %.1f" % (0.5 * per_ggg * discount_multiplier))
        if planet.size == fo.planetSize.gasGiant:
            if not (species and species.name == "SP_SUPER_TEST"):
                detail.append("Can't Settle GG")
                return 0
            else:
                planet_env = fo.planetEnvironment.adequate  # I think
                planet_size = 6  # I think
        elif planet.size == fo.planetSize.asteroids:
            planet_size = 3  # I think
            if not species or species.name not in ["SP_EXOBOT", "SP_SUPER_TEST"]:
                detail.append("Can't settle Asteroids")
                return 0
            elif species.name == "SP_EXOBOT":
                planet_env = fo.planetEnvironment.poor
            elif species.name == "SP_SUPER_TEST":
                planet_env = fo.planetEnvironment.adequate  # I think
            else:
                return 0
        else:
            planet_env = ENVIRONS[str(species.getPlanetEnvironment(planet.type))]
        if not planet_env:
            return -9999
        pop_size_mod = 0
        conditional_pop_size_mod = 0
        post_pop_size_mod = 0
        pop_size_mod += POP_SIZE_MOD_MAP["env"][planet_env]
        detail.append("EnvironPopSizeMod(%d)" % pop_size_mod)
        if "SELF_SUSTAINING" in tag_list:
            pop_size_mod *= 2
            detail.append("SelfSustaining_PSM(2)")
        if "PHOTOTROPHIC" in tag_list:
            pop_size_mod += star_pop_mod
            detail.append("Phototropic Star Bonus_PSM(%0.1f)" % star_pop_mod)
        if tech_is_complete("GRO_SUBTER_HAB"):
            conditional_pop_size_mod += POP_SIZE_MOD_MAP["subHab"][planet_env]
            detail.append("Sub_Hab_PSM(%d)" % POP_SIZE_MOD_MAP["subHab"][planet_env])
        for gTech, gKey in [("GRO_SYMBIOTIC_BIO", "symBio"),
                            ("GRO_XENO_GENETICS", "xenoGen"),
                            ("GRO_XENO_HYBRID", "xenoHyb"),
                            ("GRO_CYBORG", "cyborg")]:
            if tech_is_complete(gTech):
                pop_size_mod += POP_SIZE_MOD_MAP[gKey][planet_env]
                detail.append("%s_PSM(%d)" % (gKey, POP_SIZE_MOD_MAP[gKey][planet_env]))
        for gTech, gKey in [("CON_NDIM_STRUC", "ndim"), ("CON_ORBITAL_HAB", "orbit")]:
            if tech_is_complete(gTech):
                conditional_pop_size_mod += POP_SIZE_MOD_MAP[gKey][planet_env]
                detail.append("%s_PSM(%d)" % (gKey, POP_SIZE_MOD_MAP[gKey][planet_env]))

        # exobots can't ever get to good environ so no gaiai bonus, for others we'll assume they'll get there
        if "GAIA_SPECIAL" in planet.specials and species.name != "SP_EXOBOT":
            conditional_pop_size_mod += 3
            detail.append("Gaia_PSM(3)")

        for special in ["SLOW_ROTATION_SPECIAL", "SOLID_CORE_SPECIAL"]:
            if special in planet_specials:
                post_pop_size_mod -= 1
                detail.append("%s_PSM(-1)" % special)

        applicable_boosts = set()
        for thisTag in [tag for tag in tag_list if tag in AIDependencies.metabolismBoostMap]:
            metab_boosts = AIDependencies.metabolismBoostMap.get(thisTag, [])
            if pop_size_mod > 0:
                for key in active_growth_specials.keys():
                    if len(active_growth_specials[key]) > 0 and key in metab_boosts:
                        applicable_boosts.add(key)
                        detail.append("%s boost active" % key)
            for boost in metab_boosts:
                if boost in planet_specials:
                    applicable_boosts.add(boost)
                    detail.append("%s boost present" % boost)

        n_boosts = len(applicable_boosts)
        if n_boosts:
            conditional_pop_size_mod += n_boosts
            detail.append("boosts_PSM(%d from %s)" % (n_boosts, applicable_boosts))
        if pop_size_mod >= 0:
            pop_size_mod += conditional_pop_size_mod
        pop_size_mod += post_pop_size_mod

        if planet_id in species.homeworlds:  # TODO: check for homeworld growth focus
            pop_size_mod += 2

        max_pop_size = planet_size * pop_size_mod * pop_tag_mod
        detail.append(
            "baseMaxPop size*psm %d * %d * %.2f = %d" % (planet_size, pop_size_mod, pop_tag_mod, max_pop_size))

        if "DIM_RIFT_MASTER_SPECIAL" in planet.specials:
            max_pop_size -= 4
            detail.append("DIM_RIFT_MASTER_SPECIAL(maxPop-4)")
        if "ECCENTRIC_ORBIT_SPECIAL" in planet.specials:
            max_pop_size -= 3
            detail.append("ECCENTRIC_ORBIT_SPECIAL(maxPop-3)")

        detail.append("maxPop %.1f" % max_pop_size)

        for special in ["MINERALS_SPECIAL", "CRYSTALS_SPECIAL", "METALOIDS_SPECIAL"]:
            if special in planet_specials:
                mining_bonus += 1

        pro_sing_val = [0, 4][(len(claimed_stars.get(fo.starType.blackHole, [])) > 0)]
        base_pop_ind = 0.2
        ind_mult = 1 * max(ind_tag_mod,
                           0.5 * (ind_tag_mod + res_tag_mod))  # TODO: repport an actual calc for research value
        ind_tech_map = {"GRO_ENERGY_META": 0.5,
                        "PRO_ROBOTIC_PROD": 0.4,
                        "PRO_FUSION_GEN": 1.0,
                        "PRO_INDUSTRY_CENTER_I": 1,
                        "PRO_INDUSTRY_CENTER_II": 1,
                        "PRO_INDUSTRY_CENTER_III": 1,
                        "PRO_SOL_ORB_GEN": 2.0,  # TODO don't assume will build a gen at a blue/white star
                        AIDependencies.PRO_SINGULAR_GEN: pro_sing_val,
                        }

        for tech in ind_tech_map:
            if tech_is_complete(tech):
                ind_mult += ind_tech_map[tech]
        max_ind_factor = 0
        if tech_is_complete("PRO_SENTIENT_AUTOMATION"):
            fixed_ind += discount_multiplier * 5
        if AIFocusType.FOCUS_INDUSTRY in species.foci:
            max_ind_factor += base_pop_ind * mining_bonus
            max_ind_factor += base_pop_ind * ind_mult
        cur_pop = 1.0  # assume an initial colonization vale
        if planet.speciesName != "":
            cur_pop = planet.currentMeterValue(fo.meterType.population)
        elif tech_is_complete("GRO_LIFECYCLE_MAN"):
            cur_pop = 3.0
        cur_industry = planet.currentMeterValue(fo.meterType.industry)
        ind_val = project_ind_val(cur_pop, max_pop_size, cur_industry, max_ind_factor, flat_industry,
                                  discount_multiplier)
        detail.append("ind_val %.1f" % ind_val)
        # used to give preference to closest worlds

        for special in [spec for spec in planet_specials if spec in AIDependencies.metabolismBoosts]:
            gbonus = discount_multiplier * base_pop_ind * ind_mult * empire_metabolisms.get(
                AIDependencies.metabolismBoosts[special], 0)  # due to growth applicability to other planets
            growth_val += gbonus
            detail.append("Bonus for %s: %.1f" % (special, gbonus))

        base_pop_res = 0.2  # will also be doubling value of research, below
        if AIFocusType.FOCUS_RESEARCH in species.foci:
            research_bonus += discount_multiplier * 2 * base_pop_res * max_pop_size
            if "ANCIENT_RUINS_SPECIAL" in planet.specials or "ANCIENT_RUINS_DEPLETED_SPECIAL" in planet.specials:
                research_bonus += discount_multiplier * 2 * base_pop_res * max_pop_size * 5
                detail.append("Ruins Research")
            if "COMPUTRONIUM_SPECIAL" in planet.specials:
                research_bonus += discount_multiplier * 2 * 10  # TODO: do actual calc
                detail.append("COMPUTRONIUM_SPECIAL")

        if max_pop_size <= 0:
            detail.append("Non-positive population projection for species '%s', so no colonization value" % (
                species and species.name))
            return 0

        retval += max(ind_val + asteroid_bonus + gas_giant_bonus, research_bonus,
                      growth_val) + fixed_ind + fixed_res + supplyval
        retval *= priority_scaling
        if thrt_factor < 1.0:
            retval *= thrt_factor
            detail.append("threat reducing value by %3d %%" % (100 * (1 - thrt_factor)))
        if have_existing_presence:
            detail.append("preexisting system colony")
            retval *= 1.5
    return retval


def assign_colony_fleets_to_colonise():
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    all_outpost_base_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(
        AIFleetMissionType.FLEET_MISSION_ORBITAL_OUTPOST)
    avail_outpost_base_fleet_ids = FleetUtilsAI.extract_fleet_ids_without_mission_types(all_outpost_base_fleet_ids)
    for fid in avail_outpost_base_fleet_ids:
        fleet = universe.getFleet(fid)
        if not fleet:
            continue
        sys_id = fleet.systemID
        system = universe.getSystem(sys_id)
        avail_planets = set(system.planetIDs).intersection(set(foAI.foAIstate.qualifyingOutpostBaseTargets.keys()))
        targets = [pid for pid in avail_planets if foAI.foAIstate.qualifyingOutpostBaseTargets[pid][1] != -1]
        if not targets:
            print "Error found no valid target for outpost base in system %s (%d)" % (system.name, sys_id)
            continue
        target_id = -1
        best_score = -1
        for pid, rating in assign_colonisation_values(targets, AIFleetMissionType.FLEET_MISSION_OUTPOST, None,
                                                      empire).items():
            if rating[0] > best_score:
                best_score = rating[0]
                target_id = pid
        if target_id != -1:
            foAI.foAIstate.qualifyingOutpostBaseTargets[target_id][1] = -1  # TODO: should probably delete
            ai_target = AITarget.AITarget(TargetType.TARGET_PLANET, target_id)
            ai_fleet_mission = foAI.foAIstate.get_fleet_mission(fid)
            ai_fleet_mission.add_target(AIFleetMissionType.FLEET_MISSION_ORBITAL_OUTPOST, ai_target)

    # assign fleet targets to colonisable planets
    send_colony_ships(AIstate.colonyFleetIDs, foAI.foAIstate.colonisablePlanetIDs.items(),
                      AIFleetMissionType.FLEET_MISSION_COLONISATION)

    # assign fleet targets to colonisable outposts
    send_colony_ships(AIstate.outpostFleetIDs, foAI.foAIstate.colonisableOutpostIDs.items(),
                      AIFleetMissionType.FLEET_MISSION_OUTPOST)


def send_colony_ships(colony_fleet_ids, evaluated_planets, mission_type):
    """sends a list of colony ships to a list of planet_value_pairs"""
    fleet_pool = colony_fleet_ids[:]
    try_all = False
    if mission_type == AIFleetMissionType.FLEET_MISSION_OUTPOST:
        cost = 20 + AIDependencies.OUTPOST_POD_COST * (1 + len(AIstate.popCtrIDs) * AIDependencies.COLONY_POD_UPKEEP)
    else:
        try_all = True
        cost = 20 + AIDependencies.COLONY_POD_COST * (1 + len(AIstate.popCtrIDs) * AIDependencies.COLONY_POD_UPKEEP)
        if fo.currentTurn() < 50:
            cost *= 0.4  # will be making fast tech progress so value is underestimated
        elif fo.currentTurn() < 80:
            cost *= 0.8  # will be making fast-ish tech progress so value is underestimated

    potential_targets = [(pid, (score, specName)) for (pid, (score, specName)) in evaluated_planets if
                         score > (0.8 * cost)]

    print "colony/outpost ship matching -- fleets %s to planets %s" % (fleet_pool, evaluated_planets)

    if try_all:
        print "trying best matches to current colony ships"
        best_scores = dict(evaluated_planets)
        potential_targets = []
        for pid, ratings in all_colony_opportunities.items():
            for rating in ratings:
                if rating[0] >= 0.75 * best_scores.get(pid, [9999])[0]:
                    potential_targets.append((pid, rating))
        potential_targets.sort(lambda x, y: cmp(x[1], y[1]), reverse=True)

    # added a lot of checking because have been getting mysterious exception, after too many recursions to get info
    fleet_pool = set(fleet_pool)
    universe = fo.getUniverse()
    for fid in fleet_pool:
        fleet = universe.getFleet(fid)
        if not fleet or fleet.empty:
            print "Error: bad fleet ( ID %d ) given to colonization routine; will be skipped" % fid
            fleet_pool.remove(fid)
            continue
        report_str = "Fleet ID (%d): %d ships; species: " % (fid, fleet.numShips)
        for sid in fleet.shipIDs:
            ship = universe.getShip(sid)
            if not ship:
                report_str += "NoShip, "
            else:
                report_str += "%s, " % ship.speciesName
        print report_str
    print
    already_targeted = []
    # for planetID_value_pair in evaluatedPlanets:
    while fleet_pool and potential_targets:
        target = potential_targets.pop(0)
        if target in already_targeted:
            continue
        planet_id = target[0]
        if planet_id in already_targeted:
            continue
        planet = universe.getPlanet(planet_id)
        sys_id = planet.systemID
        if foAI.foAIstate.systemStatus.setdefault(sys_id, {}).setdefault('monsterThreat', 0) > 2000 \
                or fo.currentTurn() < 20 and foAI.foAIstate.systemStatus[sys_id]['monsterThreat'] > 200:
            print "Skipping colonization of system %s due to Big Monster, threat %d" % (
                PlanetUtilsAI.sys_name_ids([sys_id]), foAI.foAIstate.systemStatus[sys_id]['monsterThreat'])
            already_targeted.append(planet_id)
            continue
        this_spec = target[1][1]
        found_fleets = []
        try:
            this_fleet_list = FleetUtilsAI.get_fleets_for_mission(nships=1, target_stats={}, min_stats={}, cur_stats={},
                                                                  species=this_spec, systems_to_check=[sys_id],
                                                                  systems_checked=[],
                                                                  fleet_pool_set=fleet_pool, fleet_list=found_fleets,
                                                                  tried_fleets=set(), verbose=False)
        except:
            continue
        if not this_fleet_list:
            fleet_pool.update(found_fleets)  # just to be safe
            continue  # must have no compatible colony/outpost ships
        fleet_id = this_fleet_list[0]
        already_targeted.append(planet_id)
        ai_target = AITarget.AITarget(TargetType.TARGET_PLANET, planet_id)
        foAI.foAIstate.get_fleet_mission(fleet_id).add_target(mission_type, ai_target)
