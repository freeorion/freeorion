import math
import traceback
import random

import freeOrionAIInterface as fo # pylint: disable=import-error


import AIstate
import FleetUtilsAI
import FreeOrionAI as foAI
import PlanetUtilsAI
import PriorityAI
import ColonisationAI
import EnumsAI
import MilitaryAI

bestMilRatingsHistory={}
shipTypeMap = {   EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_EXPLORATION:   EnumsAI.AIShipDesignTypes.explorationShip,
                                        EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_OUTPOST:             EnumsAI.AIShipDesignTypes.outpostShip,
                                        EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_ORBITAL_OUTPOST:             EnumsAI.AIShipDesignTypes.outpostBase,
                                        EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION:  EnumsAI.AIShipDesignTypes.colonyShip,
                                        EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_INVASION:                EnumsAI.AIShipDesignTypes.troopShip,
                                        EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY:                  EnumsAI.AIShipDesignTypes.attackShip,
                                        EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_ORBITAL_DEFENSE:                  EnumsAI.AIShipDesignTypes.defenseBase,
                                        EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_ORBITAL_INVASION:                  EnumsAI.AIShipDesignTypes.troopBase,
                                        }

#TODO: dynamic lookup of hull stats
hullStats = {

             }

doDoubleShields=False

def dictFromMap(this_map):
    return dict(  [  (el.key(),  el.data() ) for el in this_map ] )

#get key routines declared for import by others before completing present imports, to avoid circularity problems
def curBestMilShipRating():
    if (fo.currentTurn()+1) in bestMilRatingsHistory:
        bestMilRatingsHistory.clear()
    if fo.currentTurn() not in bestMilRatingsHistory:
        milBuildChoices = getBestShipRatings()
        if milBuildChoices == []:
            return 0.00001
        top = milBuildChoices[0]
        bestDesignID,  bestDesign,  buildChoices = top[2],  top[3],  [top[0]]
        #bestShip,  bestDesign,  buildChoices = getBestShipInfo( EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY)
        if bestDesign is None:
            return 0.00001  #  empire cannot currently produce any military ships, don't make zero though, to avoid divide-by-zero
        #stats = foAI.foAIstate.getDesignIDStats(bestDesign.id)
        stats = foAI.foAIstate.get_weighted_design_stats(bestDesignID,  ColonisationAI.empireSpeciesByPlanet.get(buildChoices[0],  ''))
        foAI.foAIstate.adjust_stats_vs_enemy(stats)
        bestMilRatingsHistory[ fo.currentTurn() ] = stats['attack'] * stats['structure']
    return bestMilRatingsHistory[ fo.currentTurn() ]

def getBestShipInfo(priority,  loc=None):
    "returns designID,  design,  buildLocList"
    empire = fo.getEmpire()
    empireID = empire.empireID
    capitolID = PlanetUtilsAI.getCapital()
    if loc is None:
        shipyards=set()
        for yardlist in ColonisationAI.empireShipBuilders.values():
            shipyards.update(yardlist)
        shipyards.discard(capitolID)
        planetIDs = [capitolID] + list(shipyards)
    elif isinstance(loc,  list):
        planetIDs=loc
    elif isinstance(loc,  int):
        planetIDs=[loc]
    else: #problem
        return None,  None,  None
    theseDesignIDs = []
    designNameBases= shipTypeMap.get(priority,  ["nomatch"])
    for baseName in designNameBases:
        theseDesignIDs.extend(  [(designNameBases[baseName]+fo.getShipDesign(shipDesign).name(False) ,  shipDesign ) for shipDesign in empire.availableShipDesigns if baseName  in fo.getShipDesign(shipDesign).name(False) ] )
    if theseDesignIDs == []:
        return None,  None,  None #must be missing a Shipyard (or checking for outpost ship but missing tech)
    #ships = [ ( fo.getShipDesign(shipDesign).name(False),  shipDesign) for shipDesign in theseDesignIDs ]

    for _ , shipDesignID in sorted( theseDesignIDs,  reverse=True):
        shipDesign = fo.getShipDesign(shipDesignID)
        validLocs=[]
        for pid in planetIDs:
            if pid == None:
                continue
            if shipDesign.productionLocationForEmpire(empireID, pid):
                validLocs.append(pid)
        if validLocs:
            return shipDesignID,  shipDesign, validLocs
    return None,  None,  None #must be missing a Shipyard or other orbital (or missing tech)

def getBestShipRatings(loc=None):
    "returns list of [partition, pid, designID, design] sublists"
    verbose=False
    priority = EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY
    empire = fo.getEmpire()
    empireID = empire.empireID
    capitolID = PlanetUtilsAI.getCapital()
    if loc is None:
        planetIDs = ColonisationAI.empireShipyards
    elif isinstance(loc,  list):
        planetIDs=set(loc).intersection(ColonisationAI.empireShipyards)
    elif isinstance(loc,  int):
        if loc in ColonisationAI.empireShipyards:
            planetIDs=[loc]
        else:
            return []
    else: #problem
        return []
    theseDesignIDs = []
    designNameBases= shipTypeMap.get(priority,  ["nomatch"])
    for baseName in designNameBases:
        theseDesignIDs.extend(  [(designNameBases[baseName]+fo.getShipDesign(shipDesignID).name(False) ,  shipDesignID ) for shipDesignID in empire.availableShipDesigns if baseName  in fo.getShipDesign(shipDesignID).name(False) ] )
    if theseDesignIDs == []:
        return [] #must be missing a Shipyard (or checking for outpost ship but missing tech)
    #ships = [ ( fo.getShipDesign(shipDesign).name(False),  shipDesign) for shipDesign in theseDesignIDs ]
    locDetail=[]
    theseDesignIDs.sort(reverse=True)
    bestCostRating = 0.0
    if verbose:
        print "getBestShipRatings checking %d designs on planetIDs %s from loc set %s" % (len(theseDesignIDs),  planetIDs, loc)
    for pid in planetIDs:
        localBestCostRating = 0.0
        bestDesignID = -1
        bestDesign = None
        if pid == None: #TODO: is this check still necessary?
            continue
        for _ , shipDesignID in theseDesignIDs:
            designStats = foAI.foAIstate.get_weighted_design_stats(shipDesignID,  ColonisationAI.empireSpeciesByPlanet.get(pid,  ''))
            shipDesign = fo.getShipDesign(shipDesignID)
            if not shipDesign.productionLocationForEmpire(empireID, pid):
                continue
            cost = shipDesign.productionCost(empireID, pid)
            nattacks = sum( designStats.get('attacks', {1:1}).keys() )
            costRating = (designStats['attack'] * (designStats['structure'] + nattacks*designStats['shields']))/(max( 0.1,  cost))  # TODO: improve shield treatment here
            if costRating < 0.1* bestCostRating:
                break
            if costRating > localBestCostRating:
                localBestCostRating = costRating
                bestDesignID = shipDesignID
                bestDesign = shipDesign
                if costRating > bestCostRating:
                    bestCostRating = costRating
        if localBestCostRating > 0.0:
            if verbose:
                print "\t\t adding design id %d (%s) with costRating %.4f at pid %d" % (bestDesignID,  bestDesign.name,  costRating,  pid)
            locDetail.append( [localBestCostRating,  pid,  bestDesignID,  bestDesign] )
    if locDetail == []:
        return []
    locDetail.sort(reverse=True)
    tally = 0
    idx = 0
    for detail in locDetail:
        idx += 1
        if detail[0] < 0.7 * bestCostRating:
            break
        weight = math.exp(10*detail[0]/bestCostRating - 10)
        tally += weight
        detail[0]= tally
    for detail in locDetail:
        detail[0] /= tally
    return locDetail

def addDesigns(shipType,  newDesigns,  shipProdPriority):
    designNameBases= [key for key, val in sorted( shipTypeMap.get(shipProdPriority,  {"nomatch":0}).items(),  key=lambda x:x[1])]
    empire = fo.getEmpire()
    designIDs=[]
    for baseName in designNameBases:
        designIDs.extend(  [shipDesignID for shipDesignID in empire.allShipDesigns if baseName  in fo.getShipDesign(shipDesignID).name(False) ] )
    shipNames = [fo.getShipDesign(shipDesignID).name(False) for shipDesignID in designIDs]
    print "Current %s Designs: %s"%(shipType,  shipNames)

    needsAdding=[ spec for spec in newDesigns if spec[0] not in shipNames   ]   #spec = ( name,  desc,  hull,  partslist,  icon,  model)
    if needsAdding != []:  #needsAdding = [ (name,  desc,  hull,  partslist,  icon,  model), ... ]
        print "--------------"
        print "%s design names apparently needing to be added: %s"%(shipType,  [spec[0] for  spec in needsAdding] )
        print "-------"
        for name,  desc,  hull,  partslist,  icon,  model in needsAdding:
            try:
                res=fo.issueCreateShipDesignOrder( name,  desc,  hull,  partslist,  icon,  model, False)
                print "added  %s Design %s, with result %d"%(shipType,  name,  res)
            except:
                print "Error: exception triggered and caught adding %s %s:  "%(shipType, name),  traceback.format_exc()
        # the following loop is added since the above call into C++ code seems to be the garbage collector from
        # automatically reclaiming these
        while len(needsAdding) > 0:
            design_tup = needsAdding.pop()
            partslist = design_tup[3]
            while len(partslist) > 0:
                this_part = partslist.pop()
                del this_part
            del design_tup
            
    bestShip,  bestDesign,  buildChoices = getBestShipInfo( shipProdPriority)
    if bestDesign:
        print "Best %s  buildable is %s"%(shipType,  bestDesign.name(False))
    else:
        print "%s apparently unbuildable at present,  ruh-roh"%shipType

def addBaseTroopDesigns():
    shipType,  shipProdPriority ="BaseTroopers",  EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_ORBITAL_INVASION
    designNameBases= [key for key, val in sorted( shipTypeMap.get(shipProdPriority,  {"nomatch":0}).items(),  key=lambda x:x[1])]

    newTroopDesigns = []
    desc,  model = "StormTrooper Ship", "fighter"
    tp = "GT_TROOP_POD"
    nb,  hull =  designNameBases[0],   "SH_COLONY_BASE"
    newTroopDesigns += [ (nb,  desc,  hull,  [tp],  "",  model) ]
    addDesigns(shipType,   newTroopDesigns,  shipProdPriority)


def addTroopDesigns():
    addBaseTroopDesigns()
    shipType,  shipProdPriority ="Troopers",  EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_INVASION
    designNameBases= [key for key, val in sorted( shipTypeMap.get(shipProdPriority,  {"nomatch":0}).items(),  key=lambda x:x[1])]
    newTroopDesigns = []
    desc,  model = "Troop Ship", "fighter"
    srb = "SR_WEAPON_1_%1d"
    tp = "GT_TROOP_POD"
    nb,  hull =  designNameBases[1]+"%1d",   "SH_BASIC_MEDIUM"
    newTroopDesigns += [ (nb%(1),  desc,  hull,  3*[tp],  "",  model) ]
    nb,  hull =  designNameBases[1]+"%1d",   "SH_ORGANIC"
    newTroopDesigns += [ (nb%(2),  desc,  hull,  4*[tp],  "",  model) ]
    #newTroopDesigns += [ (nb%(iw),  desc,  hull,  [srb%iw]+ 3*[tp],  "",  model) for iw in [2, 3, 4] ]
    nb,  hull =  designNameBases[1]+"%1d",   "SH_STATIC_MULTICELLULAR"
    #newTroopDesigns += [ (nb%(0) +"0",  desc,  hull,  5*[tp],  "",  model) for iw in [2, 3, 4] ]
    #newTroopDesigns += [ (nb%(iw+3),  desc,  hull,  [srb%iw]+ 4*[tp],  "",  model) for iw in [2, 3, 4] ]

    ar1,  ar2,  ar3 = "AR_STD_PLATE", "AR_ZORTRIUM_PLATE",  "AR_NEUTRONIUM_PLATE"
    arL=[ar1,  ar2,  ar3]
    for ari in [1, 2]: #naming below only works because skipping Lead armor
        nb,  hull =  designNameBases[ari+1]+"%1d-%1d",   "SH_STATIC_MULTICELLULAR"
        #newTroopDesigns += [ (nb%(ari,  iw),  desc,  hull,  [srb%iw,  arL[ari]]+ 3*[tp],  "",  model) for iw in [2, 3, 4] ]
        nb,  hull =  designNameBases[ari+1]+"%1d-%1d",   "SH_ENDOMORPHIC"
        newTroopDesigns += [ (nb%(ari,  0),  desc,  hull,  [arL[ari]]+ 5*[tp],  "",  model) ]
        #newTroopDesigns += [ (nb%(ari,  iw+3),  desc,  hull,  [srb%iw,  arL[ari]]+ 3*[tp],  "",  model) for iw in [2, 3, 4] ]
    addDesigns(shipType,   newTroopDesigns,  shipProdPriority)

def addScoutDesigns():
    shipType,  shipProdPriority ="Scout",  EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_EXPLORATION
    designNameBases= [key for key, val in sorted( shipTypeMap.get(shipProdPriority,  {"nomatch":0}).items(),  key=lambda x:x[1])]
    newScoutDesigns = []
    desc,  model  = "Scout",  "fighter"
    srb = "SR_WEAPON_1_%1d"
    db = "DT_DETECTOR_%1d"
    is1,  is2  = "FU_BASIC_TANK",  "SH_DEFLECTOR"

    nb,  hull =  designNameBases[1]+"-%1d-%1d",   "SH_ORGANIC"
    for d_id in [1, 2, 3, 4]:
        newScoutDesigns += [ (nb%(d_id, 0),  desc,  "SH_BASIC_SMALL",  [ db%d_id],  "",  model)   ]
    nb,  hull =  designNameBases[2]+"-%1d-%1d",   "SH_ENDOMORPHIC"
    #for d_id in [3, 4]:
    #    newScoutDesigns += [ (nb%(d_id, iw),  desc,  hull,  [ db%d_id,  srb%iw, "", "",  "",  ""],  "",  model)    for iw in range(5, 9) ]
    addDesigns(shipType,   newScoutDesigns,  shipProdPriority)

