import math

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
import AIDependencies
from freeorion_debug import Timer
from freeorion_tools import tech_is_complete

prioritiees_timer = Timer('calculate_priorities()')

allottedInvasionTargets=0
allottedColonyTargets=0
colonyGrowthBarrier = 2
scoutsNeeded = 0
unmetThreat = 0


def calculate_priorities():
    """calculates the priorities of the AI player"""
    print("checking statuses")
    # Industry, Research, Colony, Invasion, Military

    prioritiees_timer.start('setting Production Priority')
    foAI.foAIstate.set_priority(EnumsAI.AIPriorityType.PRIORITY_RESOURCE_PRODUCTION, 50) # let this one stay fixed & just adjust Research
    prioritiees_timer.start('setting Research Priority')
    foAI.foAIstate.set_priority(EnumsAI.AIPriorityType.PRIORITY_RESOURCE_RESEARCH, calculateResearchPriority()) #TODO: do univ _survey before this
    prioritiees_timer.start('Evaluating Colonization Status')

    ColonisationAI.get_colony_fleets() # sets foAI.foAIstate.colonisablePlanetIDs and foAI.foAIstate.outpostPlanetIDs and many other values used by other modules
    prioritiees_timer.start('Evaluating Invasion Status')
    InvasionAI.get_invasion_fleets() # sets AIstate.invasionFleetIDs, AIstate.opponentPlanetIDs, and AIstate.invasionTargetedPlanetIDs
    prioritiees_timer.start('Evaluating Military Status')
    MilitaryAI.get_military_fleets() # sets AIstate.militaryFleetIDs and AIstate.militaryTargetedSystemIDs
    prioritiees_timer.start('reporting Production Priority')
    print("calculating priorities")
    calculateIndustryPriority()#purely for reporting purposes
    prioritiees_timer.start('setting Exploration Priority')

    foAI.foAIstate.set_priority(EnumsAI.AIPriorityType.PRIORITY_RESOURCE_TRADE, 0)
    foAI.foAIstate.set_priority(EnumsAI.AIPriorityType.PRIORITY_RESOURCE_CONSTRUCTION, 0)

    foAI.foAIstate.set_priority(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_EXPLORATION, calculateExplorationPriority())
    prioritiees_timer.start('setting Colony Priority')
    foAI.foAIstate.set_priority(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION, calculateColonisationPriority())
    prioritiees_timer.start('setting Outpost Priority')
    foAI.foAIstate.set_priority(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_OUTPOST, calculateOutpostPriority())
    prioritiees_timer.start('setting Invasion Priority')
    foAI.foAIstate.set_priority(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_INVASION, calculateInvasionPriority())
    prioritiees_timer.start('setting Military Priority')
    foAI.foAIstate.set_priority(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY, calculateMilitaryPriority())
    prioritiees_timer.start('setting other priorities')
    foAI.foAIstate.set_priority(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_BUILDINGS, 25)

    foAI.foAIstate.set_priority(EnumsAI.AIPriorityType.PRIORITY_RESEARCH_LEARNING, calculateLearningPriority())
    foAI.foAIstate.set_priority(EnumsAI.AIPriorityType.PRIORITY_RESEARCH_GROWTH, calculateGrowthPriority())
    foAI.foAIstate.set_priority(EnumsAI.AIPriorityType.PRIORITY_RESEARCH_PRODUCTION, calculateTechsProductionPriority())
    foAI.foAIstate.set_priority(EnumsAI.AIPriorityType.PRIORITY_RESEARCH_CONSTRUCTION, calculateConstructionPriority())
    foAI.foAIstate.set_priority(EnumsAI.AIPriorityType.PRIORITY_RESEARCH_ECONOMICS, 0)
    foAI.foAIstate.set_priority(EnumsAI.AIPriorityType.PRIORITY_RESEARCH_SHIPS, calculateShipsPriority())
    foAI.foAIstate.set_priority(EnumsAI.AIPriorityType.PRIORITY_RESEARCH_DEFENSE, 0)
    prioritiees_timer.end()

    # foAI.foAIstate.print_priorities()


