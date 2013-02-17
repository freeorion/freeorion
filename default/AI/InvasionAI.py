import freeOrionAIInterface as fo
import FreeOrionAI as foAI
import AIstate
from EnumsAI import AIFleetMissionType, AIExplorableSystemType, AITargetType,  AIPriorityType
import FleetUtilsAI
import PlanetUtilsAI
import AITarget
import math
from ProductionAI import  getBestShipInfo
from ColonisationAI import evaluatePlanet,  annexableSystemIDs,  annexableRing1,  annexableRing2,  annexableRing3
import ColonisationAI

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
    
    primeInvadableSystemIDs = set(ColonisationAI.annexableSystemIDs)
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
    print "All Visible and accessible Populated PlanetIDs (including this empire's):              " + str(PlanetUtilsAI.planetNameIDs(allPopulatedPlanets))

    empireOwnedPlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    # print "Empire Owned PlanetIDs:            " + str(empireOwnedPlanetIDs)

    invadablePlanetIDs = set(primeInvadablePlanetIDs).intersection(set(allPopulatedPlanets) - set(empireOwnedPlanetIDs))
    print "Prime Invadable PlanetIDs:              " + str(PlanetUtilsAI.planetNameIDs(invadablePlanetIDs))

    print ""
    print "Current Invasion Targeted SystemIDs:       " + str(PlanetUtilsAI.sysNameIDs(AIstate.invasionTargetedSystemIDs))
    invasionTargetedPlanetIDs = getInvasionTargetedPlanetIDs(universe.planetIDs, AIFleetMissionType.FLEET_MISSION_INVASION, empireID)
    allInvasionTargetedSystemIDs = PlanetUtilsAI.getSystems(invasionTargetedPlanetIDs)
 
    # export invasion targeted systems for other AI modules
    AIstate.invasionTargetedSystemIDs = allInvasionTargetedSystemIDs
    print "Current Invasion Targeted PlanetIDs:       " + str(PlanetUtilsAI.planetNameIDs(invasionTargetedPlanetIDs))

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
        print "Invadable planets\nIDs,    ID | Score | Name           | Race             | Troops"
        for pid,  pscore,  ptroops in sortedPlanets:
            planet = universe.getPlanet(pid)
            if planet:
                print "%6d | %6d | %16s | %16s | %d"%(pid,  pscore,  planet.name,  planet.speciesName,  ptroops)
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
    buildingValues = {"BLD_IMPERIAL_PALACE":                    1000, 
                                            "BLD_CULTURE_ARCHIVES":                 1000, 
                                            "BLD_SHIPYARD_BASE":                        100, 
                                            "BLD_SHIPYARD_ORG_ORB_INC":     200, 
                                            "BLD_SHIPYARD_ORG_XENO_FAC": 200, 
                                            "BLD_SHIPYARD_ORG_CELL_GRO_CHAMB": 200, 
                                            "BLD_SHIPYARD_CON_NANOROBO": 300, 
                                            "BLD_SHIPYARD_CON_GEOINT":      400, 
                                            "BLD_SHIPYARD_CON_ADV_ENGINE": 1000, 
                                            "BLD_SHIPYARD_AST":                             150, 
                                            "BLD_SHIPYARD_AST_REF":                     500, 
                                            "BLD_SHIPYARD_ENRG_COMP":           500, 
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
        print "invasion AI couldn't get current info on planet %d"%planetID
        return 0, 0
        
    bldTally=0
    for bldType in [universe.getObject(bldg).buildingTypeName for bldg in planet.buildingIDs]:
        bldTally += buildingValues.get(bldType,  50)
        
        capitolID = PlanetUtilsAI.getCapital()
        if capitolID:
            homeworld = universe.getPlanet(capitolID)
            if homeworld:
                homeSystemID = homeworld.systemID
                evalSystemID = planet.systemID
                leastJumpsPath = len(universe.leastJumpsPath(homeSystemID, evalSystemID, empireID))
            maxJumps =  leastJumpsPath
        
    troops = planet.currentMeterValue(fo.meterType.troops)
    maxTroops = planet.currentMeterValue(fo.meterType.maxTroops)
    specName=planet.speciesName
    species=fo.getSpecies(specName)
    productionVal = 20*(planet.currentMeterValue(fo.meterType.targetPopulation)/planet.currentMeterValue(fo.meterType.population) )*max(planet.currentMeterValue(fo.meterType.targetIndustry),  2*planet.currentMeterValue(fo.meterType.targetResearch))
    supplyVal=0
    enemyVal=0
    if planet.owner!=-1 :
        enemyVal=productionVal #value in taking this away from an enemy
    if not species:#TODO:  perhaps stealth makes planet inacccesible & should abort
        try:
            targetPop=planet.currentMeterValue(fo.meterType.targetPopulation)
            popVal =  2*targetPop
        except:
            popVal=0
    else:
        popVal = evaluatePlanet(planetID,  AIFleetMissionType.FLEET_MISSION_COLONISATION,  [planetID],  species,  empire) #evaluatePlanet is imported from ColonisationAI
    if planetID not in fleetSupplyablePlanetIDs: #extends supply and probably visibility
        supplyVal =  20#TODO: better analysis here if supply obstructed by defending ships
    planetSpecials = list(planet.specials)
    specialVal=0
    if  ( ( planet.size  ==  fo.planetSize.asteroids ) and  (empire.getTechStatus("PRO_ASTEROID_MINE") == fo.techStatus.complete ) ): 
            specialVal= 15   #TODO: should do more eval re asteroid mining here
    for special in [ "MINERALS_SPECIAL",  "CRYSTALS_SPECIAL",  "METALOIDS_SPECIAL"] :
        if special in planetSpecials:
            specialVal +=10 #
    buildTime=4
    return max(0,  popVal+supplyVal+specialVal+bldTally+productionVal+enemyVal-8*troops),  min(troops+maxJumps+buildTime,  maxTroops)

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
    allInvasionFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_INVASION)
    for fid in  FleetUtilsAI.extractFleetIDsWithoutMissionTypes(allInvasionFleetIDs):
        thisMission = foAI.foAIstate.getAIFleetMission(fid)
        thisMission.checkMergers(context="Post-send consolidation of unassigned troops")

