import freeOrionAIInterface as fo
import FreeOrionAI as foAI
from EnumsAI import AIExplorableSystemType, AIPriorityType, getAIPriorityResourceTypes, getAIPriorityProductionTypes, AIFocusType,  AIEmpireProductionTypes
from EnumsAI import AIShipDesignTypes, AIShipRoleType,  AIFleetMissionType,  AIPriorityNames
import PlanetUtilsAI
import AIstate
import FleetUtilsAI
from random import choice
import sys
import traceback
import math

shipTypeMap = dict( zip( [AIPriorityType.PRIORITY_PRODUCTION_EXPLORATION,  AIPriorityType.PRIORITY_PRODUCTION_OUTPOST,  AIPriorityType.PRIORITY_PRODUCTION_COLONISATION,  AIPriorityType.PRIORITY_PRODUCTION_INVASION,  AIPriorityType.PRIORITY_PRODUCTION_MILITARY], 
                                            [AIShipDesignTypes.explorationShip,  AIShipDesignTypes.outpostShip,  AIShipDesignTypes.colonyShip,  AIShipDesignTypes.troopShip,  AIShipDesignTypes.attackShip ] ) )

def curBestMilShipRating():
    bestShip,  bestDesign,  buildChoices = getBestShipInfo( AIPriorityType.PRIORITY_PRODUCTION_MILITARY)
    if bestDesign is None:
        return 0.00001  #  empire cannot currently produce any military ships, don't make zero though, to avoid divide-by-zero
    stats = foAI.foAIstate.getDesignIDStats(bestDesign.id)
    return stats['attack'] * ( stats['structure'] + stats['shields'] )

def checkTroopShips():
    empire = fo.getEmpire()
    troopDesignIDs = [shipDesignID for shipDesignID in empire.allShipDesigns if shipTypeMap.get(AIPriorityType.PRIORITY_PRODUCTION_INVASION,  "nomatch")  in fo.getShipDesign(shipDesignID).name(False) ]
    troopShipNames = [fo.getShipDesign(shipDesignID).name(False) for shipDesignID in troopDesignIDs]
    print "Current Troopship Designs: %s"%troopShipNames
    if len(troopDesignIDs) ==1 : 
        try:
            res=fo.issueCreateShipDesignOrder("SD_TROOP_SHIP_A2",  "Medium Hulled Troopship for economical large quantities of troops",  
                                                                                    "SH_BASIC_MEDIUM",  ["GT_TROOP_POD", "GT_TROOP_POD", "GT_TROOP_POD"],  "",  "fighter")
            print "added  Troopship SD_TROOP_SHIP_A2, with result %d"%res
        except:
            print "Error: exception triggered:  ",  traceback.format_exc()
    if "SD_TROOP_SHIP_A3" not in troopShipNames:
        try:
            res=fo.issueCreateShipDesignOrder("SD_TROOP_SHIP_A3",  "multicell Hulled Troopship for economical large quantities of troops",  
                                                                                    "SH_STATIC_MULTICELLULAR",  ["GT_TROOP_POD",  "GT_TROOP_POD",  "SR_WEAPON_2",  "SH_DEFENSE_GRID", "SH_DEFENSE_GRID"],  "",  "fighter")
            print "added  Troopship SD_TROOP_SHIP_A3, with result %d"%res
        except:
            print "Error: exception triggered:  ",  traceback.format_exc()
    if "SD_TROOP_SHIP_A4" not in troopShipNames:
        try:
            res=fo.issueCreateShipDesignOrder("SD_TROOP_SHIP_A4",  "multicell Hulled Troopship for economical large quantities of troops",  
                                                                                    "SH_STATIC_MULTICELLULAR",  ["GT_TROOP_POD",  "GT_TROOP_POD",  "SR_WEAPON_5",  "GT_TROOP_POD", "SH_DEFLECTOR"],  "",  "fighter")
            print "added  Troopship SD_TROOP_SHIP_A4, with result %d"%res
        except:
            print "Error: exception triggered:  ",  traceback.format_exc()
    bestShip,  bestDesign,  buildChoices = getBestShipInfo( AIPriorityType.PRIORITY_PRODUCTION_INVASION)
    if bestDesign:
        print "Best Troopship buildable is %s"%bestDesign.name(False)
    else:
        print "Troopships apparently unbuildable at present,  ruh-roh"