def calculateIndustryPriority(): #currently only used to print status
    """calculates the demand for industry"""
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    # get current industry production & Target
    industryProduction = empire.resourceProduction(fo.resourceType.industry)
    ownedPlanetIDs = PlanetUtilsAI.get_owned_planets_by_empire(universe.planetIDs)
    planets = map(universe.getPlanet, ownedPlanetIDs)
    targetPP = sum( map( lambda x: x.currentMeterValue(fo.meterType.targetIndustry), planets) )

    # currently, previously set to 50 in calculatePriorities(), this is just for reporting
    industryPriority = foAI.foAIstate.get_priority(EnumsAI.AIPriorityType.PRIORITY_RESOURCE_PRODUCTION)

    print
    print "Industry Production (current/target) : ( %.1f / %.1f ) at turn %s"%(industryProduction, targetPP, fo.currentTurn())
    print "Priority for Industry: " + str(industryPriority)

    return industryPriority


def calculateResearchPriority():
    """calculates the AI empire's demand for research"""
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    current_turn = fo.currentTurn()
    enemies_sighted = foAI.foAIstate.misc.get('enemies_sighted',{})
    recent_enemies = [x for x in enemies_sighted if x > current_turn - 8]

    industryPriority = foAI.foAIstate.get_priority(EnumsAI.AIPriorityType.PRIORITY_RESOURCE_PRODUCTION)

    gotAlgo = tech_is_complete(AIDependencies.LRN_ALGO_ELEGANCE)
    got_quant = tech_is_complete(AIDependencies.LRN_QUANT_NET)
    researchQueueList = ResearchAI.get_research_queue_techs()
    orb_gen_tech = AIDependencies.PRO_ORBITAL_GEN
    got_orb_gen = tech_is_complete(orb_gen_tech)
    mgrav_prod_tech = AIDependencies.PRO_MICROGRAV_MAN
    got_mgrav_prod = tech_is_complete(mgrav_prod_tech)
    got_solar_gen = tech_is_complete(AIDependencies.PRO_SOL_ORB_GEN)
    
    milestone_techs = ["PRO_SENTIENT_AUTOMATION", "LRN_DISTRIB_THOUGHT", "LRN_QUANT_NET", "SHP_WEAPON_2_4", "SHP_WEAPON_3_2", "SHP_WEAPON_4_2"  ]
    milestones_done = [mstone for mstone in milestone_techs if tech_is_complete(mstone)]
    print "Research Milestones accomplished at turn %d: %s"%(current_turn, milestones_done)

    totalPP = empire.productionPoints
    totalRP = empire.resourceProduction(fo.resourceType.research)
    industrySurge = ((foAI.foAIstate.aggression > fo.aggression.cautious) and
                     ((totalPP + 1.6 * totalRP) <(50* foAI.foAIstate.aggression)) and
                     (((orb_gen_tech in researchQueueList[:2] or got_orb_gen) and ColonisationAI.got_gg) or
                      ((mgrav_prod_tech in researchQueueList[:2] or got_mgrav_prod) and ColonisationAI.got_ast)) and
                     (not (len(AIstate.popCtrIDs) >= 12 )))
    # get current industry production & Target
    ownedPlanetIDs = PlanetUtilsAI.get_owned_planets_by_empire(universe.planetIDs)
    planets = map(universe.getPlanet, ownedPlanetIDs)
    targetRP = sum( map( lambda x: x.currentMeterValue(fo.meterType.targetResearch), planets) )
    galaxy_is_sparse = ColonisationAI.galaxy_is_sparse()
    enemies_sighted = foAI.foAIstate.misc.get('enemies_sighted',{})


    styleIndex = empireID%2
    if foAI.foAIstate.aggression >=fo.aggression.maniacal:
        styleIndex+= 1

    cutoffSets = [ [25, 45, 70, 110 ], [30, 45, 70, 150 ], [25, 40, 80, 160 ]    ]
    cutoffs = cutoffSets[styleIndex ]
    settings = [ [1.3, .7, .5, .4, .35 ], [1.4, 0.8, 0.6, 0.5, 0.35 ], [1.4, 0.8, 0.6, 0.5, 0.4 ]    ][styleIndex ]

    if (current_turn < cutoffs[0]) or (not gotAlgo) or ((styleIndex ==0) and not got_orb_gen and (current_turn < cutoffs[1])):
        researchPriority = settings[0] * industryPriority # high research at beginning of game to get easy gro tech and to get research booster Algotrithmic Elegance
    elif (not got_orb_gen) or (current_turn < cutoffs[1]) :
        researchPriority = settings[1] * industryPriority# med-high research
    elif current_turn < cutoffs[2]:
        researchPriority = settings[2] * industryPriority # med-high industry
    elif current_turn < cutoffs[3]:
        researchPriority = settings[3] * industryPriority # med-high industry
    else:
        researchQueue = list(empire.researchQueue)
        researchPriority = settings[4] * industryPriority # high industry , low research
        if len(researchQueue) == 0 :
            researchPriority = 0 # done with research
        elif len(researchQueue) <5 and researchQueue[-1].allocation > 0 :
            researchPriority = len(researchQueue) * 0.01 * industryPriority # barely not done with research
        elif len(researchQueue) <10 and researchQueue[-1].allocation > 0 :
            researchPriority = (4+ 2*len(researchQueue)) * 0.01 * industryPriority  # almost done with research
        elif len(researchQueue) <20 and researchQueue[int(len(researchQueue)/2)].allocation > 0 :
            researchPriority *= 0.7  # closing in on end of research
    if industrySurge:
        if galaxy_is_sparse and not any(enemies_sighted):
            researchPriority *= 0.5
        else:
            researchPriority *= 0.8
                
    if ((tech_is_complete("SHP_WEAPON_2_4") or
         tech_is_complete("SHP_WEAPON_4_1")) and
            tech_is_complete(AIDependencies.PROD_AUTO_NAME)):
        #industry_factor = [ [0.25, 0.2], [0.3, 0.25], [0.3, 0.25] ][styleIndex ]
        #researchPriority = min(researchPriority, industry_factor[got_solar_gen]*industryPriority)
        researchPriority *= 0.9
    if got_quant:
        researchPriority = min(researchPriority + 0.1*industryPriority, researchPriority * 1.3)
    researchPriority = int(researchPriority)
    print ""
    print "Research Production (current/target) : ( %.1f / %.1f )"%(totalRP, targetRP)
    print "Priority for Research: %d (new target ~ %d RP)"%(researchPriority, totalPP * researchPriority/industryPriority)

    if len(enemies_sighted) < (2 + current_turn/20.0): #TODO: adjust for colonisation priority
        researchPriority *= 1.2
    if (current_turn > 20) and (len(recent_enemies) > 3):
        researchPriority *= 0.8
    return researchPriority


