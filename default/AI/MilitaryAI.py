import freeOrionAIInterface as fo
import FreeOrionAI as foAI
import AIstate
import AITarget
from EnumsAI import AIFleetMissionType, AITargetType
import FleetUtilsAI
import PlanetUtilsAI
from random import choice,  random
import ExplorationAI
import PriorityAI
import ProductionAI
import ColonisationAI

MinThreat = 10 # the minimum threat level that will be ascribed to an unkown threat capable of killing scouts
MilitaryAllocations = []
minMilAllocations = {}
totMilRating=0
milShips=0

def tryAgain(milFleetIDs,  tryReset=False,  round=""):
    for fid in milFleetIDs:
        thisMission=foAI.foAIstate.getAIFleetMission(fid)
        thisMission.clearAIFleetOrders()
        thisMission.clearAITargets(-1)
    getMilitaryFleets(tryReset=tryReset,  round=round)
    return
    
def ratingNeeded(target,  current=0):
    if current >= target:
        return 0
    else:
        return target + current - (4*target*current)**0.5

def getMilitaryFleets(milFleetIDs=None,  tryReset=True,  round="Main"):
    "get armed military fleets"
    global MilitaryAllocations, totMilRating

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    capitalID = PlanetUtilsAI.getCapital()
    if capitalID == None:
        homeworld=None
    else:
        homeworld = universe.getPlanet(capitalID)
    if homeworld:
        homeSystemID = homeworld.systemID
    else:
        homeSystemID=-1

    if milFleetIDs !=None:
        allMilitaryFleetIDs = milFleetIDs
    else:
        allMilitaryFleetIDs =  FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_MILITARY )
    if tryReset and ((fo.currentTurn()+empireID) % 10 ==0) and round=="Main":
        tryAgain(allMilitaryFleetIDs,  tryReset=False,  round = round+" Reset")
    
    milShips = 0
    for fid in allMilitaryFleetIDs:
        milShips += foAI.foAIstate.fleetStatus.get(fid,  {}).get('nships', 0)
        
    thisTotMilRating = sum(  map(lambda x: foAI.foAIstate.getRating(x).get('overall', 0),  allMilitaryFleetIDs   )  )
    if "Main" in round:
        totMilRating = thisTotMilRating
        print "=================================================="
        print "%s Round Total Military Rating: %d"%(round,  totMilRating)
        print "---------------------------------"
        foAI.foAIstate.militaryRating=totMilRating
        
    milFleetIDs = list( FleetUtilsAI.extractFleetIDsWithoutMissionTypes(allMilitaryFleetIDs))
    availMilRating = sum(  map(lambda x: foAI.foAIstate.getRating(x).get('overall', 0),  milFleetIDs   )  )
    if "Main" in round:
        print "=================================================="
        print "%s Round Available Military Rating: %d"%(round,  availMilRating)
        print "---------------------------------"
    remainingMilRating = availMilRating
    allocations = []
    allocationGroups={}
    
    if milFleetIDs == []:
        if "Main" in round:
            MilitaryAllocations = []
        return []
        
    #for each system, get total rating of fleets assigned to it
    alreadyAssignedRating={}
    assignedAttack={}
    assignedHP={}
    for sysID in universe.systemIDs:
        assignedAttack[sysID]=0
        assignedHP[sysID]=0
    for fleetID in [fid for fid in allMilitaryFleetIDs if fid not in milFleetIDs]:
        aiFleetMission = foAI.foAIstate.getAIFleetMission(fleetID)
        sysTargets= []
        for aiFleetMissionType in aiFleetMission.getAIMissionTypes():
            aiTargets = aiFleetMission.getAITargets(aiFleetMissionType)
            for aiTarget in aiTargets:
                sysTargets.extend(aiTarget.getRequiredSystemAITargets())
        if not sysTargets: #shouldn't really be possible
            continue
        lastSys = sysTargets[-1].getTargetID() # will count this fleet as assigned to last system in target list
        assignedAttack[lastSys] +=  foAI.foAIstate.getRating(fleetID).get('attack', 0)
        assignedHP[lastSys] +=  foAI.foAIstate.getRating(fleetID).get('health', 0)
    for sysID in universe.systemIDs:
        mydefenses =  foAI.foAIstate.systemStatus.get(sysID,  {}).get( 'mydefenses', {} ) 
        mypattack = mydefenses.get('attack', 0)
        myphealth = mydefenses.get('health',  0)
        alreadyAssignedRating[sysID] = ( assignedAttack[sysID]  + mypattack ) * ( assignedHP[sysID] + myphealth )

    # get systems to defend
    capitalID = PlanetUtilsAI.getCapital()
    if capitalID != None:
        capitalPlanet = universe.getPlanet(capitalID)
    else:
        capitalPlanet=None
    #TODO: if no owned planets try to capture one!
    if  capitalPlanet:  
        capitalSysID = capitalPlanet.systemID
    else: # should be rare, but so as to not break code below, pick a randomish  mil-centroid  system
        capitalSysID=None #unless we can find one to use
        systemDict = {}
        for fleetID in allMilitaryFleetIDs:
            status = foAI.foAIstate.fleetStatus.get(fleetID,  None)
            if status is not None:
                sysID = status['sysID']
                if len( list( universe.getSystem(sysID).planetIDs  ) ) ==0:
                    continue
                systemDict[sysID] = systemDict.get( sysID,  0) + status.get('rating',  {}).get('overall',  0)
        rankedSystems = sorted( [(val,  sysID) for sysID, val in systemDict.items()  ]   )
        if rankedSystems:
            capitalSysID = rankedSystems[-1][-1]
        else:
            try:
                capitalSysID = foAI.foAIstate.fleetStatus.items()[0][1]['sysID']
            except:
                pass

    if False:
        if fo.currentTurn() < 20:
            threatBias = 0
        elif fo.currentTurn() < 40:
            threatBias = 10
        elif fo.currentTurn() < 60:
            threatBias = 80
        elif fo.currentTurn() < 80:
            threatBias = 200
        else:
            threatBias = 400
    else:
        threatBias = 0

    safetyFactor = [ 4.0,  3.0,  1.5,  1.0,  1.0,  0.95    ][foAI.foAIstate.aggression] 
    
    topTargetPlanets = [pid for pid, pscore, trp in AIstate.invasionTargets[:PriorityAI.allottedInvasionTargets]  if pscore > 20]  + [pid for pid,  pscore in foAI.foAIstate.colonisablePlanetIDs[:10]  if pscore > 20] 
    topTargetSystems = []
    for sysID in   AIstate.invasionTargetedSystemIDs + PlanetUtilsAI.getSystems(  topTargetPlanets  ):
        if sysID not in topTargetSystems:
            topTargetSystems.append(sysID) #doing this rather than set, to preserve order

    # allocation format: ( sysID, newAllocation, takeAny, maxMultiplier )    
    #================================
    #--------Capital Threat ----------
    capitalThreat = safetyFactor*(2* threatBias +sum( [ foAI.foAIstate.systemStatus[capitalSysID][thrtKey] for thrtKey in ['totalThreat',  'neighborThreat']] ))
    neededRating = ratingNeeded(1.4*capitalThreat,  alreadyAssignedRating[capitalSysID])
    newAlloc=0
    if tryReset:
        if (neededRating > 0.5*availMilRating) :
            tryAgain(allMilitaryFleetIDs)
            return
    if neededRating > 0:
        newAlloc = min(remainingMilRating,  neededRating )
        allocations.append( ( capitalSysID,  newAlloc,  True,  2*capitalThreat)  ) 
        allocationGroups.setdefault('capitol',  []).append( ( capitalSysID,  newAlloc,  True,  2*capitalThreat)  ) 
        remainingMilRating -= newAlloc
    if "Main" in round or newAlloc >0:
        print "Empire Capital System:   (%d) %s    -- threat : %d, military allocation: existing: %d  ; new: %d"%(capitalSysID,  universe.getSystem(capitalSysID).name ,  capitalThreat,  alreadyAssignedRating[capitalSysID],  newAlloc)
        print "-----------------"

    #================================
    #--------Empire Occupied Systems ----------
    empirePlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    empireOccupiedSystemIDs = list( set(PlanetUtilsAI.getSystems(empirePlanetIDs))  - set([capitalSysID] )  )
    if "Main" in round:
        print "Empire-Occupied  Systems:  %s"%(   [ "| %d %s |"%(eoSysID,  universe.getSystem(eoSysID).name)  for eoSysID in empireOccupiedSystemIDs  ]  )
        print "-----------------"
    newAlloc=0
    if len( empireOccupiedSystemIDs ) > 0:
        ocSysTotThreat = [  ( oSID,  threatBias +safetyFactor*sum( [ foAI.foAIstate.systemStatus.get(oSID,  {}).get(thrtKey, 0) for thrtKey in ['totalThreat',  'neighborThreat']] ))  
                                                                                                                                                for oSID in empireOccupiedSystemIDs ]
        totocSysThreat = sum( [thrt  for sid,  thrt in ocSysTotThreat] )
        totCurAlloc = sum( [alreadyAssignedRating[sid]  for sid,  thrt in ocSysTotThreat] )
        allocationFactor = min(  1.5,  remainingMilRating /max(0.01,  ( totocSysThreat -totCurAlloc) ))
        ocSysAlloc = 0
        for sid,  thrt in ocSysTotThreat:
            curAlloc=alreadyAssignedRating[sid]
            neededRating = ratingNeeded( 1.4*thrt,  curAlloc)
            if (neededRating > 0.8*(remainingMilRating )) and tryReset:
                tryAgain(allMilitaryFleetIDs)
                return
            thisAlloc=0
            if ( neededRating>0 ) and remainingMilRating > 0:
                thisAlloc = max(0,  min( neededRating,  0.5*availMilRating,  remainingMilRating))
                newAlloc+=thisAlloc
                allocations.append(  (sid,  thisAlloc,  True,  2*thrt) )
                allocationGroups.setdefault('occupied',  []).append( (sid,  thisAlloc,  True,  2*thrt) )
                remainingMilRating -= thisAlloc
                ocSysAlloc += thisAlloc
            if "Main" in round or thisAlloc >0:
                print "Provincial Occupied system %4d ( %10s )  has local threat %8d  ; existing military allocation %d and new allocation %8d"%(sid,  universe.getSystem(sid).name,  thrt, curAlloc,   thisAlloc)
        if "Main" in round or newAlloc >0:
            print "Provincial Empire-Occupied Sytems under total threat: %d  -- total mil allocation: existing %d  ; new: %d"%(totocSysThreat,  totCurAlloc,  ocSysAlloc )
            print "-----------------"

    #================================
    #--------Top Targeted Systems ----------
    #TODO: do native invasions highest priority
    otherTargetedSystemIDs = topTargetSystems
    if "Main" in round:
        print "Top Colony and Invasion Targeted Systems :  %s"%(   [ "| %d %s |"%(sysID,  universe.getSystem(sysID).name)  for sysID in otherTargetedSystemIDs  ]  )
        print "-----------------"
    # for these, calc local threat only, no neighbor threat, but use a multiplier for fleet safety
    newAlloc=0
    if len( otherTargetedSystemIDs ) > 0:
        otSysAlloc = 0
        otSysThreat = [  ( oSID,  threatBias +safetyFactor*(foAI.foAIstate.systemStatus.get(oSID, {}).get('totalThreat', 0)+0.5*foAI.foAIstate.systemStatus.get(oSID, {}).get('neightborThreat', 0)  )  )   for oSID in   otherTargetedSystemIDs      ]
        tototSysThreat = sum( [thrt for sid,  thrt in otSysThreat] )
        totCurAlloc = sum( [alreadyAssignedRating[sid] for sid,  thrt in otSysThreat] )
        for sid,  thrt in otSysThreat:
            curAlloc=alreadyAssignedRating[sid]
            neededRating = ratingNeeded( 1.4*thrt,  curAlloc)
            thisAlloc=0
            #only record more allocation for this invasion if we already started or have enough rating available
            takeAny= alreadyAssignedRating[sid] > 0
            if ( neededRating>0 ) and (remainingMilRating > neededRating  or takeAny):
                thisAlloc = max(0,  min( neededRating,  remainingMilRating))
                maxAlloc = safetyFactor*3*max(  foAI.foAIstate.systemStatus.get(sid, {}).get('totalThreat', 0),  foAI.foAIstate.systemStatus.get(sid, {}).get('neightborThreat', 0))
                newAlloc+=thisAlloc
                allocations.append(  (sid,  thisAlloc, takeAny , maxAlloc) )
                allocationGroups.setdefault('topTargets', []).append( (sid,  thisAlloc, takeAny  , maxAlloc) )
                remainingMilRating -= thisAlloc
                otSysAlloc += thisAlloc
            if "Main" in round or thisAlloc >0:
                print "Targeted system %4d ( %10s )  has local threat %8d  ; existing military allocation %d and new allocation %8d"%(sid,  universe.getSystem(sid).name,  thrt, curAlloc,   thisAlloc)
        if "Main" in round or newAlloc >0:
            print "-----------------"
            print "Top Colony and Invasion Targeted Systems  under total threat: %d  -- total mil allocation-- existing: %d   ; new: %d"%(tototSysThreat,  totCurAlloc,  otSysAlloc )
            print "-----------------"

    #================================
    #--------Targeted Systems ----------
    #TODO: do native invasions highest priority
    otherTargetedSystemIDs = [sysID for sysID in set( PlanetUtilsAI.getSystems(AIstate.opponentPlanetIDs))  if sysID not in topTargetSystems]
    if "Main" in round:
        print "Other Invasion Targeted Systems :  %s"%(   [ "| %d %s |"%(sysID,  universe.getSystem(sysID).name)  for sysID in otherTargetedSystemIDs  ]  )
        print "-----------------"
    # for these, calc local threat only, no neighbor threat, but use a multiplier for fleet safety
    newAlloc=0
    if len( otherTargetedSystemIDs ) > 0:
        otSysAlloc = 0
        otSysThreat = [  ( oSID, threatBias +safetyFactor*(foAI.foAIstate.systemStatus.get(oSID, {}).get('totalThreat', 0)+0.5*foAI.foAIstate.systemStatus.get(oSID, {}).get('neighborThreat', 0) ) )  for oSID in   otherTargetedSystemIDs      ]
        tototSysThreat = sum( [thrt for sid,  thrt in otSysThreat] )
        totCurAlloc = sum( [alreadyAssignedRating[sid] for sid,  thrt in otSysThreat] )
        for sid,  thrt in otSysThreat:
            curAlloc=alreadyAssignedRating[sid]
            neededRating = ratingNeeded( 1.4*thrt,  curAlloc)
            thisAlloc=0
            #only record more allocation for this invasion if we already started or have enough rating available
            takeAny= alreadyAssignedRating[sid] > 0
            if ( neededRating>0 ) and (remainingMilRating > neededRating  or takeAny):
                thisAlloc = max(0,  min( neededRating,  remainingMilRating))
                newAlloc+=thisAlloc
                maxAlloc = safetyFactor*2*max(  foAI.foAIstate.systemStatus.get(sid, {}).get('totalThreat', 0),  foAI.foAIstate.systemStatus.get(sid, {}).get('neightborThreat', 0))
                allocations.append(  (sid,  thisAlloc, takeAny , maxAlloc) )
                allocationGroups.setdefault('otherTargets', []).append( (sid,  thisAlloc, takeAny  , maxAlloc) )
                remainingMilRating -= thisAlloc
                otSysAlloc += thisAlloc
            if "Main" in round or thisAlloc >0:
                print "Targeted system %4d ( %10s )  has local threat %8d  ; existing military allocation %d and new allocation %8d"%(sid,  universe.getSystem(sid).name,  thrt, curAlloc,   thisAlloc)
        if "Main" in round or newAlloc >0:
            print "-----------------"
            print "Invasion Targeted Systems  under total threat: %d  -- total mil allocation-- existing: %d   ; new: %d"%(tototSysThreat,  totCurAlloc,  otSysAlloc )
            print "-----------------"

    otherTargetedSystemIDs = [sysID for sysID in  list(set(AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs)) if sysID not in topTargetSystems]
    if "Main" in round:
        print "Other Targeted Systems :  %s"%(   [ "| %d %s |"%(sysID,  universe.getSystem(sysID).name)  for sysID in otherTargetedSystemIDs  ]  )
        print "-----------------"
    if len( otherTargetedSystemIDs ) > 0:
        otSysAlloc = 0
        otSysThreat = [  ( oSID,  safetyFactor*(threatBias +foAI.foAIstate.systemStatus.get(oSID, {}).get('totalThreat', 0)+0.5*foAI.foAIstate.systemStatus.get(oSID, {}).get('neighborThreat', 0) ) )   for oSID in   otherTargetedSystemIDs      ]
        tototSysThreat = sum( [thrt for sid,  thrt in otSysThreat] )
        totCurAlloc = sum( [alreadyAssignedRating[sid] for sid,  thrt in otSysThreat] )
        newAlloc=0
        for sid,  thrt in otSysThreat:
            curAlloc=alreadyAssignedRating[sid]
            neededRating = ratingNeeded( 1.2*thrt,  curAlloc)
            thisAlloc=0
            #only record more allocation for this invasion if we already started or have enough rating available
            takeAny= alreadyAssignedRating[sid] > 0
            if ( neededRating>0 ) and (remainingMilRating > neededRating  or takeAny):
                thisAlloc = max(0,  min( neededRating,  remainingMilRating))
                newAlloc+=thisAlloc
                maxAlloc = safetyFactor*2*max(  foAI.foAIstate.systemStatus.get(sid, {}).get('totalThreat', 0),  foAI.foAIstate.systemStatus.get(sid, {}).get('neightborThreat', 0))
                allocations.append(  (sid,  thisAlloc, takeAny , maxAlloc) )
                allocationGroups.setdefault('otherTargets', []).append( (sid,  thisAlloc, takeAny  , maxAlloc) )
                remainingMilRating -= thisAlloc
                otSysAlloc += thisAlloc
            if "Main" in round or thisAlloc >0:
                print "Targeted system %4d ( %10s )  has local threat %8d  ; existing military allocation %d and new allocation %8d"%(sid,  universe.getSystem(sid).name,  thrt, curAlloc,   thisAlloc)
        if "Main" in round or newAlloc >0:
            print "-----------------"
            print "Other Targeted Systems  under total threat: %d  -- total mil allocation-- existing: %d   ; new: %d"%(tototSysThreat,  totCurAlloc,  otSysAlloc )
            print "-----------------"

    otherTargetedSystemIDs = []
    targetableIDs = ColonisationAI.annexableSystemIDs.union( empire.fleetSupplyableSystemIDs )
    for sysID in  AIstate.opponentSystemIDs:
        if sysID in targetableIDs:
            otherTargetedSystemIDs.append(sysID)
        else:
            for nID in  universe.getImmediateNeighbors(sysID,  empireID):
                if nID in targetableIDs:
                    otherTargetedSystemIDs.append(sysID)
                    break
        
    if "Main" in round:
        print "Blockade Targeted Systems :  %s"%(   [ "| %d %s |"%(sysID,  universe.getSystem(sysID).name)  for sysID in otherTargetedSystemIDs  ]  )
        print "-----------------"
    if len( otherTargetedSystemIDs ) > 0:
        otSysAlloc = 0
        otSysThreat = [  ( oSID,  threatBias +safetyFactor*(foAI.foAIstate.systemStatus.get(oSID, {}).get('totalThreat', 0)+ 0.5*foAI.foAIstate.systemStatus.get(oSID, {}).get('neighborThreat', 0) ) )   for oSID in   otherTargetedSystemIDs      ]
        tototSysThreat = sum( [thrt for sid,  thrt in otSysThreat] )
        totCurAlloc = sum( [alreadyAssignedRating[sid] for sid,  thrt in otSysThreat] )
        newAlloc=0
        for sid,  thrt in otSysThreat:
            curAlloc=alreadyAssignedRating[sid]
            neededRating = ratingNeeded( 1.2*thrt,  curAlloc)
            thisAlloc=0
            #only record more allocation for this invasion if we already started or have enough rating available
            takeAny= alreadyAssignedRating[sid] > 0
            if ( neededRating>0 ) and (remainingMilRating > neededRating  or takeAny):
                thisAlloc = max(0,  min( neededRating,  remainingMilRating))
                newAlloc+=thisAlloc
                maxAlloc = safetyFactor*2*max(  foAI.foAIstate.systemStatus.get(sid, {}).get('totalThreat', 0),  foAI.foAIstate.systemStatus.get(sid, {}).get('neightborThreat', 0))
                allocations.append(  (sid,  thisAlloc, takeAny , maxAlloc) )
                allocationGroups.setdefault('otherTargets', []).append( (sid,  thisAlloc, takeAny  , maxAlloc) )
                remainingMilRating -= thisAlloc
                otSysAlloc += thisAlloc
            if "Main" in round or thisAlloc >0:
                print "Blockade Targeted system %4d ( %10s )  has local threat %8d  ; existing military allocation %d and new allocation %8d"%(sid,  universe.getSystem(sid).name,  thrt, curAlloc,   thisAlloc)
        if "Main" in round or newAlloc >0:
            print "-----------------"
            print "Blockade Targeted Systems  under total threat: %d  -- total mil allocation-- existing: %d   ; new: %d"%(tototSysThreat,  totCurAlloc,  otSysAlloc )
            print "-----------------"

    currentMilSystems = [sid for sid, alloc, takeAny, mm  in allocations ]
    interiorIDs = list( foAI.foAIstate.expInteriorSystemIDs)
    interiorTargets1 =   (targetableIDs.union(interiorIDs)).difference( currentMilSystems )
    interiorTargets = [sid for sid in interiorTargets1 if ( (threatBias + foAI.foAIstate.systemStatus.get(sid, {}).get('totalThreat', 0) >0.8*alreadyAssignedRating[sid] ) ) ]
    if "Main" in round:
        print ""
        print "Other Empire-Proximal Systems :  %s"%(   [ "| %d %s |"%(sysID,  universe.getSystem(sysID).name)  for sysID in interiorTargets1  ]  )
        print "-----------------"
    # for these, calc fleet  threat only, no neighbor threat, but use a multiplier for fleet safety
    newAlloc=0
    if len(interiorTargets) >0:
        otSysAlloc = 0
        otSysThreat = [  ( oSID,  threatBias +safetyFactor*(foAI.foAIstate.systemStatus.get(oSID, {}).get('totalThreat', 0))  )   for oSID in   interiorTargets      ]
        tototSysThreat = sum( [thrt for sid,  thrt in otSysThreat] )
        totCurAlloc = sum( [alreadyAssignedRating[sid] for sid,  thrt in otSysThreat] )
        for sid,  thrt in otSysThreat:
            curAlloc=alreadyAssignedRating[sid]
            neededRating = ratingNeeded( 1.2*thrt,  curAlloc)
            thisAlloc=0
            #only record more allocation for this invasion if we already started or have enough rating available
            takeAny= alreadyAssignedRating[sid] > 0
            if ( neededRating>0 ) and (remainingMilRating > neededRating  or takeAny):
                thisAlloc = max(0,  min( neededRating,  remainingMilRating))
                newAlloc+=thisAlloc
                maxAlloc = safetyFactor*2*max(  foAI.foAIstate.systemStatus.get(sid, {}).get('totalThreat', 0),  foAI.foAIstate.systemStatus.get(sid, {}).get('neightborThreat', 0))
                allocations.append(  (sid,  thisAlloc, takeAny , maxAlloc) )
                allocationGroups.setdefault('otherTargets', []).append( (sid,  thisAlloc, takeAny  , maxAlloc) )
                remainingMilRating -= thisAlloc
                otSysAlloc += thisAlloc
            if "Main" in round or thisAlloc >0:
               print "Other interior system %4d ( %10s )  has local threat %8d  ; existing military allocation %d and new allocation %8d"%(sid,  universe.getSystem(sid).name,  thrt, curAlloc,   thisAlloc)
        if "Main" in round or newAlloc >0:
            print "-----------------"
            print "Other Interior Systems  under total threat: %d  -- total mil allocation-- existing: %d   ; new: %d"%(tototSysThreat,  totCurAlloc,  otSysAlloc )
            print "-----------------"
    elif "Main" in round:
        print "-----------------"
        print "No Other Interior Systems  with fleet threat "
        print "-----------------"

    monsterDens=[]

    #exploTargetIDs,  _ = ExplorationAI.getCurrentExplorationInfo(verbose=False)
    exploTargetIDs=[]
    if "Main" in round:
        print ""
        print "Exploration-targeted Systems:  %s"%(   [ "| %d %s |"%(sysID,  universe.getSystem(sysID).name)  for sysID in exploTargetIDs  ]  )
        print "-----------------"
    # for these, calc fleet  threat only, no neighbor threat, but use a multiplier for fleet safety
    newAlloc=0
    if len(exploTargetIDs) > 0:
        otSysAlloc = 0
        otSysThreat = [  ( oSID,  safetyFactor*(foAI.foAIstate.systemStatus.get(oSID, {}).get('fleetThreat', 0) + foAI.foAIstate.systemStatus.get(oSID, {}).get('monsterThreat', 0)+ foAI.foAIstate.systemStatus.get(oSID, {}).get('planetThreat', 0) ))   for oSID in   exploTargetIDs      ]
        tototSysThreat = sum( [thrt for sid,  thrt in otSysThreat] )
        totCurAlloc = sum( [0.8*alreadyAssignedRating[sid] for sid,  thrt in otSysThreat] )
        if availMilRating <1125:
            maxMilRating = availMilRating
        else:
            maxMilRating = 0.5*availMilRating
        for sid,  thrt in otSysThreat:
            curAlloc=alreadyAssignedRating[sid]
            neededRating = ratingNeeded( 1.2*thrt,  curAlloc)
            thisAlloc=0
            #only record more allocation for this invasion if we already started or have enough rating available
            takeAny= False
            if ( neededRating>0 ) and (remainingMilRating > neededRating  or takeAny):
                thisAlloc = max(0,  min( neededRating,  remainingMilRating))
                newAlloc+=thisAlloc
                maxAlloc = safetyFactor*2*max(  foAI.foAIstate.systemStatus.get(sid, {}).get('totalThreat', 0),  foAI.foAIstate.systemStatus.get(sid, {}).get('neightborThreat', 0))
                allocations.append(  (sid,  thisAlloc, takeAny , maxAlloc) )
                allocationGroups.setdefault('exploreTargets', []).append( (sid,  thisAlloc, takeAny  , maxAlloc) )
                remainingMilRating -= thisAlloc
                otSysAlloc += thisAlloc
            if "Main" in round or thisAlloc >0:
                print "Exploration-targeted  %4d ( %10s )  has local threat %8d  ; existing military allocation %d and new allocation %8d"%(sid,  universe.getSystem(sid).name,  thrt, curAlloc,   thisAlloc)
        if "Main" in round or newAlloc >0:
            print "-----------------"
            print "Exploration-targeted s  under total threat: %d  -- total mil allocation-- existing: %d   ; new: %d"%(tototSysThreat,  totCurAlloc,  otSysAlloc )
            print "-----------------"

    visibleSystemIDs = foAI.foAIstate.visInteriorSystemIDs.keys() + foAI.foAIstate. visBorderSystemIDs.keys()
    accessibleSystemIDs = [sysID for sysID in visibleSystemIDs if  universe.systemsConnected(sysID, homeSystemID, empireID) ]
    currentMilSystems = [sid for sid, alloc,  takeAny,  multiplier  in allocations if alloc > 0 ]
    borderTargets1 = [sid for sid in accessibleSystemIDs  if (  ( sid not in currentMilSystems )) ]
    borderTargets = [sid for sid in borderTargets1 if ( ( threatBias +foAI.foAIstate.systemStatus.get(sid, {}).get('fleetThreat', 0)  + foAI.foAIstate.systemStatus.get(sid, {}).get('planetThreat', 0) > 0.8*alreadyAssignedRating[sid] )) ]
    if "Main" in round:
        print ""
        print "Empire-Accessible Systems not yet allocated military:  %s"%(   [ "| %d %s |"%(sysID,  universe.getSystem(sysID) and universe.getSystem(sysID).name)  for sysID in borderTargets1  ]  )
        print "-----------------"
    # for these, calc fleet  threat only, no neighbor threat, but use a multiplier for fleet safety
    newAlloc=0
    if len(borderTargets) > 0:
        otSysAlloc = 0
        otSysThreat = [  ( oSID,  threatBias +safetyFactor*(foAI.foAIstate.systemStatus.get(oSID, {}).get('fleetThreat', 0) + foAI.foAIstate.systemStatus.get(oSID, {}).get('monsterThreat', 0)+ foAI.foAIstate.systemStatus.get(oSID, {}).get('planetThreat', 0))  )   for oSID in   borderTargets      ]
        tototSysThreat = sum( [thrt for sid,  thrt in otSysThreat] )
        totCurAlloc = sum( [0.8*alreadyAssignedRating[sid] for sid,  thrt in otSysThreat] )
        for sid,  thrt in otSysThreat:
            curAlloc=alreadyAssignedRating[sid]
            neededRating = ratingNeeded( 1.2*thrt,  curAlloc)
            thisAlloc=0
            #only record more allocation for this invasion if we already started or have enough rating available
            takeAny= False
            if ( neededRating>0 ) and (remainingMilRating > neededRating  or takeAny):
                thisAlloc = max(0,  min( neededRating,  remainingMilRating))
                newAlloc+=thisAlloc
                maxAlloc = safetyFactor*2*max(  foAI.foAIstate.systemStatus.get(sid, {}).get('totalThreat', 0),  foAI.foAIstate.systemStatus.get(sid, {}).get('neightborThreat', 0))
                allocations.append(  (sid,  thisAlloc, takeAny , maxAlloc) )
                allocationGroups.setdefault('accessibleTargets', []).append( (sid,  thisAlloc, takeAny  , maxAlloc) )
                remainingMilRating -= thisAlloc
                otSysAlloc += thisAlloc
            if "Main" in round or thisAlloc >0:
                print "Other Empire-Accessible system %4d ( %10s )  has local biased threat %8d  ; existing military allocation %d and new allocation %8d"%(sid,  universe.getSystem(sid).name,  thrt, curAlloc,   thisAlloc)
        if "Main" in round or newAlloc >0:
            print "-----------------"
            print "Other Empire-Accessible Systems  under total biased threat: %d  -- total mil allocation-- existing: %d   ; new: %d"%(tototSysThreat,  totCurAlloc,  otSysAlloc )
            print "-----------------"
    elif "Main" in round:
        print "-----------------"
        print "No Other Empire-Accessible Systems  with biased local threat "
        print "-----------------"

    #monster den treatment probably unnecessary now
    if "Main" in round:
        print ""
        print "Big-Monster Dens:  %s"%(   [ "| %d %s |"%(sysID,  universe.getSystem(sysID).name)  for sysID in monsterDens  ]  )
        print "-----------------"
    # for these, calc fleet  threat only, no neighbor threat, but use a multiplier for fleet safety
    newAlloc=0
    if len(monsterDens) > 0:
        otSysAlloc = 0
        otSysThreat = [  ( oSID,  safetyFactor*(foAI.foAIstate.systemStatus.get(oSID, {}).get('fleetThreat', 0)+foAI.foAIstate.systemStatus.get(oSID, {}).get('monsterThreat', 0) + foAI.foAIstate.systemStatus.get(oSID, {}).get('planetThreat', 0) ) )   for oSID in   monsterDens      ]
        tototSysThreat = sum( [thrt for sid,  thrt in otSysThreat] )
        totCurAlloc = sum( [0.8*alreadyAssignedRating[sid] for sid,  thrt in otSysThreat] )
        for sid,  thrt in otSysThreat:
            curAlloc=0.8*alreadyAssignedRating[sid]
            thisAlloc=0
            if (thrt > curAlloc) and remainingMilRating > 2* thrt:
                thisAlloc = int(0.99999 + (thrt-curAlloc)*1.5)
                newAlloc+=thisAlloc
                allocations.append(  (sid,  thisAlloc,  False,  5) )
                remainingMilRating -= thisAlloc
                otSysAlloc += thisAlloc
            if "Main" in round or thisAlloc >0:
                print "Monster Den  %4d ( %10s )  has local threat %8d  ; existing military allocation %d and new allocation %8d"%(sid,  universe.getSystem(sid).name,  thrt, curAlloc,   thisAlloc)
        if "Main" in round or newAlloc >0:
            print "-----------------"


    if remainingMilRating <=6:
        newAllocations = [ (sid,  alc,  alc,  ta) for (sid,  alc,  ta,  mm) in allocations ]
    else:
        #oldAllocations = dict( [ (entry[0],  entry ) for entry in allocations ] )
        try:
            totAlloc = sum( [alloc for sid,  alloc,  takeAny,  maxAlloc  in allocations ] )
        except:
            print "error unpacking sid,  alloc,  takeAny, maxAlloc  from ", allocations 
        factor =(2.0* remainingMilRating ) / ( totAlloc  + 0.1)
        #print "Remaining military strength allocation %d will be allocated  as %.1f %% surplus allocation to top current priorities"%(remainingMilRating,  100*factor)
        print "%s Round Remaining military strength allocation %d will be allocated  as surplus allocation to top current priorities"%(round,  remainingMilRating)
        newAllocations = []
        for cat in ['capitol', 'topTargets',  'otherTargets',  'accessibleTargets',  'occupied',  'exploreTargets']:
            for sid,  alloc,  takeAny,  maxAlloc in allocationGroups.get(cat,  []):
                if remainingMilRating <= 0 :
                    newAllocations.append(  ( sid, alloc,  alloc,  takeAny )  )
                else:
                    newRating = min(remainingMilRating+alloc, max(alloc,  ratingNeeded( maxAlloc,  alreadyAssignedRating[sid]) ) )
                    newAllocations.append(  ( sid, newRating, alloc,  takeAny )  )
                    remainingMilRating -= ( newRating - alloc )

    if "Main" in round:
        MilitaryAllocations = newAllocations
    minMilAllocations = dict( [ (sid, alloc) for sid, alloc, takeAny,  mm in allocations   ]   )
    print "------------------------------\nFinal %s Round Military Allocations: %s \n-----------------------"%(round,  dict( [ (sid, alloc) for sid, alloc, minalloc,  takeAny in newAllocations   ]   ) )

    # export military systems for other AI modules
    if "Main" in round:
        AIstate.militarySystemIDs = list( set([sid for sid, alloc,  minalloc,  takeAny  in newAllocations]).union( [sid for sid in alreadyAssignedRating if alreadyAssignedRating[sid]>0 ]   ))
    else:
        AIstate.militarySystemIDs = list( set([sid for sid, alloc,  minalloc,  takeAny  in newAllocations]).union( AIstate.militarySystemIDs) )
    return newAllocations
    
