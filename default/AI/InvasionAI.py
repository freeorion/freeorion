import freeOrionAIInterface as fo
import FreeOrionAI as foAI
import AIstate
from EnumsAI import AIFleetMissionType, AIExplorableSystemType, AITargetType,  AIPriorityType
import FleetUtilsAI
import PlanetUtilsAI
import AITarget
import math
from ProductionAI import  getBestShipInfo
from ColonisationAI import evaluatePlanet

def sysNameIDs(sysIDs):
    universe = fo.getUniverse()
    res=[]
    for sysID in sysIDs:
        sys = universe.getSystem(sysID)
        if sys:
            res.append( "%s:%d"%(sys.name, sysID ) )
    return res

def planetNameIDs(planetIDs):
    universe = fo.getUniverse()
    res=[]
    for pid in planetIDs:
        planet = universe.getSystem(pid)
        if planet:
            res.append( "%s:%d"%(planet.name, pid ) )
    return res

def getInvasionFleets():
    "get invasion fleets"

    allInvasionFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_INVASION)
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
    
    primeInvadableSystemIDs1 = set([])
    primeInvadableSystemIDs = set([])
    print "Current Fleet Supplyable Systems: ",  sysNameIDs(empire.fleetSupplyableSystemIDs)
    for sysID in empire.fleetSupplyableSystemIDs:
        primeInvadableSystemIDs.add(sysID)
        for nID in  universe.getImmediateNeighbors(sysID,  empireID):
            primeInvadableSystemIDs.add(nID)
            primeInvadableSystemIDs1.add(nID)
    primeInvadableSystemIDs1.difference_update(empire.fleetSupplyableSystemIDs)
    print "First Ring of invadable systems: ",  sysNameIDs(primeInvadableSystemIDs1)
    if empire.getTechStatus("CON_ORBITAL_CON") == fo.techStatus.complete:
        primeInvadableSystemIDs2 = set([])
        for sysID in list(primeInvadableSystemIDs1):
            for nID in  universe.getImmediateNeighbors(sysID,  empireID):
                primeInvadableSystemIDs2.add(nID)
        primeInvadableSystemIDs2.difference_update(primeInvadableSystemIDs)
        print "Second Ring of invadable systems: ",  sysNameIDs(primeInvadableSystemIDs2)
        primeInvadableSystemIDs.update(primeInvadableSystemIDs2)
        primeInvadableSystemIDs3 = set([])
        if foAI.foAIstate.aggression > 1:
            for sysID in list(primeInvadableSystemIDs2):
                for nID in  universe.getImmediateNeighbors(sysID,  empireID):
                    primeInvadableSystemIDs3.add(nID)
            primeInvadableSystemIDs3.difference_update(primeInvadableSystemIDs)
            print "Third Ring of invadable systems: ",  sysNameIDs(primeInvadableSystemIDs3)
            primeInvadableSystemIDs.update(primeInvadableSystemIDs3)
    primeInvadablePlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(primeInvadableSystemIDs)

    # get competitor planets
    exploredSystemIDs = empire.exploredSystemIDs
    exploredPlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(exploredSystemIDs)
    
    visibleSystemIDs = foAI.foAIstate.visInteriorSystemIDs.keys() + foAI.foAIstate. visBorderSystemIDs.keys()
    visiblePlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(visibleSystemIDs)
    accessibleSystemIDs = [sysID for sysID in visibleSystemIDs if  universe.systemsConnected(sysID, homeSystemID, empireID) ]
    acessiblePlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(accessibleSystemIDs)
    

    #allOwnedPlanetIDs = PlanetUtilsAI.getAllOwnedPlanetIDs(exploredPlanetIDs)
    allOwnedPlanetIDs = PlanetUtilsAI.getAllOwnedPlanetIDs(acessiblePlanetIDs)
    # print "All Owned and Populated PlanetIDs: " + str(allOwnedPlanetIDs)
    
    allPopulatedPlanets=PlanetUtilsAI.getPopulatedPlanetIDs(acessiblePlanetIDs)
    print "All Visible and accessible Populated PlanetIDs (including this empire's):              " + str(planetNameIDs(allPopulatedPlanets))

    empireOwnedPlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    # print "Empire Owned PlanetIDs:            " + str(empireOwnedPlanetIDs)

    invadablePlanetIDs = set(primeInvadablePlanetIDs).intersection(set(allPopulatedPlanets) - set(empireOwnedPlanetIDs))
    print "Prime Invadable PlanetIDs:              " + str(planetNameIDs(invadablePlanetIDs))

    print ""
    print "Invasion Targeted SystemIDs:       " + str(sysNameIDs(AIstate.invasionTargetedSystemIDs))
    invasionTargetedPlanetIDs = getInvasionTargetedPlanetIDs(universe.planetIDs, AIFleetMissionType.FLEET_MISSION_INVASION, empireID)
    allInvasionTargetedSystemIDs = PlanetUtilsAI.getSystems(invasionTargetedPlanetIDs)
 
    # export invasion targeted systems for other AI modules
    AIstate.invasionTargetedSystemIDs = allInvasionTargetedSystemIDs
    print "Invasion Targeted PlanetIDs:       " + str(planetNameIDs(invasionTargetedPlanetIDs))

    invasionFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_INVASION)
    if not invasionFleetIDs:
        print "Available Invasion Fleets:           0"
    else:
        print "Invasion FleetIDs:                 " + str(FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_INVASION))
 
    numInvasionFleets = len(FleetUtilsAI.extractFleetIDsWithoutMissionTypes(invasionFleetIDs))
    print "Invasion Fleets Without Missions:    " + str(numInvasionFleets)

    evaluatedPlanetIDs = list(set(invadablePlanetIDs) - set(invasionTargetedPlanetIDs))
    print "Evaluating potential invasions, PlanetIDs:               " + str(evaluatedPlanetIDs)

    evaluatedPlanets = assignInvasionValues(evaluatedPlanetIDs, AIFleetMissionType.FLEET_MISSION_INVASION, fleetSupplyablePlanetIDs, empire)

    sortedPlanets = [(pid,  pscore,  ptroops) for (pid,  (pscore, ptroops)) in evaluatedPlanets.items() ]
    sortedPlanets.sort(lambda x, y: cmp(x[1], y[1]), reverse=True)

    print ""
    if sortedPlanets:
        print "Invadable planetIDs,    ID | Score | Race | Troops | Name:"
        for pid,  pscore,  ptroops in sortedPlanets:
            planet = universe.getPlanet(pid)
            if planet:
                print "%6d | %6d | %s | %s | %d"%(pid,  pscore,  planet.name,  planet.speciesName,  ptroops)
            else:
                print "%6d | %6d | Error: invalid planet ID"%(pid,  pscore)
    else:
        print "No Invadable planets identified"

    # export opponent planets for other AI modules
    AIstate.opponentPlanetIDs = [pid for pid, pscore, trp in sortedPlanets]
    AIstate.invasionTargets = sortedPlanets