def addOrbitalDefenseDesigns():
    shipType,  shipProdPriority ="OrbitalDefense",  EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_ORBITAL_DEFENSE
    designNameBases= [key for key, val in sorted( shipTypeMap.get(shipProdPriority,  {"nomatch":0}).items(),  key=lambda x:x[1])]
    newDesigns = []
    desc,  hull,  model  = "Orbital Defense Ship",  "SH_COLONY_BASE",  "fighter"
    is1,  is2,  is3 = "SH_DEFENSE_GRID",  "SH_DEFLECTOR",  "SH_MULTISPEC"
    newDesigns += [ (designNameBases[0],  desc,  hull,  [""],  "",  model) ]
    if False:
        if foAI.foAIstate.aggression<=fo.aggression.cautious:
            newDesigns += [ (designNameBases[1],  desc,  hull,  [is1],  "",  model) ]
        newDesigns += [ (designNameBases[2],  desc,  hull,  [is2],  "",  model) ]
        newDesigns += [ (designNameBases[3],  desc,  hull,  [is3],  "",  model) ]

    addDesigns(shipType,   newDesigns,  shipProdPriority)

def addMarkDesigns():
    addOrbitalDefenseDesigns()
    shipType,  shipProdPriority ="Attack Ships",  EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY
    designRankList = sorted( shipTypeMap.get(shipProdPriority,  {"nomatch":0}).items(),  key=lambda x:x[1])
    print "AttackShip DesignRankList: %s"%([x for x in enumerate(designRankList)])
    # [(0, ('SD_MARK', 'A')), (1, ('Lynx', 'B')), (2, ('Griffon', 'C')), (3, ('Wyvern', 'D')), (4, ('Manticore', 'E')),
    #(5, ('Atlas', 'EA')), (6, ('Pele', 'EB')), (7, ('Xena', 'EC')), (8, ('Devil', 'F')), (9, ('Reaver', 'G')), (10, ('Obliterator', 'H'))]
    designNameBases= [key for key, val in designRankList]
    newMarkDesigns = []
    desc,  model  = "military ship", "fighter"
    srb = "SR_WEAPON_1_%1d"
    srb2 = "SR_WEAPON_2_%1d"
    srb3 = "SR_WEAPON_3_%1d"
    srb4 = "SR_WEAPON_4_%1d"
    clk = "ST_CLOAK_%1d"
    db = "DT_DETECTOR_%1d"
    if1  = "FU_BASIC_TANK"
    is1,  is2,  is3,  is4,  is5 = "SH_DEFENSE_GRID",  "SH_DEFLECTOR", "SH_PLASMA",   "SH_MULTISPEC",  "SH_BLACK"
    isList=["", is1, is2, is3, is4, is5]
    ar1,  ar2,  ar3,  ar4,  ar5 = "AR_STD_PLATE", "AR_ZORTRIUM_PLATE", "AR_DIAMOND_PLATE", "AR_XENTRONIUM_PLATE",   "AR_NEUTRONIUM_PLATE"
    arList = ["", ar1, ar2, ar3, ar4, ar5]

    if foAI.foAIstate.aggression in [fo.aggression.beginner, fo.aggression.turtle]:
        maxEM= 8
    elif foAI.foAIstate.aggression ==fo.aggression.cautious:
        maxEM= 12
    else:
        maxEM= 10

    nb,  hull =  designNameBases[1]+"-%1d",   "SH_BASIC_MEDIUM"
    newMarkDesigns += [ (nb%iw,  desc,  hull,  [ srb%iw,  srb%iw,  ""],  "",  model)    for iw in [1, 2, 3, 4] ]
    newMarkDesigns += [ (nb%(iw+4),  desc,  hull,  [ srb2%iw,  srb2%iw,  ""],  "",  model)    for iw in [1, 2, 3, 4] ]

    nb,  hull =  designNameBases[2]+"-1-%1d",   "SH_ORGANIC"
    newMarkDesigns += [ (nb%iw,  desc,  hull,  [ ar1,  ar1, srb%iw,  if1],  "",  model)    for iw in [1, 2, 3, 4] ]
    newMarkDesigns += [ (nb%(iw+4),  desc,  hull,  [ ar1,  ar1, srb2%iw,  if1],  "",  model)    for iw in [1, 2, 3, 4] ]
    nb,  hull =  designNameBases[2]+"-1G-%1d",   "SH_ORGANIC"
    newMarkDesigns += [ (nb%iw,  desc,  hull,  [ ar1,  srb%iw, srb%iw,  is1],  "",  model)    for iw in [1, 2, 3, 4] ]
    newMarkDesigns += [ (nb%(iw+4),  desc,  hull,  [ ar1,  srb2%iw, srb2%iw,  is1],  "",  model)    for iw in [1, 2, 3, 4] ]
    nb,  hull =  designNameBases[2]+"-2-%1d",   "SH_ORGANIC"
    newMarkDesigns += [ (nb%iw,  desc,  hull,  [ ar1,  srb%iw, srb%iw,  is2],  "",  model)    for iw in [4] ]
    newMarkDesigns += [ (nb%(iw+4),  desc,  hull,  [ ar1,  srb2%iw, srb2%iw,  is2],  "",  model)    for iw in [1, 2, 3, 4] ]
    nb,  hull =  designNameBases[2]+"-2z-%1d",   "SH_ORGANIC"
    newMarkDesigns += [ (nb%iw,  desc,  hull,  [ ar2,  srb%iw, srb%iw,  is2],  "",  model)    for iw in [4] ]
    newMarkDesigns += [ (nb%(iw+4),  desc,  hull,  [ ar2,  srb2%iw, srb2%iw,  is2],  "",  model)    for iw in [1, 2, 3, 4] ]
    
    nb,  hull =  designNameBases[2]+"-3ms-%1d",   "SH_ORGANIC"
    newMarkDesigns += [ (nb%iw,  desc,  hull,  3*[ srb%iw]+[is4],  "",  model)    for iw in [4] ]
    newMarkDesigns += [ (nb%(iw+4),  desc,  hull,  3*[ srb2%iw]+[is4],  "",  model)    for iw in [1, 2, 3, 4] ]
        
    #nb,  hull =  designNameBases[2]+"-3-%1d",   "SH_STATIC_MULTICELLULAR"
    #newMarkDesigns += [ (nb%(iw+4),  desc,  hull,  [ ar1,  srb2%iw, srb2%iw,  is2,  if1],  "",  model)    for iw in [2, 3, 4] ]
    #nb,  hull =  designNameBases[2]+"-3z-%1d",   "SH_STATIC_MULTICELLULAR"
    #newMarkDesigns += [ (nb%(iw+4),  desc,  hull,  [ ar2,  srb2%iw, srb2%iw,  is2,  if1],  "",  model)    for iw in [2, 3, 4] ]

    nb,  hull =  designNameBases[3]+"-1-%1x",   "SH_ENDOMORPHIC"
    newMarkDesigns += [ (nb%(iw+4),  desc,  hull,  3*[srb2%iw] + [ar1,  is2, if1],  "",  model)    for iw in [ 4 ] ]
    newMarkDesigns += [ (nb%(iw+8),  desc,  hull,  3*[srb3%iw] + [ ar1, is2, if1],  "",  model)    for iw in [ 2 ] ]
    nb =  designNameBases[3]+"-1b-%1x"
    newMarkDesigns += [ (nb%(iw+4),  desc,  hull,  3*[srb2%iw] + [ ar1,  is4, if1],  "",  model)    for iw in [ 4 ] ]
    newMarkDesigns += [ (nb%(iw+8),  desc,  hull,  3*[srb3%iw] + [ ar1,  is4, if1],  "",  model)    for iw in [ 2 ] ]

    nb =  designNameBases[3]+"-2-%1x"
    newMarkDesigns += [ (nb%(iw+4),  desc,  hull,  3*[srb2%iw]+[ar2]  + [ is2,  if1],  "",  model)    for iw in [4] ]
    newMarkDesigns += [ (nb%(iw+8),  desc,  hull,  3*[srb3%iw]+[ar2]  + [ is2,  if1],  "",  model)    for iw in [2, 3, 4] ]
    nb =  designNameBases[3]+"2b-%1x"
    newMarkDesigns += [ (nb%(iw+8),  desc,  hull,  3*[srb3%iw]+[ar2]  + [ is3,  if1],  "",  model)    for iw in [3, 4] ]
    nb =  designNameBases[3]+"2c-%1x"
    newMarkDesigns += [ (nb%(iw+4),  desc,  hull,  3*[srb2%iw]+[ar2]  + [ is4,  if1],  "",  model)    for iw in [4] ]
    newMarkDesigns += [ (nb%(iw+8),  desc,  hull,  3*[srb3%iw]+[ar2]  + [ is4,  if1],  "",  model)    for iw in [2, 3, 4] ]

    nb =  designNameBases[3]+"-3-%1x"
    newMarkDesigns += [ (nb%(iw+8),  desc,  hull,  3*[srb3%iw]+[ar3] + [ is2,  if1],  "",  model)    for iw in [2, 3, 4] ]
    nb =  designNameBases[3]+"3b-%1x"
    newMarkDesigns += [ (nb%(iw+8),  desc,  hull,  3*[srb3%iw]+[ar3] + [ is3,  if1],  "",  model)    for iw in [ 3, 4] ]
    nb =  designNameBases[3]+"3c-%1x"
    newMarkDesigns += [ (nb%(iw+8),  desc,  hull,  3*[srb3%iw]+[ar3] + [ is4,  if1],  "",  model)    for iw in [2, 3, 4] ]

    nb =  designNameBases[3]+"-4b-%1x"
    newMarkDesigns += [ (nb%(iw+12),  desc,  hull,  3*[srb4%iw]+[ar3] + [ is3,  if1],  "",  model)    for iw in [ 2, 3, 4] ]
    nb =  designNameBases[3]+"4c-%1x"
    newMarkDesigns += [ (nb%(iw+12),  desc,  hull,  3*[srb4%iw]+[ar3] + [ is4,  if1],  "",  model)    for iw in [2, 3, 4] ]

    if foAI.foAIstate.aggression < fo.aggression.typical:  #won't advance past EM hulls
        nb =  designNameBases[3]+"4d-%1x"
        newMarkDesigns += [ (nb%(iw+12),  desc,  hull,  3*[srb4%iw]+[ar4] + [ is3,  if1],  "",  model)    for iw in [2, 3, 4] ]
        nb =  designNameBases[3]+"4e-%1x"
        newMarkDesigns += [ (nb%(iw+12),  desc,  hull,  3*[srb4%iw]+[ar4] + [ is4,  if1],  "",  model)    for iw in [2, 3, 4] ]
        nb =  designNameBases[3]+"4f-%1x"
        newMarkDesigns += [ (nb%(iw+12),  desc,  hull,  3*[srb4%iw]+[ar5] + [ is3,  if1],  "",  model)    for iw in [2, 3, 4] ]
        nb =  designNameBases[3]+"4g-%1x"
        newMarkDesigns += [ (nb%(iw+12),  desc,  hull,  3*[srb4%iw]+[ar5] + [ is4,  if1],  "",  model)    for iw in [2, 3, 4] ]
        nb =  designNameBases[3]+"4h-%1x"
        newMarkDesigns += [ (nb%(iw+12),  desc,  hull,  3*[srb4%iw]+[ar5] + [ is5,  if1],  "",  model)    for iw in [2, 3, 4] ]
    else:
        nb,  hull =  designNameBases[4]+"-%1x-%1x",   "SH_ENDOSYMBIOTIC"
        #newMarkDesigns += [ (nb%(1, iw),  desc,  hull,  4*[srb%iw] + 3*[ is2],  "",  model)    for iw in range(7,  15) ]
        #newMarkDesigns += [ (nb%(2, iw),  desc,  hull,  4*[srb%iw] + 3*[ is3],  "",  model)    for iw in range(7,  15) ]

        #newMarkDesigns += [ (nb%(3, iw),  desc,  hull,  3*[srb%iw]+[ar2] + 3*[ is2],  "",  model)    for iw in range(7,  14) ]
        #newMarkDesigns += [ (nb%(4, iw),  desc,  hull,  3*[srb%iw]+[ar3] + 3*[ is2],  "",  model)    for iw in range(8,  14) ]
        #newMarkDesigns += [ (nb%(5, iw),  desc,  hull,  3*[srb%iw]+[ar2] + 3*[ is3],  "",  model)    for iw in range(7,  14) ]
        #newMarkDesigns += [ (nb%(6, iw),  desc,  hull,  3*[srb%iw]+[ar3] + 3*[ is3],  "",  model)    for iw in range(8,  14) ]

        nb,  hull =  designNameBases[4]+"-%1xb-%1x",   "SH_BIOADAPTIVE"
        newMarkDesigns += [ (nb%(1, iw)      ,  desc,  hull,  2*[srb3%iw]+[ar4] + [ is3,  if1, if1],  "",  model)    for iw in [4] ]
        newMarkDesigns += [ (nb%(1, iw+4),  desc,  hull,  2*[srb4%iw]+[ar4] + [ is3,  if1, if1],  "",  model)    for iw in [2, 3, 4] ]
        newMarkDesigns += [ (nb%(2, iw)      ,  desc,  hull,  2*[srb3%iw]+[ar4] + [ is4,  if1, if1],  "",  model)    for iw in [4] ]
        newMarkDesigns += [ (nb%(2, iw+4),  desc,  hull,  2*[srb4%iw]+[ar4] + [ is4,  if1, if1],  "",  model)    for iw in [2, 3, 4] ]
        newMarkDesigns += [ (nb%(2, iw+8),  desc,  hull,  2*[srb4%iw]+[ar4] + [ is5,  if1, if1],  "",  model)    for iw in [2, 3, 4] ]
        newMarkDesigns += [ (nb%(3, iw)      ,  desc,  hull,  2*[srb3%iw]+[ar5] + [ is3,  if1, if1],  "",  model)    for iw in [4] ]
        newMarkDesigns += [ (nb%(3, iw+4),  desc,  hull,  2*[srb4%iw]+[ar5] + [ is3,  if1, if1],  "",  model)    for iw in [2, 3, 4] ]
        newMarkDesigns += [ (nb%(4, iw)      ,  desc,  hull,  2*[srb3%iw]+[ar5] + [ is4,  if1, if1],  "",  model)    for iw in [4] ]
        newMarkDesigns += [ (nb%(4, iw+4),  desc,  hull,  2*[srb4%iw]+[ar5] + [ is4,  if1, if1],  "",  model)    for iw in [2, 3, 4] ]
        newMarkDesigns += [ (nb%(4, iw+8),  desc,  hull,  2*[srb4%iw]+[ar5] + [ is5,  if1, if1],  "",  model)    for iw in [2, 3, 4] ]

        nb,  hull =  designNameBases[5]+"-%1x-%1x",   "SH_HEAVY_ASTEROID"  #5, 6, 7 = "Atlas":"FA",  "Pele":"FB",  "Xena":"FC"
        newMarkDesigns += [ (nb%(1, iw)      ,  desc,  hull,  [srb%iw]  +4*[""] +[db%1] + [if1,  if1, if1],  "",  model)    for iw in [2, 3, 4] ]
        newMarkDesigns += [ (nb%(2, iw)      ,  desc,  hull,  [srb2%iw]+4*[""] +[db%1] + [if1,  if1, if1],  "",  model)    for iw in [1, 2, 3, 4] ]
        newMarkDesigns += [ (nb%(3, iw)      ,  desc,  hull,  [srb%iw]  +2*[""] +2*[ar2]+[db%2]+ [if1,  if1, if1],  "",  model)    for iw in [3, 4] ]
        newMarkDesigns += [ (nb%(4, iw)      ,  desc,  hull,  [srb2%iw]+2*[""] +2*[ar2]+[db%1]+ [if1,  if1, if1],  "",  model)    for iw in [1, 2, 3, 4] ]
        newMarkDesigns += [ (nb%(5, iw)      ,  desc,  hull,  [srb2%iw]+2*[""] +2*[ar2]+[db%2]+ [if1,  if1, if1],  "",  model)    for iw in [1, 2, 3, 4] ]
        newMarkDesigns += [ (nb%(6, isp )    ,  desc,  hull,  [srb2%4] +2*[""] +2*[ar2]+[db%2]+ [isList[isp],  if1, if1],  "",  model)    for isp in [0, 1, 2] ]
        newMarkDesigns += [ (nb%(7, isp )    ,  desc,  hull,  3*[srb2%4] +2*[ar2] +[db%2]+ [isList[isp],  if1, if1],  "",  model)    for isp in [0, 1, 2,  3] ]
        for iar in [1, 2]:
            newMarkDesigns += [ (nb%(8, 2*iar+idb-2 )    ,  desc,  hull,  3*[srb%4] +2*[arList[iar]] +[db%idb]+ [is4,  if1, if1],  "",  model)    for idb in [1, 2] ]
        newMarkDesigns += [ (nb%(9, iw )    ,  desc,  hull,  3*[srb2%iw] +2*[ar2] +[db%2]+ [is4,  if1, if1],  "",  model)    for iw in [1, 2, 3, 4] ]
        
        nb,  hull =  designNameBases[6]+"-%1x-%1x",   "SH_HEAVY_ASTEROID"  #5, 6, 7 = "Atlas":"FA",  "Pele":"FB",  "Xena":"FC"
        newMarkDesigns += [ (nb%(1, iw)      ,  desc,  hull,  2*[srb4%iw]+3*[ar2]+ [db%2]+ [is2,  if1, if1],  "",  model)    for iw in [1] ]
        newMarkDesigns += [ (nb%(2, iw)      ,  desc,  hull,  3*[srb4%iw]+2*[ar3]+ [db%2]+ [is3,  if1, if1],  "",  model)    for iw in [1] ]
        newMarkDesigns += [ (nb%(3, iw)      ,  desc,  hull,  2*[srb4%iw]+3*[ar2]+ [db%2]+ [is4,  if1, if1],  "",  model)    for iw in [1] ]
        newMarkDesigns += [ (nb%(4, iw)      ,  desc,  hull,  3*[srb4%iw]+2*[ar3]+ [db%2]+ [is4,  if1, if1],  "",  model)    for iw in [1] ]
        newMarkDesigns += [ (nb%(5, iw)      ,  desc,  hull,  [srb3%iw]+3*[""] +[ar2] + [db%2] + [if1,  if1, if1],  "",  model)    for iw in [3, 4] ]
        newMarkDesigns += [ (nb%(6, iw)      ,  desc,  hull,  [srb3%iw]+2*[""] +2*[ar3]+ [db%2]+ [if1,  if1, if1],  "",  model)    for iw in [3, 4] ]
        newMarkDesigns += [ (nb%(7, iw)      ,  desc,  hull,  3*[srb3%iw]+2*[ar3]+ [db%2]+ [is3,  if1, if1],  "",  model)    for iw in [3, 4] ]
        newMarkDesigns += [ (nb%(8, iw)      ,  desc,  hull,  4*[srb3%iw]+1*[ar3]+ [db%2]+ [is4,  if1, if1],  "",  model)    for iw in [3, 4] ]
        newMarkDesigns += [ (nb%(9, iw)      ,  desc,  hull,  4*[srb3%iw]+1*[ar3]+ [db%3]+ [is4,  if1, if1],  "",  model)    for iw in [3, 4] ]
        newMarkDesigns += [ (nb%(10, iw)      ,  desc,  hull,  4*[srb3%iw]+1*[ar4]+ [db%2]+ [is4,  if1, if1],  "",  model)    for iw in [3, 4] ]
        newMarkDesigns += [ (nb%(11, iw)      ,  desc,  hull,  4*[srb3%iw]+1*[ar4]+ [db%3]+ [is4,  if1, if1],  "",  model)    for iw in [3, 4] ]
        
        nb,  hull =  designNameBases[7]+"-%1x-%1x",   "SH_HEAVY_ASTEROID"  #5, 6, 7 = "Atlas":"FA",  "Pele":"FB",  "Xena":"FC"
        newMarkDesigns += [ (nb%(1, iw)      ,  desc,  hull,  4*[srb4%iw]+1*[ar3]+ [db%3]+ [is3,  if1, if1],  "",  model)    for iw in [2, 3, 4] ]
        newMarkDesigns += [ (nb%(2, iw)      ,  desc,  hull,  4*[srb4%iw]+1*[ar4]+ [db%3]+ [is3,  if1, if1],  "",  model)    for iw in [2, 3, 4] ]
        newMarkDesigns += [ (nb%(3, iw)      ,  desc,  hull,  4*[srb4%iw]+1*[ar3]+ [db%3]+ [is4,  if1, if1],  "",  model)    for iw in [2, 3, 4] ]
        newMarkDesigns += [ (nb%(4, iw)      ,  desc,  hull,  4*[srb4%iw]+1*[ar4]+ [db%3]+ [is4,  if1, if1],  "",  model)    for iw in [2, 3, 4] ]
        newMarkDesigns += [ (nb%(5, iw)      ,  desc,  hull,  4*[srb4%iw]+1*[ar3]+ [db%3]+ [is5,  if1, if1],  "",  model)    for iw in [2, 3, 4] ]
        newMarkDesigns += [ (nb%(6, iw)    ,  desc,  hull,  4*[srb4%iw]+1*[ar4]+ [db%3]+ [is5,  if1, if1],  "",  model)    for iw in [2, 3, 4] ]

        if False and foAI.foAIstate.aggression >fo.aggression.typical:
            hull =  "SH_SENTIENT"
            for cld in [1, 2, 3]:
                nb =  designNameBases[8]+"-%%1xa%1d-%%1x"%cld
                this_cloak = [ "if1", clk%cld ][ cld > 1 ] # fuel or a cloak
                for isd in [3, 4, 5]:
                    newMarkDesigns += [ (nb%(isd-2, 0),    desc,  hull,  4*[srb4%iw]+2*[ar4] + [ isList[isd],  if1, this_cloak],  "",  model)    for iw in [1] ]
                    newMarkDesigns += [ (nb%(isd-2, 1),    desc,  hull,  4*[srb3%iw]+2*[ar4] + [ isList[isd],  if1, this_cloak],  "",  model)    for iw in [3, 4] ]
                    newMarkDesigns += [ (nb%(isd-2, iw),  desc,  hull,  4*[srb4%iw]+2*[ar4] + [ isList[isd],  if1,  this_cloak],  "",  model)    for iw in [2, 3, 4] ]

                nb =  designNameBases[8]+"-%%1xb%1d-%%1x"%cld
                for isd in [3, 4, 5]:
                    newMarkDesigns += [ (nb%(isd-2, 0),    desc,  hull,  4*[srb4%iw]+2*[ar5] + [ isList[isd],  if1,  this_cloak],  "",  model)    for iw in [1] ]
                    newMarkDesigns += [ (nb%(isd-2, 1),    desc,  hull,  4*[srb3%iw]+2*[ar5] + [ isList[isd],  if1,  this_cloak],  "",  model)    for iw in [4] ]
                    newMarkDesigns += [ (nb%(isd-2, iw),  desc,  hull,  4*[srb4%iw]+2*[ar5] + [ isList[isd],  if1,  this_cloak],  "",  model)    for iw in [2, 3, 4] ]


        if foAI.foAIstate.aggression >fo.aggression.typical:
            nb,  hull =  designNameBases[9]+"-%1x-%02x",   "SH_FRACTAL_ENERGY"
            #newMarkDesigns += [ (nb%(1, iw),  desc,  hull,  8*[srb%iw]+3*[ar2] + 3*[ is2],  "",  model)    for iw in range(10,  18) ]

    addDesigns(shipType,   newMarkDesigns,  shipProdPriority)
    #TODO: add more advanced designs