def calculateExplorationPriority():
    """calculates the demand for scouts by unexplored systems"""
    global scoutsNeeded

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    numUnexploredSystems = len( ExplorationAI.borderUnexploredSystemIDs )  #len(foAI.foAIstate.get_explorable_systems(AIExplorableSystemType.EXPLORABLE_SYSTEM_UNEXPLORED))
    numScouts = sum( [ foAI.foAIstate.fleetStatus.get(fid, {}).get('nships', 0) for fid in FleetUtilsAI.get_empire_fleet_ids_by_role(
        EnumsAI.AIFleetMissionType.FLEET_MISSION_EXPLORATION)] ) # FleetUtilsAI.get_empire_fleet_ids_by_role(AIFleetMissionType.FLEET_MISSION_EXPLORATION)
    productionQueue = empire.productionQueue
    queuedScoutShips=0
    for queue_index in range(0, len(productionQueue)):
        element=productionQueue[queue_index]
        if element.buildType == EnumsAI.AIEmpireProductionTypes.BT_SHIP:
            if foAI.foAIstate.get_ship_role(element.designID) == EnumsAI.AIShipRoleType.SHIP_ROLE_CIVILIAN_EXPLORATION :
                queuedScoutShips += element.remaining * element.blocksize

    milShips = MilitaryAI.num_milships
    # intent of the following calc is essentially 
    # new_scouts_needed = min(need_cap_A, need_cap_B, base_need) - already_got_or_queued
    # where need_cap_A is to help prevent scouting needs from swamping military needs, and 
    # need_cap_B is to help regulate investment into scouting while the empire is small.
    # These caps could perhaps instead be tied more directly to military priority and 
    # total empire production.
    scoutsNeeded = max(0, min( 4+int(milShips/5), 4+int(fo.currentTurn()/50) , 2+ numUnexploredSystems**0.5 ) - numScouts - queuedScoutShips )
    explorationPriority = int(40*scoutsNeeded)

    print
    print "Number of Scouts : " + str(numScouts)
    print "Number of Unexplored systems: " + str(numUnexploredSystems)
    print "military size: ", milShips
    print "Priority for scouts : " + str(explorationPriority)

    return explorationPriority


