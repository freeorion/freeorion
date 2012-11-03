import freeOrionAIInterface as fo
import FreeOrionAI as foAI
import AIstate
from EnumsAI import AIFleetMissionType, AIExplorableSystemType, AITargetType,  AIPriorityType
import FleetUtilsAI
import PlanetUtilsAI
import AITarget
import math
from ProductionAI import  getBestShipInfo

def getInvasionFleets():
    "get invasion fleets"

    allInvasionFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_INVASION)
    AIstate.invasionFleetIDs = FleetUtilsAI.extractFleetIDsWithoutMissionTypes(allInvasionFleetIDs)

    # get supplyable planets
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID

    fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
    fleetSupplyablePlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(fleetSupplyableSystemIDs)

    # get competitor planets
    exploredSystemIDs = empire.exploredSystemIDs
    exploredPlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(exploredSystemIDs)

    allOwnedPlanetIDs = PlanetUtilsAI.getAllOwnedPlanetIDs(exploredPlanetIDs)
    # print "All Owned and Populated PlanetIDs: " + str(allOwnedPlanetIDs)
    
    allPopulatedPlanets=PlanetUtilsAI.getPopulatedPlanetIDs(exploredPlanetIDs)

    empireOwnedPlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    # print "Empire Owned PlanetIDs:            " + str(empireOwnedPlanetIDs)

    invadablePlanetIDs = list(set(allPopulatedPlanets) - set(empireOwnedPlanetIDs))
    #print "Invadable PlanetIDs:              " + str(invadablePlanetIDs)

    print ""
    print "Invasion Targeted SystemIDs:       " + str(AIstate.invasionTargetedSystemIDs)
    invasionTargetedPlanetIDs = getInvasionTargetedPlanetIDs(universe.planetIDs, AIFleetMissionType.FLEET_MISSION_INVASION, empireID)
    allInvasionTargetedSystemIDs = PlanetUtilsAI.getSystems(invasionTargetedPlanetIDs)
 
    # export invasion targeted systems for other AI modules
    AIstate.invasionTargetedSystemIDs = allInvasionTargetedSystemIDs
    print "Invasion Targeted PlanetIDs:       " + str(invasionTargetedPlanetIDs)

    invasionFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_INVASION)
    if not invasionFleetIDs:
        print "Available Invasion Fleets:           0"
    else:
        print "Invasion FleetIDs:                 " + str(FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_INVASION))
 
    numInvasionFleets = len(FleetUtilsAI.extractFleetIDsWithoutMissionTypes(invasionFleetIDs))
    print "Invasion Fleets Without Missions:    " + str(numInvasionFleets)

    evaluatedPlanetIDs = list(set(invadablePlanetIDs) - set(invasionTargetedPlanetIDs))
    # print "Evaluated PlanetIDs:               " + str(evaluatedPlanetIDs)

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
        planetValues[planetID] = evaluatePlanet(planetID, missionType, fleetSupplyablePlanetIDs, empire)

    return planetValues

def evaluatePlanet(planetID, missionType, fleetSupplyablePlanetIDs, empire):
    "return the invasion value of a planet"

    universe = fo.getUniverse()
    planet = universe.getPlanet(planetID)
    if (planet == None): return 0, 0

    # give preference to closest invadable planets
    empireID = empire.empireID
    capitalID = PlanetUtilsAI.getCapital()
    homeworld = universe.getPlanet(capitalID)
    if homeworld:
        homeSystemID = homeworld.systemID
        evalSystemID = planet.systemID
        leastJumpsPath = len(universe.leastJumpsPath(homeSystemID, evalSystemID, empireID))
        distanceFactor = 1.001/(leastJumpsPath + 1)
    else:
        distanceFactor = 0

    if planetID in fleetSupplyablePlanetIDs:
        popVal =  getPlanetPopulation(planetID) * planet.size * planet.type * distanceFactor
    else:
        popVal =  getPlanetPopulation(planetID) * planet.size * planet.type * .5 * distanceFactor

    troops = planet.currentMeterValue(fo.meterType.troops)
    planetSpecials = list(planet.specials)
    specialVal=0
    if  ( ( planet.size  ==  fo.planetSize.asteroids ) and  (empire.getTechStatus("PRO_ASTEROID_MINE") == fo.techStatus.complete ) ): 
            specialVal= 15   # asteroid mining is great, fast return
    for special in [ "MINERALS_SPECIAL",  "CRYSTALS_SPECIAL",  "METALOIDS_SPECIAL"] :
        if special in planetSpecials:
            specialVal = 40 #TODO:  need to enable exobots for asteroid crystals  
    return popVal+specialVal,  troops

        

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
        troopsPerBestShip = 5*(  list(bestDesign.parts).count("GT_TROOP_POD") )
    else:
        troopsPerBestShip=5 #may actually not have any troopers available, but this num will do for now

    for pID,  pscore,  ptroops in evaluatedPlanets: # evaluatedPlanets is a dictionary
        if invasionPool ==[]: return
        planet=universe.getPlanet(pID)
        if not planet: continue
        sysID = planet.systemID
        foundFleets = []
        shipsNeeded= math.ceil( (ptroops+4)/5)
        theseFleets = FleetUtilsAI.getFleetsForMission(shipsNeeded+1,  0, 0,   [0],  "",  systemsToCheck=[sysID],  systemsChecked=[], fleetPool=invasionPool,   fleetList=foundFleets,  verbose=False)
        if theseFleets == []:
            if sum([len(universe.getFleet(fID).shipIDs) for fID in foundFleets] ) < shipsNeeded:
                print "Insufficient invasion troop  allocation for system %d ( %s ) -- requested  %8d , found %d"%(sysID,  universe.getSystem(sysID).name,  shipsNeeded,  len(foundFleets))
                continue
            else:
                theseFleets = foundFleets
        aiTarget = AITarget.AITarget(AITargetType.TARGET_PLANET, pID)
        for fleetID in theseFleets:
            fleet=universe.getFleet(fleetID)
            aiFleetMission = foAI.foAIstate.getAIFleetMission(fleetID)
            aiFleetMission.clearAIFleetOrders()
            aiFleetMission.clearAITargets( (aiFleetMission.getAIMissionTypes() + [-1])[0] )
            aiFleetMission.addAITarget(missionType, aiTarget)

def assignInvasionFleetsToInvade():
    # assign fleet targets to invadable planets

    sendInvasionFleets(AIstate.invasionFleetIDs, AIstate.invasionTargets, AIFleetMissionType.FLEET_MISSION_INVASION)

