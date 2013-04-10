import math
import traceback
import random

import freeOrionAIInterface as fo


import AIstate
import FleetUtilsAI
import FreeOrionAI as foAI
import PlanetUtilsAI
import PriorityAI
import ColonisationAI
import EnumsAI

bestMilRatingsHistory={}
shipTypeMap = {   EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_EXPLORATION:   EnumsAI.AIShipDesignTypes.explorationShip,  
                                        EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_OUTPOST:             EnumsAI.AIShipDesignTypes.outpostShip,  
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

#get key routines declared for import by others before completing present imports, to avoid circularity problems
def curBestMilShipRating():
    if (fo.currentTurn()+1) in bestMilRatingsHistory:
        bestMilRatingsHistory.clear()
    if fo.currentTurn() not in bestMilRatingsHistory:
        bestShip,  bestDesign,  buildChoices = getBestShipInfo( EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY)
        if bestDesign is None:
            return 0.00001  #  empire cannot currently produce any military ships, don't make zero though, to avoid divide-by-zero
        stats = foAI.foAIstate.getDesignIDStats(bestDesign.id)
        bestMilRatingsHistory[ fo.currentTurn() ] = stats['attack'] * ( stats['structure'] + stats['shields'] )
    return bestMilRatingsHistory[ fo.currentTurn() ]

def getBestShipInfo(priority,  loc=None):
    "returns designID,  design,  buildLocList"
    empire = fo.getEmpire()
    empireID = empire.empireID
    capitolID = PlanetUtilsAI.getCapital()
    if loc is None:
        planetIDs = [capitolID] + list(AIstate.popCtrIDs) #TODO: restrict to planets with shipyards
    else:
        planetIDs=[loc]
    theseDesignIDs = []
    designNameBases= shipTypeMap.get(priority,  ["nomatch"])
    for baseName in designNameBases:
        theseDesignIDs.extend(  [(designNameBases[baseName]+fo.getShipDesign(shipDesign).name(False) ,  shipDesign ) for shipDesign in empire.availableShipDesigns if baseName  in fo.getShipDesign(shipDesign).name(False) ] )
    if theseDesignIDs == []: 
        return None,  None,  None #must be missing a Shipyard (or checking for outpost ship but missing tech)
    #ships = [ ( fo.getShipDesign(shipDesign).name(False),  shipDesign) for shipDesign in theseDesignIDs ]
    
    for _ , shipDesignID in sorted( theseDesignIDs,  reverse=True):
        shipDesign = fo.getShipDesign(shipDesignID)
        for pid in planetIDs:
            if pid == None:
                continue
            if shipDesign.productionLocationForEmpire(empireID, pid):
                return shipDesignID,  shipDesign,  getAvailableBuildLocations(shipDesignID)
    return None,  None,  None #must be missing a Shipyard or other orbital (or missing tech)

def shipTypeNames(shipProdPriority):
    empire = fo.getEmpire()
    designIDs=[]
    designNameBases= [key for key, val in sorted( shipTypeMap.get(shipProdPriority,  {"nomatch":0}).items(),  key=lambda x:x[1])]
    for baseName in designNameBases:
        designIDs.extend(  [shipDesignID for shipDesignID in empire.allShipDesigns if baseName  in fo.getShipDesign(shipDesignID).name(False) ] )
    shipNames = [fo.getShipDesign(shipDesignID).name(False) for shipDesignID in designIDs]
    

def addDesigns(shipType,  namesToAdd,  needsAdding,  shipProdPriority):
    if needsAdding != []:
        print "--------------"
        print "%s design names apparently needing to be added: %s"%(shipType,  namesToAdd)
        print "-------"
        for name,  desc,  hull,  partslist,  icon,  model in needsAdding:
            try:
                res=fo.issueCreateShipDesignOrder( name,  desc,  hull,  partslist,  icon,  model, False)
                print "added  %s Design %s, with result %d"%(shipType,  name,  res)
            except:
                print "Error: exception triggered and caught adding %s %s:  "%(shipType, name),  traceback.format_exc()
    bestShip,  bestDesign,  buildChoices = getBestShipInfo( shipProdPriority)
    if bestDesign:
        print "Best %s  buildable is %s"%(shipType,  bestDesign.name(False))
    else:
        print "%s apparently unbuildable at present,  ruh-roh"%shipType

def addBaseTroopDesigns():
    empire = fo.getEmpire()
    troopDesignIDs=[]
    designNameBases= [key for key, val in sorted( shipTypeMap.get(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_ORBITAL_INVASION,  {"nomatch":0}).items(),  key=lambda x:x[1])]
    for baseName in designNameBases:
        troopDesignIDs.extend(  [shipDesignID for shipDesignID in empire.allShipDesigns if baseName  in fo.getShipDesign(shipDesignID).name(False) ] )
    troopShipNames = [fo.getShipDesign(shipDesignID).name(False) for shipDesignID in troopDesignIDs]
    print "Current BaseTroopship Designs: %s"%troopShipNames
    newTroopDesigns = []
    desc = "StormTrooper Ship"
    model = "fighter"
    tp = "GT_TROOP_POD"
    nb,  hull =  designNameBases[0],   "SH_COLONY_BASE"
    newTroopDesigns += [ (nb,  desc,  hull,  [tp],  "",  model) ]

    currentTurn=fo.currentTurn()
    needsAdding=[]
    namesToAdd=[]
    for name,  desc,  hull,  partslist,  icon,  model in newTroopDesigns:
        if name not in troopShipNames:
            needsAdding.append( ( name,  desc,  hull,  partslist,  icon,  model) )
            namesToAdd.append( name )

    if needsAdding != []:
        print "--------------"
        print "Current Troop Base Designs: %s"%troopShipNames
        print "-----------"
        print "Troop base design names apparently needing to be added: %s"%namesToAdd
        print "-------"
        if currentTurn ==1:  #due to some apparent problem with these repeatedly being added, only do it on first turn
            for name,  desc,  hull,  partslist,  icon,  model in needsAdding:
                try:
                    res=fo.issueCreateShipDesignOrder( name,  desc,  hull,  partslist,  icon,  model, False)
                    print "added  Troop Base Design %s, with result %d"%(name,  res)
                except:
                    print "Error: exception triggered and caught adding troop base %s:  "%name,  traceback.format_exc()

    bestShip,  bestDesign,  buildChoices = getBestShipInfo( EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_ORBITAL_INVASION)
    if bestDesign:
        print "Best TroopBaseship buildable is %s"%bestDesign.name(False)
    else:
        print "TroopBaseships apparently unbuildable at present,  ruh-roh"
        
def addTroopDesigns():
    addBaseTroopDesigns()
    empire = fo.getEmpire()
    troopDesignIDs=[]
    designNameBases= [key for key, val in sorted( shipTypeMap.get(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_INVASION,  {"nomatch":0}).items(),  key=lambda x:x[1])]
    for baseName in designNameBases:
        troopDesignIDs.extend(  [shipDesignID for shipDesignID in empire.allShipDesigns if baseName  in fo.getShipDesign(shipDesignID).name(False) ] )
    troopShipNames = [fo.getShipDesign(shipDesignID).name(False) for shipDesignID in troopDesignIDs]
    print "Current Troopship Designs: %s"%troopShipNames
    
#    if designNameBases[1] not in troopShipNames:
#        try:
#            res=fo.issueCreateShipDesignOrder(designNameBases[1],  "organic Hulled Troopship for economical large quantities of troops",  
#                                                                                    "SH_ORGANIC",  ["GT_TROOP_POD",  "GT_TROOP_POD",  "GT_TROOP_POD", "GT_TROOP_POD"],  "",  "fighter",  False)
#            print "added  Troopship %s, with result %d"%(designNameBases[1] , res)
#        except:
#            print "Error: exception triggered and caught:  ",  traceback.format_exc()
#    if designNameBases[2] not in troopShipNames:
#        try:
#            res=fo.issueCreateShipDesignOrder(designNameBases[2],  "multicell Hulled Troopship for economical large quantities of troops",  
#                                                                                    "SH_STATIC_MULTICELLULAR",  ["GT_TROOP_POD",  "GT_TROOP_POD",  "SR_WEAPON_5",  "GT_TROOP_POD", "GT_TROOP_POD"],  "",  "fighter",  False)
#            print "added  Troopship %s, with result %d"%(designNameBases[2] , res)
#        except:
#            print "Error: exception triggered and caught:  ",  traceback.format_exc()


    newTroopDesigns = []
    desc = "Troop Ship"
    model = "fighter"
    tp = "GT_TROOP_POD"
    nb,  hull =  designNameBases[1]+"%1d",   "SH_BASIC_MEDIUM"
    newTroopDesigns += [ (nb%(1),  desc,  hull,  3*[tp],  "",  model) ]
    nb,  hull =  designNameBases[1]+"%1d",   "SH_ORGANIC"
    newTroopDesigns += [ (nb%(2),  desc,  hull,  ["SR_WEAPON_2"]+ 3*[tp],  "",  model) ]
    newTroopDesigns += [ (nb%(3),  desc,  hull,  ["SR_WEAPON_5"]+ 3*[tp],  "",  model) ]
    nb,  hull =  designNameBases[1]+"%1d",   "SH_STATIC_MULTICELLULAR"
    newTroopDesigns += [ (nb%(4),  desc,  hull,  ["SR_WEAPON_2"]+ 4*[tp],  "",  model) ]
    newTroopDesigns += [ (nb%(5),  desc,  hull,  ["SR_WEAPON_5"]+ 4*[tp],  "",  model) ]
    
    ar1 = "AR_LEAD_PLATE"
    ar2= "AR_ZORTRIUM_PLATE"
    ar3= "AR_NEUTRONIUM_PLATE"
    arL=[ar1,  ar2,  ar3]
    
    for ari in [1, 2]: #naming below only works because skipping Lead armor
        nb,  hull =  designNameBases[ari+1]+"%1d-%1d",   "SH_ORGANIC"
        newTroopDesigns += [ (nb%(1,  ari),  desc,  hull,  ["SR_WEAPON_5",  arL[ari],  tp,  tp],  "",  model) ]
        newTroopDesigns += [ (nb%(2,  ari),  desc,  hull,  ["SR_WEAPON_8",  arL[ari],  tp,  tp],  "",  model) ]
        nb,  hull =  designNameBases[ari+1]+"%1d-%1d",   "SH_STATIC_MULTICELLULAR"
        newTroopDesigns += [ (nb%(3,  ari),  desc,  hull,  ["SR_WEAPON_5",  arL[ari],  tp,  tp,  tp],  "",  model) ]
        newTroopDesigns += [ (nb%(4,  ari),  desc,  hull,  ["SR_WEAPON_8",  arL[ari],  tp,  tp,  tp],  "",  model) ]
        nb,  hull =  designNameBases[ari+1]+"%1d-%1d",   "SH_ENDOMORPHIC"
        newTroopDesigns += [ (nb%(5,  ari),  desc,  hull,  ["SR_WEAPON_5",  arL[ari],  tp,  tp,  tp,  tp],  "",  model) ]
        newTroopDesigns += [ (nb%(6,  ari),  desc,  hull,  ["SR_WEAPON_8",  arL[ari],  tp,  tp,  tp,  tp],  "",  model) ]

    currentTurn=fo.currentTurn()
    needsAdding=[]
    namesToAdd=[]
    for name,  desc,  hull,  partslist,  icon,  model in newTroopDesigns:
        if name not in troopShipNames:
            needsAdding.append( ( name,  desc,  hull,  partslist,  icon,  model) )
            namesToAdd.append( name )

    if needsAdding != []:
        print "--------------"
        print "Current Troop Designs: %s"%troopShipNames
        print "-----------"
        print "Troop design names apparently needing to be added: %s"%namesToAdd
        print "-------"
        if currentTurn ==1:  #due to some apparent problem with these repeatedly being added, only do it on first turn
            for name,  desc,  hull,  partslist,  icon,  model in needsAdding:
                try:
                    res=fo.issueCreateShipDesignOrder( name,  desc,  hull,  partslist,  icon,  model, False)
                    print "added  Troop Design %s, with result %d"%(name,  res)
                except:
                    print "Error: exception triggered and caught adding troop %s:  "%name,  traceback.format_exc()

    bestShip,  bestDesign,  buildChoices = getBestShipInfo( EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_INVASION)
    if bestDesign:
        print "Best Troopship buildable is %s"%bestDesign.name(False)
    else:
        print "Troopships apparently unbuildable at present,  ruh-roh"

def addScoutDesigns():
    empire = fo.getEmpire()
    scoutDesignIDs=[]
    designNameBases= [key for key, val in sorted( shipTypeMap.get(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_EXPLORATION,  {"nomatch":0}).items(),  key=lambda x:x[1])]
    for baseName in designNameBases:
        scoutDesignIDs.extend(   [shipDesignID for shipDesignID in empire.allShipDesigns if baseName  in fo.getShipDesign(shipDesignID).name(False) ]  )
    scoutShipNames = [fo.getShipDesign(shipDesignID).name(False) for shipDesignID in scoutDesignIDs]
    #print "Current Scout Designs: %s"%scoutShipNames
    #                                                            name               desc            hull                partslist                              icon                 model

    newScoutDesigns = []
    desc = "Scout"
    model = "fighter"
    srb = "SR_WEAPON_%1d"
    nb,  hull =  designNameBases[1]+"-%1d-%1d",   "SH_ORGANIC"
    db = "DT_DETECTOR_%1d"
    #is1,  is2 = "FU_BASIC_TANK",  "ST_CLOAK_1"
    is1  = "FU_BASIC_TANK"
    is2 = "SH_DEFLECTOR"

    for id in [1, 2, 3, 4]:
        newScoutDesigns += [ (nb%(id, 0),  desc,  "SH_BASIC_SMALL",  [ db%id],  "",  model)   ]
    #for id in [1, 2, 3, 4]:
    #    newScoutDesigns += [ (nb%(id, iw),  desc,  hull,  [ db%id,  srb%iw, srb%iw,  is1],  "",  model)    for iw in range(1, 9) ]
    nb,  hull =  designNameBases[2]+"-%1d-%1d",   "SH_ENDOMORPHIC"
    for id in [1, 2, 3, 4]:
        #newScoutDesigns += [ (nb%(id, 0),  desc,  hull,  [ db%id,  "", "", "",  is1,  is1],  "",  model)  ]
        newScoutDesigns += [ (nb%(id, iw),  desc,  hull,  [ db%id,  srb%iw, srb%iw, srb%iw,  is2,  is2],  "",  model)    for iw in range(5, 9) ]

    currentTurn=fo.currentTurn()
    needsAdding=[]
    namesToAdd=[]
    for name,  desc,  hull,  partslist,  icon,  model in newScoutDesigns:
        if name not in scoutShipNames:
            needsAdding.append( ( name,  desc,  hull,  partslist,  icon,  model) )
            namesToAdd.append( name )

    if needsAdding != []:
        print "--------------"
        print "Current Scout Designs: %s"%scoutShipNames
        print "-----------"
        print "Scout design names apparently needing to be added: %s"%namesToAdd
        print "-------"
        if currentTurn ==1:  #due to some apparent problem with these repeatedly being added, only do it on first turn
            for name,  desc,  hull,  partslist,  icon,  model in needsAdding:
                try:
                    res=fo.issueCreateShipDesignOrder( name,  desc,  hull,  partslist,  icon,  model, False)
                    print "added  Scout Design %s, with result %d"%(name,  res)
                except:
                    print "Error: exception triggered and caught adding scout %s:  "%name,  traceback.format_exc()

    bestShip,  bestDesign,  buildChoices = getBestShipInfo( EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_EXPLORATION)
    if bestDesign:
        print "Best Scout buildable is %s"%bestDesign.name(False)
    else:
        print "Scouts apparently unbuildable at present,  ruh-roh"
    #TODO: add more advanced designs

def addOrbitalDefenseDesigns():
    shipType="OrbitalDefense"
    shipProdPriority=EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_ORBITAL_DEFENSE
    empire = fo.getEmpire()
    designIDs=[]
    designNameBases= [key for key, val in sorted( shipTypeMap.get(shipProdPriority,  {"nomatch":0}).items(),  key=lambda x:x[1])]
    for baseName in designNameBases:
        designIDs.extend(  [shipDesignID for shipDesignID in empire.allShipDesigns if baseName  in fo.getShipDesign(shipDesignID).name(False) ] )
    shipNames = [fo.getShipDesign(shipDesignID).name(False) for shipDesignID in designIDs]
    print "Current %s Designs: %s"%(shipType,  shipNames)
    newDesigns = []
    desc = "Orbital Defense Ship"
    model = "fighter"
    is1= "SH_DEFENSE_GRID"
    is2 = "SH_DEFLECTOR"
    is3= "SH_MULTISPEC"
    hull =  "SH_COLONY_BASE"
    if foAI.foAIstate.aggression<=fo.aggression.cautious:
        newDesigns += [ (designNameBases[0],  desc,  hull,  [is1],  "",  model) ]
    newDesigns += [ (designNameBases[1],  desc,  hull,  [is2],  "",  model) ]
    newDesigns += [ (designNameBases[2],  desc,  hull,  [is3],  "",  model) ]

    currentTurn=fo.currentTurn()
    needsAdding=[]
    namesToAdd=[]
    for name,  desc,  hull,  partslist,  icon,  model in newDesigns:
        if name not in shipNames:
            needsAdding.append( ( name,  desc,  hull,  partslist,  icon,  model) )
            namesToAdd.append( name )
    addDesigns(shipType,  namesToAdd,  needsAdding,  shipProdPriority)
    
def addMarkDesigns():
    addOrbitalDefenseDesigns()
    empire = fo.getEmpire()
    markDesignIDs = []
    designNameBases= [key for key, val in sorted( shipTypeMap.get(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY,  {"nomatch":0}).items(),  key=lambda x:x[1])]
    for baseName in designNameBases:
        markDesignIDs.extend(  [shipDesignID for shipDesignID in empire.allShipDesigns if baseName  in fo.getShipDesign(shipDesignID).name(False) ] )
    markShipNames = [fo.getShipDesign(shipDesignID).name(False) for shipDesignID in markDesignIDs]
    #print "Current Mark Designs: %s"%markShipNames
    #                                                            name               desc            hull                partslist                              icon                 model
    newMarkDesigns = []
    desc = "military ship"
    model = "fighter"
    srb = "SR_WEAPON_%1d"
    is1= "FU_BASIC_TANK"
    is2 = "SH_DEFLECTOR"
    is3= "SH_MULTISPEC"
    ar1 = "AR_LEAD_PLATE"
    ar2= "AR_ZORTRIUM_PLATE"
    ar3= "AR_NEUTRONIUM_PLATE"

    if foAI.foAIstate.aggression in [fo.aggression.beginner, fo.aggression.turtle]: 
        maxEM= 8
    elif foAI.foAIstate.aggression ==fo.aggression.cautious: 
        maxEM= 12
    else:
        maxEM= 10
    
    nb,  hull =  designNameBases[1]+"-%1d",   "SH_BASIC_MEDIUM"
    newMarkDesigns += [ (nb%iw,  desc,  hull,  [ srb%iw,  srb%iw,  ""],  "",  model)    for iw in range(1, 8) ]

    #newMarkDesigns += [ ((nb%iw)+'N',  desc,  hull,  [ srb%iw,  ar3,  ""],  "",  model)    for iw in range(1, 8) ]

    nb,  hull =  designNameBases[2]+"-1-%1d",   "SH_ORGANIC"
    newMarkDesigns += [ (nb%iw,  desc,  hull,  [ srb%iw,  srb%iw, srb%iw,  is1],  "",  model)    for iw in range(3, 9) ]
    nb,  hull =  designNameBases[2]+"-3-%1d",   "SH_STATIC_MULTICELLULAR"
    newMarkDesigns += [ (nb%iw,  desc,  hull,  [ srb%iw,  srb%iw, srb%iw,  is1,  is2],  "",  model)    for iw in range(6, 9) ]

    if doDoubleShields or (empire.empireID%2 == 0) :
        nb,  hull =  designNameBases[2]+"-2-%1x",   "SH_ORGANIC"
        newMarkDesigns += [ (nb%iw,  desc,  hull,  [ srb%iw,  srb%iw,  is2,  is2],  "",  model)    for iw in range(5, maxEM+1) ]
        nb,  hull =  designNameBases[2]+"-4-%1d",   "SH_STATIC_MULTICELLULAR"
        newMarkDesigns += [ (nb%iw,  desc,  hull,  [ srb%iw,  srb%iw, srb%iw,  is2,  is2],  "",  model)    for iw in range(6, maxEM+1) ]

    
    nb,  hull =  designNameBases[3]+"-1-%1x",   "SH_ENDOMORPHIC"
    newMarkDesigns += [ (nb%iw,  desc,  hull,  4*[srb%iw] + 2*[ is1],  "",  model)    for iw in [5, 6, 7 ] ]

    nb =  designNameBases[3]+"-2-%1x"
    newMarkDesigns += [ (nb%iw,  desc,  hull,  3*[srb%iw] + [ is2,  is2, is2],  "",  model)    for iw in range(6,  maxEM+1) ]

    nb =  designNameBases[3]+"3-%1x"
    #newMarkDesigns += [ (nb%iw,  desc,  hull,  2*[srb%iw]+[ar1] + [ is1,  is1],  "",  model)    for iw in [5, 6, 7, 8,   10, 11, 12 ] ]

    nb =  designNameBases[3]+"-4-%1x"
    #newMarkDesigns += [ (nb%iw,  desc,  hull,  2*[srb%iw]+[ar1] + [ is1, is2],  "",  model)    for iw in [7, 8,   10, 11, 12 ] ]

    nb =  designNameBases[3]+"-5-%1x"
    newMarkDesigns += [ (nb%iw,  desc,  hull,  3*[srb%iw]+[ar2] + 2*[ is2],  "",  model)    for iw in range(7,  maxEM+1) ]
    
    nb =  designNameBases[3]+"-6-%1x"
    if foAI.foAIstate.aggression <=fo.aggression.turtle: 
        newMarkDesigns += [ (nb%iw,  desc,  hull,  2*[srb%iw] +[ar3, ar3]+ 2*[ is2],  "",  model)    for iw in range(8,  maxEM+1) ]
    else:
        newMarkDesigns += [ (nb%iw,  desc,  hull,  3*[srb%iw] +[ar3]+ 2*[ is2],  "",  model)    for iw in range(8,  maxEM+1) ]

    nb =  designNameBases[3]+"-7-%1x"
    newMarkDesigns += [ (nb%iw,  desc,  hull,  3*[srb%iw] + [ is3,  is3, is3],  "",  model)    for iw in range(8,  maxEM+1) ]
    
    nb =  designNameBases[3]+"-8-%1x"
    newMarkDesigns += [ (nb%iw,  desc,  hull,  3*[srb%iw]+[ar2] + [ is3,  is3],  "",  model)    for iw in range(8,  maxEM+1) ]
    
    nb =  designNameBases[3]+"-9-%1x"
    newMarkDesigns += [ (nb%iw,  desc,  hull,  3*[srb%iw]+[ar3] + [ is3,  is3],  "",  model)    for iw in range(8,  maxEM+1) ]
    
    if foAI.foAIstate.aggression >=fo.aggression.typical: 
        nb,  hull =  designNameBases[4]+"-%1x-%1x",   "SH_ENDOSYMBIOTIC"
        newMarkDesigns += [ (nb%(1, iw),  desc,  hull,  4*[srb%iw] + 3*[ is2],  "",  model)    for iw in range(7,  15) ]
        newMarkDesigns += [ (nb%(2, iw),  desc,  hull,  4*[srb%iw] + 3*[ is3],  "",  model)    for iw in range(7,  15) ]
        
        newMarkDesigns += [ (nb%(3, iw),  desc,  hull,  3*[srb%iw]+[ar2] + 3*[ is2],  "",  model)    for iw in range(7,  14) ]
        newMarkDesigns += [ (nb%(4, iw),  desc,  hull,  3*[srb%iw]+[ar3] + 3*[ is2],  "",  model)    for iw in range(8,  14) ]
        newMarkDesigns += [ (nb%(5, iw),  desc,  hull,  3*[srb%iw]+[ar2] + 3*[ is3],  "",  model)    for iw in range(7,  14) ]
        newMarkDesigns += [ (nb%(6, iw),  desc,  hull,  3*[srb%iw]+[ar3] + 3*[ is3],  "",  model)    for iw in range(8,  14) ]
        #bioadaptive
        nb,  hull =  designNameBases[4]+"-%1x-%1x",   "SH_BIOADAPTIVE"
        newMarkDesigns += [ (nb%(7, iw),  desc,  hull,  3*[srb%iw] + 3*[ is2],  "",  model)    for iw in range(9,  14) ]
        newMarkDesigns += [ (nb%(8, iw),  desc,  hull,  2*[srb%iw]+[ar2] + 3*[ is2],  "",  model)    for iw in range(9,  14) ]
        newMarkDesigns += [ (nb%(9, iw),  desc,  hull,  3*[srb%iw] + 3*[ is3],  "",  model)    for iw in range(9,  14) ]
        newMarkDesigns += [ (nb%(10, iw),  desc,  hull,  2*[srb%iw]+[ar3] + 3*[ is3],  "",  model)    for iw in range(9,  14) ]
    if foAI.foAIstate.aggression >fo.aggression.typical: 
        nb,  hull =  designNameBases[5]+"-%1x-%02x",   "SH_SENTIENT"
        newMarkDesigns += [ (nb%(1, iw),  desc,  hull,  4*[srb%iw]+2*[ar2] + 3*[ is2],  "",  model)    for iw in range(10,  18) ]
        newMarkDesigns += [ (nb%(2, iw),  desc,  hull,  5*[srb%iw]+[ar3] + 3*[ is2],  "",  model)    for iw in range(10,  18) ]
        newMarkDesigns += [ (nb%(3, iw),  desc,  hull,  5*[srb%iw]+[ar2] + 3*[ is3],  "",  model)    for iw in range(10,  18) ]
        newMarkDesigns += [ (nb%(4, iw),  desc,  hull,  5*[srb%iw]+[ar3] + 3*[ is3],  "",  model)    for iw in range(10,  18) ]

    if foAI.foAIstate.aggression >fo.aggression.typical: 
        nb,  hull =  designNameBases[6]+"-%1x-%02x",   "SH_FRACTAL_ENERGY"
        newMarkDesigns += [ (nb%(1, iw),  desc,  hull,  8*[srb%iw]+3*[ar2] + 3*[ is2],  "",  model)    for iw in range(10,  18) ]
        newMarkDesigns += [ (nb%(2, iw),  desc,  hull,  9*[srb%iw]+3*[ar3] + 2*[ is2],  "",  model)    for iw in range(10,  18) ]
        newMarkDesigns += [ (nb%(3, iw),  desc,  hull,  8*[srb%iw]+3*[ar2] + 3*[ is3],  "",  model)    for iw in range(10,  18) ]
        newMarkDesigns += [ (nb%(4, iw),  desc,  hull,  9*[srb%iw]+3*[ar3] + 2*[ is3],  "",  model)    for iw in range(10,  18) ]


    currentTurn=fo.currentTurn()
    needsAdding=[]
    namesToAdd=[]
    for name,  desc,  hull,  partslist,  icon,  model in newMarkDesigns:
        if name not in markShipNames:
            needsAdding.append( ( name,  desc,  hull,  partslist,  icon,  model) )
            namesToAdd.append( name )

    if needsAdding != []:
        print "--------------"
        print "Current Mark Designs: %s"%markShipNames
        print "-----------"
        print "Mark design names apparently needing to be added: %s"%namesToAdd
        print "-------"
        if currentTurn ==1:  #due to some apparent problem with these repeatedly being added, only do it on first turn
            for name,  desc,  hull,  partslist,  icon,  model in needsAdding:
                try:
                    res=fo.issueCreateShipDesignOrder( name,  desc,  hull,  partslist,  icon,  model,  False)
                    print "added  Mark Design %s, with result %d"%(name,  res)
                except:
                    print "Error: exception triggered and caught adding mark %s:  "%name,  traceback.format_exc()

    bestShip,  bestDesign,  buildChoices = getBestShipInfo( EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY)
    if bestDesign:
        print "Best Mark buildable is %s"%bestDesign.name(False)
    else:
        print "Marks apparently unbuildable at present,  ruh-roh"
    #TODO: add more advanced designs

def addOutpostDesigns():
    empire = fo.getEmpire()
    outpostDesignIDs = []
    designNameBases= [key for key, val in sorted( shipTypeMap.get(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_OUTPOST,  {"nomatch":0}).items(),  key=lambda x:x[1])]
    for baseName in designNameBases:
        outpostDesignIDs.extend( [shipDesignID for shipDesignID in empire.allShipDesigns if baseName in fo.getShipDesign(shipDesignID).name(False) ] )
    outpostShipNames = [fo.getShipDesign(shipDesignID).name(False) for shipDesignID in outpostDesignIDs]
    #print "Current Outpost Designs: %s"%scoutShipNames
    #                                                            name               desc            hull                partslist                              icon                 model
    newOutpostDesigns = []
    desc = "Outpost Ship"
    srb = "SR_WEAPON_%1d"
    model = "seed"
    nb,  hull =  designNameBases[1]+"%1d_%1d",   "SH_ORGANIC"
    op = "CO_OUTPOST_POD"
    db = "DT_DETECTOR_%1d"
    is1,  is2 = "FU_BASIC_TANK",  "ST_CLOAK_1"
    for id in [1, 2]:
        newOutpostDesigns += [ (nb%(id, iw),  desc,  hull,  [ srb%iw, db%id, "",  op],  "",  model)    for iw in [2, 4, 6, 8] ]

    currentTurn=fo.currentTurn()
    needsAdding=[]
    namesToAdd=[]
    for name,  desc,  hull,  partslist,  icon,  model in newOutpostDesigns:
        if name not in outpostShipNames:
            needsAdding.append( ( name,  desc,  hull,  partslist,  icon,  model) )
            namesToAdd.append( name )

    if needsAdding != []:
        print "--------------"
        print "Current Outpost Designs: %s"%outpostShipNames
        print "-----------"
        print "Outpost design names apparently needing to be added: %s"%namesToAdd
        print "-------"
        if currentTurn ==1:  #due to some apparent problem with these repeatedly being added, only do it on first turn
            for name,  desc,  hull,  partslist,  icon,  model in needsAdding:
                try:
                    res=fo.issueCreateShipDesignOrder( name,  desc,  hull,  partslist,  icon,  model,  False)
                    print "added  Outposter Design %s, with result %d"%(name,  res)
                except:
                    print "Error: exception triggered and caught adding outposter %s:  "%name,  traceback.format_exc()

    bestShip,  bestDesign,  buildChoices = getBestShipInfo( EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_OUTPOST)
    if bestDesign:
        print "Best Outpost buildable is %s"%bestDesign.name(False)
    else:
        print "Outpost ships apparently unbuildable at present,  ruh-roh"

def addColonyDesigns():
    empire = fo.getEmpire()
    colonyDesignIDs = []
    designNameBases= [key for key, val in sorted( shipTypeMap.get(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION,  {"nomatch":0}).items(),  key=lambda x:x[1])]
    for baseName in designNameBases:
        colonyDesignIDs.extend(  [shipDesignID for shipDesignID in empire.allShipDesigns if baseName  in fo.getShipDesign(shipDesignID).name(False) ] )
    colonyShipNames = [fo.getShipDesign(shipDesignID).name(False) for shipDesignID in colonyDesignIDs]
    #print "Current Outpost Designs: %s"%scoutShipNames
    #                                                            name               desc            hull                partslist                              icon                 model
    newColonyDesigns = []
    desc = "Colony Ship"
    model = "seed"
    srb = "SR_WEAPON_%1d"
    nb,  hull =  designNameBases[1]+"%1d_%1d",   "SH_ORGANIC"
    cp = "CO_COLONY_POD"
    db = "DT_DETECTOR_%1d"
    is1,  is2 = "FU_BASIC_TANK",  "ST_CLOAK_1"
    for id in [1, 2, 3]:
        newColonyDesigns += [ (nb%(id, iw),  desc,  hull,  [ srb%iw, srb%iw, db%id,  cp],  "",  model)    for iw in [1, 2, 5, 8] ]

    nb =  designNameBases[2]+"%1d_%1d"
    cp2 = "CO_SUSPEND_ANIM_POD"
    for id in [1, 2, 3]:
        newColonyDesigns += [ (nb%(id, iw),  desc,  hull,  [ srb%iw, db%id, "",  cp2],  "",  model)    for iw in [5, 6, 7, 8]  ]

    currentTurn=fo.currentTurn()
    needsAdding=[]
    namesToAdd=[]
    for name,  desc,  hull,  partslist,  icon,  model in newColonyDesigns:
        if name not in colonyShipNames:
            needsAdding.append( ( name,  desc,  hull,  partslist,  icon,  model) )
            namesToAdd.append( name )

    if needsAdding != []:
        print "--------------"
        print "Current Colony  Designs: %s"%colonyShipNames
        print "-----------"
        print "Colony design names apparently needing to be added: %s"%namesToAdd
        print "-------"
        if currentTurn ==1:  #due to some apparent problem with these repeatedly being added, only do it on first turn
            for name,  desc,  hull,  partslist,  icon,  model in needsAdding:
                try:
                    res=fo.issueCreateShipDesignOrder( name,  desc,  hull,  partslist,  icon,  model,  False)
                    print "added  Colony Design %s, with result %d"%(name,  res)
                except:
                    print "Error: exception triggered and caught adding colony  %s:  "%name,  traceback.format_exc()

    bestShip,  bestDesign,  buildChoices = getBestShipInfo( EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION)
    if bestDesign:
        print "Best Colony ship buildable is %s"%bestDesign.name(False)
    else:
        print "Colony ships apparently unbuildable at present,  ruh-roh"

    
def generateProductionOrders():
    "generate production orders"
    empire = fo.getEmpire()
    universe = fo.getUniverse()
    capitolID = PlanetUtilsAI.getCapital()
    if capitolID == None:
        homeworld=None
        capitolSysID=None
    else:
        homeworld = universe.getPlanet(capitolID)
        capitolSysID = homeworld.systemID
    print "Production Queue Management:"
    empire = fo.getEmpire()
    productionQueue = empire.productionQueue
    totalPP = empire.productionPoints
    currentTurn = fo.currentTurn()
    print ""
    print "  Total Available Production Points: " + str(totalPP)
    
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
        bestShip,  bestDesign,  buildChoices = getBestShipInfo(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_EXPLORATION)
        if bestShip:
            retval  = fo.issueEnqueueShipProductionOrder(bestShip, buildChoices[0])
            if retval:
                fo.issueChangeProductionQuantityOrder(productionQueue.size -1,  1,  2)
        fo.updateProductionQueue()
    movedCapital=False
    bldgExpense=0.0
    bldgRatio = [ 0.4,  0.35,  0.30  ][fo.empireID()%3]
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
                res=fo.issueEnqueueBuildingProductionOrder("BLD_INDUSTRY_CENTER", empire.capitalID)
                print "Enqueueing BLD_INDUSTRY_CENTER, with result %d"%res
                if res: 
                    cost,  time =   empire.productionCostAndTime( productionQueue[productionQueue.size -1]  )
                    bldgExpense += cost/time

            if  ("BLD_SHIPYARD_BASE" in possibleBuildingTypes) and ("BLD_SHIPYARD_BASE" not in (capitalBldgs+queuedBldgNames)):
                try:
                    res=fo.issueEnqueueBuildingProductionOrder("BLD_SHIPYARD_BASE", empire.capitalID)
                    print "Enqueueing BLD_SHIPYARD_BASE, with result %d"%res
                except:
                    print "Error: cant build shipyard at new capital,  probably no population; we're hosed"
                    print "Error: exception triggered and caught:  ",  traceback.format_exc()

            for bldName in [ "BLD_SHIPYARD_ORG_ORB_INC" ]:
                if  (bldName in possibleBuildingTypes) and (bldName not in (capitalBldgs+queuedBldgNames)) and (bldgExpense<bldgRatio*totalPP):
                    try:
                        res=fo.issueEnqueueBuildingProductionOrder(bldName, empire.capitalID)
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
                        res=fo.issueEnqueueBuildingProductionOrder(bldName, empire.capitalID)
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
                        res=fo.issueEnqueueBuildingProductionOrder("BLD_EXOBOT_SHIP", empire.capitalID)
                        print "Enqueueing BLD_EXOBOT_SHIP, with result %d"%res
                        if res:
                            res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                            print "Requeueing %s to front of build queue, with result %d"%("BLD_EXOBOT_SHIP",  res)
                    except:
                        print "Error: exception triggered and caught:  ",  traceback.format_exc()

            if  ("BLD_IMPERIAL_PALACE" in possibleBuildingTypes) and ("BLD_IMPERIAL_PALACE" not in (capitalBldgs+queuedBldgNames)):
                res=fo.issueEnqueueBuildingProductionOrder("BLD_IMPERIAL_PALACE", empire.capitalID)
                print "Enqueueing BLD_IMPERIAL_PALACE, with result %d"%res
                if res:
                    res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                    print "Requeueing BLD_IMPERIAL_PALACE to front of build queue, with result %d"%res


            if  ("BLD_NEUTRONIUM_SYNTH" in possibleBuildingTypes) and ("BLD_NEUTRONIUM_SYNTH" not in (capitalBldgs+queuedBldgNames)):
                res=fo.issueEnqueueBuildingProductionOrder("BLD_NEUTRONIUM_SYNTH", empire.capitalID)
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
        orbitalDefenseNames = shipTypeNames( EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_ORBITAL_DEFENSE )
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
                    #print "Found %d existing Orbital Defenses in %s :"%(foAI.foAIstate.fleetStatus.get(fid,  {}).get('nships', 0),   PlanetUtilsAI.sysNameIDs([sysID]))
                    sysOrbitalDefenses[sysID] += foAI.foAIstate.fleetStatus.get(fid,  {}).get('nships', 0)
            for pid in ColonisationAI.empireSpeciesSystems.get(sysID, {}).get('pids',  []):
                sysOrbitalDefenses[sysID] += queuedDefenses.get(pid,  0)
            if sysOrbitalDefenses[sysID] < targetOrbitals:
                numNeeded =  targetOrbitals - sysOrbitalDefenses[sysID] 
                for pid in ColonisationAI.empireSpeciesSystems.get(sysID, {}).get('pids',  []):
                    bestShip,  colDesign,  buildChoices = getBestShipInfo(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_ORBITAL_DEFENSE,  pid)
                    if not bestShip:
                        print "no orbital defenses can be built at ",  PlanetUtilsAI.planetNameIDs([pid])
                        continue
                    #print "selecting  ",  PlanetUtilsAI.planetNameIDs([pid]),  " to build Orbital Defenses"
                    retval  = fo.issueEnqueueShipProductionOrder(bestShip, pid)
                    print "queueing %d Orbital Defenses at %s"%(numNeeded,  PlanetUtilsAI.planetNameIDs([pid]))
                    if retval !=0:
                        if numNeeded > 1  :
                            fo.issueChangeProductionQuantityOrder(productionQueue.size -1,  1, numNeeded )
                        cost,  time =   empire.productionCostAndTime( productionQueue[productionQueue.size -1]  )
                        defenseAllocation += productionQueue[productionQueue.size -1].blocksize *  cost/time
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
                        break

    queuedShipyardLocs = [element.locationID for element in productionQueue if (element.name=="BLD_SHIPYARD_BASE") ]
    systemColonies={}
    colonySystems={} 
    for specName in ColonisationAI.empireColonizers:
        if (len( ColonisationAI.empireColonizers[specName])==0) and (specName in ColonisationAI.empireSpecies): #no current shipyards for this species#TODO: also allow orbital incubators and/or asteroid ships
            for pID in ColonisationAI.empireSpecies.get(specName, []): #SP_EXOBOT may not actually have a colony yet but be in empireColonizers
                if pID in queuedShipyardLocs:
                    break
            else:  #no queued shipyards, get planets with target pop >3, and queue a shipyard on the one with biggest current pop
                planetList = zip( map( universe.getPlanet,  ColonisationAI.empireSpecies[specName]),  ColonisationAI.empireSpecies[specName] )
                pops = sorted(  [ (planet.currentMeterValue(fo.meterType.population), pID) for planet, pID in planetList if ( planet and planet.currentMeterValue(fo.meterType.targetPopulation)>=3.0)] )
                if pops != []:
                    buildLoc= pops[-1][1]
                    res=fo.issueEnqueueBuildingProductionOrder("BLD_SHIPYARD_BASE", buildLoc)
                    print "Enqueueing BLD_SHIPYARD_BASE at planet %d (%s) for colonizer species %s, with result %d"%(buildLoc, universe.getPlanet(buildLoc).name,  specName,   res)
        for pid in ColonisationAI.empireSpecies.get(specName, []): 
            planet=universe.getPlanet(pid)
            if planet:
                systemColonies.setdefault(planet.systemID,  {}).setdefault('pids', []).append(pid)
                colonySystems[pid]=planet.systemID
    
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

    for bldName in [ "BLD_SHIPYARD_BASE" ]:
        if empire.buildingTypeAvailable(bldName) and (bldgExpense<bldgRatio*totalPP) and ( totalPP >50 or currentTurn > 80 ):
            queuedBldLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
            bldType = fo.getBuildingType(bldName)
            for pid in enrgyShipyardLocs:
                if  pid not in queuedBldLocs and bldType.canBeProduced(empire.empireID,  pid):#TODO: verify that canBeProduced() checks for prexistence of a barring building
                    res=fo.issueEnqueueBuildingProductionOrder(bldName, pid)
                    print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName,  pid, universe.getPlanet(pid).name,  res)
                    if res: 
                        queuedBldLocs.append(pid)
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
                        try:
                            distanceMap[sysID] = len(universe.leastJumpsPath(homeworld.systemID, sysID, empire.empireID))
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
                        print "problem queueing BLD_SOL_ORB_GEN at planet",  useloc,  "of system ",  useSys
                        pass

    bldName = "BLD_ART_BLACK_HOLE"
    if ( ( empire.buildingTypeAvailable(bldName) ) and (foAI.foAIstate.aggression > fo.aggression.typical) and 
                        (len( AIstate.empireStars.get(fo.starType.blackHole,  [])) == 0 )  and   (len( AIstate.empireStars.get(fo.starType.red,  [])) > 0)   ):
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
                    try:
                        distanceMap[sysID] = len(universe.leastJumpsPath(homeworld.systemID, sysID, empire.empireID))
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
                        print "problem queueing %s at planet"%bldName,  useloc,  "of system ",  useSys

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
                    try:
                        distanceMap[sysID] = len(universe.leastJumpsPath(homeworld.systemID, sysID, empire.empireID))
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
                    print "problem queueing BLD_BLACK_HOLE_POW_GEN at planet",  useloc,  "of system ",  useSys
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
                    try:
                        distanceMap[sysID] = len(universe.leastJumpsPath(homeworld.systemID, sysID, empire.empireID))
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
                    print "problem queueing BLD_NEUTRONIUM_EXTRACTOR at planet",  useloc,  "of system ",  useSys
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

    bldName = "BLD_CONC_CAMP"
    bldType = fo.getBuildingType(bldName)
    for pid in AIstate.popCtrIDs:
        planet=universe.getPlanet(pid)
        if not planet:
            continue
        tPop = planet.currentMeterValue(fo.meterType.targetPopulation)
        tInd=planet.currentMeterValue(fo.meterType.targetIndustry)
        cInd=planet.currentMeterValue(fo.meterType.industry)
        cPop = planet.currentMeterValue(fo.meterType.population)
        if (cPop < 23) or cPop < 0.8*tPop:  #check even if not aggressive, etc, just in case acquired planet with a ConcCamp on it
            for bldg in planet.buildingIDs:
                if universe.getObject(bldg).buildingTypeName  == bldName:
                    res=fo.issueScrapOrder( bldg)
                    print "Tried scrapping %s at planet %s,  got result %d"%(bldName,  planet.name,  res)
        elif foAI.foAIstate.aggression>fo.aggression.typical and empire.buildingTypeAvailable(bldName) and (tPop >= 32) :
            if  (planet.focus== EnumsAI.AIFocusType.FOCUS_GROWTH) or ("COMPUTRONIUM_SPECIAL" in planet.specials):
                #continue
                pass  # now that focus setting takes these into account, probably works ok to have conc camp
            queuedBldLocs = [element.locationID for element in productionQueue if (element.name==bldName) ]
            if (cPop >=0.95*tPop):# and cInd < 1.5* tInd:
                if  pid not in queuedBldLocs and bldType.canBeProduced(empire.empireID,  pid):#TODO: verify that canBeProduced() checks for prexistence of a barring building
                    #if planet.focus not in [ EnumsAI.AIFocusType.FOCUS_INDUSTRY ]:
                    #     fo.issueChangeFocusOrder(pid, EnumsAI.AIFocusType.FOCUS_INDUSTRY)
                    res=fo.issueEnqueueBuildingProductionOrder(bldName, pid)
                    print "Enqueueing %s at planet %d (%s) , with result %d"%(bldName,  pid, universe.getPlanet(pid).name,  res)
                    if res: 
                        queuedBldLocs.append(pid)
                        res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front

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
    fo.updateProductionQueue()
    for queue_index  in range( len(productionQueue)):
        element=productionQueue[queue_index]
        blockStr = ["",  "in blocks of %d "%element.blocksize][element.blocksize>1]
        print "    " + element.name+blockStr + " turns:" + str(element.turnsLeft) + " PP:%.2f"%element.allocation + " being built at " + universe.getObject(element.locationID).name
        if element.turnsLeft == -1:
            print "element %s will never be completed as stands  "%element.name 
            #fo.issueDequeueProductionOrder(queue_index) 
            break
        elif element.buildType == EnumsAI.AIEmpireProductionTypes.BT_SHIP:
             if foAI.foAIstate.getShipRole(element.designID) ==       EnumsAI.AIShipRoleType.SHIP_ROLE_CIVILIAN_COLONISATION:
                 thisSpec=universe.getPlanet(element.locationID).speciesName
                 queuedColonyShips[thisSpec] =  queuedColonyShips.get(thisSpec, 0) +  element.remaining*element.blocksize
             if foAI.foAIstate.getShipRole(element.designID) ==       EnumsAI.AIShipRoleType.SHIP_ROLE_CIVILIAN_OUTPOST:
                 queuedOutpostShips+=  element.remaining*element.blocksize
             if foAI.foAIstate.getShipRole(element.designID) ==       EnumsAI.AIShipRoleType.SHIP_ROLE_MILITARY_INVASION:
                 queuedTroopShips+=  element.remaining*element.blocksize
    if queuedColonyShips:
        print "\nFound  colony ships in build queue: %s"%queuedColonyShips
    if queuedOutpostShips:
        print "\nFound  colony ships in build queue: %s"%queuedOutpostShips
        
    allTroopFleetIDs =  FleetUtilsAI.getEmpireFleetIDsByRole(EnumsAI.AIFleetMissionType.FLEET_MISSION_INVASION )
    nTroopTot = sum( [ foAI.foAIstate.fleetStatus.get(fid,  {}).get('nships', 0) for fid in allTroopFleetIDs ] )
    availTroopFleetIDs = list( FleetUtilsAI.extractFleetIDsWithoutMissionTypes(allTroopFleetIDs))
    nAvailTroopTot = sum( [ foAI.foAIstate.fleetStatus.get(fid,  {}).get('nships', 0) for fid in availTroopFleetIDs ] )
    print "Trooper Status: %d total,  with %d unassigned.  %d queued"%(nTroopTot,  nAvailTroopTot,  queuedTroopShips)
    if ( capitolID!=None and currentTurn>=40 and foAI.foAIstate.systemStatus.get(capitolSysID,  {}).get('fleetThreat', 0)==0   and 
                                                                                           foAI.foAIstate.systemStatus.get(capitolSysID,  {}).get('neighborThreat', 0)==0):
        bestShip,  bestDesign,  buildChoices = getBestShipInfo( EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_INVASION)
        if  buildChoices!=None and  len(buildChoices)>0: 
            loc = random.choice(buildChoices)
            prodTime = bestDesign.productionTime(empire.empireID,  loc)
            prodCost=bestDesign.productionCost(empire.empireID,  loc)
            troopersNeededForcing = max(0,  int( 0.99+  (currentTurn/20 - nAvailTroopTot)/max(2, prodTime-1)) )
            numShips=troopersNeededForcing
            perTurnCost = (float(prodCost) / prodTime)
            if troopersNeededForcing>0  and totalPP > 3*perTurnCost*queuedTroopShips:
                retval  = fo.issueEnqueueShipProductionOrder(bestShip, loc)
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
    bestShip,  bestDesign,  buildChoices = getBestShipInfo(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION)
    colonyBuildLocs=[]
    speciesMap = {}
    for loc in (buildChoices or []):
        thisSpec = universe.getPlanet(loc).speciesName
        speciesMap.setdefault(thisSpec,  []).append( loc)
    colonyBuildChoices=[]
    for pid,  score_Spec_tuple in foAI.foAIstate.colonisablePlanetIDs:
        score,  thisSpec = score_Spec_tuple
        colonyBuildChoices.extend( int(math.ceil(score))*speciesMap.get(thisSpec,  [])  )

    numOutpostTargs=len(foAI.foAIstate.colonisableOutpostIDs )
    numOutpostFleets=len( FleetUtilsAI.getEmpireFleetIDsByRole( EnumsAI.AIFleetMissionType.FLEET_MISSION_OUTPOST) )# counting existing outpost fleets each as one ship
    totOutpostFleets = queuedOutpostShips + numOutpostFleets
    
    #maxColonyFleets = max(  min( numColonyTargs+1+currentTurn/10 ,  numTotalFleets/4  ),  3+int(3*len(empireColonizers)))
    #maxOutpostFleets = min(numOutpostTargs+1+currentTurn/10,  numTotalFleets/4  )  
    maxColonyFleets = PriorityAI.allottedColonyTargets 
    maxOutpostFleets = maxColonyFleets

    print "Production Queue Priorities:"
    filteredPriorities = {}
    for ID,  score in sortedPriorities:
        if topscore < score:
            topPriority = ID
            topscore = score #don't really need topscore nor sorting with current handling
        print " Score: %4d -- %s "%(score,  EnumsAI.AIPriorityNames[ID] )
        if ID != EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_BUILDINGS:
            if ( ID == EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION ) and (  totColonyFleets <  maxColonyFleets) and len(colonyBuildChoices) >0:
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
        
    while (availPP > 0) and (loopCount <100) and (priorityChoices != [] ): #make sure don't get stuck in some nonbreaking loop like if all shipyards captured
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
        bestShip,  bestDesign,  buildChoices = bestShips[thisPriority]
        if makingColonyShip:
            loc = random.choice(colonyBuildChoices)
            bestShip,  bestDesign,  buildChoices = getBestShipInfo(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_COLONISATION,  loc)
        else:
            loc = random.choice(buildChoices)
        numShips=1
        perTurnCost = (float(bestDesign.productionCost(empire.empireID,  loc)) / bestDesign.productionTime(empire.empireID,  loc))
        if  thisPriority==EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_MILITARY : #TODO: consider whether to allow multiples of colony  ships; if not, priority sampling gets skewed
            while ( totalPP > 40*perTurnCost):
                numShips *= 2
                perTurnCost *= 2
        retval  = fo.issueEnqueueShipProductionOrder(bestShip, loc)
        if retval !=0:
            print "adding %d new ship(s) to production queue:  %s; per turn production cost %.1f"%(numShips,  bestDesign.name(True),  perTurnCost)
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
                if leadingBlockPP > 0.5* totalPP:
                    res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front
    print ""
    fo.updateProductionQueue()

def getAvailableBuildLocations(shipDesignID):
    "returns locations where shipDesign can be built"
    result = []
    #systemIDs = foAI.foAIstate.getExplorableSystems(AIExplorableSystemType.EXPLORABLE_SYSTEM_EXPLORED)
    #planetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(systemIDs)
    shipDesign = fo.getShipDesign(shipDesignID)
    empire = fo.getEmpire()
    empireID = empire.empireID
    capitolID = PlanetUtilsAI.getCapital()
    planetIDs = set([capitolID]  + list(AIstate.popCtrIDs)) #TODO: restrict to planets with shipyards
    for planetID in planetIDs:
        if shipDesign.productionLocationForEmpire(empireID, planetID):
            result.append(planetID)
    return result

def spentPP():
    "calculate PPs spent this turn so far"

    queue = fo.getEmpire().productionQueue
    return queue.totalSpent


