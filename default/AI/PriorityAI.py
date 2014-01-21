import math
import random

import freeOrionAIInterface as fo # pylint: disable=import-error

import AIstate
import ColonisationAI
import ExplorationAI
import FleetUtilsAI
import FreeOrionAI as foAI
import InvasionAI
import MilitaryAI
import PlanetUtilsAI
import EnumsAI
import ProductionAI
import ResearchAI
from time import time


allottedInvasionTargets=0
allottedColonyTargets=0
scoutsNeeded = 0
unmetThreat = 0

def calculatePriorities():
    "calculates the priorities of the AI player"
    print("checking statuses")
    # Industry, Research, Colony, Invasion, Military
    times=[]
    tasks = []
    times.append( time() )
    tasks.append("init")
    foAI.foAIstate.setPriority(EnumsAI.AIPriorityType.PRIORITY_RESOURCE_PRODUCTION, 50) # let this one stay fixed & just adjust Research
    times.append( time() )
    tasks.append( "setting Production Priority" )
    foAI.foAIstate.setPriority(EnumsAI.AIPriorityType.PRIORITY_RESOURCE_RESEARCH, calculateResearchPriority())
    times.append( time() )
    tasks.append(  "setting Research Priority")
    ColonisationAI.getColonyFleets() # sets foAI.foAIstate.colonisablePlanetIDs and foAI.foAIstate.outpostPlanetIDs  and many other values used by other modules
    times.append( time() )
    tasks.append(  "Evaluating Colonization Status")
    InvasionAI.getInvasionFleets() # sets AIstate.invasionFleetIDs, AIstate.opponentPlanetIDs, and AIstate.invasionTargetedPlanetIDs
    times.append( time() )
    tasks.append(  "Evaluating Invasion Status")
    MilitaryAI.getMilitaryFleets() # sets AIstate.militaryFleetIDs and AIstate.militaryTargetedSystemIDs
    times.append( time() )
    tasks.append(  "Evaluating Military Status")

    print("calculating priorities")
    calculateIndustryPriority()#purely for reporting purposes
    times.append( time() )
    tasks.append(  "reporting Production Priority")

    foAI.foAIstate.setPriority(EnumsAI.AIPriorityType.PRIORITY_RESOURCE_TRADE, 0)
    foAI.foAIstate.setPriority(EnumsAI.AIPriorityType.PRIORITY_RESOURCE_CONSTRUCTION, 0)

    foAI.foAIstate.setPriority(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_EXPLORATION, calculateExplorationPriority())
    times.append( time() )
    tasks.append(  "setting Exploration Priority")
    foAI.foAIstate.setPriority(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_OUTPOST, calculateOutpostPriority())
    times.append( time() )
    tasks.append(  "setting Outpost Priority")
    foAI.foAIstate.setPriority(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION, calculateColonisationPriority())
    times.append( time() )
    tasks.append(  "setting Colony Priority")
    foAI.foAIstate.setPriority(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_INVASION, calculateInvasionPriority())
    times.append( time() )
    tasks.append(  "setting Invasion Priority")
    foAI.foAIstate.setPriority(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY, calculateMilitaryPriority())
    times.append( time() )
    tasks.append(  "setting Military Priority")
    foAI.foAIstate.setPriority(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_BUILDINGS, 25)

    foAI.foAIstate.setPriority(EnumsAI.AIPriorityType.PRIORITY_RESEARCH_LEARNING, calculateLearningPriority())
    foAI.foAIstate.setPriority(EnumsAI.AIPriorityType.PRIORITY_RESEARCH_GROWTH, calculateGrowthPriority())
    foAI.foAIstate.setPriority(EnumsAI.AIPriorityType.PRIORITY_RESEARCH_PRODUCTION, calculateTechsProductionPriority())
    foAI.foAIstate.setPriority(EnumsAI.AIPriorityType.PRIORITY_RESEARCH_CONSTRUCTION, calculateConstructionPriority())
    foAI.foAIstate.setPriority(EnumsAI.AIPriorityType.PRIORITY_RESEARCH_ECONOMICS, 0)
    foAI.foAIstate.setPriority(EnumsAI.AIPriorityType.PRIORITY_RESEARCH_SHIPS, calculateShipsPriority())
    foAI.foAIstate.setPriority(EnumsAI.AIPriorityType.PRIORITY_RESEARCH_DEFENSE, 0)
    times.append( time() )
    tasks.append(  "total processing")
    
    for t_index in range(1, len(times)-1):
        print "calculatePriorities(): %40s took %d msec"%(tasks[t_index],  int(1000*(times[t_index]-times[t_index-1])))
    print "calculatePriorities(): %40s took %d msec"%(tasks[-1],  int(1000*(times[-1]-times[0])))

    # foAI.foAIstate.printPriorities()

