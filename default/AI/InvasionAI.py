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

def dictFromMap(thismap):
    return dict(  [  (el.key(),  el.data() ) for el in thismap ] )

def getInvasionFleets():
    "get invasion fleets"

    allInvasionFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION)
    AIstate.invasionFleetIDs = FleetUtilsAI.extractFleetIDsWithoutMissionTypes(allInvasionFleetIDs)

    # get supplyable planets
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    capitalID = PlanetUtilsAI.getCapital()
    #capitalID = empire.capitalID
    homeworld=None
    if capitalID:
        homeworld = universe.getPlanet(capitalID)
    if homeworld:
        homeSystemID = homeworld.systemID
    else:
        speciesName = ""
        homeworldName=" no remaining homeworld "
        homeSystemID = -1

    fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
    fleetSupplyablePlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(fleetSupplyableSystemIDs)

    primeInvadableSystemIDs = set(ColonisationAI.annexableSystemIDs)
    primeInvadablePlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(primeInvadableSystemIDs)

    # get competitor planets
    exploredSystemIDs = empire.exploredSystemIDs
    exploredPlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(exploredSystemIDs)

    visibleSystemIDs = foAI.foAIstate.visInteriorSystemIDs.keys() + foAI.foAIstate. visBorderSystemIDs.keys()
    visiblePlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(visibleSystemIDs)
    if homeSystemID != -1:
        accessibleSystemIDs = [sysID for sysID in visibleSystemIDs if  (sysID != -1 ) and universe.systemsConnected(sysID, homeSystemID, empireID) ]
    else:
        print "Invasion Warning: this empire has no identifiable homeworld,  will therefor treat all visible planets as accessible."
        accessibleSystemIDs = visibleSystemIDs #TODO: check if any troop ships still owned, use their system as home system
    acessiblePlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(accessibleSystemIDs)
    print "Accessible Systems: " + str(PlanetUtilsAI.sysNameIDs(accessibleSystemIDs))
    print

    #allOwnedPlanetIDs = PlanetUtilsAI.getAllOwnedPlanetIDs(exploredPlanetIDs)
    allOwnedPlanetIDs = PlanetUtilsAI.getAllOwnedPlanetIDs(acessiblePlanetIDs)#need these for unpopulated outposts
    # print "All Owned and Populated PlanetIDs: " + str(allOwnedPlanetIDs)

    allPopulatedPlanets=PlanetUtilsAI.getPopulatedPlanetIDs(acessiblePlanetIDs)#need this for natives
    print "All Visible and accessible Populated PlanetIDs (including this empire's): " + str(PlanetUtilsAI.planetNameIDs(allPopulatedPlanets))
    print
    print "Prime Invadable Target Systems: " + str(PlanetUtilsAI.sysNameIDs(primeInvadableSystemIDs))
    print

    empireOwnedPlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    # print "Empire Owned PlanetIDs:            " + str(empireOwnedPlanetIDs)

    invadablePlanetIDs = set(primeInvadablePlanetIDs).intersection(set(allOwnedPlanetIDs).union(allPopulatedPlanets) - set(empireOwnedPlanetIDs))
    print "Prime Invadable PlanetIDs:              " + str(PlanetUtilsAI.planetNameIDs(invadablePlanetIDs))

    print ""
    print "Current Invasion Targeted SystemIDs:       " + str(PlanetUtilsAI.sysNameIDs(AIstate.invasionTargetedSystemIDs))
    invasionTargetedPlanetIDs = getInvasionTargetedPlanetIDs(universe.planetIDs, EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION, empireID)
    invasionTargetedPlanetIDs.extend( getInvasionTargetedPlanetIDs(universe.planetIDs, EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION, empireID))
    allInvasionTargetedSystemIDs = set(PlanetUtilsAI.getSystems(invasionTargetedPlanetIDs))

    print "Current Invasion Targeted PlanetIDs:       " + str(PlanetUtilsAI.planetNameIDs(invasionTargetedPlanetIDs))

    invasionFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION)
    if not invasionFleetIDs:
        print "Available Invasion Fleets:           0"
    else:
        print "Invasion FleetIDs:                 " + str(FleetUtilsAI.getEmpireFleetIDsByRole(EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION))

    numInvasionFleets = len(FleetUtilsAI.extractFleetIDsWithoutMissionTypes(invasionFleetIDs))
    print "Invasion Fleets Without Missions:    " + str(numInvasionFleets)
    
    availablePP = {}
    for el in  empire.planetsWithAvailablePP:  #keys are sets of ints; data is doubles
        avail_pp = el.data()
        for pid in el.key():
            availablePP[pid] = avail_pp
    if len (invadablePlanetIDs) > 0:
        #print "Evaluating Troop Bases (SpaceInvaders) for %s"%(invadablePlanetIDs)
        pass
    for pid in invadablePlanetIDs: #TODO: reorganize
        planet = universe.getPlanet(pid)
        if not planet: 
            continue
        sysID = planet.systemID
        sysPartialVisTurn = dictFromMap(universe.getVisibilityTurnsMap(planet.systemID,  empireID)).get(fo.visibility.partial, -9999)
        planetPartialVisTurn = dictFromMap(universe.getVisibilityTurnsMap(pid,  empireID)).get(fo.visibility.partial, -9999)
        if (planetPartialVisTurn < sysPartialVisTurn):
            #print "rejecting %s due to stealth"%planet.name
            continue
        for pid2 in ColonisationAI.empireSpeciesSystems.get(sysID,  {}).get('pids', []):
            if availablePP.get(pid2,  0) < 2: #TODO: improve troop base PP sufficiency determination
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
        planet = universe.getPlanet(pid)
        if planet and planet.owner == empireID:
            del foAI.foAIstate.qualifyingTroopBaseTargets[pid]

    reserved_troop_base_targets = []
    secureAIFleetMissions = foAI.foAIstate.getAIFleetMissionsWithAnyMissionTypes([EnumsAI.AIFleetMissionType.FLEET_MISSION_SECURE])
    #print "considering possible troop bases at %s" % (foAI.foAIstate.qualifyingTroopBaseTargets.keys())
    for pid in (set(foAI.foAIstate.qualifyingTroopBaseTargets.keys()) - set(invasionTargetedPlanetIDs)): #TODO: consider overriding standard invasion mission
        planet = universe.getPlanet(pid)
        if  foAI.foAIstate.qualifyingTroopBaseTargets[pid][1] != -1: 
            reserved_troop_base_targets.append(pid)
            if planet:
                allInvasionTargetedSystemIDs.add( planet.systemID )
            continue  #already building for here
        loc = foAI.foAIstate.qualifyingTroopBaseTargets[pid][0]
        this_score,  p_troops = evaluateInvasionPlanet(pid, EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION, fleetSupplyablePlanetIDs, empire,  secureAIFleetMissions,  False)
        if  (planet.currentMeterValue(fo.meterType.shield)) > 0:
            continue
        bestShip,  colDesign,  buildChoices = ProductionAI.getBestShipInfo(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_ORBITAL_INVASION,  loc)
        if not bestShip:
            #print "Error: no troop base can be built at ",  PlanetUtilsAI.planetNameIDs([loc])
            continue
        #print "selecting  ",  PlanetUtilsAI.planetNameIDs([loc]),  " to build Orbital troop bases"
        n_bases = math.ceil((p_troops+1) / 2)#TODO: reconsider this +1 safety factor
        retval  = fo.issueEnqueueShipProductionOrder(bestShip, loc)
        print "Enqueueing %d Troop Bases at %s for %s"%( n_bases,  PlanetUtilsAI.planetNameIDs([loc]),  PlanetUtilsAI.planetNameIDs([pid]))
        if retval !=0:
            allInvasionTargetedSystemIDs.add( planet.systemID )
            reserved_troop_base_targets.append(pid)
            foAI.foAIstate.qualifyingTroopBaseTargets[pid][1] = loc
            fo.issueChangeProductionQuantityOrder(empire.productionQueue.size -1,  1,  int(n_bases))
            res=fo.issueRequeueProductionOrder(empire.productionQueue.size -1,  0) 

    #TODO: check if any invasionTargetedPlanetIDs need more troops assigned
    evaluatedPlanetIDs = list(set(invadablePlanetIDs) - set(invasionTargetedPlanetIDs) - set(reserved_troop_base_targets)  ) 
    print "Evaluating potential invasions, PlanetIDs:               " + str(evaluatedPlanetIDs)

    evaluatedPlanets = assignInvasionValues(evaluatedPlanetIDs, EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION, fleetSupplyablePlanetIDs, empire)

    sortedPlanets = [(pid,  pscore,  ptroops) for (pid,  (pscore, ptroops)) in evaluatedPlanets.items() ]
    sortedPlanets.sort(lambda x, y: cmp(x[1], y[1]), reverse=True)
    sortedPlanets = [(pid,  pscore%10000,  ptroops) for (pid,  pscore, ptroops) in sortedPlanets ]

    print ""
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
    AIstate.invasionTargetedSystemIDs = list(allInvasionTargetedSystemIDs)