def calculateColonisationPriority():
    """calculates the demand for colony ships by colonisable planets"""
    global allottedColonyTargets, colonyGrowthBarrier
    enemies_sighted = foAI.foAIstate.misc.get('enemies_sighted',{})
    galaxy_is_sparse = ColonisationAI.galaxy_is_sparse()
    totalPP=fo.getEmpire().productionPoints
    num_colonies = len( list(AIstate.popCtrIDs) )
    colonyGrowthBarrier = 2 + ((0.5+foAI.foAIstate.aggression)**2)*fo.currentTurn()/50.0 #significant for low aggression, negligible for high aggression
    if num_colonies > colonyGrowthBarrier:
        return 0.0
    colonyCost =  AIDependencies.COLONY_POD_COST * (1 + AIDependencies.COLONY_POD_UPKEEP * num_colonies)
    turnsToBuild=8#TODO: check for susp anim pods, build time 10
    mil_prio = foAI.foAIstate.get_priority(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY)
    allottedPortion = [[[0.6, 0.8],[0.3, 0.4]],[[0.8, 0.9],[0.3, 0.4]]][galaxy_is_sparse][any(enemies_sighted)][fo.empireID() % 2]  #
    #if ( foAI.foAIstate.get_priority(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION)
    # > 2 * foAI.foAIstate.get_priority(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY)):
    # allottedPortion *= 1.5
    if mil_prio < 100:
        allottedPortion *= 2
    elif mil_prio < 200:
        allottedPortion *= 1.5
    elif fo.currentTurn() > 100:
        allottedPortion *= 0.75**(num_colonies/10.0)
    #allottedColonyTargets = 1+ int(fo.currentTurn()/50)
    allottedColonyTargets = 1 + int( totalPP*turnsToBuild*allottedPortion/colonyCost)

    numColonisablePlanetIDs = len( [  pid for (pid, (score, specName) ) in foAI.foAIstate.colonisablePlanetIDs.items() if score > 60 ][:allottedColonyTargets+2] )
    if numColonisablePlanetIDs == 0: return 1

    colonyshipIDs = FleetUtilsAI.get_empire_fleet_ids_by_role(EnumsAI.AIFleetMissionType.FLEET_MISSION_COLONISATION)
    numColonyships = len(FleetUtilsAI.extract_fleet_ids_without_mission_types(colonyshipIDs))
    colonisationPriority = 60 * (1+numColonisablePlanetIDs - numColonyships) / (numColonisablePlanetIDs+1)

    # print
    # print "Number of Colony Ships : " + str(numColonyships)
    # print "Number of Colonisable planets : " + str(numColonisablePlanetIDs)
    # print "Priority for colony ships : " + str(colonisationPriority)

    if colonisationPriority < 1: return 1
    return colonisationPriority