def calculateFoodPriority():
    "calculates the AI's need for food"
    # attempts to sustain a food stockpile whose size depends on the AI empires population
    # foodStockpile == 0 => returns 100, foodStockpile == foodTarget => returns 0

    empire = fo.getEmpire()
    foodProduction = empire.resourceProduction(fo.resourceType.food)
    foodStockpile = empire.resourceStockpile(fo.resourceType.food)
    #foodTarget = 10 * empire.population() * AIstate.foodStockpileSize
    foodTarget=0

    if (foodTarget == 0):
        return 0

    foodPriority = (foodTarget - foodStockpile) / foodTarget * 115

    print ""
    print "Food Production:        " + str(foodProduction)
    print "Size of Food Stockpile: " + str(foodStockpile)
    print "Target Food Stockpile : " + str (foodTarget)
    print "Priority for Food     : " + str(foodPriority)

    if foodPriority < 0:
        return 0

    return foodPriority

def calculateIndustryPriority(): #currently only used to print status
    "calculates the demand for industry"

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    # get current industry production & Target
    industryProduction = empire.resourceProduction(fo.resourceType.industry)
    ownedPlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    planets = map(universe.getPlanet,  ownedPlanetIDs)
    targetPP = sum( map( lambda x: x.currentMeterValue(fo.meterType.targetIndustry),  planets) )

    industryPriority = foAI.foAIstate.getPriority(EnumsAI.AIPriorityType.PRIORITY_RESOURCE_PRODUCTION)

    print ""
    print "Industry Production (current/target) : ( %.1f / %.1f )  at turn %s"%(industryProduction,  targetPP,  fo.currentTurn())
    print "Priority for Industry: " + str(industryPriority)

    return industryPriority