def getInvasionTargetedPlanetIDs(planetIDs, missionType, empireID):
    "return list of being invaded planets"

    universe = fo.getUniverse()
    invasionAIFleetMissions = foAI.foAIstate.getAIFleetMissionsWithAnyMissionTypes([missionType])

    targetedPlanets = []

    for planetID in planetIDs:
        planet = universe.getPlanet(planetID)
        # add planets that are target of a mission
        for invasionAIFleetMission in invasionAIFleetMissions:
            aiTarget = AITarget.AITarget(EnumsAI.AITargetType.TARGET_PLANET, planetID)
            if invasionAIFleetMission.hasTarget(missionType, aiTarget):
                targetedPlanets.append(planetID)

    return targetedPlanets

def assignInvasionValues(planetIDs, missionType, fleetSupplyablePlanetIDs, empire):
    "creates a dictionary that takes planetIDs as key and their invasion score as value"

    planetValues = {}
    secureAIFleetMissions = foAI.foAIstate.getAIFleetMissionsWithAnyMissionTypes([EnumsAI.AIFleetMissionType.FLEET_MISSION_SECURE])

    for planetID in planetIDs:
        planetValues[planetID] = evaluateInvasionPlanet(planetID, missionType, fleetSupplyablePlanetIDs, empire,  secureAIFleetMissions)

    return planetValues