def calculateOutpostPriority():
    """calculates the demand for outpost ships by colonisable planets"""
    baseOutpostCost = AIDependencies.OUTPOST_POD_COST

    numOutpostPlanetIDs = len(foAI.foAIstate.colonisableOutpostIDs)
    numOutpostPlanetIDs = len( [  pid for (pid, (score, specName) ) in foAI.foAIstate.colonisableOutpostIDs.items() if score > 1.0*baseOutpostCost/3.0 ][:allottedColonyTargets] )
    completedTechs = ResearchAI.get_completed_techs()
    if numOutpostPlanetIDs == 0 or not AIDependencies.OUTPOSTING_TECH in completedTechs:
        return 0

    outpostShipIDs = FleetUtilsAI.get_empire_fleet_ids_by_role(EnumsAI.AIFleetMissionType.FLEET_MISSION_OUTPOST)
    numOutpostShips = len(FleetUtilsAI.extract_fleet_ids_without_mission_types(outpostShipIDs))
    outpostPriority = 50 * (numOutpostPlanetIDs - numOutpostShips) / numOutpostPlanetIDs

    # print
    # print "Number of Outpost Ships : " + str(numOutpostShips)
    # print "Number of Colonisable outposts: " + str(numOutpostPlanetIDs)
    print "Priority for outpost ships : " + str(outpostPriority)

    if outpostPriority < 1: return 1

    return outpostPriority


def calculateInvasionPriority():
    """calculates the demand for troop ships by opponent planets"""
    
    global allottedInvasionTargets
    if foAI.foAIstate.aggression <= fo.aggression.turtle:
        return 0
    
    empire = fo.getEmpire()
    enemies_sighted = foAI.foAIstate.misc.get('enemies_sighted', {})
    multiplier = 1
    num_colonies = len(list(AIstate.popCtrIDs))
    if num_colonies > colonyGrowthBarrier:
        return 0.0
    
    if len(foAI.foAIstate.colonisablePlanetIDs) > 0:
        best_colony_score = max(2, foAI.foAIstate.colonisablePlanetIDs.items()[0][1][0])
    else:
        best_colony_score = 2

    if foAI.foAIstate.aggression == fo.aggression.beginner and fo.currentTurn() < 150:
        return 0

    allottedInvasionTargets = 1 + int(fo.currentTurn()/25)
    total_val = 0
    troops_needed = 0
    for pid, pscore, trp in AIstate.invasionTargets[:allottedInvasionTargets]:
        if pscore > best_colony_score:
            multiplier += 1
            total_val += 2 * pscore
        else:
            total_val += pscore
        troops_needed += trp+4  # ToDo: This seems like it could be improved by some dynamic calculation of buffer

    if total_val == 0:
        return 0

    production_queue = empire.productionQueue
    queued_troop_capacity = 0
    for queue_index in range(0, len(production_queue)):
        element = production_queue[queue_index]
        if element.buildType == EnumsAI.AIEmpireProductionTypes.BT_SHIP:
            if foAI.foAIstate.get_ship_role(element.designID) in [EnumsAI.AIShipRoleType.SHIP_ROLE_MILITARY_INVASION,
                                                                  EnumsAI.AIShipRoleType.SHIP_ROLE_BASE_INVASION]:
                design = fo.getShipDesign(element.designID)
                queued_troop_capacity += element.remaining * element.blocksize * design.troopCapacity
    _, best_design, _ = ProductionAI.getBestShipInfo(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_INVASION)
    if best_design:
        troops_per_best_ship = best_design.troopCapacity
    else:
        return 1e-6  # if we can not build troop ships, we don't want to build (non-existing) invasion ships

    # don't count troop bases hereas these cannot be redeployed after misplaning
    # troopFleetIDs = FleetUtilsAI.get_empire_fleet_ids_by_role(AIFleetMissionType.FLEET_MISSION_INVASION)\
    #                 + FleetUtilsAI.get_empire_fleet_ids_by_role(AIFleetMissionType.FLEET_MISSION_ORBITAL_INVASION)
    troop_fleet_ids = FleetUtilsAI.get_empire_fleet_ids_by_role(EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION)
    total_troop_capacity = sum([FleetUtilsAI.count_troops_in_fleet(fid) for fid in troop_fleet_ids])
    troop_ships_needed = \
        math.ceil((troops_needed - (total_troop_capacity+queued_troop_capacity)) / troops_per_best_ship)

    # invasion_priority = max( 10+ 200*max(0, troop_ships_needed ) , int(0.1* total_val) )
    invasion_priority = multiplier * (30 + 150*max(0, troop_ships_needed))
    if not ColonisationAI.colony_status.get('colonies_under_attack', []):
        if not ColonisationAI.colony_status.get('colonies_under_threat', []):
            invasion_priority *= 2.0
        else:
            invasion_priority *= 1.5
    if not enemies_sighted:
        invasion_priority *= 1.5
        
    if invasion_priority < 0:
        return 0
    if foAI.foAIstate.aggression == fo.aggression.beginner:
        return 0.5 * invasion_priority
    else:
        return invasion_priority