def calculateResearchPriority():
    "calculates the AI empire's demand for research"

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID

    industryPriority = foAI.foAIstate.getPriority(EnumsAI.AIPriorityType.PRIORITY_RESOURCE_PRODUCTION)

    gotAlgo = empire.getTechStatus("LRN_ALGO_ELEGANCE") == fo.techStatus.complete
    got_quant = empire.getTechStatus("LRN_QUANT_NET") == fo.techStatus.complete
    researchQueueList = ResearchAI.getResearchQueueTechs()
    orbGenTech = "PRO_ORBITAL_GEN"
    got_orb_gen = (empire.getTechStatus("PRO_ORBITAL_GEN") == fo.techStatus.complete)
    got_solar_gen = (empire.getTechStatus("PRO_SOL_ORB_GEN") == fo.techStatus.complete)

    totalPP = empire.productionPoints
    totalRP = empire.resourceProduction(fo.resourceType.research)
    industrySurge=   (foAI.foAIstate.aggression > fo.aggression.cautious) and  ( totalPP <(30*(foAI.foAIstate.aggression))  )  and (orbGenTech  in researchQueueList[:3]  or  empire.getTechStatus(orbGenTech) == fo.techStatus.complete)
    # get current industry production & Target
    ownedPlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    planets = map(universe.getPlanet,  ownedPlanetIDs)
    targetRP = sum( map( lambda x: x.currentMeterValue(fo.meterType.targetResearch),  planets) )

    styleIndex =  empireID%2
    if foAI.foAIstate.aggression >=fo.aggression.maniacal:
        styleIndex+= 1

    cutoffSets =  [ [25, 45, 70,  110  ],  [35,  50,  70,  150  ],  [25,  50,  70,  150  ]    ]
    cutoffs = cutoffSets[styleIndex  ]
    settings = [ [1.4, .7, .5,  .4, .35  ],  [1.6,  1.0,  .8,  .6, .35   ],  [2.0,  1.4,  1.0,  .7, .35   ]    ][styleIndex  ]

    if industrySurge and True:
        researchPriority =  0.2 * industryPriority
    else:
        if  (fo.currentTurn() < cutoffs[0]) or (not gotAlgo) or ((styleIndex ==0) and not got_orb_gen):
            researchPriority = settings[0] * industryPriority # high research at beginning of game to get easy gro tech and to get research booster Algotrithmic Elegance
        elif (not got_orb_gen) or (fo.currentTurn() < cutoffs[1]) :
            researchPriority = settings[1] * industryPriority# med-high research
        elif (fo.currentTurn() < cutoffs[2]):
            researchPriority = settings[2] * industryPriority # med-high  industry
        elif (fo.currentTurn() < cutoffs[3]):
            researchPriority = settings[3] * industryPriority # med-high  industry
        else:
            researchQueue = list(empire.researchQueue)
            researchPriority = settings[4] * industryPriority # high  industry , low research
            if len(researchQueue) == 0 :
                researchPriority = 0 # done with research
            elif len(researchQueue) <5 and researchQueue[-1].allocation > 0 :
                researchPriority =  len(researchQueue) # barely not done with research
            elif len(researchQueue) <10 and researchQueue[-1].allocation > 0 :
                researchPriority = 4+ len(researchQueue) # almost done with research
            elif len(researchQueue) <20 and researchQueue[int(len(researchQueue)/2)].allocation > 0 :
                researchPriority = 0.5 * researchPriority # closing in on end of research
            elif len(researchQueue) <20:
                researchPriority = 0.7*researchPriority # high  industry , low research
                
    if (  ((empire.getTechStatus("SHP_WEAPON_2_4") == fo.techStatus.complete) or
            (empire.getTechStatus("SHP_WEAPON_4_1") == fo.techStatus.complete)) and
            (empire.getTechStatus("PRO_SENTIENT_AUTOMATION") == fo.techStatus.complete) ):
        industry_factor = [ [0.25,  0.2],  [0.3,  0.25],  [0.3,  0.25] ][styleIndex ]
        researchPriority = min(researchPriority,  industry_factor[got_solar_gen]*industryPriority) 
    if got_quant:
        researchPriority = min(researchPriority + 0.1*industryPriority,  researchPriority * 1.3) 
    researchPriority = int(researchPriority)
    print  ""
    print  "Research Production (current/target) : ( %.1f / %.1f )"%(totalRP,  targetRP)
    print  "Priority for Research: %d (new target ~ %d RP)"%(researchPriority,  totalPP * researchPriority/industryPriority)

    return researchPriority

def calculateExplorationPriority():
    "calculates the demand for scouts by unexplored systems"

    global scoutsNeeded

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    numUnexploredSystems =  len( ExplorationAI.borderUnexploredSystemIDs   )  #len(foAI.foAIstate.getExplorableSystems(AIExplorableSystemType.EXPLORABLE_SYSTEM_UNEXPLORED))
    numScouts = sum( [  foAI.foAIstate.fleetStatus.get(fid,  {}).get('nships', 0) for fid in  ExplorationAI.currentScoutFleetIDs] ) #    FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_EXPLORATION)
    productionQueue = empire.productionQueue
    queuedScoutShips=0
    for queue_index  in range(0,  len(productionQueue)):
        element=productionQueue[queue_index]
        if element.buildType == EnumsAI.AIEmpireProductionTypes.BT_SHIP:
            if foAI.foAIstate.getShipRole(element.designID) ==       EnumsAI.AIShipRoleType.SHIP_ROLE_CIVILIAN_EXPLORATION  :
                queuedScoutShips += element.remaining * element.blocksize

    milShips = MilitaryAI.num_milships
    scoutsNeeded = max(0,  min( 4+int(milShips/5),    4+int(fo.currentTurn()/50) ,  2+ numUnexploredSystems**0.5 ) - numScouts - queuedScoutShips )
    explorationPriority = int(40*scoutsNeeded)

    print ""
    print "Number of Scouts            : " + str(numScouts)
    print "Number of Unexplored systems: " + str(numUnexploredSystems)
    print "military size: ",  milShips
    print "Priority for scouts         : " + str(explorationPriority)

    return explorationPriority