def getInvasionTargetedPlanetIDs(planetIDs, missionType, empireID):
    "return list of being invaded planets"

    universe = fo.getUniverse()
    invasionAIFleetMissions = foAI.foAIstate.getAIFleetMissionsWithAnyMissionTypes([missionType])

    targetedPlanets = []

    for planetID in planetIDs:
        planet = universe.getPlanet(planetID)
        # add planets that are target of a mission
        for invasionAIFleetMission in invasionAIFleetMissions:
            aiTarget = AITarget.AITarget(AITargetType.TARGET_PLANET, planetID)
            if invasionAIFleetMission.hasTarget(missionType, aiTarget):
                targetedPlanets.append(planetID)

    return targetedPlanets

def assignInvasionValues(planetIDs, missionType, fleetSupplyablePlanetIDs, empire):
    "creates a dictionary that takes planetIDs as key and their invasion score as value"

    planetValues = {}

    for planetID in planetIDs:
        planetValues[planetID] = evaluateInvasionPlanet(planetID, missionType, fleetSupplyablePlanetIDs, empire)

    return planetValues

def evaluateInvasionPlanet(planetID, missionType, fleetSupplyablePlanetIDs, empire):
    "return the invasion value of a planet"
    #TODO: add more factors, as used for colonization
    universe = fo.getUniverse()
    empireID = empire.empireID
    distanceFactor = 0
    planet = universe.getPlanet(planetID)
    if (planet == None) :  #TODO: exclude planets with stealth higher than empireDetection
        print "invasion AI couldn't get current info on planet %d"%planetID
        return 0, 0
        
    capitalID = PlanetUtilsAI.getCapital()
    homeworld = universe.getPlanet(capitalID)
    if homeworld:
        homeSystemID = homeworld.systemID
        evalSystemID = planet.systemID
        leastJumpsPath = len(universe.leastJumpsPath(homeSystemID, evalSystemID, empireID))
        distanceFactor = 4.0/(leastJumpsPath + 1)
        
    troops = planet.currentMeterValue(fo.meterType.troops)
    specName=planet.speciesName
    species=fo.getSpecies(specName)
    if not species:# perhaps stealth might make species info inaccessible
        targetPop=planet.currentMeterValue(fo.meterType.targetPopulation)
        if planetID in fleetSupplyablePlanetIDs:
            popVal =  4*targetPop
        else:
            popVal =  6*targetPop#assign higher value if the colony would extend our supply range
        planetSpecials = list(planet.specials)
        specialVal=0
        if  ( ( planet.size  ==  fo.planetSize.asteroids ) and  (empire.getTechStatus("PRO_ASTEROID_MINE") == fo.techStatus.complete ) ): 
                specialVal= 15   # asteroid mining is great, fast return
        for special in [ "MINERALS_SPECIAL",  "CRYSTALS_SPECIAL",  "METALOIDS_SPECIAL"] :
            if special in planetSpecials:
                specialVal = 40 
        return popVal+specialVal,  troops
    else:
        popVal = evaluatePlanet(planetID,  AIFleetMissionType.FLEET_MISSION_COLONISATION,  [planetID],  species,  empire) #evaluatePlanet is implorted from ColonisationAI
        if planetID not in fleetSupplyablePlanetIDs:
            popVal =  2.0*popVal#assign higher value if the colony would extend our supply range
        return popVal,  troops

        

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
    bestShip,  bestDesign,  buildChoices = getBestShipInfo( AIPriorityType.PRIORITY_PRODUCTION_INVASION)
    if bestDesign:
        troopsPerBestShip = 2*(  list(bestDesign.parts).count("GT_TROOP_POD") )
    else:
        troopsPerBestShip=5 #may actually not have any troopers available, but this num will do for now
        
    sortedTargets=sorted( [  ( pscore-ptroops/2 ,  pID,  pscore,  ptroops) for pID,  pscore,  ptroops in evaluatedPlanets ] ,  reverse=True)

    invasionPool=set(invasionPool)
    for modscrore,  pID,  pscore,  ptroops in sortedTargets: # evaluatedPlanets is a dictionary
        if not invasionPool: return
        planet=universe.getPlanet(pID)
        if not planet: continue
        sysID = planet.systemID
        foundFleets = []
        podsNeeded= int(math.ceil( (ptroops+2)/2)+0.0001)
        foundStats={}
        minStats= {'rating':0, 'troopPods':podsNeeded}
        targetStats={'rating':10,'troopPods':podsNeeded+2} 
        theseFleets = FleetUtilsAI.getFleetsForMission(1, targetStats , minStats,   foundStats,  "",  systemsToCheck=[sysID],  systemsChecked=[], fleetPoolSet=invasionPool,   fleetList=foundFleets,  verbose=False)
        if theseFleets == []:
            if not FleetUtilsAI.statsMeetReqs(foundStats,  minStats):
                print "Insufficient invasion troop  allocation for system %d ( %s ) -- requested  %s , found %s"%(sysID,  universe.getSystem(sysID).name,  minStats,  foundStats)
                invasionPool.update( foundFleets )
                continue
            else:
                theseFleets = foundFleets
        aiTarget = AITarget.AITarget(AITargetType.TARGET_PLANET, pID)
        print "assigning invasion fleets %s to target %s"%(theseFleets,  aiTarget)
        for fleetID in theseFleets:
            fleet=universe.getFleet(fleetID)
            aiFleetMission = foAI.foAIstate.getAIFleetMission(fleetID)
            aiFleetMission.clearAIFleetOrders()
            aiFleetMission.clearAITargets( (aiFleetMission.getAIMissionTypes() + [-1])[0] )
            aiFleetMission.addAITarget(missionType, aiTarget)

def assignInvasionFleetsToInvade():
    # assign fleet targets to invadable planets
    invasionFleetIDs = AIstate.invasionFleetIDs

    sendInvasionFleets(invasionFleetIDs, AIstate.invasionTargets, AIFleetMissionType.FLEET_MISSION_INVASION)