def calculateMilitaryPriority():
    """calculates the demand for military ships by military targeted systems"""
    global unmetThreat

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    capitalID = PlanetUtilsAI.get_capital()
    if capitalID is not None and capitalID != -1:
        homeworld = universe.getPlanet(capitalID)
    else:
        return 0# no capitol (not even a capitol-in-the-making), means can't produce any ships
        
    have_l1_weaps = (tech_is_complete("SHP_WEAPON_1_4") or
                     (tech_is_complete("SHP_WEAPON_1_3") and tech_is_complete("SHP_MIL_ROBO_CONT")) or
                     tech_is_complete("SHP_WEAPON_2_1") or
                     tech_is_complete("SHP_WEAPON_4_1"))
    have_l2_weaps = (tech_is_complete("SHP_WEAPON_2_3") or
                     tech_is_complete("SHP_WEAPON_4_1"))
    enemies_sighted = foAI.foAIstate.misc.get('enemies_sighted',{})
        
    allottedInvasionTargets = 1+ int(fo.currentTurn()/25)
    targetPlanetIDs = [pid for pid, pscore, trp in AIstate.invasionTargets[:allottedInvasionTargets] ] + [pid for pid, pscore in foAI.foAIstate.colonisablePlanetIDs.items()[:allottedColonyTargets] ] + [pid for pid, pscore in foAI.foAIstate.colonisableOutpostIDs.items()[:allottedColonyTargets] ]

    mySystems = set( AIstate.popCtrSystemIDs ).union( AIstate.outpostSystemIDs )
    targetSystems = set( PlanetUtilsAI.get_systems(targetPlanetIDs) )

    curShipRating = ProductionAI.cur_best_mil_ship_rating()
    cSRR = curShipRating**0.5

    defense_ships_needed = 0
    currentTurn=fo.currentTurn()
    ships_needed=0
    defense_ships_needed = 0
    ships_needed_allocation = []
    for sysID in mySystems.union(targetSystems) :
        status=foAI.foAIstate.systemStatus.get( sysID, {} )
        myRating = status.get('myFleetRating', 0)
        my_defenses = status.get('mydefenses', {}).get('overall', 0)
        baseMonsterThreat = status.get('monsterThreat', 0)
        #scale monster threat so that in early - mid game big monsters don't over-drive military production
        monsterThreat = baseMonsterThreat
        if currentTurn>200:
            pass
        elif currentTurn>100:
            if baseMonsterThreat >=2000:
                monsterThreat = 2000 + (currentTurn/100.0 - 1) *(baseMonsterThreat-2000)
        elif currentTurn>30:
            if baseMonsterThreat >=2000:
                monsterThreat = 0
        else:
            if baseMonsterThreat >200:
                monsterThreat = 0
        if sysID in mySystems:
            threatRoot = status.get('fleetThreat', 0)**0.5 + 0.8*status.get('max_neighbor_threat', 0)**0.5 + 0.2*status.get('neighborThreat', 0)**0.5 + monsterThreat**0.5 + status.get('planetThreat', 0)**0.5
        else:
            threatRoot = status.get('fleetThreat', 0)**0.5 + monsterThreat**0.5 + status.get('planetThreat', 0)**0.5
        ships_needed_here = math.ceil(( max(0,   (threatRoot - (myRating**0.5 + my_defenses**0.5)))**2)/curShipRating)
        ships_needed += ships_needed_here
        ships_needed_allocation.append((sysID, ships_needed_here))
        if sysID in mySystems:
            defense_ships_needed += ships_needed_here

    scale = (75 + ProductionAI.curBestMilShipCost()) / 2.0
    #militaryPriority = int( 40 + max(0, 75*unmetThreat / curShipRating) )
    part1 = min(1*fo.currentTurn(), 40)
    part2 = max(0, int(75*ships_needed) )
    militaryPriority = part1 + part2
    #militaryPriority = min(1*fo.currentTurn(), 40) + max(0, int(scale*ships_needed))
    if not have_l1_weaps:
        militaryPriority /= 2.0
    elif not (have_l2_weaps and enemies_sighted):
        militaryPriority /= 1.5
    #print "Calculating Military Priority:  40 + 75 * unmetThreat/curShipRating \n\t  Priority: %d    \t unmetThreat  %.0f        curShipRating: %.0f"%(militaryPriority,  unmetThreat,  curShipRating)
    fmt_string = "Calculating Military Priority:  min(t,40) + %d * ships_needed \n\t  Priority: %d  \t ships_needed: %d \t defense_ships_needed: %d \t curShipRating: %.0f \t l1_weaps: %s \t enemies_sighted: %s"
    print fmt_string%(scale, militaryPriority, ships_needed, defense_ships_needed,  curShipRating, have_l1_weaps, enemies_sighted)
    print "Source of milship demand: ", ships_needed_allocation
    
    if foAI.foAIstate.aggression < fo.aggression.typical:
        militaryPriority *= (1.0 + foAI.foAIstate.aggression) / (1.0 + fo.aggression.typical)
    return max( militaryPriority, 0)