def calculateColonisationPriority():
    "calculates the demand for colony ships by colonisable planets"
    global allottedColonyTargets
    totalPP=fo.getEmpire().productionPoints
    colonyCost=120*(1+ 0.06*len( list(AIstate.popCtrIDs) ))
    turnsToBuild=8#TODO: check for susp anim pods, build time 10
    allottedPortion = [0.4,  0.5][ random.choice([0, 1]) ]    #fo.empireID() % 2
    if ( foAI.foAIstate.getPriority(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION) 
            > 2 * foAI.foAIstate.getPriority(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY)):
        allottedPortion *= 1.5
    #allottedColonyTargets = 1+ int(fo.currentTurn()/50)
    allottedColonyTargets = 1 + int( totalPP*turnsToBuild*allottedPortion/colonyCost)

    numColonisablePlanetIDs = len(    [  pid   for (pid,  (score, specName) ) in  foAI.foAIstate.colonisablePlanetIDs if score > 60 ][:allottedColonyTargets+2] )
    if (numColonisablePlanetIDs == 0): return 1

    colonyshipIDs = FleetUtilsAI.getEmpireFleetIDsByRole(EnumsAI.AIFleetMissionType.FLEET_MISSION_COLONISATION)
    numColonyships = len(FleetUtilsAI.extractFleetIDsWithoutMissionTypes(colonyshipIDs))
    colonisationPriority = 121 * (1+numColonisablePlanetIDs - numColonyships) / (numColonisablePlanetIDs+1)

    # print ""
    # print "Number of Colony Ships        : " + str(numColonyships)
    # print "Number of Colonisable planets : " + str(numColonisablePlanetIDs)
    # print "Priority for colony ships     : " + str(colonisationPriority)

    if colonisationPriority < 1: return 1

    return colonisationPriority

def calculateOutpostPriority():
    "calculates the demand for outpost ships by colonisable planets"
    baseOutpostCost=80

    numOutpostPlanetIDs = len(foAI.foAIstate.colonisableOutpostIDs)
    numOutpostPlanetIDs = len(    [  pid   for (pid,  (score, specName) ) in  foAI.foAIstate.colonisableOutpostIDs if score > 1.0*baseOutpostCost/3.0 ][:allottedColonyTargets] )
    completedTechs = ResearchAI.getCompletedTechs()
    if numOutpostPlanetIDs == 0 or not 'CON_ENV_ENCAPSUL' in completedTechs:
        return 0

    outpostShipIDs = FleetUtilsAI.getEmpireFleetIDsByRole(EnumsAI.AIFleetMissionType.FLEET_MISSION_OUTPOST)
    numOutpostShips = len(FleetUtilsAI.extractFleetIDsWithoutMissionTypes(outpostShipIDs))
    outpostPriority = 102 * (numOutpostPlanetIDs - numOutpostShips) / numOutpostPlanetIDs

    # print ""
    # print "Number of Outpost Ships       : " + str(numOutpostShips)
    # print "Number of Colonisable outposts: " + str(numOutpostPlanetIDs)
    print "Priority for outpost ships    : " + str(outpostPriority)

    if outpostPriority < 1: return 1

    return outpostPriority

