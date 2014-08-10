import freeOrionAIInterface as fo # pylint: disable=import-error
import FreeOrionAI as foAI
import AIstate
import AIDependencies
import EnumsAI
import FleetUtilsAI
import PlanetUtilsAI
import AITarget
import math
import ProductionAI
import ColonisationAI
import MilitaryAI
from tools import dict_from_map


from timing import Timer

invasion_timer = Timer('get_invasion_fleets()', write_log=False)

def get_invasion_fleets():
    """get invasion fleets"""
    invasion_timer.start("gathering initial info")

    all_invasion_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION)
    AIstate.invasionFleetIDs = FleetUtilsAI.extract_fleet_ids_without_mission_types(all_invasion_fleet_ids)

    # get supplyable planets
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empire_id = empire.empireID
    capital_id = PlanetUtilsAI.get_capital()
    homeworld=None
    if capital_id:
        homeworld = universe.getPlanet(capital_id)
    if homeworld:
        home_system_id = homeworld.systemID
    else:
        home_system_id = -1

    fleet_supplyable_system_ids = empire.fleetSupplyableSystemIDs
    fleet_supplyable_planet_ids = PlanetUtilsAI.get_planets_in__systems_ids(fleet_supplyable_system_ids)

    prime_invadable_system_ids = set(ColonisationAI.annexableSystemIDs)
    prime_invadable_planet_ids = PlanetUtilsAI.get_planets_in__systems_ids(prime_invadable_system_ids)

    visible_system_ids = foAI.foAIstate.visInteriorSystemIDs.keys() + foAI.foAIstate. visBorderSystemIDs.keys()

    if home_system_id != -1:
        accessible_system_ids = [sys_id for sys_id in visible_system_ids if  (sys_id != -1 ) and universe.systemsConnected(sys_id, home_system_id, empire_id) ]
    else:
        print "Invasion Warning: this empire has no identifiable homeworld,  will therefor treat all visible planets as accessible."
        accessible_system_ids = visible_system_ids #TODO: check if any troop ships still owned, use their system as home system
    acessible_planet_ids = PlanetUtilsAI.get_planets_in__systems_ids(accessible_system_ids)
    print "Accessible Systems: ",  ",  ".join(PlanetUtilsAI.sys_name_ids(accessible_system_ids))
    print

    #all_owned_planet_ids = PlanetUtilsAI.get_all_owned_planet_ids(exploredPlanetIDs)
    all_owned_planet_ids = PlanetUtilsAI.get_all_owned_planet_ids(acessible_planet_ids)#need these for unpopulated outposts
    # print "All Owned and Populated PlanetIDs: " + str(all_owned_planet_ids)

    all_populated_planets = PlanetUtilsAI.get_populated_planet_ids(acessible_planet_ids)#need this for natives
    print "All Visible and accessible Populated PlanetIDs (including this empire's): ",  ",  ".join(PlanetUtilsAI.planet_name_ids(all_populated_planets ))
    print
    print "Prime Invadable Target Systems: ",  ",  ".join(PlanetUtilsAI.sys_name_ids(prime_invadable_system_ids))
    print

    empire_owned_planet_ids = PlanetUtilsAI.get_owned_planets_by_empire(universe.planetIDs, empire_id)
    # print "Empire Owned PlanetIDs:            " + str(empire_owned_planet_ids)

    invadable_planet_ids = set(prime_invadable_planet_ids).intersection(set(all_owned_planet_ids).union(all_populated_planets ) - set(empire_owned_planet_ids))
    print "Prime Invadable PlanetIDs:              ",  ", ".join(PlanetUtilsAI.planet_name_ids(invadable_planet_ids))

    print
    print "Current Invasion Targeted SystemIDs:       ",  ", ".join(PlanetUtilsAI.sys_name_ids(AIstate.invasionTargetedSystemIDs))
    invasion_targeted_planet_ids = get_invasion_targeted_planet_ids(universe.planetIDs, EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION, empire_id)
    invasion_targeted_planet_ids.extend( get_invasion_targeted_planet_ids(universe.planetIDs, EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION, empire_id))
    all_invasion_targeted_system_ids = set(PlanetUtilsAI.get_systems(invasion_targeted_planet_ids))

    print "Current Invasion Targeted PlanetIDs:       ",  ", ".join(PlanetUtilsAI.planet_name_ids(invasion_targeted_planet_ids))

    invasion_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION)
    if not invasion_fleet_ids:
        print "Available Invasion Fleets:           0"
    else:
        print "Invasion FleetIDs:                 " + str(FleetUtilsAI.get_empire_fleet_ids_by_role(EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION))

    num_invasion_fleets = len(FleetUtilsAI.extract_fleet_ids_without_mission_types(invasion_fleet_ids))
    print "Invasion Fleets Without Missions:    " + str(num_invasion_fleets)
    
    invasion_timer.start("planning troop base production")
    # only do base invasions if aggression is typical or above
    reserved_troop_base_targets = []
    if foAI.foAIstate.aggression > fo.aggression.typical:
        available_pp = {}
        for el in  empire.planetsWithAvailablePP:  #keys are sets of ints; data is doubles
            avail_pp = el.data()
            for pid in el.key():
                available_pp[pid] = avail_pp
        if len (invadable_planet_ids) > 0:
            #print "Evaluating Troop Bases (SpaceInvaders) for %s"%(invadable_planet_ids)
            pass
        for pid in invadable_planet_ids: #TODO: reorganize
            planet = universe.getPlanet(pid)
            if not planet: 
                continue
            sys_id = planet.systemID
            sys_partial_vis_turn = dict_from_map(universe.getVisibilityTurnsMap(planet.systemID, empire_id)).get(fo.visibility.partial, -9999)
            planet_partial_vis_turn = dict_from_map(universe.getVisibilityTurnsMap(pid, empire_id)).get(fo.visibility.partial, -9999)
            if planet_partial_vis_turn < sys_partial_vis_turn:
                #print "rejecting %s due to stealth"%planet.name
                continue
            for pid2 in ColonisationAI.empireSpeciesSystems.get(sys_id,  {}).get('pids', []):
                if available_pp.get(pid2,  0) < 2: #TODO: improve troop base PP sufficiency determination
                    #print "rejecting %s due to insufficient avail PP"%planet.name
                    break
                planet2 = universe.getPlanet(pid2)
                if not planet2: 
                    continue
                if (pid not in  foAI.foAIstate.qualifyingTroopBaseTargets) and (planet2.speciesName  in ColonisationAI.empireShipBuilders):
                    #print "Adding %s to Troop Bases (SpaceInvaders) potential target list, from %s"%(planet.name, planet2.name) 
                    foAI.foAIstate.qualifyingTroopBaseTargets.setdefault(pid,  [pid2,  -1])
                    break

        for pid in list(foAI.foAIstate.qualifyingTroopBaseTargets):
            planet = universe.getPlanet(pid) #TODO: also check that still have a colony in this system that can make troops
            if planet and planet.owner == empire_id:
                del foAI.foAIstate.qualifyingTroopBaseTargets[pid]

        secureAIFleetMissions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([EnumsAI.AIFleetMissionType.FLEET_MISSION_SECURE])
        #print "considering possible troop bases at %s" % (foAI.foAIstate.qualifyingTroopBaseTargets.keys())
        for pid in (set(foAI.foAIstate.qualifyingTroopBaseTargets.keys()) - set(invasion_targeted_planet_ids)): #TODO: consider overriding standard invasion mission
            planet = universe.getPlanet(pid)
            if  foAI.foAIstate.qualifyingTroopBaseTargets[pid][1] != -1: 
                reserved_troop_base_targets.append(pid)
                if planet:
                    all_invasion_targeted_system_ids.add( planet.systemID )
                continue  #already building for here
            sys_id = planet.systemID
            this_sys_status = foAI.foAIstate.systemStatus.get( sys_id,  {} )
            if  ((planet.currentMeterValue(fo.meterType.shield) > 0) and 
                        (this_sys_status.get('myFleetRating', 0) < (0.8 * this_sys_status.get('totalThreat', 0)))):
                continue
            loc = foAI.foAIstate.qualifyingTroopBaseTargets[pid][0]
            this_score,  p_troops = evaluate_invasion_planet(pid, EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION, fleet_supplyable_planet_ids, empire,  secureAIFleetMissions,  False)
            bestShip,  colDesign,  buildChoices = ProductionAI.getBestShipInfo(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_ORBITAL_INVASION,  loc)
            if not bestShip:
                #print "Error: no troop base can be built at ",  PlanetUtilsAI.planet_name_ids([loc])
                continue
            #print "selecting  ",  PlanetUtilsAI.planet_name_ids([loc]),  " to build Orbital troop bases"
            n_bases = math.ceil((p_troops+1) / 2)#TODO: reconsider this +1 safety factor
            retval  = fo.issueEnqueueShipProductionOrder(bestShip, loc)
            print "Enqueueing %d Troop Bases at %s for %s"%( n_bases,  PlanetUtilsAI.planet_name_ids([loc]),  PlanetUtilsAI.planet_name_ids([pid]))
            if retval !=0:
                all_invasion_targeted_system_ids.add( planet.systemID )
                reserved_troop_base_targets.append(pid)
                foAI.foAIstate.qualifyingTroopBaseTargets[pid][1] = loc
                fo.issueChangeProductionQuantityOrder(empire.productionQueue.size -1,  1,  int(n_bases))
                res=fo.issueRequeueProductionOrder(empire.productionQueue.size -1,  0)

    invasion_timer.start("evaluating target planets")
    #TODO: check if any invasion_targeted_planet_ids need more troops assigned
    evaluatedPlanetIDs = list(set(invadable_planet_ids) - set(invasion_targeted_planet_ids) - set(reserved_troop_base_targets)  )
    print "Evaluating potential invasions, PlanetIDs:               " + str(evaluatedPlanetIDs)

    evaluatedPlanets = assign_invasion_values(evaluatedPlanetIDs, EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION, fleet_supplyable_planet_ids, empire)

    sortedPlanets = [(pid,  pscore,  ptroops) for (pid,  (pscore, ptroops)) in evaluatedPlanets.items() ]
    sortedPlanets.sort(lambda x, y: cmp(x[1], y[1]), reverse=True)
    sortedPlanets = [(pid,  pscore%10000,  ptroops) for (pid,  pscore, ptroops) in sortedPlanets ]

    print
    if sortedPlanets:
        print "Invadable planets\nIDs,    ID | Score | Name           | Race             | Troops"
        for pid,  pscore,  ptroops in sortedPlanets:
            planet = universe.getPlanet(pid)
            if planet:
                print "%6d | %6d | %16s | %16s | %d"%(pid,  pscore,  planet.name,  planet.speciesName,  ptroops)
            else:
                print "%6d | %6d | Error: invalid planet ID"%(pid,  pscore)
    else:
        print "No Invadable planets identified"

    sortedPlanets = [(pid,  pscore,  ptroops) for (pid,  pscore, ptroops) in sortedPlanets  if pscore > 0]
    # export opponent planets for other AI modules
    AIstate.opponentPlanetIDs = [pid for pid, pscore, trp in sortedPlanets]
    AIstate.invasionTargets = sortedPlanets

    # export invasion targeted systems for other AI modules
    AIstate.invasionTargetedSystemIDs = list(all_invasion_targeted_system_ids)
    invasion_timer.stop(section_name="evaluating %d target planets" % (len(evaluatedPlanetIDs)))
    invasion_timer.end()