def getMilitaryTargetedSystemIDs(systemIDs, missionType, empireID):
    "return list of military targeted systems"

    universe = fo.getUniverse()
    militaryAIFleetMissions = foAI.foAIstate.getAIFleetMissionsWithAnyMissionTypes([missionType])

    targetedSystems = []

    for systemID in systemIDs:
        system = universe.getSystem(systemID)
        # add systems that are target of a mission
        for militaryAIFleetMission in militaryAIFleetMissions:
            aiTarget = AITarget.AITarget(AITargetType.TARGET_SYSTEM, systemID)
            if militaryAIFleetMission.hasTarget(missionType, aiTarget):
                targetedSystems.append(systemID)

    return targetedSystems

def assignMilitaryValues(systemIDs, missionType, empireProvinceSystemIDs, otherTargetedSystemIDs, empire):
    "creates a dictionary that takes systemIDs as key and their military score as value"

    systemValues = {}

    for systemID in systemIDs:
        systemValues[systemID] = evaluateSystem(systemID, missionType, empireProvinceSystemIDs, otherTargetedSystemIDs, empire)

    return systemValues

def evaluateSystem(systemID, missionType, empireProvinceSystemIDs, otherTargetedSystemIDs, empire):
    "return the military value of a system"

    universe = fo.getUniverse()
    system = universe.getSystem(systemID)
    if (system == None): return 0

    # give preference to home system then closest systems
    empireID = empire.empireID
    capitalID = PlanetUtilsAI.getCapital()
    homeworld = universe.getPlanet(capitalID)
    if homeworld:
        homeSystemID = homeworld.systemID
        evalSystemID = system.systemID
        leastJumpsPath = len(universe.leastJumpsPath(homeSystemID, evalSystemID, empireID))
        distanceFactor = 1.001/(leastJumpsPath + 1)
    else:
        homeSystemID=-1
        distanceFactor=0

    if systemID == homeSystemID:
        return 10
    elif systemID in empireProvinceSystemIDs:
        return 4 + distanceFactor
    elif systemID in otherTargetedSystemIDs:
        return 2 + distanceFactor
    else:
        return 1 + .25 * distanceFactor