def calculateInvasionPriority():
    "calculates the demand for troop ships by opponent planets"
    global allottedInvasionTargets
    troopsPerPod=2
    empire=fo.getEmpire()
    multiplier = 1
    
    if len(foAI.foAIstate.colonisablePlanetIDs) > 0:
        bestColonyScore = max( 2,  foAI.foAIstate.colonisablePlanetIDs[0][1][0] )
    else:
        bestColonyScore = 2

    if foAI.foAIstate.aggression==fo.aggression.beginner and fo.currentTurn()<150:
        return 0

    allottedInvasionTargets = 1+ int(fo.currentTurn()/25)
    totalVal = 0
    troopsNeeded = 0
    for pid, pscore, trp in AIstate.invasionTargets[:allottedInvasionTargets]:
        if pscore > bestColonyScore:
            multiplier += 1
            totalVal += 2 * pscore
        else:
            totalVal += pscore
        troopsNeeded += trp+4

    if totalVal == 0:
        return 0
    opponentTroopPods = int(troopsNeeded/troopsPerPod)

    productionQueue = empire.productionQueue
    queuedTroopPods=0
    for queue_index  in range(0,  len(productionQueue)):
        element=productionQueue[queue_index]
        if element.buildType == EnumsAI.AIEmpireProductionTypes.BT_SHIP:
            if foAI.foAIstate.getShipRole(element.designID) in  [ EnumsAI.AIShipRoleType.SHIP_ROLE_MILITARY_INVASION,  EnumsAI.AIShipRoleType.SHIP_ROLE_BASE_INVASION] :
                design = fo.getShipDesign(element.designID)
                queuedTroopPods += element.remaining*element.blocksize * list(design.parts).count("GT_TROOP_POD")
    bestShip,  bestDesign,  buildChoices = ProductionAI.getBestShipInfo( EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_INVASION)
    if bestDesign:
        troopsPerBestShip = troopsPerPod*(  list(bestDesign.parts).count("GT_TROOP_POD") )
    else:
        troopsPerBestShip=troopsPerPod #may actually not have any troopers available, but this num will do for now

    #don't cound troop bases here since if through misplanning cannot be used where made, cannot be redeployed
    #troopFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_INVASION)  + FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION)
    troopFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION)
    numTroopPods =  sum([ FleetUtilsAI.countPartsFleetwide(fleetID,  ["GT_TROOP_POD"]) for fleetID in  troopFleetIDs])
    troopShipsNeeded = math.ceil((opponentTroopPods - (numTroopPods+ queuedTroopPods ))/troopsPerBestShip)

    #invasionPriority = max(  10+ 200*max(0,  troopShipsNeeded ) , int(0.1* totalVal) )
    invasionPriority = multiplier * (30+ 150*max(0,  troopShipsNeeded ))
    if ColonisationAI.colony_status['colonies_under_attack'] ==[]:
        if ColonisationAI.colony_status['colonies_under_threat'] ==[]:
            invasionPriority *= 2.0
        else:
            invasionPriority *= 1.5
        
    if invasionPriority < 0:
        return 0
    if foAI.foAIstate.aggression==fo.aggression.beginner:
        return 0.5* invasionPriority
    else:
        return invasionPriority