def calculateTopProductionQueuePriority():
    """calculates the top production queue priority"""
    productionQueuePriorities = {}
    for priorityType in EnumsAI.get_priority_production_types():
        productionQueuePriorities[priorityType] = foAI.foAIstate.get_priority(priorityType)

    sortedPriorities = productionQueuePriorities.items()
    sortedPriorities.sort(lambda x,y: cmp(x[1], y[1]), reverse=True)
    topProductionQueuePriority = -1
    for evaluationPair in sortedPriorities:
        if topProductionQueuePriority < 0:
            topProductionQueuePriority = evaluationPair[0]

    return topProductionQueuePriority


def calculateLearningPriority():
    """calculates the demand for techs learning category"""
    currentturn = fo.currentTurn()
    if currentturn == 1:
        return 100
    elif currentturn > 1:
        return 0


def calculateGrowthPriority():
    """calculates the demand for techs growth category"""
    productionPriority = calculateTopProductionQueuePriority()
    if productionPriority == 8:
        return 70
    elif productionPriority != 8:
        return 0


def calculateTechsProductionPriority():
    """calculates the demand for techs production category"""
    productionPriority = calculateTopProductionQueuePriority()
    if productionPriority == 7 or productionPriority == 9:
        return 60
    elif productionPriority != 7 or productionPriority != 9:
        return 0


def calculateConstructionPriority():
    """calculates the demand for techs construction category"""
    productionPriority = calculateTopProductionQueuePriority()
    if productionPriority == 6 or productionPriority == 11:
        return 80
    elif productionPriority != 6 or productionPriority != 11:
        return 30


def calculateShipsPriority():
    """calculates the demand for techs ships category"""
    productionPriority = calculateTopProductionQueuePriority()
    if productionPriority == 10:
        return 90
    elif productionPriority != 10:
        return 0