def get_invasion_targeted_planet_ids(planetIDs, missionType, empireID):
    """return list of being invaded planets"""
    universe = fo.getUniverse()
    invasionAIFleetMissions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([missionType])

    targetedPlanets = []

    for planetID in planetIDs:
        planet = universe.getPlanet(planetID)
        # add planets that are target of a mission
        for invasionAIFleetMission in invasionAIFleetMissions:
            aiTarget = AITarget.AITarget(EnumsAI.AITargetType.TARGET_PLANET, planetID)
            if invasionAIFleetMission.has_target(missionType, aiTarget):
                targetedPlanets.append(planetID)

    return targetedPlanets


def retaliation_risk_factor(empire_id):
    """a multiplicative adjustment to planet scores to account for risk of retaliation from planet owner"""
    #TODO implement (in militaryAI) actual military risk assessment of other empires
    if empire_id == -1:  # unowned
        return 1.5  # since no risk of retaliation, increase score
    else:
        return 1.0


def assign_invasion_values(planetIDs, missionType, fleetSupplyablePlanetIDs, empire):
    """creates a dictionary that takes planetIDs as key and their invasion score as value"""
    planetValues = {}
    neighbor_values = {}
    neighbor_val_ratio = .95
    universe = fo.getUniverse()    
    secureAIFleetMissions = foAI.foAIstate.get_fleet_missions_with_any_mission_types([EnumsAI.AIFleetMissionType.FLEET_MISSION_SECURE])
    for planetID in planetIDs:
        planetValues[planetID] = neighbor_values.setdefault(planetID, evaluate_invasion_planet(planetID, missionType, fleetSupplyablePlanetIDs, empire,  secureAIFleetMissions))
        print "planet %d,  values %s"%(planetID,  planetValues[planetID])
        planet = universe.getPlanet(planetID)
        specName = (planet and planet.speciesName) or ""
        species = fo.getSpecies(specName)
        if species and species.canProduceShips:
            system = universe.getSystem(planet.systemID)
            if not system:
                continue
            planet_industries = {}
            for pid2 in system.planetIDs:
                planet2 = universe.getPlanet(pid2)
                specName2 = (planet2 and planet2.speciesName) or ""
                species2 = fo.getSpecies(specName2)
                if species2 and species2.canProduceShips:
                    planet_industries[pid2] = planet2.currentMeterValue(fo.meterType.industry) + 0.1 # to prevent divide-by-zero
            industry_ratio = planet_industries[planetID] / max(planet_industries.values())
            for pid2 in system.planetIDs:
                if pid2 == planetID:
                    continue
                planet2 = universe.getPlanet(pid2)
                if planet2 and (planet2.owner != empire.empireID) and ((planet2.owner != -1) or (planet.currentMeterValue(fo.meterType.population) > 0)): #TODO check for allies
                    planetValues[planetID][0] += industry_ratio * neighbor_val_ratio * (neighbor_values.setdefault( pid2, evaluate_invasion_planet(pid2, missionType, fleetSupplyablePlanetIDs, empire,  secureAIFleetMissions))[0])
    return planetValues