def evaluateInvasionPlanet(planetID, missionType, fleetSupplyablePlanetIDs, empire,  secureAIFleetMissions,  verbose=True):
    "return the invasion value (score, troops) of a planet"
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
    if (planet == None) :  #TODO: exclude planets with stealth higher than empireDetection
        print "invasion AI couldn't access any info for planet id %d"%planetID
        return 0, 0

    sysPartialVisTurn = dictFromMap(universe.getVisibilityTurnsMap(planet.systemID,  empireID)).get(fo.visibility.partial, -9999)
    planetPartialVisTurn = dictFromMap(universe.getVisibilityTurnsMap(planetID,  empireID)).get(fo.visibility.partial, -9999)

    if planetPartialVisTurn < sysPartialVisTurn:
        print "invasion AI couldn't get current info on planet id %d (was stealthed at last sighting)"%planetID
        return 0, 0  #last time we had partial vis of the system, the planet was stealthed to us #TODO: track detection strength, order new scouting when it goes up

    specName=planet.speciesName
    species=fo.getSpecies(specName)
    if not species: #this call iterates over this Empire's available species with which it could colonize after an invasion
        planetEval = ColonisationAI.assignColonisationValues([planetID],  EnumsAI.AIFleetMissionType.FLEET_MISSION_COLONISATION,  [planetID],  None,  empire, detail) #evaluatePlanet is imported from ColonisationAI
        popVal = max( 0.75*planetEval.get(planetID,  [0])[0],   ColonisationAI.evaluatePlanet(planetID,  EnumsAI.AIFleetMissionType.FLEET_MISSION_OUTPOST,  [planetID],  None,  empire, detail)  )
    else:
        popVal = ColonisationAI.evaluatePlanet(planetID,  EnumsAI.AIFleetMissionType.FLEET_MISSION_COLONISATION,  [planetID],  specName,  empire, detail) #evaluatePlanet is imported from ColonisationAI

    bldTally=0
    for bldType in [universe.getObject(bldg).buildingTypeName for bldg in planet.buildingIDs]:
        bval = buildingValues.get(bldType,  50)
        bldTally += bval
        detail.append("%s: %d"%(bldType,  bval))

        capitolID = PlanetUtilsAI.getCapital()
        if capitolID:
            homeworld = universe.getPlanet(capitolID)
            if homeworld:
                homeSystemID = homeworld.systemID
                evalSystemID = planet.systemID
                if (homeSystemID != -1) and (evalSystemID != -1):
                    leastJumpsPath = len(universe.leastJumpsPath(homeSystemID, evalSystemID, empireID))
                    maxJumps =  leastJumpsPath

    troops = planet.currentMeterValue(fo.meterType.troops)
    maxTroops = planet.currentMeterValue(fo.meterType.maxTroops)

    pSysID = planet.systemID
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
        for ai_target in mission.getAITargets(EnumsAI.AIFleetMissionType.FLEET_MISSION_SECURE):
            target_obj = ai_target.target_obj
            if (target_obj is not None) and target_obj.id in secure_targets:
                system_secured = True
                break

    pmaxShield = planet.currentMeterValue(fo.meterType.maxShield)
    sysFThrt = foAI.foAIstate.systemStatus.get(pSysID, {}).get('fleetThreat', 1000 )
    sysMThrt = foAI.foAIstate.systemStatus.get(pSysID, {}).get('monsterThreat', 0 )
    sysPThrt = foAI.foAIstate.systemStatus.get(pSysID, {}).get('planetThreat', 0 )
    sysTotThrt = sysFThrt + sysMThrt + sysPThrt
    if verbose:
        print "invasion eval of %s  %d --- maxShields %.1f -- sysFleetThreat %.1f  -- sysMonsterThreat %.1f"%(planet.name,  planetID,  pmaxShield,  sysFThrt,  sysMThrt)
    supplyVal=0
    enemyVal=0
    if planet.owner!=-1 : #value in taking this away from an enemy
        enemyVal= 20* (planet.currentMeterValue(fo.meterType.targetIndustry) +  2*planet.currentMeterValue(fo.meterType.targetResearch))
    if planetID  in fleetSupplyablePlanetIDs: #TODO: extend to rings
        supplyVal =  100
        if planet.owner== -1:
        #if  (pmaxShield <10):
            if ( sysFThrt < 0.5*ProductionAI.curBestMilShipRating() ):
                if ( sysMThrt < 3*ProductionAI.curBestMilShipRating()):
                    supplyVal = 50
                else:
                    supplyVal = 20
            else:
                supplyVal *= int( min(1, ProductionAI.curBestMilShipRating() /  sysFThrt )  )
    threatFactor = min(1,  0.2*MilitaryAI.totMilRating/(sysTotThrt+0.001))**2  #devalue invasions that would require too much military force
    buildTime=4
    if system_secured:
        plannedTroops = troops
    else:
        plannedTroops = min(troops+maxJumps+buildTime,  maxTroops)
    if ( empire.getTechStatus("SHP_ORG_HULL") != fo.techStatus.complete ):
        troopCost = math.ceil( plannedTroops/6.0) *  ( 40*( 1+foAI.foAIstate.shipCount * AIDependencies.shipUpkeep ) )
    else:
        troopCost = math.ceil( plannedTroops/6.0) *  ( 20*( 1+foAI.foAIstate.shipCount * AIDependencies.shipUpkeep ) )
    invscore =  threatFactor*max(0,  popVal+supplyVal+bldTally+enemyVal-0.8*troopCost),  plannedTroops
    print invscore, "projected Troop Cost:",  troopCost,  ", threatFactor: ", threatFactor,  ", planet detail ",   detail, "popval,  supplyval,  bldval,  enemyval",   popVal,  supplyVal,  bldTally,  enemyVal
    return   invscore