def addOutpostDesigns():
    shipType,  shipProdPriority ="Outpost Ships",  EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_OUTPOST
    designNameBases= [key for key, val in sorted( shipTypeMap.get(shipProdPriority,  {"nomatch":0}).items(),  key=lambda x:x[1])]
    newOutpostDesigns = []
    desc = "Outpost Ship"
    srb = "SR_WEAPON_1_%1d"
    model = "seed"
    nb,  hull =  designNameBases[1]+"%1d_%1d",   "SH_ORGANIC"
    op = "CO_OUTPOST_POD"
    db = "DT_DETECTOR_%1d"
    is1,  is2 = "FU_BASIC_TANK",  "ST_CLOAK_1"
    for p_id in [1, 2]:
        newOutpostDesigns += [ (nb%(p_id, iw),  desc,  hull,  [ srb%iw, db%p_id, "",  op],  "",  model)    for iw in [2, 3, 4] ]
    addDesigns(shipType,   newOutpostDesigns,  shipProdPriority)

def addColonyDesigns():
    shipType,  shipProdPriority ="Colony Ships",  EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION
    designNameBases= [key for key, val in sorted( shipTypeMap.get(shipProdPriority,  {"nomatch":0}).items(),  key=lambda x:x[1])]
    newColonyDesigns = []
    desc,  model = "Colony Ship",  "seed"
    srb = "SR_WEAPON_1_%1d"
    nb,  hull =  designNameBases[1]+"%1d_%1d",   "SH_ORGANIC"
    cp,  cp2 = "CO_COLONY_POD",  "CO_SUSPEND_ANIM_POD"
    db = "DT_DETECTOR_%1d"
    is1,  is2 = "FU_BASIC_TANK",  "ST_CLOAK_1"
    ar1,  ar2,  ar3,  ar4,  ar5 = "AR_STD_PLATE", "AR_ZORTRIUM_PLATE", "AR_DIAMOND_PLATE", "AR_XENTRONIUM_PLATE",   "AR_NEUTRONIUM_PLATE"
    for p_id in [1, 2, 3]:
        newColonyDesigns += [ (nb%(p_id, iw)+"S",  desc,  hull,  [ srb%iw, ar1, db%p_id,  cp],  "",  model)    for iw in [1, 2, 3, 4] ]
        newColonyDesigns += [ (nb%(p_id, iw)+"Z",  desc,  hull,  [ srb%iw, ar2, db%p_id,  cp],  "",  model)    for iw in [1, 2, 3, 4] ]

    nb =  designNameBases[2]+"%1d_%1dZ"
    for p_id in [1, 2, 3]:
        newColonyDesigns += [ (nb%(p_id, iw),  desc,  hull,  [ srb%iw, db%p_id, ar2,  cp2],  "",  model)    for iw in [3.4]  ]
    addDesigns(shipType,   newColonyDesigns,  shipProdPriority)