def evaluate_invasion_planet(planetID, missionType, fleetSupplyablePlanetIDs, empire,  secureAIFleetMissions,  verbose=True):
    """return the invasion value (score, troops) of a planet"""
    detail = []
    buildingValues = {"BLD_IMPERIAL_PALACE":                    1000,
                                            "BLD_CULTURE_ARCHIVES":                 1000,
                                            "BLD_SHIPYARD_BASE":                        100,
                                            "BLD_SHIPYARD_ORG_ORB_INC":     200,
                                            "BLD_SHIPYARD_ORG_XENO_FAC": 200,
                                            "BLD_SHIPYARD_ORG_CELL_GRO_CHAMB": 200,
                                            "BLD_SHIPYARD_CON_NANOROBO": 300,
                                            "BLD_SHIPYARD_CON_GEOINT":      400,
                                            "BLD_SHIPYARD_CON_ADV_ENGINE": 1000,
                                            "BLD_SHIPYARD_AST":                             300,
                                            "BLD_SHIPYARD_AST_REF":                     1000,
                                            "BLD_SHIPYARD_ENRG_SOLAR":          1500,
                                            "BLD_INDUSTRY_CENTER":                   500,
                                            "BLD_GAS_GIANT_GEN":                           200,
                                            "BLD_SOL_ORB_GEN":                              800,
                                            "BLD_BLACK_HOLE_POW_GEN":       2000,
                                            "BLD_ENCLAVE_VOID":                             500,
                                            "BLD_NEUTRONIUM_EXTRACTOR": 2000,
                                            "BLD_NEUTRONIUM_SYNTH":             2000,
                                            "BLD_NEUTRONIUM_FORGE":             1000,
                                            "BLD_CONC_CAMP":                                    100,
                                            "BLD_BIOTERROR_PROJECTOR":      1000,
                                            "BLD_SHIPYARD_ENRG_COMP":         3000,
                                            }
    #TODO: add more factors, as used for colonization
    universe = fo.getUniverse()
    empireID = empire.empireID
    maxJumps=8
    planet = universe.getPlanet(planetID)
    if planet is None:  #TODO: exclude planets with stealth higher than empireDetection
        print "invasion AI couldn't access any info for planet id %d"%planetID
        return [0, 0]

    sysPartialVisTurn = dict_from_map(universe.getVisibilityTurnsMap(planet.systemID, empireID)).get(fo.visibility.partial, -9999)
    planetPartialVisTurn = dict_from_map(universe.getVisibilityTurnsMap(planetID, empireID)).get(fo.visibility.partial, -9999)

    if planetPartialVisTurn < sysPartialVisTurn:
        print "invasion AI couldn't get current info on planet id %d (was stealthed at last sighting)"%planetID
        return [0, 0]  #last time we had partial vis of the system, the planet was stealthed to us #TODO: track detection strength, order new scouting when it goes up

    specName=planet.speciesName
    species=fo.getSpecies(specName)
    if not species: #this call iterates over this Empire's available species with which it could colonize after an invasion
        planetEval = ColonisationAI.assign_colonisation_values([planetID],  EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION,  [planetID],  None,  empire, detail)
        popVal = max( 0.75*planetEval.get(planetID,  [0])[0],   ColonisationAI.evaluate_planet(planetID,  EnumsAI.AIFleetMissionType.FLEET_MISSION_OUTPOST,  [planetID],  None,  empire, detail)  )
    else:
        popVal = ColonisationAI.evaluate_planet(planetID,  EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION,  [planetID],  specName,  empire, detail)

    bldTally=0
    for bldType in [universe.getObject(bldg).buildingTypeName for bldg in planet.buildingIDs]:
        bval = buildingValues.get(bldType,  50)
        bldTally += bval
        detail.append("%s: %d"%(bldType,  bval))

    pSysID = planet.systemID
    capitolID = PlanetUtilsAI.get_capital()
    leastJumpsPath = []
    clear_path = True
    if capitolID:
        homeworld = universe.getPlanet(capitolID)
        if homeworld:
            homeSystemID = homeworld.systemID
            evalSystemID = planet.systemID
            if (homeSystemID != -1) and (evalSystemID != -1):
                leastJumpsPath = list(universe.leastJumpsPath(homeSystemID, evalSystemID, empireID))
                maxJumps =  len(leastJumpsPath)
    system_status = foAI.foAIstate.systemStatus.get(pSysID, {})
    sysFThrt = system_status.get('fleetThreat', 1000 )
    sysMThrt =system_status.get('monsterThreat', 0 )
    sysPThrt = system_status.get('planetThreat', 0 )
    sysTotThrt = sysFThrt + sysMThrt + sysPThrt
    max_path_threat = sysFThrt
    mil_ship_rating = ProductionAI.curBestMilShipRating()
    for path_sys_id in leastJumpsPath:
        path_leg_status = foAI.foAIstate.systemStatus.get(path_sys_id,  {})
        path_leg_threat = path_leg_status.get('fleetThreat', 1000 ) + path_leg_status.get('monsterThreat', 0 )
        if  path_leg_threat > 0.5 * mil_ship_rating:
            clear_path = False
            if path_leg_threat > max_path_threat:
                max_path_threat = path_leg_threat

    troops = planet.currentMeterValue(fo.meterType.troops)
    maxTroops = planet.currentMeterValue(fo.meterType.maxTroops)

    this_system = universe.getSystem(pSysID)
    secure_targets = [pSysID] + list(this_system.planetIDs)
    system_secured = False
    for mission in secureAIFleetMissions:
        if system_secured:
            break
        secure_fleet_id = mission.target_id
        s_fleet = universe.getFleet(secure_fleet_id)
        if (not s_fleet) or (s_fleet.systemID != pSysID):
            continue
        for ai_target in mission.get_targets(EnumsAI.AIFleetMissionType.FLEET_MISSION_SECURE):
            target_obj = ai_target.target_obj
            if (target_obj is not None) and target_obj.id in secure_targets:
                system_secured = True
                break

    pmaxShield = planet.currentMeterValue(fo.meterType.maxShield)
    if verbose:
        print "invasion eval of %s  %d --- maxShields %.1f -- sysFleetThreat %.1f  -- sysMonsterThreat %.1f"%(planet.name,  planetID,  pmaxShield,  sysFThrt,  sysMThrt)
    supplyVal=0
    enemyVal=0
    if planet.owner!=-1 : #value in taking this away from an enemy
        enemyVal= 20* (planet.currentMeterValue(fo.meterType.targetIndustry) +  2*planet.currentMeterValue(fo.meterType.targetResearch))
    if pSysID  in ColonisationAI.annexableSystemIDs: #TODO: extend to rings
        supplyVal =  100
    elif pSysID in ColonisationAI.annexableRing1:
        supplyVal =  200
    elif pSysID in ColonisationAI.annexableRing2:
        supplyVal =  300
    elif pSysID in ColonisationAI.annexableRing3:
        supplyVal =  400
    if max_path_threat > 0.5 * mil_ship_rating:
        if max_path_threat < 3 * mil_ship_rating:
            supplyVal *= 0.5
        else:
            supplyVal *= 0.2
        
    threatFactor = min(1,  0.2*MilitaryAI.totMilRating/(sysTotThrt+0.001))**2  #devalue invasions that would require too much military force
    buildTime=4
    if system_secured:
        plannedTroops = troops
    else:
        plannedTroops = min(troops+maxJumps+buildTime,  maxTroops)
    if empire.getTechStatus("SHP_ORG_HULL") != fo.techStatus.complete:
        troopCost = math.ceil( plannedTroops/6.0) *  ( 40*( 1+foAI.foAIstate.shipCount * AIDependencies.shipUpkeep ) )
    else:
        troopCost = math.ceil( plannedTroops/6.0) *  ( 20*( 1+foAI.foAIstate.shipCount * AIDependencies.shipUpkeep ) )
    planet_score = retaliation_risk_factor(planet.owner) * threatFactor * max(0,  popVal+supplyVal+bldTally+enemyVal-0.8*troopCost)
    if clear_path:
        planet_score *= 1.5
    invscore =  [ planet_score,  plannedTroops ]
    print invscore, "projected Troop Cost:",  troopCost,  ", threatFactor: ", threatFactor,  ", planet detail ",   detail, "popval,  supplyval,  bldval,  enemyval",   popVal,  supplyVal,  bldTally,  enemyVal
    return   invscore