def checkScouts():
    empire = fo.getEmpire()
    scoutDesignIDs = [shipDesignID for shipDesignID in empire.allShipDesigns if shipTypeMap.get(AIPriorityType.PRIORITY_PRODUCTION_EXPLORATION,  "nomatch")  in fo.getShipDesign(shipDesignID).name(False) ]
    scoutShipNames = [fo.getShipDesign(shipDesignID).name(False) for shipDesignID in scoutDesignIDs]
    print "Current Scout Designs: %s"%scoutShipNames
    if "SD_SCOUT_A1" not in scoutShipNames:
        try:
            res=fo.issueCreateShipDesignOrder("SD_SCOUT_A1",  "multicell Hulled economical scout",  "SH_STATIC_MULTICELLULAR",  
                                                                                    ["DT_OPTICAL_SCANNER",  "SH_DEFENSE_GRID",  "SR_WEAPON_2",  "FU_BASIC_TANK", "FU_BASIC_TANK"],  "",  "fighter")
            print "added  Scout SD_SCOUT_A2, with result %d"%res
        except:
            print "Error: exception triggered:  ",  traceback.format_exc()
    if "SD_SCOUT_A2" not in scoutShipNames:
        try:
            res=fo.issueCreateShipDesignOrder("SD_SCOUT_A2",  "multicell Hulled economical scout",  "SH_STATIC_MULTICELLULAR",  
                                                                                    ["DT_OPTICAL_SCANNER",  "SH_DEFENSE_GRID",  "SR_WEAPON_2",  "FU_BASIC_TANK", "ST_EM_DAMPER"],  "",  "fighter")
            print "added  Scout SD_SCOUT_A2, with result %d"%res
        except:
            print "Error: exception triggered:  ",  traceback.format_exc()
    if "SD_SCOUT_B1" not in scoutShipNames:
        try:
            res=fo.issueCreateShipDesignOrder("SD_SCOUT_B1",  "multicell Hulled economical scout",  "SH_STATIC_MULTICELLULAR",  
                                                                                    ["DT_ACTIVE_RADAR",  "SH_DEFLECTOR",  "SR_WEAPON_5",  "FU_BASIC_TANK", "FU_BASIC_TANK"],  "",  "fighter")
            print "added  Scout SD_SCOUT_A2, with result %d"%res
        except:
            print "Error: exception triggered:  ",  traceback.format_exc()
    if "SD_SCOUT_B2" not in scoutShipNames:
        try:
            res=fo.issueCreateShipDesignOrder("SD_SCOUT_B2",  "multicell Hulled economical scout",  "SH_STATIC_MULTICELLULAR",  
                                                                                    ["DT_ACTIVE_RADAR",  "SH_DEFLECTOR",  "SR_WEAPON_5",  "FU_BASIC_TANK", "ST_EM_DAMPER"],  "",  "fighter")
            print "added  Scout SD_SCOUT_A2, with result %d"%res
        except:
            print "Error: exception triggered:  ",  traceback.format_exc()
    bestShip,  bestDesign,  buildChoices = getBestShipInfo( AIPriorityType.PRIORITY_PRODUCTION_EXPLORATION)
    if bestDesign:
        print "Best Scout buildable is %s"%bestDesign.name(False)
    else:
        print "Scouts apparently unbuildable at present,  ruh-roh"
    #TODO: add more advanced designs

    