def generateProductionOrders():
    "generate production orders"
    # first check ship designs
    # next check for buildings etc that could be placed on queue regardless of locally available PP
    # next loop over resource groups, adding buildings & ships
    empire = fo.getEmpire()
    universe = fo.getUniverse()
    capitolID = PlanetUtilsAI.getCapital()
    if capitolID is None or capitolID == -1:
        homeworld=None
        capitolSysID=None
    else:
        homeworld = universe.getPlanet(capitolID)
        capitolSysID = homeworld.systemID
    print "Production Queue Management:"
    empire = fo.getEmpire()
    productionQueue = empire.productionQueue
    totalPP = empire.productionPoints
    #prodResPool = empire.getResourcePool(fo.resourceType.industry)
    #availablePP = dictFromMap( productionQueue.availablePP(prodResPool))
    #allocatedPP = dictFromMap(productionQueue.allocatedPP)
    #objectsWithWastedPP = productionQueue.objectsWithWastedPP(prodResPool)

    currentTurn = fo.currentTurn()
    print ""
    print "  Total Available Production Points: " + str(totalPP)

    if empire.productionPoints <100:
        backupFactor = 0.0
    else:
        backupFactor = min(1.0,  (empire.productionPoints/200.0)**2 )

    claimedStars= foAI.foAIstate.misc.get('claimedStars',  {} )
    if claimedStars == {}:
        for sType in AIstate.empireStars:
            claimedStars[sType] = list( AIstate.empireStars[sType] )
        for sysID in set( AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs):
            tSys = universe.getSystem(sysID)
            if not tSys: continue
            claimedStars.setdefault( tSys.starType, []).append(sysID)


    try:    addScoutDesigns()
    except: print "Error: exception triggered and caught:  ",  traceback.format_exc()
    try:    addTroopDesigns()
    except: print "Error: exception triggered and caught:  ",  traceback.format_exc()
    try:    addMarkDesigns()
    except: print "Error: exception triggered and caught:  ",  traceback.format_exc()
    try:    addColonyDesigns()
    except: print "Error: exception triggered and caught:  ",  traceback.format_exc()
    try:    addOutpostDesigns()
    except: print "Error: exception triggered and caught:  ",  traceback.format_exc()

    if (currentTurn==1) and (productionQueue.totalSpent < totalPP):
        bestDesignID,  bestDesign,  buildChoices = getBestShipInfo(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_EXPLORATION)
        if len(AIstate.opponentPlanetIDs) == 0:
            init_scouts = 4
        else:
            init_scouts = 0
        if bestDesignID:
            for scout_count in range(init_scouts):
                retval  = fo.issueEnqueueShipProductionOrder(bestDesignID, buildChoices[0])
        fo.updateProductionQueue()

    movedCapital=False
    bldgExpense=0.0
    bldgRatio = [ 0.4,  0.35,  0.30  ][fo.empireID()%3]
    print "Buildings present on all owned planets:"
    for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
        planet = universe.getPlanet(pid)
        if planet:
            print "%30s: %s"%(planet.name,  [universe.getObject(bldg).name for bldg in planet.buildingIDs])
    print

    if not homeworld:
        print "if no capitol, no place to build, should get around to capturing or colonizing a new one"#TODO
    else:
        print "Empire ID %d has current Capital  %s:"%(empire.empireID,  homeworld.name )
        print "Buildings present at empire Capital (ID, Name, Type, Tags, Specials, OwnedbyEmpire):"
        for bldg in homeworld.buildingIDs:
            thisObj=universe.getObject(bldg)
            tags=",".join( thisObj.tags)
            specials=",".join(thisObj.specials)
            print  "%8s | %20s | type:%20s | tags:%20s | specials: %20s | owner:%d "%(bldg,  thisObj.name,  "_".join(thisObj.buildingTypeName.split("_")[-2:])[:20],  tags,  specials,  thisObj.owner )
        print 
        capitalBldgs = [universe.getObject(bldg).buildingTypeName for bldg in homeworld.buildingIDs]

        #possibleBuildingTypeIDs = [bldTID for bldTID in empire.availableBuildingTypes if   fo.getBuildingType(bldTID).canBeProduced(empire.empireID,  homeworld.id)]
        possibleBuildingTypeIDs = []
        for bldTID in empire.availableBuildingTypes:
            try:
                if   fo.getBuildingType(bldTID).canBeProduced(empire.empireID,  homeworld.id):
                    possibleBuildingTypeIDs.append(bldTID)
            except:
                if fo.getBuildingType(bldTID) == None:
                    print "For empire %d,  'available Building Type ID' %s  returns Nonetype from fo.getBuildingType(bldTID)"%(empire.empireID,  bldTID)
                else:
                    print "For empire %d,  problem getting BuildingTypeID for  'available Building Type ID' %s"%(empire.empireID,  bldTID)
        if  possibleBuildingTypeIDs:
            print "Possible building types to build:"
            for buildingTypeID in possibleBuildingTypeIDs:
                buildingType = fo.getBuildingType(buildingTypeID)
                #print "buildingType  object:",  buildingType
                #print "dir(buildingType): ",  dir(buildingType)
                print "    " + str(buildingType.name)  + " cost: " +str(buildingType.productionCost(empire.empireID,  homeworld.id)) + " time: " + str(buildingType.productionTime(empire.empireID,  homeworld.id))

            possibleBuildingTypes = [fo.getBuildingType(buildingTypeID) and  fo.getBuildingType(buildingTypeID).name  for buildingTypeID in possibleBuildingTypeIDs ] #makes sure is not None before getting name

            print ""
            print "Buildings already in Production Queue:"
            capitolQueuedBldgs=[]
            for element in [e for e in productionQueue if (e.buildType == EnumsAI.AIEmpireProductionTypes.BT_BUILDING)]:
                bldgExpense += element.allocation
                if ( element.locationID==homeworld.id):
                    capitolQueuedBldgs.append ( element )
            for bldg in capitolQueuedBldgs:
                print "    " + bldg.name + " turns:" + str(bldg.turnsLeft) + " PP:" + str(bldg.allocation)
            if capitolQueuedBldgs == []: print "None"
            print
            queuedBldgNames=[ bldg.name for bldg in capitolQueuedBldgs ]

            if ( totalPP >40 or currentTurn > 40 ) and ("BLD_INDUSTRY_CENTER" in possibleBuildingTypes) and ("BLD_INDUSTRY_CENTER" not in (capitalBldgs+queuedBldgNames)) and (bldgExpense<bldgRatio*totalPP):
                res=fo.issueEnqueueBuildingProductionOrder("BLD_INDUSTRY_CENTER", homeworld.id)
                print "Enqueueing BLD_INDUSTRY_CENTER, with result %d"%res
                if res:
                    cost,  time =   empire.productionCostAndTime( productionQueue[productionQueue.size -1]  )
                    bldgExpense += cost/time

            if  ("BLD_SHIPYARD_BASE" in possibleBuildingTypes) and ("BLD_SHIPYARD_BASE" not in (capitalBldgs+queuedBldgNames)):
                try:
                    res=fo.issueEnqueueBuildingProductionOrder("BLD_SHIPYARD_BASE", homeworld.id)
                    print "Enqueueing BLD_SHIPYARD_BASE, with result %d"%res
                except:
                    print "Error: cant build shipyard at new capital,  probably no population; we're hosed"
                    print "Error: exception triggered and caught:  ",  traceback.format_exc()

            for bldName in [ "BLD_SHIPYARD_ORG_ORB_INC" ]:
                if  (bldName in possibleBuildingTypes) and (bldName not in (capitalBldgs+queuedBldgNames)) and (bldgExpense<bldgRatio*totalPP):
                    try:
                        res=fo.issueEnqueueBuildingProductionOrder(bldName, homeworld.id)
                        print "Enqueueing %s at capitol, with result %d"%(bldName,  res)
                        if res:
                            cost,  time =   empire.productionCostAndTime( productionQueue[productionQueue.size -1]  )
                            bldgExpense += cost/time
                            res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                            print "Requeueing %s to front of build queue, with result %d"%(bldName,  res)
                    except:
                        print "Error: exception triggered and caught:  ",  traceback.format_exc()

            for bldName in [ "BLD_SHIPYARD_ORG_XENO_FAC",  "BLD_SHIPYARD_ORG_CELL_GRO_CHAMB"   ]:
                if  ( totalPP >30 or currentTurn > 30 ) and (bldName in possibleBuildingTypes) and (bldName not in (capitalBldgs+queuedBldgNames)) and (bldgExpense<bldgRatio*totalPP):
                    try:
                        res=fo.issueEnqueueBuildingProductionOrder(bldName, homeworld.id)
                        print "Enqueueing %s at capitol, with result %d"%(bldName,  res)
                        if res:
                            cost,  time =   empire.productionCostAndTime( productionQueue[productionQueue.size -1]  )
                            bldgExpense += cost/time
                            res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                            print "Requeueing %s to front of build queue, with result %d"%(bldName,  res)
                    except:
                        print "Error: exception triggered and caught:  ",  traceback.format_exc()

            numExobotShips=0 #TODO: do real calc here
            if  ("BLD_EXOBOT_SHIP" in possibleBuildingTypes) and ("BLD_EXOBOT_SHIP" not in queuedBldgNames):
                if  len( ColonisationAI.empireColonizers.get("SP_EXOBOT", []))==0  or numExobotShips==0: #don't have an exobot shipyard yet
                    try:
                        res=fo.issueEnqueueBuildingProductionOrder("BLD_EXOBOT_SHIP", homeworld.id)
                        print "Enqueueing BLD_EXOBOT_SHIP, with result %d"%res
                        if res:
                            res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                            print "Requeueing %s to front of build queue, with result %d"%("BLD_EXOBOT_SHIP",  res)
                    except:
                        print "Error: exception triggered and caught:  ",  traceback.format_exc()

            if  ("BLD_IMPERIAL_PALACE" in possibleBuildingTypes) and ("BLD_IMPERIAL_PALACE" not in (capitalBldgs+queuedBldgNames)):
                res=fo.issueEnqueueBuildingProductionOrder("BLD_IMPERIAL_PALACE", homeworld.id)
                print "Enqueueing BLD_IMPERIAL_PALACE at %s, with result %d"%(homeworld.name,  res)
                if res:
                    res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                    print "Requeueing BLD_IMPERIAL_PALACE to front of build queue, with result %d"%res


            # ok, BLD_NEUTRONIUM_SYNTH is not currently unlockable, but just in case...   ;-p
            if  ("BLD_NEUTRONIUM_SYNTH" in possibleBuildingTypes) and ("BLD_NEUTRONIUM_SYNTH" not in (capitalBldgs+queuedBldgNames)):
                res=fo.issueEnqueueBuildingProductionOrder("BLD_NEUTRONIUM_SYNTH", homeworld.id)
                print "Enqueueing BLD_NEUTRONIUM_SYNTH, with result %d"%res
                if res:
                    res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                    print "Requeueing BLD_NEUTRONIUM_SYNTH to front of build queue, with result %d"%res