def send_invasion_fleets(invasionFleetIDs, evaluatedPlanets, missionType):
    """sends a list of invasion fleets to a list of planet_value_pairs"""
    universe=fo.getUniverse()
    invasionPool = invasionFleetIDs[:]  #need to make a copy
    bestShip,  bestDesign,  buildChoices = ProductionAI.getBestShipInfo( EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_INVASION)
    if bestDesign:
        troopsPerBestShip = 2*(  list(bestDesign.parts).count("GT_TROOP_POD") )
    else:
        troopsPerBestShip=5 #may actually not have any troopers available, but this num will do for now

    #sortedTargets=sorted( [  ( pscore-ptroops/2 ,  pID,  pscore,  ptroops) for pID,  pscore,  ptroops in evaluatedPlanets ] ,  reverse=True)

    invasionPool=set(invasionPool)
    for  pID,  pscore,  ptroops in evaluatedPlanets: #
        if not invasionPool: return
        planet=universe.getPlanet(pID)
        if not planet: continue
        sysID = planet.systemID
        foundFleets = []
        podsNeeded= math.ceil( (ptroops+0.05)/2.0)
        foundStats={}
        minStats= {'rating':0, 'troopPods':podsNeeded}
        targetStats={'rating':10,'troopPods':podsNeeded+1}
        theseFleets = FleetUtilsAI.get_fleets_for_mission(1, targetStats , minStats,   foundStats,  "",  systems_to_check=[sysID],  systems_checked=[], fleet_pool_set=invasionPool,   fleet_list=foundFleets,  verbose=False)
        if not theseFleets:
            if not FleetUtilsAI.stats_meet_reqs(foundStats,  minStats):
                print "Insufficient invasion troop  allocation for system %d ( %s ) -- requested  %s , found %s"%(sysID,  universe.getSystem(sysID).name,  minStats,  foundStats)
                invasionPool.update( foundFleets )
                continue
            else:
                theseFleets = foundFleets
        aiTarget = AITarget.AITarget(EnumsAI.AITargetType.TARGET_PLANET, pID)
        print "assigning invasion fleets %s to target %s"%(theseFleets,  aiTarget)
        for fleetID in theseFleets:
            fleet=universe.getFleet(fleetID)
            aiFleetMission = foAI.foAIstate.get_fleet_mission(fleetID)
            aiFleetMission.clear_fleet_orders()
            aiFleetMission.clear_targets( (aiFleetMission.get_mission_types() + [-1])[0] )
            aiFleetMission.add_target(missionType, aiTarget)