def getPlanetPopulation(planetID):
    "return planet population"

    universe = fo.getUniverse()

    planet = universe.getPlanet(planetID)
    planetPopulation = planet.currentMeterValue(fo.meterType.population)

    if planet == None: return 0
    else:
        return planetPopulation

def sendInvasionFleets(invasionFleetIDs, evaluatedPlanets, missionType):
    "sends a list of invasion fleets to a list of planet_value_pairs"
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
        theseFleets = FleetUtilsAI.getFleetsForMission(1, targetStats , minStats,   foundStats,  "",  systemsToCheck=[sysID],  systemsChecked=[], fleetPoolSet=invasionPool,   fleetList=foundFleets,  verbose=False)
        if theseFleets == []:
            if not FleetUtilsAI.statsMeetReqs(foundStats,  minStats):
                print "Insufficient invasion troop  allocation for system %d ( %s ) -- requested  %s , found %s"%(sysID,  universe.getSystem(sysID).name,  minStats,  foundStats)
                invasionPool.update( foundFleets )
                continue
            else:
                theseFleets = foundFleets
        aiTarget = AITarget.AITarget(EnumsAI.AITargetType.TARGET_PLANET, pID)
        print "assigning invasion fleets %s to target %s"%(theseFleets,  aiTarget)
        for fleetID in theseFleets:
            fleet=universe.getFleet(fleetID)
            aiFleetMission = foAI.foAIstate.getAIFleetMission(fleetID)
            aiFleetMission.clearAIFleetOrders()
            aiFleetMission.clearAITargets( (aiFleetMission.getAIMissionTypes() + [-1])[0] )
            aiFleetMission.addAITarget(missionType, aiTarget)