#TODO: add totalPP checks below, so don't overload queue

    maxDefensePortion = [0.7,  0.4,  0.3,  0.2,  0.1,  0.0  ][ foAI.foAIstate.aggression]
    aggrIndex=max(1, foAI.foAIstate.aggression)
    if ( (currentTurn %( aggrIndex))==0) and foAI.foAIstate.aggression < fo.aggression.maniacal:
        sysOrbitalDefenses={}
        queuedDefenses={}
        defenseAllocation=0.0
        targetOrbitals=  min(  int( ((currentTurn+4)/( 8.0*(aggrIndex)**1.5))**0.8) ,  fo.aggression.maniacal - aggrIndex )
        print "Orbital Defense Check -- target Defense Orbitals: ",  targetOrbitals
        for element in productionQueue:
            if ( element.buildType == EnumsAI.AIEmpireProductionTypes.BT_SHIP) and (foAI.foAIstate.getShipRole(element.designID) ==  EnumsAI.AIShipRoleType.SHIP_ROLE_BASE_DEFENSE):
                bldPlanet = universe.getPlanet(element.locationID)
                if not bldPlanet:
                    print "Error: Problem getting Planet for build loc %s"%element.locationID
                    continue
                sysID = bldPlanet.systemID
                queuedDefenses[sysID] = queuedDefenses.get( sysID,  0) + element.blocksize*element.remaining
                defenseAllocation += element.allocation
        print "Queued Defenses:",  [( PlanetUtilsAI.sysNameIDs([sysID]),  num) for sysID,  num in   queuedDefenses.items()]
        for sysID in ColonisationAI.empireSpeciesSystems:
            if foAI.foAIstate.systemStatus.get(sysID,  {}).get('fleetThreat',  1) > 0:
                continue#don't build  orbital shields if enemy fleet present
            if defenseAllocation > maxDefensePortion * totalPP:
                break
            #print "checking ",  PlanetUtilsAI.sysNameIDs([sysID])
            sysOrbitalDefenses[sysID]=0
            fleetsHere = foAI.foAIstate.systemStatus.get(sysID,  {}).get('myfleets',  [])
            for fid in fleetsHere:
                if foAI.foAIstate.getFleetRole(fid)==EnumsAI.AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE:
                    print "Found %d existing Orbital Defenses in %s :"%(foAI.foAIstate.fleetStatus.get(fid,  {}).get('nships', 0),   PlanetUtilsAI.sysNameIDs([sysID]))
                    sysOrbitalDefenses[sysID] += foAI.foAIstate.fleetStatus.get(fid,  {}).get('nships', 0)
            for pid in ColonisationAI.empireSpeciesSystems.get(sysID, {}).get('pids',  []):
                sysOrbitalDefenses[sysID] += queuedDefenses.get(pid,  0)
            if sysOrbitalDefenses[sysID] < targetOrbitals:
                numNeeded =  targetOrbitals - sysOrbitalDefenses[sysID]
                for pid in ColonisationAI.empireSpeciesSystems.get(sysID, {}).get('pids',  []):
                    bestDesignID,  colDesign,  buildChoices = getBestShipInfo(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_ORBITAL_DEFENSE,  pid)
                    if not bestDesignID:
                        print "no orbital defenses can be built at ",  PlanetUtilsAI.planetNameIDs([pid])
                        continue
                    #print "selecting  ",  PlanetUtilsAI.planetNameIDs([pid]),  " to build Orbital Defenses"
                    retval  = fo.issueEnqueueShipProductionOrder(bestDesignID, pid)
                    print "queueing %d Orbital Defenses at %s"%(numNeeded,  PlanetUtilsAI.planetNameIDs([pid]))
                    if retval !=0:
                        if numNeeded > 1  :
                            fo.issueChangeProductionQuantityOrder(productionQueue.size -1,  1, numNeeded )
                        cost,  time =   empire.productionCostAndTime( productionQueue[productionQueue.size -1]  )
                        defenseAllocation += productionQueue[productionQueue.size -1].blocksize *  cost/time
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                        break


    bldType = fo.getBuildingType("BLD_SHIPYARD_BASE")
    queuedShipyardLocs = [element.locationID for element in productionQueue if (element.name=="BLD_SHIPYARD_BASE") ]
    systemColonies={}
    colonySystems={}
    for specName in ColonisationAI.empireColonizers:
        if (len( ColonisationAI.empireColonizers[specName])<=int(currentTurn/100)) and (specName in ColonisationAI.empireSpecies): #not enough current shipyards for this species#TODO: also allow orbital incubators and/or asteroid ships
            for pID in ColonisationAI.empireSpecies.get(specName, []): #SP_EXOBOT may not actually have a colony yet but be in empireColonizers
                if pID in queuedShipyardLocs:
                    break #won't try building more than one shipyard at once, per colonizer
            else:  #no queued shipyards, get planets with target pop >=3, and queue a shipyard on the one with biggest current pop
                planetList = zip( map( universe.getPlanet,  ColonisationAI.empireSpecies[specName]),  ColonisationAI.empireSpecies[specName] )
                pops = sorted(  [ (planet.currentMeterValue(fo.meterType.population), pID) for planet, pID in planetList if ( planet and planet.currentMeterValue(fo.meterType.targetPopulation)>=3.0)] )
                pids = [pid for  pop, pid  in pops if bldType.canBeProduced(empire.empireID,  pid)]
                if pids != []:
                    buildLoc= pids[-1]
                    res=fo.issueEnqueueBuildingProductionOrder("BLD_SHIPYARD_BASE", buildLoc)
                    print "Enqueueing BLD_SHIPYARD_BASE at planet %d (%s) for colonizer species %s, with result %d"%(buildLoc, universe.getPlanet(buildLoc).name,  specName,   res)
                    if res:
                        queuedShipyardLocs.append(buildLoc)
                        break # only start at most one new shipyard per species per turn
        for pid in ColonisationAI.empireSpecies.get(specName, []):
            planet=universe.getPlanet(pid)
            if planet:
                systemColonies.setdefault(planet.systemID,  {}).setdefault('pids', []).append(pid)
                colonySystems[pid]=planet.systemID

    aciremaSystems={}
    for pid  in ColonisationAI.empireSpecies.get("SP_ACIREMA",  []):
        aciremaSystems.setdefault( universe.getPlanet(pid).systemID,  [] ).append(pid)
        if (pid in queuedShipyardLocs ) or  not bldType.canBeProduced(empire.empireID,  pid) :
            continue #but not 'break' because we want to build shipyards at *every* Acirema planet
        res=fo.issueEnqueueBuildingProductionOrder("BLD_SHIPYARD_BASE", pid)
        print "Enqueueing BLD_SHIPYARD_BASE at planet %d (%s) for Acirema, with result %d"%(pid, universe.getPlanet(pid).name,   res)
        if res:
            queuedShipyardLocs.append(pid)
            res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
            print "Requeueing Acirema BLD_SHIPYARD_BASE to front of build queue, with result %d"%(res)

    topPilotSystems={}
    for pid, rating  in ColonisationAI.pilotRatings.items() :
        if rating < ColonisationAI.curMidPilotRating:
            continue
        topPilotSystems.setdefault( universe.getPlanet(pid).systemID,  [] ).append( (pid, rating) )
        if (pid in queuedShipyardLocs ) or  not bldType.canBeProduced(empire.empireID,  pid) :
            continue #but not 'break' because we want to build shipyards all top pilot planets
        res=fo.issueEnqueueBuildingProductionOrder("BLD_SHIPYARD_BASE", pid)
        print "Enqueueing BLD_SHIPYARD_BASE at planet %d (%s) for top pilot, with result %d"%(pid, universe.getPlanet(pid).name,   res)
        if res:
            queuedShipyardLocs.append(pid)
            res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
            print "Requeueing Acirema BLD_SHIPYARD_BASE to front of build queue, with result %d"%(res)

    popCtrs = list(AIstate.popCtrIDs)
    enrgyShipyardLocs=[]
    for bldName in [ "BLD_SHIPYARD_ENRG_COMP"  ]:
        if empire.buildingTypeAvailable(bldName) and (bldgExpense<bldgRatio*totalPP) and ( totalPP >200 or currentTurn > 150 ):
            queuedBldLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
            bldType = fo.getBuildingType(bldName)
            for pid in popCtrs:
                if len(queuedBldLocs)>1: #build a max of 2 at once
                    break
                if colonySystems.get(pid,  -1) not in   ( AIstate.empireStars.get(fo.starType.blackHole,  [])  +  AIstate.empireStars.get(fo.starType.blue,  [])  ):
                    continue
                enrgyShipyardLocs.append(pid)
                if  pid not in queuedBldLocs and bldType.canBeProduced(empire.empireID,  pid):#TODO: verify that canBeProduced() checks for prexistence of a barring building
                    res=fo.issueEnqueueBuildingProductionOrder(bldName, pid)
                    print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName,  pid, universe.getPlanet(pid).name,  res)
                    if res:
                        queuedBldLocs.append(pid)
                        cost,  time =   empire.productionCostAndTime( productionQueue[productionQueue.size -1]  )
                        bldgExpense +=  cost/time  # productionQueue[productionQueue.size -1].blocksize *
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                        print "Requeueing %s to front of build queue, with result %d"%(bldName,  res)

    queuedShipyards=[]
    bldName = "BLD_SHIPYARD_BASE"
    if empire.buildingTypeAvailable(bldName) and  (bldgExpense<bldgRatio*totalPP) and ( totalPP >50 or currentTurn > 80 ):
        bldType = fo.getBuildingType(bldName)
        for pid in enrgyShipyardLocs:
            if  pid not in queuedShipyardLocs and bldType.canBeProduced(empire.empireID,  pid):#TODO: verify that canBeProduced() checks for prexistence of a barring building
                res=fo.issueEnqueueBuildingProductionOrder(bldName, pid)
                print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName,  pid, universe.getPlanet(pid).name,  res)
                if res:
                    queuedShipyardLocs.append(pid)
                    cost,  time =   empire.productionCostAndTime( productionQueue[productionQueue.size -1]  )
                    bldgExpense +=  cost/time  # productionQueue[productionQueue.size -1].blocksize *
                    res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                    print "Requeueing %s to front of build queue, with result %d"%(bldName,  res)

    for bldName in [ "BLD_SHIPYARD_ORG_ORB_INC" ,  "BLD_SHIPYARD_ORG_XENO_FAC" ]:
        if empire.buildingTypeAvailable(bldName) and (bldgExpense<bldgRatio*totalPP) and ( totalPP >40 or currentTurn > 40 ):
            queuedBldLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
            bldType = fo.getBuildingType(bldName)
            for pid in popCtrs:
                if len(queuedBldLocs)>1+int(totalPP/200.0) : # limit build  at once
                    break
                if  pid not in queuedBldLocs and bldType.canBeProduced(empire.empireID,  pid):#TODO: verify that canBeProduced() checks for prexistence of a barring building
                    res=fo.issueEnqueueBuildingProductionOrder(bldName, pid)
                    print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName,  pid, universe.getPlanet(pid).name,  res)
                    if res:
                        queuedBldLocs.append(pid)
                        cost,  time =   empire.productionCostAndTime( productionQueue[productionQueue.size -1]  )
                        bldgExpense +=  cost/time  # productionQueue[productionQueue.size -1].blocksize *
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                        print "Requeueing %s to front of build queue, with result %d"%(bldName,  res)

    for bldName in [ "BLD_SHIPYARD_ORG_CELL_GRO_CHAMB" ]:
        if empire.buildingTypeAvailable(bldName) and (bldgExpense<bldgRatio*totalPP) and ( totalPP >50 or currentTurn > 80 ):
            queuedBldLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
            bldType = fo.getBuildingType(bldName)
            for pid in popCtrs:
                if len(queuedBldLocs)>1: #build a max of 2 at once
                    break
                if  pid not in queuedBldLocs and bldType.canBeProduced(empire.empireID,  pid):#TODO: verify that canBeProduced() checks for prexistence of a barring building
                    res=fo.issueEnqueueBuildingProductionOrder(bldName, pid)
                    print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName,  pid, universe.getPlanet(pid).name,  res)
                    if res:
                        queuedBldLocs.append(pid)
                        cost,  time =   empire.productionCostAndTime( productionQueue[productionQueue.size -1]  )
                        bldgExpense +=  cost/time  # productionQueue[productionQueue.size -1].blocksize *
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                        print "Requeueing %s to front of build queue, with result %d"%(bldName,  res)

    ShipYardType = fo.getBuildingType("BLD_SHIPYARD_BASE")
    bldName = "BLD_SHIPYARD_AST"
    if empire.buildingTypeAvailable(bldName) and foAI.foAIstate.aggression > fo.aggression.beginner:
        queuedBldLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
        if not queuedBldLocs:
            bldType = fo.getBuildingType(bldName)
            asteroidSystems = {}
            asteroidYards = {}
            queuedSystems=set()
            shipyardSystems={}
            builderSystems={}
            for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
                planet=universe.getPlanet(pid)
                thisSpec = planet.speciesName
                sysID = planet.systemID
                if planet.size ==  fo.planetSize.asteroids and sysID in ColonisationAI.empireSpeciesSystems:
                    asteroidSystems.setdefault(sysID,  []).append(pid)
                    if (pid in queuedBldLocs) or (bldName in [universe.getObject(bldg).buildingTypeName for bldg in planet.buildingIDs]):
                        asteroidYards[sysID]=pid #shouldn't ever overwrite another, but ok if it did
                if thisSpec in ColonisationAI.empireShipBuilders:
                    if pid in ColonisationAI.empireShipBuilders[thisSpec]:
                        shipyardSystems.setdefault(sysID,  []).append(pid)
                    else:
                        builderSystems.setdefault(sysID,  []).append((planet.speciesName, pid))
            #check if we need to build another asteroid processor:
            # check if  local shipyard to go with the asteroid processor
            yardLocs = []
            needYard ={}
            topPilotLocs = []
            for sysID in set(asteroidSystems.keys()).difference(asteroidYards.keys()):
                if sysID in topPilotSystems:
                    for pid, rating in topPilotSystems[sysID]:
                        if pid not in queuedShipyardLocs: #will catch it later if shipyard already present
                            topPilotLocs.append( (rating, pid, sysID) )
            topPilotLocs.sort(reverse=True)
            for rating, pid, sysID in topPilotLocs:
                if sysID not in yardLocs:
                    yardLocs.append(sysID ) #prioritize asteroid yards for acirema and/or other top pilots
                    for pid, rating in topPilotSystems[sysID]:
                        if pid not in queuedShipyardLocs: #will catch it later if shipyard already present
                            needYard[sysID] = pid
            if  ( not yardLocs ) and len(asteroidYards.values())<= int(currentTurn/50): # not yet building & not enough current locs,  find a location to build one
                queuedYardSystems = set(PlanetUtilsAI.getSystems(queuedShipyardLocs))
                colonizerLocChoices=[]
                builderLocChoices=[]
                bldSystems =  set(asteroidSystems.keys()).difference(asteroidYards.keys())
                for sysID in bldSystems.intersection(builderSystems.keys()):
                    for thisSpec,  pid in builderSystems[sysID]:
                        if thisSpec in ColonisationAI.empireColonizers:
                            if pid in (ColonisationAI.empireColonizers[thisSpec]+queuedShipyardLocs):
                                colonizerLocChoices.insert(0, sysID)
                            else:
                                colonizerLocChoices.append(sysID)
                                needYard[sysID] = pid
                        else:
                            if pid in (ColonisationAI.empireShipBuilders.get(thisSpec, [])+queuedShipyardLocs):
                                builderLocChoices.insert(0, sysID)
                            else:
                                builderLocChoices.append(sysID)
                                needYard[sysID] = pid
                yardLocs.extend(  (colonizerLocChoices+builderLocChoices)[:1]  ) #add at most one of these non top pilot locs
            newYardCount=len(queuedBldLocs)
            for sysID in yardLocs : #build at most 2 new asteroid yards at a time
                if newYardCount >= 2:
                    break
                pid = asteroidSystems[sysID][0]
                if sysID in needYard:
                    pid2 = needYard[sysID]
                    if ShipYardType.canBeProduced(empire.empireID,  pid2):
                        res=fo.issueEnqueueBuildingProductionOrder("BLD_SHIPYARD_BASE", pid2)
                        print "Enqueueing %s at planet %d (%s) to go with Aseroid Processor , with result %d"%("BLD_SHIPYARD_BASE",  pid2, universe.getPlanet(pid2).name,  res)
                        if res:
                            queuedShipyardLocs.append(pid2)
                            cost,  time =   empire.productionCostAndTime( productionQueue[productionQueue.size -1]  )
                            bldgExpense +=  cost/time  # productionQueue[productionQueue.size -1].blocksize *
                            res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                            print "Requeueing %s to front of build queue, with result %d"%("BLD_SHIPYARD_BASE",  res)
                if  pid not in queuedBldLocs and  bldType.canBeProduced(empire.empireID,  pid):
                    res=fo.issueEnqueueBuildingProductionOrder(bldName, pid)
                    print "Enqueueing %s at planet %d (%s) , with result %d on turn %d"%(bldName,  pid, universe.getPlanet(pid).name,  res,  currentTurn)
                    if res:
                        newYardCount += 1
                        queuedBldLocs.append(pid)
                        cost,  time =   empire.productionCostAndTime( productionQueue[productionQueue.size -1]  )
                        bldgExpense +=  cost/time  # productionQueue[productionQueue.size -1].blocksize *
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                        print "Requeueing %s to front of build queue, with result %d"%(bldName,  res)

    bldName = "BLD_GAS_GIANT_GEN"
    maxGGGs=1
    if empire.buildingTypeAvailable(bldName) and foAI.foAIstate.aggression > fo.aggression.beginner:
        queuedBldLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
        bldType = fo.getBuildingType(bldName)
        for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):#TODO: check to ensure that a resource center exists in system, or GGG would be wasted
            if  pid not in queuedBldLocs and bldType.canBeProduced(empire.empireID,  pid):#TODO: verify that canBeProduced() checks for prexistence of a barring building
                thisPlanet=universe.getPlanet(pid)
                if thisPlanet.systemID in ColonisationAI.empireSpeciesSystems:
                    GGList=[]
                    canUseGGG=False
                    system=universe.getSystem(thisPlanet.systemID)
                    for opid in system.planetIDs:
                        otherPlanet=universe.getPlanet(opid)
                        if otherPlanet.size== fo.planetSize.gasGiant:
                            GGList.append(opid)
                        if  opid!=pid and otherPlanet.owner==empire.empireID and (EnumsAI.AIFocusType.FOCUS_INDUSTRY  in list(otherPlanet.availableFoci)+[otherPlanet.focus]):
                            canUseGGG=True
                    if pid in sorted(GGList)[:maxGGGs] and canUseGGG:
                        res=fo.issueEnqueueBuildingProductionOrder(bldName, pid)
                        if res:
                            queuedBldLocs.append(pid)
                            cost,  time =   empire.productionCostAndTime( productionQueue[productionQueue.size -1]  )
                            bldgExpense +=  cost/time  # productionQueue[productionQueue.size -1].blocksize *
                            res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                            print "Requeueing %s to front of build queue, with result %d"%(bldName,  res)
                        print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName,  pid, universe.getPlanet(pid).name,  res)

    bldName = "BLD_SOL_ORB_GEN"
    if empire.buildingTypeAvailable(bldName) and foAI.foAIstate.aggression > fo.aggression.turtle:
        bldType = fo.getBuildingType(bldName)
        alreadyGotOne=99
        for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
            planet=universe.getPlanet(pid)
            if planet and bldName in [bld.buildingTypeName for bld in map( universe.getObject,  planet.buildingIDs)]:
                system = universe.getSystem(planet.systemID)
                if system and system.starType < alreadyGotOne:
                    alreadyGotOne = system.starType
        bestType=fo.starType.white
        best_locs = AIstate.empireStars.get(fo.starType.blue,  []) + AIstate.empireStars.get(fo.starType.white,  [])
        if best_locs==[]:
            bestType=fo.starType.orange
            best_locs = AIstate.empireStars.get(fo.starType.yellow,  []) + AIstate.empireStars.get(fo.starType.orange,  [])
        if ( not best_locs) or ( alreadyGotOne<99 and alreadyGotOne <= bestType  ):
            pass # could consider building at a red star if have a lot of  PP but somehow no better stars
        else:
            useNewLoc=True
            queuedBldLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
            if queuedBldLocs != []:
                queuedStarTypes = {}
                for loc in queuedBldLocs:
                    planet = universe.getPlanet(loc)
                    if not planet: continue
                    system = universe.getSystem(planet.systemID)
                    queuedStarTypes.setdefault(system.starType,  []).append(loc)
                if queuedStarTypes:
                    bestQueued = sorted(queuedStarTypes.keys())[0]
                    if bestQueued > bestType:  # i.e., bestQueued is yellow, bestType available is blue or white
                        pass #should probably evaluate cancelling the existing one under construction
                    else:
                        useNewLoc=False
            if useNewLoc:  # (of course, may be only loc, not really new)
                if not homeworld:
                    useSys=best_locs[0]#as good as any
                else:
                    distanceMap={}
                    for sysID in best_locs: #want to build close to capitol for defense
                        if sysID == -1:
                            continue
                        try:
                            distanceMap[sysID] = universe.jumpDistance(homeworld.systemID, sysID)
                        except:
                            pass
                    useSys = ([(-1, -1)] + sorted(  [ (dist,  sysID) for sysID,  dist in distanceMap.items() ] ))[:2][-1][-1]  # kinda messy, but ensures a value
                if useSys!= -1:
                    try:
                        useLoc = AIstate.colonizedSystems[useSys][0]
                        res=fo.issueEnqueueBuildingProductionOrder(bldName, useLoc)
                        print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName,  useLoc, universe.getPlanet(useLoc).name,  res)
                        if res:
                            cost,  time =   empire.productionCostAndTime( productionQueue[productionQueue.size -1]  )
                            bldgExpense +=  cost/time  # productionQueue[productionQueue.size -1].blocksize *
                            res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                            print "Requeueing %s to front of build queue, with result %d"%(bldName,  res)
                    except:
                        print "problem queueing BLD_SOL_ORB_GEN at planet",  useLoc,  "of system ",  useSys
                        pass

    bldName = "BLD_ART_BLACK_HOLE"
    if ( ( empire.buildingTypeAvailable(bldName) ) and (foAI.foAIstate.aggression > fo.aggression.typical) and
                        (len( claimedStars.get(fo.starType.blackHole,  [])) == 0 )  and   (len( AIstate.empireStars.get(fo.starType.red,  [])) > 0)   ):
        bldType = fo.getBuildingType(bldName)
        alreadyGotOne=False
        for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
            planet=universe.getPlanet(pid)
            if planet and bldName in [bld.buildingTypeName for bld in map( universe.getObject,  planet.buildingIDs)]:
                alreadyGotOne = True
        queuedBldLocs = [element.locationID for element in productionQueue if (element.name==bldName) ] #TODO: check that queued locs or already built one are at red stars
        if len (queuedBldLocs)==0 and not alreadyGotOne:  #
            if not homeworld:
                useSys= AIstate.empireStars[fo.starType.red][0]
            else:
                distanceMap={}
                for sysID in AIstate.empireStars.get(fo.starType.red,  []):
                    if sysID == -1:
                        continue
                    try:
                        distanceMap[sysID] = universe.jumpDistance(homeworld.systemID, sysID)
                    except:
                        pass
                redSysList = sorted(  [ (dist,  sysID) for sysID,  dist in distanceMap.items() ] )
                useLoc=None
                for dist,  sysID in redSysList:
                    for loc in  AIstate.colonizedSystems[sysID]:
                        planet=universe.getPlanet(loc)
                        if planet and planet.speciesName not in  [ "",  None ]:
                            species= fo.getSpecies(planet.speciesName)
                            if species and "PHOTOTROPHIC" in list(species.tags):
                                break
                    else:
                        if len(AIstate.colonizedSystems[sysID]) >0:
                            useLoc = AIstate.colonizedSystems[sysID][0]
                    if useLoc !=None:
                        break
                if useLoc !=None:
                    try:
                        res=fo.issueEnqueueBuildingProductionOrder(bldName, useLoc)
                        print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName,  useLoc, universe.getPlanet(useLoc).name,  res)
                        if res:
                            res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                            print "Requeueing %s to front of build queue, with result %d"%(bldName,  res)
                    except:
                        print "problem queueing %s at planet"%bldName,  useLoc,  "of system ",  useSys

    bldName = "BLD_BLACK_HOLE_POW_GEN"
    if empire.buildingTypeAvailable(bldName) and foAI.foAIstate.aggression > fo.aggression.cautious:
        bldType = fo.getBuildingType(bldName)
        alreadyGotOne=False
        for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
            planet=universe.getPlanet(pid)
            if planet and bldName in [bld.buildingTypeName for bld in map( universe.getObject,  planet.buildingIDs)]:
                alreadyGotOne = True
        queuedBldLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
        if (len( AIstate.empireStars.get(fo.starType.blackHole,  [])) > 0) and len (queuedBldLocs)==0 and not alreadyGotOne:  #
            if not homeworld:
                useSys= AIstate.empireStars.get(fo.starType.blackHole,  [])[0]
            else:
                distanceMap={}
                for sysID in AIstate.empireStars.get(fo.starType.blackHole,  []):
                    if sysID == -1:
                        continue
                    try:
                        distanceMap[sysID] = universe.jumpDistance(homeworld.systemID, sysID)
                    except:
                        pass
                useSys = ([(-1, -1)] + sorted(  [ (dist,  sysID) for sysID,  dist in distanceMap.items() ] ))[:2][-1][-1]  # kinda messy, but ensures a value
            if useSys!= -1:
                try:
                    useLoc = AIstate.colonizedSystems[useSys][0]
                    res=fo.issueEnqueueBuildingProductionOrder(bldName, useLoc)
                    print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName,  useLoc, universe.getPlanet(useLoc).name,  res)
                    if res:
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                        print "Requeueing %s to front of build queue, with result %d"%(bldName,  res)
                except:
                    print "problem queueing BLD_BLACK_HOLE_POW_GEN at planet",  useLoc,  "of system ",  useSys
                    pass

    bldName = "BLD_ENCLAVE_VOID"
    if empire.buildingTypeAvailable(bldName):
        bldType = fo.getBuildingType(bldName)
        alreadyGotOne=False
        for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
            planet=universe.getPlanet(pid)
            if planet and bldName in [bld.buildingTypeName for bld in map( universe.getObject,  planet.buildingIDs)]:
                alreadyGotOne = True
        queuedLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
        if len (queuedLocs)==0 and homeworld and not alreadyGotOne:  #
            try:
                res=fo.issueEnqueueBuildingProductionOrder(bldName, capitolID)
                print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName,  capitolID, universe.getPlanet(capitolID).name,  res)
                if res:
                    res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                    print "Requeueing %s to front of build queue, with result %d"%(bldName,  res)
            except:
                pass

    bldName = "BLD_GENOME_BANK"
    if empire.buildingTypeAvailable(bldName):
        bldType = fo.getBuildingType(bldName)
        alreadyGotOne=False
        for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
            planet=universe.getPlanet(pid)
            if planet and bldName in [bld.buildingTypeName for bld in map( universe.getObject,  planet.buildingIDs)]:
                alreadyGotOne = True
        queuedLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
        if len (queuedLocs)==0 and homeworld and not alreadyGotOne:  #
            try:
                res=fo.issueEnqueueBuildingProductionOrder(bldName, capitolID)
                print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName,  capitolID, universe.getPlanet(capitolID).name,  res)
                if res:
                    res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                    print "Requeueing %s to front of build queue, with result %d"%(bldName,  res)
            except:
                pass

    bldName = "BLD_NEUTRONIUM_EXTRACTOR"
    alreadyGotExtractor=False
    if empire.buildingTypeAvailable(bldName) and ( [element.locationID for element in productionQueue if (element.name==bldName) ]==[]):
        #bldType = fo.getBuildingType(bldName)
        for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
            planet=universe.getPlanet(pid)
            if ( planet and (( planet.systemID in  AIstate.empireStars.get(fo.starType.neutron,  [])    and
                                                                                                 (bldName in [bld.buildingTypeName for bld in map( universe.getObject,  planet.buildingIDs)]) )   or
                                                                                                 ("BLD_NEUTRONIUM_SYNTH" in [bld.buildingTypeName for bld in map( universe.getObject,  planet.buildingIDs)])
                                                                                                 )):
                alreadyGotExtractor = True
        if not alreadyGotExtractor:
            if not homeworld:
                useSys= AIstate.empireStars.get(fo.starType.neutron,  [])[0]
            else:
                distanceMap={}
                for sysID in AIstate.empireStars.get(fo.starType.neutron,  []):
                    if sysID == -1:
                        continue
                    try:
                        distanceMap[sysID] = universe.jumpDistance(homeworld.systemID, sysID, empire.empireID)
                    except:
                        pass
                print ([-1] + sorted(  [ (dist,  sysID) for sysID,  dist in distanceMap.items() ] ))
                useSys = ([(-1, -1)] + sorted(  [ (dist,  sysID) for sysID,  dist in distanceMap.items() ] ))[:2][-1][-1]  # kinda messy, but ensures a value
            if useSys!= -1:
                try:
                    useLoc = AIstate.colonizedSystems[useSys][0]
                    res=fo.issueEnqueueBuildingProductionOrder(bldName, useLoc)
                    print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName,  useLoc, universe.getPlanet(useLoc).name,  res)
                    if res:
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                        print "Requeueing %s to front of build queue, with result %d"%(bldName,  res)
                except:
                    print "problem queueing BLD_NEUTRONIUM_EXTRACTOR at planet",  useLoc,  "of system ",  useSys
                    pass

    bldName = "BLD_NEUTRONIUM_FORGE"
    if empire.buildingTypeAvailable(bldName) and foAI.foAIstate.aggression > fo.aggression.beginner:
        print "considering building a ",  bldName
        if not alreadyGotExtractor:
            print "Apparently have no Neutronium_Extractors nor Sythesizers"
        else:
            queuedBldLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
            bldType = fo.getBuildingType(bldName)
            if len(queuedBldLocs) < 2 :  #don't build too many at once
                if homeworld:
                    tryLocs= [capitolID] + list(AIstate.popCtrIDs)
                else:
                    tryLocs= AIstate.popCtrIDs
                print "Possible locs for %s are: "%bldName,  PlanetUtilsAI.planetNameIDs(tryLocs)
                for pid in tryLocs:
                    if  pid not in queuedBldLocs:
                        planet=universe.getPlanet(pid)
                        if bldType.canBeProduced(empire.empireID,  pid):#TODO: verify that canBeProduced() checks for prexistence of a barring building
                            if  bldName not in [bld.buildingTypeName for bld in map( universe.getObject,  planet.buildingIDs)]:
                                if  "BLD_SHIPYARD_BASE"  in [bld.buildingTypeName for bld in map( universe.getObject,  planet.buildingIDs)]:
                                    res=fo.issueEnqueueBuildingProductionOrder(bldName, pid)
                                    print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName,  pid, universe.getPlanet(pid).name,  res)
                                    if res:
                                        if productionQueue.size  > 5:
                                            res=fo.issueRequeueProductionOrder(productionQueue.size -1,  3) # move to front
                                            print "Requeueing %s to front of build queue, with result %d"%(bldName,  res)
                                        break #only initiate max of one new build per turn

    colonyShipMap={}
    for fid in FleetUtilsAI.getEmpireFleetIDsByRole( EnumsAI.AIFleetMissionType.FLEET_MISSION_COLONISATION):
        fleet = universe.getFleet(fid)
        if not fleet: continue
        for shipID in fleet.shipIDs:
            thisShip=universe.getShip(shipID)
            if thisShip and (foAI.foAIstate.getShipRole(thisShip.design.id) ==EnumsAI.AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION):
                colonyShipMap.setdefault( thisShip.speciesName,  []).append(1)

    bldName = "BLD_CONC_CAMP"
    verboseCamp = False
    bldType = fo.getBuildingType(bldName)
    for pid in AIstate.popCtrIDs:
        planet=universe.getPlanet(pid)
        if not planet:
            continue
        canBuildCamp = bldType.canBeProduced(empire.empireID,  pid)
        tPop = planet.currentMeterValue(fo.meterType.targetPopulation)
        tInd=planet.currentMeterValue(fo.meterType.targetIndustry)
        cInd=planet.currentMeterValue(fo.meterType.industry)
        cPop = planet.currentMeterValue(fo.meterType.population)
        popDisqualified = (cPop <= 32) or (cPop < 0.9*tPop)
        checkedCamp=False
        builtCamp=False
        thisSpec = planet.speciesName
        safetyMarginMet = ( (thisSpec in ColonisationAI.empireColonizers and  ( len(ColonisationAI.empireSpecies.get(thisSpec, [])+colonyShipMap.get(thisSpec, []))>=2 )) or (cPop >= 50 ))
        if popDisqualified or not safetyMarginMet :  #check even if not aggressive, etc, just in case acquired planet with a ConcCamp on it
            if canBuildCamp:
                if popDisqualified:
                    if verboseCamp:
                        print "Conc Camp disqualified at %s due to low pop: current %.1f  target: %.1f"%(planet.name,  cPop,  tPop)
                else:
                    if verboseCamp:
                        print "Conc Camp disqualified at %s due to safety margin; species %s,  colonizing planets %s,  with %d colony ships"%(planet.name,  planet.speciesName,  ColonisationAI.empireSpecies.get(planet.speciesName, []),  len(colonyShipMap.get(planet.speciesName, [])))
            for bldg in planet.buildingIDs:
                if universe.getObject(bldg).buildingTypeName  == bldName:
                    res=fo.issueScrapOrder( bldg)
                    print "Tried scrapping %s at planet %s,  got result %d"%(bldName,  planet.name,  res)
        elif foAI.foAIstate.aggression>fo.aggression.typical and canBuildCamp and (tPop >= 36):
            checkedCamp=True
            if  (planet.focus== EnumsAI.AIFocusType.FOCUS_GROWTH) or ("COMPUTRONIUM_SPECIAL" in planet.specials) or (pid == capitolID):
                continue
                #pass  # now that focus setting takes these into account, probably works ok to have conc camp, but let's not push it
            queuedBldLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
            if (cPop <0.95*tPop):#
                if verboseCamp:
                    print "Conc Camp disqualified at %s due to pop: current %.1f  target: %.1f"%(planet.name,  cPop,  tPop)
            else:
                if  pid not in queuedBldLocs:
                    if planet.focus in [ EnumsAI.AIFocusType.FOCUS_INDUSTRY ]:
                        if cInd >= tInd+cPop:
                            continue
                    else:
                        oldFocus=planet.focus
                        fo.issueChangeFocusOrder(pid, EnumsAI.AIFocusType.FOCUS_INDUSTRY)
                        universe.updateMeterEstimates([pid])
                        tInd=planet.currentMeterValue(fo.meterType.targetIndustry)
                        if cInd >= tInd+cPop:
                            fo.issueChangeFocusOrder(pid, oldFocus)
                            universe.updateMeterEstimates([pid])
                            continue
                    res=fo.issueEnqueueBuildingProductionOrder(bldName, pid)
                    builtCamp = res
                    print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName,  pid, universe.getPlanet(pid).name,  res)
                    if res:
                        queuedBldLocs.append(pid)
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                    else:
                        #TODO: enable location condition reporting a la mapwnd BuildDesignatorWnd
                        print "Error enqueing Conc Camp despite bldType.canBeProduced(empire.empireID,  pid) reporting ",  canBuildCamp
        if verboseCamp:
            print "conc camp status at %s : checkedCamp: %s,  builtCamp: %s"%( planet.name,  canBuildCamp,  builtCamp)

    bldName = "BLD_SCANNING_FACILITY"
    if empire.buildingTypeAvailable(bldName):
        bldType = fo.getBuildingType(bldName)
        queuedLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
        scannerLocs={}
        for pid in list(AIstate.popCtrIDs) + list(AIstate.outpostIDs):
            planet=universe.getPlanet(pid)
            if planet:
                if ( pid in queuedLocs ) or (  bldName in [bld.buildingTypeName for bld in map( universe.getObject,  planet.buildingIDs)] ):
                    scannerLocs[planet.systemID] = True
        maxScannerBuilds = max(1,  int(empire.productionPoints/30))
        for sysID in AIstate.colonizedSystems:
            if len(queuedLocs)>= maxScannerBuilds:
                break
            if sysID in scannerLocs:
                continue
            needScanner=False
            for nSys in AIstate.dictFromMap( universe.getSystemNeighborsMap(sysID,  empire.empireID) ):
                if (universe.getVisibility(nSys,  empire.empireID) < fo.visibility.partial):
                    needScanner=True
                    break
            if not needScanner:
                continue
            buildLocs=[]
            for pid in AIstate.colonizedSystems[sysID]:
                planet=universe.getPlanet(pid)
                if not planet: continue
                buildLocs.append( (planet.currentMeterValue(fo.meterType.maxTroops),  pid) )
            if not buildLocs: continue
            for troops,  loc in sorted( buildLocs ):
                planet = universe.getPlanet(loc)
                res=fo.issueEnqueueBuildingProductionOrder(bldName, loc)
                print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName,  loc, planet.name,  res)
                if res:
                    res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                    print "Requeueing %s to front of build queue, with result %d"%(bldName,  res)
                    queuedLocs.append( planet.systemID )
                    break

    bldName = "BLD_SHIPYARD_ORBITAL_DRYDOCK"
    if empire.buildingTypeAvailable(bldName):
        bldType = fo.getBuildingType(bldName)
        queued_locs = [element.locationID for element in productionQueue if (element.name==bldName) ]
        drydock_locs={}
        drydock_locs.update([(sys_id,  True) for sys_id in queued_locs + list(ColonisationAI.empire_dry_docks)])
        max_dock_builds = int(0.5 + empire.productionPoints/80)
        for sys_id,  pids in ColonisationAI.empireSpeciesSystems.items(): #TODO: sort/prioritize in some fashion
            if len(queued_locs)>= max_dock_builds:
                break
            if sys_id in drydock_locs:
                continue
            for pid in pids:
                if pid not in ColonisationAI.empireShipyards:
                    continue
                planet = universe.getPlanet(pid)
                res=fo.issueEnqueueBuildingProductionOrder(bldName, pid)
                print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName,  pid, planet.name,  res)
                if res:
                    res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                    print "Requeueing %s to front of build queue, with result %d"%(bldName,  res)
                    queued_locs.append( planet.systemID )
                    break

    bldName = "BLD_EVACUATION"
    bldType = fo.getBuildingType(bldName)
    for pid in AIstate.popCtrIDs:
        planet=universe.getPlanet(pid)
        if not planet:
            continue
        for bldg in planet.buildingIDs:
            if universe.getObject(bldg).buildingTypeName  == bldName:
                res=fo.issueScrapOrder( bldg)
                print "Tried scrapping %s at planet %s,  got result %d"%(bldName,  planet.name,  res)

    totalPPSpent = fo.getEmpire().productionQueue.totalSpent
    print "  Total Production Points Spent:     " + str(totalPPSpent)

    wastedPP = max(0,  totalPP - totalPPSpent)
    print "  Wasted Production Points:          " + str(wastedPP)#TODO: add resource group analysis
    availPP = totalPP - totalPPSpent - 0.0001

    print ""
    if False:
        print "Possible ship designs to build:"
        if homeworld:
            for shipDesignID in empire.availableShipDesigns:
                shipDesign = fo.getShipDesign(shipDesignID)
                print "    " + str(shipDesign.name(True)) + " cost:" + str(shipDesign.productionCost(empire.empireID,  homeworld.id) )+ " time:" + str(shipDesign.productionTime(empire.empireID,  homeworld.id))

    print ""
    print "Projects already in Production Queue:"
    productionQueue = empire.productionQueue
    print "production summary: %s"%[elem.name for elem in productionQueue]
    queuedColonyShips={}
    queuedOutpostShips = 0
    queuedTroopShips=0

    #TODO:  blocked items might not need dequeuing, but rather for supply lines to be un-blockaded
    dequeueList=[]
    fo.updateProductionQueue()
    can_prioritize_troops = False
    for queue_index  in range( len(productionQueue)):
        element=productionQueue[queue_index]
        blockStr =  "%d x "%element.blocksize              #["a single ",  "in blocks of %d "%element.blocksize][element.blocksize>1]
        print "    " + blockStr + element.name+" requiring " + str(element.turnsLeft) + " more turns;  alloc: %.2f PP"%element.allocation + " with cum. progress of  %.1f"%element.progress + " being built at " + universe.getObject(element.locationID).name
        if element.turnsLeft == -1:
            if element.locationID not in AIstate.popCtrIDs+AIstate.outpostIDs:
                #dequeueList.append(queue_index) #TODO add assessment of recapture -- invasion target etc.
                #print "element %s will never be completed as stands and location %d no longer owned; deleting from queue "%(element.name,  element.locationID)
                print "element %s will never be completed as stands and location %d no longer owned; could consider deleting from queue "%(element.name,  element.locationID) #TODO:
            else:
                print "element %s is projected to never be completed as currently stands, but will remain on queue  "%element.name
        elif element.buildType == EnumsAI.AIEmpireProductionTypes.BT_SHIP:
            this_role = foAI.foAIstate.getShipRole(element.designID)
            if this_role ==       EnumsAI.AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION:
                thisSpec=universe.getPlanet(element.locationID).speciesName
                queuedColonyShips[thisSpec] =  queuedColonyShips.get(thisSpec, 0) +  element.remaining*element.blocksize
            elif this_role ==       EnumsAI.AIShipRoleType.SHIP_ROLE_CIVILIAN_OUTPOST:
                queuedOutpostShips+=  element.remaining*element.blocksize
            elif this_role ==       EnumsAI.AIShipRoleType.SHIP_ROLE_BASE_OUTPOST:
                queuedOutpostShips+=  element.remaining*element.blocksize
            elif this_role ==       EnumsAI.AIShipRoleType.SHIP_ROLE_MILITARY_INVASION:
                queuedTroopShips+=  element.remaining*element.blocksize
            elif (this_role ==       EnumsAI.AIShipRoleType.SHIP_ROLE_CIVILIAN_EXPLORATION) and (queue_index <= 1):
                if len(AIstate.opponentPlanetIDs) > 0:
                    can_prioritize_troops = True
    if queuedColonyShips:
        print "\nFound  colony ships in build queue: %s"%queuedColonyShips
    if queuedOutpostShips:
        print "\nFound  outpost ships and bases in build queue: %s"%(queuedOutpostShips)

    for queue_index in dequeueList[::-1]:
        fo.issueDequeueProductionOrder(queue_index)

    allMilitaryFleetIDs =  FleetUtilsAI.getEmpireFleetIDsByRole(EnumsAI.AIFleetMissionType.FLEET_MISSION_MILITARY )
    nMilitaryTot = sum( [ foAI.foAIstate.fleetStatus.get(fid,  {}).get('nships', 0) for fid in allMilitaryFleetIDs ] )
    allTroopFleetIDs =  FleetUtilsAI.getEmpireFleetIDsByRole(EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION )
    nTroopTot = sum( [ foAI.foAIstate.fleetStatus.get(fid,  {}).get('nships', 0) for fid in allTroopFleetIDs ] )
    availTroopFleetIDs = list( FleetUtilsAI.extractFleetIDsWithoutMissionTypes(allTroopFleetIDs))
    nAvailTroopTot = sum( [ foAI.foAIstate.fleetStatus.get(fid,  {}).get('nships', 0) for fid in availTroopFleetIDs ] )
    print "Trooper Status turn %d: %d total,  with %d unassigned.  %d queued, compared to %d total Military Attack Ships"%(currentTurn,   nTroopTot,
                                                                                                                   nAvailTroopTot,  queuedTroopShips,  nMilitaryTot)
    if ( capitolID!=None and ((currentTurn>=40) or (can_prioritize_troops)) and foAI.foAIstate.systemStatus.get(capitolSysID,  {}).get('fleetThreat', 0)==0   and
                                                                                           foAI.foAIstate.systemStatus.get(capitolSysID,  {}).get('neighborThreat', 0)==0):
        bestDesignID,  bestDesign,  buildChoices = getBestShipInfo( EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_INVASION)
        if  buildChoices!=None and  len(buildChoices)>0:
            loc = random.choice(buildChoices)
            prodTime = bestDesign.productionTime(empire.empireID,  loc)
            prodCost=bestDesign.productionCost(empire.empireID,  loc)
            troopersNeededForcing = max(0,   int( min(0.99+  (currentTurn/20.0 - nAvailTroopTot)/max(2, prodTime-1),  nMilitaryTot/3 -nTroopTot)))
            numShips=troopersNeededForcing
            perTurnCost = (float(prodCost) / prodTime)
            if troopersNeededForcing>0  and totalPP > 3*perTurnCost*queuedTroopShips:
                retval  = fo.issueEnqueueShipProductionOrder(bestDesignID, loc)
                if retval !=0:
                    print "forcing %d new ship(s) to production queue:  %s; per turn production cost %.1f"%(numShips,  bestDesign.name(True),  numShips*perTurnCost)
                    print ""
                    if numShips>1:
                        fo.issueChangeProductionQuantityOrder(productionQueue.size -1,  1,  numShips)
                    availPP -=  numShips*perTurnCost
                    res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                    fo.updateProductionQueue()
        print ""

    print ""
    # get the highest production priorities
    productionPriorities = {}
    for priorityType in EnumsAI.getAIPriorityProductionTypes():
        productionPriorities[priorityType] = int(max(0,  ( foAI.foAIstate.getPriority(priorityType) )**0.5))

    sortedPriorities = productionPriorities.items()
    sortedPriorities.sort(lambda x,y: cmp(x[1], y[1]), reverse=True)

    topPriority = -1
    topscore = -1
    numTotalFleets = len(foAI.foAIstate.fleetStatus)
    numColonyTargs=len(foAI.foAIstate.colonisablePlanetIDs )
    numColonyFleets=len( FleetUtilsAI.getEmpireFleetIDsByRole( EnumsAI.AIFleetMissionType.FLEET_MISSION_COLONISATION) )# counting existing colony fleets each as one ship
    totColonyFleets = sum(queuedColonyShips.values()) + numColonyFleets
    numOutpostTargs=len(foAI.foAIstate.colonisableOutpostIDs )
    numOutpostFleets=len( FleetUtilsAI.getEmpireFleetIDsByRole( EnumsAI.AIFleetMissionType.FLEET_MISSION_OUTPOST) )# counting existing outpost fleets each as one ship
    totOutpostFleets = queuedOutpostShips + numOutpostFleets

    #maxColonyFleets = max(  min( numColonyTargs+1+currentTurn/10 ,  numTotalFleets/4  ),  3+int(3*len(empireColonizers)))
    #maxOutpostFleets = min(numOutpostTargs+1+currentTurn/10,  numTotalFleets/4  )
    maxColonyFleets = PriorityAI.allottedColonyTargets
    maxOutpostFleets = maxColonyFleets


    _,  _,  colonyBuildChoices = getBestShipInfo(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION)
    militaryEmergency = PriorityAI.unmetThreat > (2.0 * MilitaryAI.totMilRating )

    print "Production Queue Priorities:"
    filteredPriorities = {}
    for ID,  score in sortedPriorities:
        if militaryEmergency:
            if ID == EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_EXPLORATION:
                score /= 10.0
            elif ID != EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY:
                score /= 2.0
        if topscore < score:
            topPriority = ID
            topscore = score #don't really need topscore nor sorting with current handling
        print " Score: %4d -- %s "%(score,  EnumsAI.AIPriorityNames[ID] )
        if ID != EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_BUILDINGS:
            if ( ID == EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION ) and (  totColonyFleets <  maxColonyFleets) and (colonyBuildChoices is not None) and len(colonyBuildChoices) >0:
                filteredPriorities[ID]= score
            elif ( ID == EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_OUTPOST ) and (  totOutpostFleets <  maxOutpostFleets ):
                filteredPriorities[ID]= score
            elif ID not in [EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_OUTPOST ,  EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION ]:
                filteredPriorities[ID]= score
    if filteredPriorities == {}:
        print "No non-building-production priorities with nonzero score, setting to default: Military"
        filteredPriorities [EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY ] =  1
    if topscore <= 100:
        scalingPower = 1.0
    else:
        scalingPower = math.log(100)/math.log(topscore)
    for pty in filteredPriorities:
        filteredPriorities[pty]  = filteredPriorities[pty] **scalingPower


    availablePP = dict(  [ (tuple(el.key()),  el.data() ) for el in  empire.planetsWithAvailablePP ])  #keys are sets of ints; data is doubles
    allocatedPP =  dict(  [ (tuple(el.key()),  el.data() ) for el in  empire.planetsWithAllocatedPP ])  #keys are sets of ints; data is doubles
    planetsWithWastedPP = set( [tuple(pidset) for pidset in empire.planetsWithWastedPP ] )
    print "availPP ( <systems> : pp ):"
    for pSet in availablePP:
        print "\t%s\t%.2f"%( PlanetUtilsAI.sysNameIDs(set(PlanetUtilsAI.getSystems( pSet))),  availablePP[pSet])
    print "\nallocatedPP ( <systems> : pp ):"
    for pSet in allocatedPP:
        print "\t%s\t%.2f"%( PlanetUtilsAI.sysNameIDs(set(PlanetUtilsAI.getSystems( pSet))),  allocatedPP[pSet])

    print "\n\nBuilding Ships in system groups  with remaining  PP:"
    for pSet in planetsWithWastedPP:
        totalPP = availablePP.get(pSet,  0)
        availPP =  totalPP - allocatedPP.get(pSet,  0)
        if availPP <=0.01:
            continue
        print "%.2f PP remaining in system group: %s"%(availPP,   PlanetUtilsAI.sysNameIDs(set(PlanetUtilsAI.getSystems( pSet))))
        print "\t owned planets in this group are:"
        print "\t %s"%( PlanetUtilsAI.planetNameIDs(pSet)  )
        bestDesignID,  bestDesign,  buildChoices = getBestShipInfo(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION,  list(pSet))
        speciesMap = {}
        for loc in (buildChoices or []):
            thisSpec = universe.getPlanet(loc).speciesName
            speciesMap.setdefault(thisSpec,  []).append( loc)
        colonyBuildChoices=[]
        for pid,  score_Spec_tuple in foAI.foAIstate.colonisablePlanetIDs:
            score,  thisSpec = score_Spec_tuple
            colonyBuildChoices.extend( int(math.ceil(score))*[pid2 for pid2 in speciesMap.get(thisSpec,  []) if pid2 in pSet]  )

        localPriorities = {}
        localPriorities.update( filteredPriorities )
        bestShips={}
        milBuildChoices = getBestShipRatings(list(pSet))
        for priority in list(localPriorities):
            if priority == EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY:
                if milBuildChoices == []:
                    del localPriorities[priority]
                    continue
                top = milBuildChoices[0]
                bestDesignID,  bestDesign,  buildChoices = top[2],  top[3],  [top[0]]
                #score = ColonisationAI.pilotRatings.get(pid, 0)
                #if bestScore < ColonisationAI.curMidPilotRating:
            else:
                bestDesignID,  bestDesign,  buildChoices = getBestShipInfo(priority,  list(pSet))
            if bestDesign is None:
                del localPriorities[priority] #must be missing a shipyard -- TODO build a shipyard if necessary
                continue
            bestShips[priority] = [bestDesignID,  bestDesign,  buildChoices ]
            print "bestShips[%s] = %s   \t locs are %s from %s"%( EnumsAI.AIPriorityNames[priority],  bestDesign.name(False) , buildChoices,   pSet)

        if len(localPriorities)==0:
            print "Alert!! need shipyards in systemSet ",   PlanetUtilsAI.sysNameIDs(set(PlanetUtilsAI.getSystems( sorted(pSet))))
        priorityChoices=[]
        for priority in localPriorities:
            priorityChoices.extend( int(localPriorities[priority]) * [priority] )

        loopCount = 0
        while (availPP > 0) and (loopCount < max(100,  currentTurn)) and (priorityChoices != [] ): #make sure don't get stuck in some nonbreaking loop like if all shipyards captured
            loopCount +=1
            print "Beginning  build enqueue loop %d; %.1f PP available"%(loopCount,  availPP)
            thisPriority = random.choice( priorityChoices )
            print "selected priority: ",  EnumsAI.AIPriorityNames[thisPriority]
            makingColonyShip=False
            makingOutpostShip=False
            if ( thisPriority ==  EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION ):
                if ( totColonyFleets >=  maxColonyFleets ):
                    print "Already sufficient colony ships in queue,  trying next priority choice"
                    print ""
                    for i in range( len(priorityChoices)-1,  -1,  -1):
                        if priorityChoices[i]==EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION:
                            del priorityChoices[i]
                    continue
                elif colonyBuildChoices is None or len(colonyBuildChoices)==0:
                    for i in range( len(priorityChoices)-1,  -1,  -1):
                        if priorityChoices[i]==EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION:
                            del priorityChoices[i]
                    continue
                else:
                    makingColonyShip=True
            if ( thisPriority ==  EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_OUTPOST ):
                if ( totOutpostFleets >=  maxOutpostFleets ):
                    print "Already sufficient outpost ships in queue,  trying next priority choice"
                    print ""
                    for i in range( len(priorityChoices)-1,  -1,  -1):
                        if priorityChoices[i]==EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_OUTPOST:
                            del priorityChoices[i]
                    continue
                else:
                    makingOutpostShip=True
            bestDesignID,  bestDesign,  buildChoices = bestShips[thisPriority]
            if makingColonyShip:
                loc = random.choice(colonyBuildChoices)
                bestDesignID,  bestDesign,  buildChoices = getBestShipInfo(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION,  loc)
            elif thisPriority == EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY:
                selector = random.random()
                choice = milBuildChoices[0] #milBuildChoices can't be empty due to earlier check
                for choice in milBuildChoices:
                    if choice[0] >= selector:
                        break
                loc,  bestDesignID,  bestDesign = choice[1:4]
                if bestDesign is None:
                    print "Error: problem with milBuildChoices; with selector (%s) chose loc (%s),  bestDesignID (%s), bestDesign (None) from milBuildChoices: %s" % (selector,  loc,  bestDesignID,  milBuildChoices)
                    continue
                #print "Mil ship choices ",  loc,  bestDesignID,  " from ",  choice
            else:
                loc = random.choice(buildChoices)

            numShips=1
            perTurnCost = (float(bestDesign.productionCost(empire.empireID,  loc)) / bestDesign.productionTime(empire.empireID,  loc))
            if thisPriority == EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY:
                thisRating = ColonisationAI.pilotRatings.get(loc, 0)
                ratingRatio = float(thisRating) / ColonisationAI.curBestPilotRating
                pname = ""
                if ratingRatio < 0.1:
                    locPlanet = universe.getPlanet(loc)
                    if locPlanet:
                        pname = locPlanet.name
                        thisRating = ColonisationAI.ratePlanetaryPiloting(loc)
                        ratingRatio = float(thisRating) / ColonisationAI.curBestPilotRating
                        qualifier = ["", "suboptimal"][ratingRatio < 1.0]
                        print "Building mil ship at loc %d (%s) with %s pilot Rating: %.1f; ratio to empire best is %.1f"%(loc, pname, qualifier, thisRating, ratingRatio)
                while ( totalPP > 40*perTurnCost):
                    numShips *= 2
                    perTurnCost *= 2
            retval  = fo.issueEnqueueShipProductionOrder(bestDesignID, loc)
            if retval !=0:
                prioritized = False
                print "adding %d new ship(s) at location %s to production queue:  %s; per turn production cost %.1f"%(numShips,  PlanetUtilsAI.planetNameIDs([loc]),   bestDesign.name(True),  perTurnCost)
                print ""
                if numShips>1:
                    fo.issueChangeProductionQuantityOrder(productionQueue.size -1,  1,  numShips)
                availPP -=  perTurnCost
                if makingColonyShip:
                    totColonyFleets +=numShips
                    if totalPP > 4* perTurnCost:
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                    continue
                if makingOutpostShip:
                    totOutpostFleets +=numShips
                    if totalPP > 4* perTurnCost:
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                    continue
                if totalPP > 10* perTurnCost :
                    leadingBlockPP = 0
                    for elem in [productionQueue[elemi] for elemi in range(0,  min(4,  productionQueue.size))]:
                        cost,  time =   empire.productionCostAndTime( elem )
                        leadingBlockPP +=  elem.blocksize *cost/time
                    if leadingBlockPP > 0.5* totalPP or  (militaryEmergency and thisPriority==EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY  ):
                        prioritized = True
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                if (not prioritized) and (priority == EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_INVASION):
                    res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
        print ""
    fo.updateProductionQueue()

def getAvailableBuildLocations(shipDesignID):
    "returns locations where shipDesign can be built"
    result = []
    shipDesign = fo.getShipDesign(shipDesignID)
    empire = fo.getEmpire()
    empireID = empire.empireID
    capitolID = PlanetUtilsAI.getCapital()
    shipyards=set()
    for yardlist in ColonisationAI.empireShipBuilders.values():
        shipyards.update(yardlist)
    shipyards.discard(capitolID)
    for planetID in [capitolID]  + list(shipyards):#gets capitol at front of list
        if shipDesign.productionLocationForEmpire(empireID, planetID):
            result.append(planetID)
    return result

def spentPP():
    "calculate PPs spent this turn so far"

    queue = fo.getEmpire().productionQueue
    return queue.totalSpent