def assign_invasion_fleets_to_invade():
    # assign fleet targets to invadable planets
    
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
    fleetSupplyablePlanetIDs = PlanetUtilsAI.get_planets_in__systems_ids(fleetSupplyableSystemIDs)

    allTroopBaseFleetIDs = FleetUtilsAI.get_empire_fleet_ids_by_role(EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION)
    availTroopBaseFleetIDs = set(FleetUtilsAI.extract_fleet_ids_without_mission_types(allTroopBaseFleetIDs))
    for fid in list(availTroopBaseFleetIDs):
        if fid not in availTroopBaseFleetIDs:
            continue
        fleet = universe.getFleet(fid)
        if not fleet: 
            continue
        sysID = fleet.systemID
        system = universe.getSystem(sysID)
        availPlanets = set(system.planetIDs).intersection(set( foAI.foAIstate.qualifyingTroopBaseTargets.keys()))
        print "Considering Base Troopers in %s,  found planets %s and regtistered targets %s with status %s"%(system.name,  list(system.planetIDs),  availPlanets,  
                                                                                                           [(pid,  foAI.foAIstate.qualifyingTroopBaseTargets[pid]) for pid in availPlanets])
        targets = [pid for pid in availPlanets if foAI.foAIstate.qualifyingTroopBaseTargets[pid][1] != -1 ]
        if not targets:
            print "Error found no valid target for troop base in system %s (%d)"%(system.name,  sysID)
            continue
        status=foAI.foAIstate.systemStatus.get( sysID,  {} )
        local_base_troops = set(status.get('myfleets',  [])).intersection(availTroopBaseFleetIDs)
        troop_pod_tally = 0
        for fid2 in local_base_troops:
            troop_pod_tally += FleetUtilsAI.count_parts_fleetwide(fid2, ["GT_TROOP_POD"])
        targetID=-1
        bestScore=-1
        target_troops = 0
        #
        for pid,  rating in assign_invasion_values(targets, EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION, fleetSupplyablePlanetIDs, empire).items():
            p_score,  p_troops = rating
            if p_score>bestScore:
                if p_troops >= 2*troop_pod_tally:
                    pass
                    #continue
                bestScore = p_score
                targetID = pid
                target_troops = p_troops
        if targetID != -1:
            local_base_troops.discard(fid)
            foundFleets = []
            podsNeeded= math.ceil( (target_troops - 2*(FleetUtilsAI.count_parts_fleetwide(fid, ["GT_TROOP_POD"]))+0.05)/2.0)
            foundStats={}
            minStats= {'rating':0, 'troopPods':podsNeeded}
            targetStats={'rating':10,'troopPods':podsNeeded}
            theseFleets = FleetUtilsAI.get_fleets_for_mission(1, targetStats , minStats,   foundStats,  "",  systems_to_check=[sysID],  systems_checked=[], fleet_pool_set=local_base_troops,   fleet_list=foundFleets,  verbose=False)
            for fid2 in foundFleets:
                FleetUtilsAI.merge_fleet_a_into_b(fid2,  fid)
                availTroopBaseFleetIDs.discard(fid2)
            availTroopBaseFleetIDs.discard(fid)
            foAI.foAIstate.qualifyingTroopBaseTargets[targetID][1] = -1 #TODO: should probably delete
            aiTarget = AITarget.AITarget(EnumsAI.AITargetType.TARGET_PLANET, targetID)
            aiFleetMission = foAI.foAIstate.get_fleet_mission(fid)
            aiFleetMission.add_target(EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION, aiTarget)
    
    invasionFleetIDs = AIstate.invasionFleetIDs

    send_invasion_fleets(invasionFleetIDs, AIstate.invasionTargets, EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION)
    allInvasionFleetIDs = FleetUtilsAI.get_empire_fleet_ids_by_role(EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION)
    for fid in  FleetUtilsAI.extract_fleet_ids_without_mission_types(allInvasionFleetIDs):
        thisMission = foAI.foAIstate.get_fleet_mission(fid)
        thisMission.check_mergers(context="Post-send consolidation of unassigned troops")