def assignMilitaryFleetsToSystems(useFleetIDList=None,  allocations=[]):
    # assign military fleets to military theater systems
    global MilitaryAllocations
    universe = fo.getUniverse()
    
    doingMain =  (useFleetIDList==None)
    if doingMain:
        baseDefenseIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE)
        unassignedBaseDefenseIDs = FleetUtilsAI.extractFleetIDsWithoutMissionTypes(baseDefenseIDs)
        for fleetID in unassignedBaseDefenseIDs:
            fleet = universe.getFleet(fleetID)
            if not fleet:
                continue
            sysID = fleet.systemID
            aiTarget = AITarget.AITarget(AITargetType.TARGET_SYSTEM, sysID)
            aiFleetMission = foAI.foAIstate.getAIFleetMission(fleetID)
            aiFleetMission.clearAIFleetOrders()
            aiFleetMission.clearAITargets( (aiFleetMission.getAIMissionTypes() + [-1])[0] )
            missionType = AIFleetMissionType.FLEET_MISSION_ORBITAL_DEFENSE
            aiFleetMission.addAITarget( missionType , aiTarget)

        allMilitaryFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_MILITARY)
        if allMilitaryFleetIDs == []:
            MilitaryAllocations = []
            return
        #TODO: keep some continuity of missions
        AIstate.militaryFleetIDs = FleetUtilsAI.extractFleetIDsWithoutMissionTypes(allMilitaryFleetIDs)
        availMilFleetIDs = list( AIstate.militaryFleetIDs )
        availMilRating = sum(  map(lambda x: foAI.foAIstate.getRating(x).get('overall', 0),  availMilFleetIDs   )  )
        theseAllocations = MilitaryAllocations
        print "=================================================="
        print "assigning military fleets"
        print "---------------------------------"
    else:
        availMilFleetIDs = list( useFleetIDList )
        availMilRating = sum(  map(lambda x: foAI.foAIstate.getRating(x).get('overall', 0),  availMilFleetIDs   )  )
        theseAllocations = allocations
    remainingMilRating = availMilRating

    # get systems to defend

    availMilFleetIDs = set(availMilFleetIDs)
    for sysID,  alloc,  minalloc,   takeAny in theseAllocations:
        if not doingMain and len(availMilFleetIDs)==0:
            break
        foundFleets = []
        foundStats={}
        theseFleets = FleetUtilsAI.getFleetsForMission(1,  {'rating':alloc}, {'rating':minalloc},   foundStats,  "",  systemsToCheck=[sysID],  systemsChecked=[], 
                                                        fleetPoolSet=availMilFleetIDs,   fleetList=foundFleets,  verbose=False)
        if theseFleets == []:
            if foundFleets==[]  or  not ( FleetUtilsAI.statsMeetReqs( foundStats,  {'rating':minalloc}) or takeAny):
                if doingMain:
                    print "NO available/suitable military  allocation for system %d ( %s ) -- requested allocation %8d, found available rating  %8d in fleets %s"%(sysID,  universe.getSystem(sysID).name,  minalloc,  foundStats.get('rating',  0),  foundFleets)
                availMilFleetIDs.update(foundFleets)
                continue
            else:
                theseFleets = foundFleets
                #rating = sum( map(lambda x: foAI.foAIstate.rateFleet(x),  foundFleets ) )
                ratings = map(lambda x: foAI.foAIstate.rateFleet(x),  foundFleets ) 
                rating = sum([fr.get('attack', 0) for fr in ratings]) * sum([fr.get('health', 0) for fr in ratings])
                if doingMain:
                    if rating < minMilAllocations.get(sysID,  0):
                        print "PARTIAL  military  allocation for system %d ( %s ) -- requested allocation %8d  -- got %8d with fleets %s"%(sysID,  universe.getSystem(sysID).name,  minalloc,  rating,  theseFleets)
                    else:
                        print "FULL MIN military  allocation for system %d ( %s ) -- requested allocation %8d -- got %8d with fleets %s "%(sysID,  universe.getSystem(sysID).name,  minMilAllocations.get(sysID, 0) ,  rating,  theseFleets)
        elif doingMain:
            print "FULL+  military  allocation for system %d ( %s ) -- requested allocation %8d, got %8d with fleets %s"%(sysID,  universe.getSystem(sysID).name,  alloc,  foundStats.get('rating', 0),  theseFleets)
        aiTarget = AITarget.AITarget(AITargetType.TARGET_SYSTEM, sysID)
        for fleetID in theseFleets:
            fleet=universe.getFleet(fleetID)
            fo.issueAggressionOrder(fleetID,  True)
            aiFleetMission = foAI.foAIstate.getAIFleetMission(fleetID)
            aiFleetMission.clearAIFleetOrders()
            aiFleetMission.clearAITargets( (aiFleetMission.getAIMissionTypes() + [-1])[0] )
            if sysID in list(set(AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs + AIstate.invasionTargetedSystemIDs + AIstate.blockadeTargetedSystemIDs)):
                missionType = AIFleetMissionType.FLEET_MISSION_SECURE
            else:
                missionType = AIFleetMissionType.FLEET_MISSION_MILITARY
            aiFleetMission.addAITarget( missionType , aiTarget)
            aiFleetMission.generateAIFleetOrders()
            if not doingMain:
                foAI.foAIstate.misc.setdefault('ReassignedFleetMissions',  []).append(aiFleetMission)

    if doingMain:
        print "---------------------------------"