def getBestShipInfo(priority):
    "returns designID,  design,  buildLocList"
    empire = fo.getEmpire()
    theseDesigns = [shipDesign for shipDesign in empire.availableShipDesigns if shipTypeMap.get(priority,  "nomatch")  in fo.getShipDesign(shipDesign).name(False)  and getAvailableBuildLocations(shipDesign) != [] ]
    if theseDesigns == []: 
        return None,  None,  None #must be missing a Shipyard (or checking for outpost ship but missing tech)
    ships = [ ( fo.getShipDesign(shipDesign).name(False),  shipDesign) for shipDesign in theseDesigns ]
    bestShip = sorted( ships)[-1][-1]
    buildChoices = getAvailableBuildLocations(bestShip)
    bestDesign=  fo.getShipDesign(bestShip)
    return bestShip,  bestDesign,  buildChoices

def generateProductionOrders():
    "generate production orders"
    empire = fo.getEmpire()
    universe = fo.getUniverse()
    homeworld = universe.getPlanet(PlanetUtilsAI.getCapital())
    print "Production Queue Management:"
    empire = fo.getEmpire()
    totalPP = empire.productionPoints
    print ""
    print "  Total Available Production Points: " + str(totalPP)
    
    try:    checkScouts()
    except: print "Error: exception triggered:  ",  traceback.format_exc()
    try:    checkTroopShips()
    except: print "Error: exception triggered:  ",  traceback.format_exc()

    movedCapital=False
    if not homeworld:
        print "no capital, should get around to capturing or colonizing a new one"#TODO
    else:
        print "Empire ID %d has current Capital  %s:"%(empire.empireID,  homeworld.name )
        print "Buildings present at empire Capital (ID, Name, Type, Tags, Specials, OwnedbyEmpire):"
        for bldg in homeworld.buildingIDs:
            thisObj=universe.getObject(bldg)
            tags=",".join( thisObj.tags)
            specials=",".join(thisObj.specials)
            print  "%8s | %20s | type:%20s | tags:%20s | specials: %20s | owner:%d "%(bldg,  thisObj.name,  "_".join(thisObj.buildingTypeName.split("_")[-2:])[:20],  tags,  specials,  thisObj.owner )
        
        capitalBldgs = [universe.getObject(bldg).buildingTypeName for bldg in homeworld.buildingIDs]

        possibleBuildingTypeIDs = [bldTID for bldTID in empire.availableBuildingTypes if  fo.getBuildingType(bldTID).canBeProduced(empire.empireID,  homeworld.id)]
        if  possibleBuildingTypeIDs:
            print "Possible building types to build:"
            for buildingTypeID in possibleBuildingTypeIDs:
                buildingType = fo.getBuildingType(buildingTypeID)
                print "buildingType  object:",  buildingType
                print "dir(buildingType): ",  dir(buildingType)
                print "    " + str(buildingType.name)  + " cost: " +str(buildingType.productionCost(empire.empireID,  homeworld.id)) + " time: " + str(buildingType.productionTime(empire.empireID,  homeworld.id))
                
            possibleBuildingTypes = [fo.getBuildingType(buildingTypeID) and  fo.getBuildingType(buildingTypeID).name  for buildingTypeID in possibleBuildingTypeIDs ] #makes sure is not None before getting name

            print ""
            print "Buildings already in Production Queue:"
            productionQueue = empire.productionQueue
            queuedBldgs=[element for element in productionQueue if element.buildType == AIEmpireProductionTypes.BT_BUILDING]
            for bldg in queuedBldgs:
                print "    " + bldg.name + " turns:" + str(bldg.turnsLeft) + " PP:" + str(bldg.allocation)
            if queuedBldgs == []: print "None"
            print
            queuedBldgNames=[ bldg.name for bldg in queuedBldgs ]

            if  ("BLD_INDUSTRY_CENTER" in possibleBuildingTypes) and ("BLD_INDUSTRY_CENTER" not in (capitalBldgs+queuedBldgNames)):
                res=fo.issueEnqueueBuildingProductionOrder("BLD_INDUSTRY_CENTER", empire.capitalID)
                print "Enqueueing BLD_INDUSTRY_CENTER, with result %d"%res

            if  ("BLD_SHIPYARD_BASE" in possibleBuildingTypes) and ("BLD_SHIPYARD_BASE" not in (capitalBldgs+queuedBldgNames)):
                try:
                    res=fo.issueEnqueueBuildingProductionOrder("BLD_SHIPYARD_BASE", empire.capitalID)
                    print "Enqueueing BLD_SHIPYARD_BASE, with result %d"%res
                except:
                    print "Error: cant build shipyard at new capital,  probably no population; we're hosed"
                    print "Error: exception triggered:  ",  traceback.format_exc()

            if  ("BLD_SHIPYARD_ORG_ORB_INC" in possibleBuildingTypes) and ("BLD_SHIPYARD_ORG_ORB_INC" not in (capitalBldgs+queuedBldgNames)):
                try:
                    res=fo.issueEnqueueBuildingProductionOrder("BLD_SHIPYARD_ORG_ORB_INC", empire.capitalID)
                    print "Enqueueing BLD_SHIPYARD_ORG_ORB_INC, with result %d"%res
                except:
                    print "Error: exception triggered:  ",  traceback.format_exc()



            if  ("BLD_EXOBOT_SHIP" in possibleBuildingTypes) and ("BLD_EXOBOT_SHIP" not in capitalBldgs+queuedBldgNames):
                #TODO:
                print "Would Enqueueing BLD_EXOBOT_SHIP if had better handling"
                #fo.issueEnqueueBuildingProductionOrder("BLD_EXOBOT_SHIP", empire.capitalID)

    totalPPSpent = fo.getEmpire().productionQueue.totalSpent
    print "  Total Production Points Spent:     " + str(totalPPSpent)

    wastedPP = totalPP - totalPPSpent
    print "  Wasted Production Points:          " + str(wastedPP)

    print ""
    print "Possible ship designs to build:"
    for shipDesignID in empire.availableShipDesigns:
        shipDesign = fo.getShipDesign(shipDesignID)
        print "    " + str(shipDesign.name(True)) + " cost:" + str(shipDesign.productionCost(empire.empireID,  homeworld.id) )+ " time:" + str(shipDesign.productionTime(empire.empireID,  homeworld.id))

    print ""
    print "Projects already in Production Queue:"
    productionQueue = empire.productionQueue
    print "production summary: %s"%[elem.name for elem in productionQueue]
    queuedColonyShips=0
    
    #TODO:  blocked items might not need dequeuing, but rather for supply lines to be un-blockaded 
    for queue_index  in range( len(productionQueue)):
        element=productionQueue[queue_index]
        print "    " + element.name + " turns:" + str(element.turnsLeft) + " PP:%.2f"%element.allocation + " being built at " + universe.getObject(element.locationID).name
        if element.turnsLeft == -1:
            print "element %s will never be completed as stands  "%element.name 
            #fo.issueDequeueProductionOrder(queue_index) 
            break
        elif element.buildType == AIEmpireProductionTypes.BT_SHIP:
             if foAI.foAIstate.getShipRole(element.designID) ==       AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION:
                 queuedColonyShips +=element.remaining
    if queuedColonyShips:
        print "\nFound %d colony ships in build queue"%queuedColonyShips

    print ""
    # get the highest production priorities
    productionPriorities = {}
    for priorityType in getAIPriorityProductionTypes():
        productionPriorities[priorityType] = foAI.foAIstate.getPriority(priorityType)

    sortedPriorities = productionPriorities.items()
    sortedPriorities.sort(lambda x,y: cmp(x[1], y[1]), reverse=True)

    topPriority = -1
    topscore = -1
    numColonyTargs=len(AIstate.colonisablePlanetIDs )
    numColonyFleets=len( FleetUtilsAI.getEmpireFleetIDsByRole( AIFleetMissionType.FLEET_MISSION_COLONISATION) )# counting existing colony fleets each as one ship
    totColonyFleets = queuedColonyShips + numColonyFleets
    
    print "Production Queue Priorities:"
    filteredPriorities = {}
    for ID,  score in sortedPriorities:
        if topscore < score:
            topPriority = ID
            topscore = score #don't really need topscore nor sorting with current handling
        print " Score: %4d -- %s "%(score,  AIPriorityNames[ID] )
        if ID != AIPriorityType.PRIORITY_PRODUCTION_BUILDINGS:
            if ( ID != AIPriorityType.PRIORITY_PRODUCTION_COLONISATION ) or (  totColonyFleets <  numColonyTargs ):
                filteredPriorities[ID]= score
    if filteredPriorities == {}:
        print "No non-building-production priorities with nonzero score, setting to default: Military"
        filteredPriorities [AIPriorityType.PRIORITY_PRODUCTION_MILITARY ] =  1 
    while (topscore >5000):
        topscore = 0
        for pty in filteredPriorities:
            score = filteredPriorities[pty]
            if score <= 500:
                score = math.ceil(score/50)
            else:
                score = math.ceil(score/100)
            filteredPriorities[pty] = score
            if score > topscore:
                topscore=score

    bestShips={}
    for priority in list(filteredPriorities):
        bestShip,  bestDesign,  buildChoices = getBestShipInfo(priority)
        if bestShip is None:
            del filteredPriorities[priority] #must be missing a shipyard -- TODO build a shipyard if necessary
            continue
        bestShips[priority] = [bestShip,  bestDesign,  buildChoices ]
        
    priorityChoices=[]
    for priority in filteredPriorities:
        priorityChoices.extend( int(filteredPriorities[priority]) * [priority] )

    #print "  Top Production Queue Priority: " + str(topPriority)
    #print "\n ship priority selection list: \n %s \n\n"%str(priorityChoices)
    loopCount = 0
        
    nextIdx = len(productionQueue)
    while (wastedPP > 0) and (loopCount <100) and (priorityChoices != [] ): #make sure don't get stuck in some nonbreaking loop like if all shipyards captured
        loopCount +=1
        print "Beginning  build enqueue loop %d; %.1f PP available"%(loopCount,  wastedPP)
        thisPriority = choice( priorityChoices )
        print "selected priority: ",  AIPriorityNames[thisPriority]
        if ( thisPriority ==  AIPriorityType.PRIORITY_PRODUCTION_COLONISATION ):
            if ( totColonyFleets >=  numColonyTargs + 1):
                print "Already sufficient colony ships in queue,  trying next priority choice"
                print ""
                continue
            else:
                totColonyFleets +=1  # assumes the enqueueing below succeeds, but really no harm if assumption proves wrong
        bestShip,  bestDesign,  buildChoices = bestShips[thisPriority]
        loc = choice(buildChoices)
        numShips=1
        perTurnCost = (float(bestDesign.productionCost(empire.empireID,  homeworld.id)) / bestDesign.productionTime(empire.empireID,  homeworld.id))
        while ( totalPP > 25*perTurnCost):
            numShips *= 5
            perTurnCost *= 5
        retval  = fo.issueEnqueueShipProductionOrder(bestShip, loc)
        if retval !=0:
            print "adding %d new ship(s) to production queue:  %s; per turn production cost %.1f"%(numShips,  bestDesign.name(True),  perTurnCost)
            print ""
            if numShips>1:
                fo.issueChangeProductionQuantityOrder(nextIdx,  1,  numShips)
            wastedPP -=  perTurnCost
            nextIdx+=1
    print ""

def getAvailableBuildLocations(shipDesignID):
    "returns locations where shipDesign can be built"

    result = []

    systemIDs = foAI.foAIstate.getExplorableSystems(AIExplorableSystemType.EXPLORABLE_SYSTEM_EXPLORED)
    planetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(systemIDs)
    shipDesign = fo.getShipDesign(shipDesignID)
    empire = fo.getEmpire()
    empireID = empire.empireID
    for planetID in planetIDs:
        if shipDesign.productionLocationForEmpire(empireID, planetID):
            result.append(planetID)

    return result

def spentPP():
    "calculate PPs spent this turn so far"

    queue = fo.getEmpire().productionQueue
    return queue.totalSpent