def assignInvasionFleetsToInvade():
    # assign fleet targets to invadable planets
    
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
    fleetSupplyablePlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(fleetSupplyableSystemIDs)

    allTroopBaseFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION)
    availTroopBaseFleetIDs = set(FleetUtilsAI.extractFleetIDsWithoutMissionTypes(allTroopBaseFleetIDs))
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
            troop_pod_tally += FleetUtilsAI.countPartsFleetwide(fid2, ["GT_TROOP_POD"])
        targetID=-1
        bestScore=-1
        target_troops = 0
        #
        for pid,  rating in assignInvasionValues(targets, EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION, fleetSupplyablePlanetIDs, empire).items():
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
            podsNeeded= math.ceil( (target_troops - 2*(FleetUtilsAI.countPartsFleetwide(fid, ["GT_TROOP_POD"]))+0.05)/2.0)
            foundStats={}
            minStats= {'rating':0, 'troopPods':podsNeeded}
            targetStats={'rating':10,'troopPods':podsNeeded}
            theseFleets = FleetUtilsAI.getFleetsForMission(1, targetStats , minStats,   foundStats,  "",  systemsToCheck=[sysID],  systemsChecked=[], fleetPoolSet=local_base_troops,   fleetList=foundFleets,  verbose=False)
            for fid2 in foundFleets:
                FleetUtilsAI.mergeFleetAintoB(fid2,  fid)
                availTroopBaseFleetIDs.discard(fid2)
            availTroopBaseFleetIDs.discard(fid)
            foAI.foAIstate.qualifyingTroopBaseTargets[targetID][1] = -1 #TODO: should probably delete
            aiTarget = AITarget.AITarget(EnumsAI.AITargetType.TARGET_PLANET, targetID)
            aiFleetMission = foAI.foAIstate.getAIFleetMission(fid)
            aiFleetMission.addAITarget(EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION, aiTarget)
    
    invasionFleetIDs = AIstate.invasionFleetIDs

    sendInvasionFleets(invasionFleetIDs, AIstate.invasionTargets, EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION)
    allInvasionFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION)
    for fid in  FleetUtilsAI.extractFleetIDsWithoutMissionTypes(allInvasionFleetIDs):
        thisMission = foAI.foAIstate.getAIFleetMission(fid)
        thisMission.checkMergers(context="Post-send consolidation of unassigned troops")