def calculateMilitaryPriority():
    "calculates the demand for military ships by military targeted systems"
    global unmetThreat

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    capitalID = PlanetUtilsAI.getCapital()
    if capitalID is not None and capitalID != -1:
        homeworld = universe.getPlanet(capitalID)
    else:
        return 0# no capitol (not even a capitol-in-the-making), means can't produce any ships
        
    have_mod_weaps = ( empire.getTechStatus("SHP_WEAPON_1_4") == fo.techStatus.complete or
                                   empire.getTechStatus("SHP_WEAPON_2_1") == fo.techStatus.complete or
                                   empire.getTechStatus("SHP_WEAPON_4_1") == fo.techStatus.complete )
        
    allottedInvasionTargets = 1+ int(fo.currentTurn()/25)
    targetPlanetIDs =  [pid for pid, pscore, trp in AIstate.invasionTargets[:allottedInvasionTargets] ] + [pid for pid,  pscore in foAI.foAIstate.colonisablePlanetIDs[:allottedColonyTargets]  ] + [pid for pid,  pscore in foAI.foAIstate.colonisableOutpostIDs[:allottedColonyTargets]  ]

    mySystems = set( AIstate.popCtrSystemIDs ).union( AIstate.outpostSystemIDs   )
    targetSystems = set( PlanetUtilsAI.getSystems(targetPlanetIDs)  )

    curShipRating = ProductionAI.curBestMilShipRating()
    cSRR = curShipRating**0.5

    unmetThreat = 0.0
    currentTurn=fo.currentTurn()
    shipsNeeded=0
    defenses_needed = 0
    for sysID in mySystems.union(targetSystems) :
        status=foAI.foAIstate.systemStatus.get( sysID,  {} )
        myRating = status.get('myFleetRating',  0)
        my_defenses = status.get('mydefenses',  {}).get('overall', 0)
        baseMonsterThreat = status.get('monsterThreat', 0)
        #scale monster threat so that in early - mid game big monsters don't over-drive military production
        if currentTurn>200:
            monsterThreat = baseMonsterThreat
        elif currentTurn>100:
            if baseMonsterThreat <2000:
                monsterThreat = baseMonsterThreat
            else:
                monsterThreat = 2000 + (currentTurn/100.0 - 1) *(baseMonsterThreat-2000)
        else:
            monsterThreat = 0
        if sysID in mySystems:
            threatRoot = status.get('fleetThreat', 0)**0.5 + 0.8*status.get('max_neighbor_threat', 0)**0.5 + 0.2*status.get('neighborThreat', 0)**0.5 + monsterThreat**0.5 + status.get('planetThreat', 0)**0.5
        else:
            threatRoot = status.get('fleetThreat', 0)**0.5  + monsterThreat**0.5 + status.get('planetThreat', 0)**0.5
        shipsNeeded += math.ceil(( max(0,   (threatRoot - (myRating**0.5 + my_defenses**0.5)))**2)/curShipRating)

    #militaryPriority = int( 40 + max(0,  75*unmetThreat / curShipRating) )
    militaryPriority = int( 40 + max(0,  75*shipsNeeded) )
    if not have_mod_weaps:
        militaryPriority /= 2
    #print "Calculating Military Priority:  40 + 75 * unmetThreat/curShipRating \n\t  Priority: %d    \t unmetThreat  %.0f        curShipRating: %.0f"%(militaryPriority,  unmetThreat,  curShipRating)
    print "Calculating Military Priority:  40 + 75 * shipsNeeded \n\t  Priority: %d   \t shipsNeeded %d   \t unmetThreat  %.0f        curShipRating: %.0f"%(militaryPriority, shipsNeeded,   unmetThreat,  curShipRating)
    return max( militaryPriority,  0)

def calculateTopProductionQueuePriority():
    "calculates the top production queue priority"

    productionQueuePriorities = {}
    for priorityType in EnumsAI.getAIPriorityProductionTypes():
        productionQueuePriorities[priorityType] = foAI.foAIstate.getPriority(priorityType)

    sortedPriorities = productionQueuePriorities.items()
    sortedPriorities.sort(lambda x,y: cmp(x[1], y[1]), reverse=True)
    topProductionQueuePriority = -1
    for evaluationPair in sortedPriorities:
        if topProductionQueuePriority < 0:
            topProductionQueuePriority = evaluationPair[0]

    return topProductionQueuePriority

def calculateLearningPriority():
    "calculates the demand for techs learning category"

    currentturn = fo.currentTurn()
    if currentturn == 1:
        return 100
    elif currentturn > 1:
        return 0

def calculateGrowthPriority():
    "calculates the demand for techs growth category"

    productionPriority = calculateTopProductionQueuePriority()
    if productionPriority == 8:
        return 70
    elif productionPriority != 8:
        return 0

def calculateTechsProductionPriority():
    "calculates the demand for techs production category"

    productionPriority = calculateTopProductionQueuePriority()
    if productionPriority == 7 or productionPriority == 9:
        return 60
    elif productionPriority != 7 or productionPriority != 9:
        return 0

def calculateConstructionPriority():
    "calculates the demand for techs construction category"

    productionPriority = calculateTopProductionQueuePriority()
    if productionPriority == 6 or productionPriority == 11:
        return 80
    elif productionPriority != 6 or productionPriority != 11:
        return 30

def calculateShipsPriority():
    "calculates the demand for techs ships category"

    productionPriority = calculateTopProductionQueuePriority()
    if productionPriority == 10:
        return 90
    elif productionPriority != 10:
        return 0


