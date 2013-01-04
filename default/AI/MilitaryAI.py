import freeOrionAIInterface as fo
import FreeOrionAI as foAI
import AIstate
import AITarget
from EnumsAI import AIFleetMissionType, AITargetType
import FleetUtilsAI
import PlanetUtilsAI
from random import choice
import ExplorationAI

MinThreat = 6 # the minimum threat level that will be ascribed to an unkown threat capable of killing scouts
MilitaryAllocations = []
minMilAllocations = {}
totMilRating=0

def getMilitaryFleets(tryReset=True):
    "get armed military fleets"
    global MilitaryAllocations, totMilRating

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    capitalID = PlanetUtilsAI.getCapital()
    homeworld = universe.getPlanet(capitalID)
    if homeworld:
        homeSystemID = homeworld.systemID
    else:
        homeSystemID=-1

    allMilitaryFleetIDs =  FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_MILITARY )
    totMilRating = sum(  map(lambda x: foAI.foAIstate.getRating(x).get('overall', 0),  allMilitaryFleetIDs   )  )
    print "=================================================="
    print "Total Military Rating: %d"%totMilRating
    print "---------------------------------"
        
        
    milFleetIDs = list( FleetUtilsAI.extractFleetIDsWithoutMissionTypes(allMilitaryFleetIDs))
    availMilRating = sum(  map(lambda x: foAI.foAIstate.getRating(x).get('overall', 0),  milFleetIDs   )  )
    print "=================================================="
    print "Available Military Rating: %d"%availMilRating
    print "---------------------------------"
    remainingMilRating = availMilRating
    allocations = []
    
    if milFleetIDs == []:
        MilitaryAllocations = []
        return
        
    alreadyAssignedRating={}
    for sysID in universe.systemIDs:
        alreadyAssignedRating[sysID]=0

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
        alreadyAssignedRating[lastSys] +=  foAI.foAIstate.getRating(fleetID).get('overall', 0) #TODO: would preferably tally attack and health and take product 

    # get systems to defend
    capitalID = PlanetUtilsAI.getCapital()
    capitalPlanet = universe.getPlanet(capitalID)
    #TODO: if no owned planets try to capture one!
    if  capitalPlanet:  
        capitalSysID = capitalPlanet.systemID
    else: # should be rare, but so as to not break code below, pick a randomish  mil-centroid  system
        systemDict = {}
        for fleetID in allMilitaryFleetIDs:
            status = foAI.foAIstate.fleetStatus.get(fleetID,  None)
            if status is not None:
                sysID = status['sysID']
                if len( list( universe.getSystem(sysID).planetIDs  ) ) ==0:
                    continue
                systemDict[sysID] = systemDict.get( sysID,  0) + status['rating']
        rankedSystems = sorted( [(val,  sysID) for sysID, val in systemDict.items()  ]   )
        if rankedSystems:
            capitalSysID = rankedSystems[-1][-1]
        else:
            capitalSysID = foAI.foAIstate.fleetStatus.items()[0]['sysID']

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
    

    safetyFactor = [ 2.0,  1.25,  1.0,  0.95,  0.95    ][foAI.foAIstate.aggression] 

    # allocation format: ( sysID, newAllocation, takeAny, maxMultiplier )    
    #================================
    #--------Capital Threat ----------
    capitalThreat = safetyFactor*(2* threatBias +sum( [ foAI.foAIstate.systemStatus[capitalSysID][thrtKey] for thrtKey in [tkey for tkey in  foAI.foAIstate.systemStatus.get(capitalSysID,  {}).keys() if 'Threat' in tkey]] ))
    newAlloc=0
    if (capitalThreat > (availMilRating+0.8*alreadyAssignedRating[capitalSysID]) ) and tryReset:
        if foAI.foAIstate.aggression > 0:
            for fid in allMilitaryFleetIDs:
                thisMission=foAI.foAIstate.getAIFleetMission(fid)
                thisMission.clearAIFleetOrders()
                thisMission.clearAITargets(-1)
            getMilitaryFleets(tryReset=False)
            return
    
    if capitalThreat > 0.8*alreadyAssignedRating[capitalSysID]:
        newAlloc = min(remainingMilRating,  int( 0.999 + 1.2*(capitalThreat- 0.8*alreadyAssignedRating[capitalSysID]) ) )
        allocations.append( ( capitalSysID,  newAlloc,  True,  3)  )  
        remainingMilRating -= newAlloc
    print "Empire Capital System:   (%d) %s    -- threat : %d, military allocation: existing: %d  ; new: %d"%(capitalSysID,  universe.getSystem(capitalSysID).name ,  capitalThreat,  alreadyAssignedRating[capitalSysID],  newAlloc)
    print "-----------------"

    #================================
    #--------Empire Occupied Systems ----------
    empirePlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    empireOccupiedSystemIDs = list( set(PlanetUtilsAI.getSystems(empirePlanetIDs))  - set([capitalSysID] )  )
    print "Empire-Occupied  Systems:  %s"%(   [ "| %d %s |"%(eoSysID,  universe.getSystem(eoSysID).name)  for eoSysID in empireOccupiedSystemIDs  ]  )
    print "-----------------"
    if len( empireOccupiedSystemIDs ) > 0:
        ocSysTotThreat = [  ( oSID,  threatBias +safetyFactor*sum( [ foAI.foAIstate.systemStatus.get(oSID,  {}).get(thrtKey, 0) for thrtKey in ['fleetThreat', 'planetThreat', 'neighborThreat']] ) )  for oSID in   empireOccupiedSystemIDs      ]
        totocSysThreat = sum( [thrt  for sid,  thrt in ocSysTotThreat] )
        totCurAlloc = sum( [0.8*alreadyAssignedRating[sid]  for sid,  thrt in ocSysTotThreat] )
        allocationFactor = min(  1.2,  remainingMilRating /max(0.01,  ( totocSysThreat -totCurAlloc) ))
        ocSysAlloc = 0
        for sid,  thrt in ocSysTotThreat:
            curAlloc=0.8*alreadyAssignedRating[sid]
            thisAlloc=0
            if (thrt > curAlloc) and remainingMilRating > 0:
                thisAlloc = max(0,  min( min( int(0.99999 + (thrt-curAlloc)*allocationFactor ),  0.5*availMilRating) ,  remainingMilRating))
                allocations.append(  (sid,  thisAlloc,  True,  2) )
                remainingMilRating -= thisAlloc
                ocSysAlloc += thisAlloc
        print "Provincial Empire-Occupied Sytems under total threat: %d  -- total mil allocation: existing %d  ; new: %d"%(totocSysThreat,  totCurAlloc,  ocSysAlloc )
        print "-----------------"

    #================================
    #--------Targeted Systems ----------
    #TODO: do native invasions highest priority
    otherTargetedSystemIDs = list(AIstate.invasionTargetedSystemIDs)
    print "Invasion Targeted Systems :  %s"%(   [ "| %d %s |"%(sysID,  universe.getSystem(sysID).name)  for sysID in otherTargetedSystemIDs  ]  )
    print "-----------------"
    # for these, calc local threat only, no neighbor threat, but use a multiplier for fleet safety
    if len( otherTargetedSystemIDs ) > 0:
        otSysAlloc = 0
        otSysThreat = [  ( oSID,  threatBias +safetyFactor*(foAI.foAIstate.systemStatus.get(oSID, {}).get('fleetThreat', 0)+ foAI.foAIstate.systemStatus.get(oSID, {}).get('planetThreat', 0))  )   for oSID in   otherTargetedSystemIDs      ]
        tototSysThreat = sum( [thrt for sid,  thrt in otSysThreat] )
        totCurAlloc = sum( [0.8*alreadyAssignedRating[sid] for sid,  thrt in otSysThreat] )
        for sid,  thrt in otSysThreat:
            curAlloc=0.8*alreadyAssignedRating[sid]
            thisAlloc=0
            if thrt>curAlloc and remainingMilRating > 10+ 1.5*(thrt-curAlloc) and foAI.foAIstate.systemStatus.get(sid, {}).get('monsterThreat', 0) < 0.4*totMilRating: #only record an allocation for this invasion if we have enough rating available
                thisAlloc =int(10.99999 + (thrt-curAlloc)*1.5)
                allocations.append(  (sid,  thisAlloc,  False,  3) )
                remainingMilRating -= thisAlloc
                otSysAlloc += thisAlloc
            print "Targeted system %4d ( %10s )  has local threat %8d  ; existing military allocation %d and new allocation %8d"%(sid,  universe.getSystem(sid).name,  thrt, curAlloc,   thisAlloc)
        print "-----------------"
        print "Invasion Targeted Systems  under total threat: %d  -- total mil allocation-- existing: %d   ; new: %d"%(tototSysThreat,  totCurAlloc,  otSysAlloc )
        print "-----------------"

    otherTargetedSystemIDs = list(set(AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs))
    print "Other Targeted Systems :  %s"%(   [ "| %d %s |"%(sysID,  universe.getSystem(sysID).name)  for sysID in otherTargetedSystemIDs  ]  )
    print "-----------------"
    # for these, calc local threat only, no neighbor threat, but use a multiplier for fleet safety
    if len( otherTargetedSystemIDs ) > 0:
        otSysAlloc = 0
        otSysThreat = [  ( oSID,  safetyFactor*(threatBias +foAI.foAIstate.systemStatus.get(oSID, {}).get('fleetThreat', 0)+ foAI.foAIstate.systemStatus.get(oSID, {}).get('planetThreat', 0)  ))   for oSID in   otherTargetedSystemIDs      ]
        tototSysThreat = sum( [thrt for sid,  thrt in otSysThreat] )
        totCurAlloc = sum( [0.8*alreadyAssignedRating[sid] for sid,  thrt in otSysThreat] )
        for sid,  thrt in otSysThreat:
            curAlloc=0.8*alreadyAssignedRating[sid]
            thisAlloc=0
            if (thrt > curAlloc) and remainingMilRating > 1.5*(thrt-curAlloc)  and foAI.foAIstate.systemStatus.get(sid, {}).get('monsterThreat', 0) < 0.4*totMilRating:
                thisAlloc = min( min( int(0.99999 + (thrt-curAlloc)*1.5),  remainingMilRating ),  0.5*availMilRating)
                allocations.append(  (sid,  thisAlloc,  False,  2.0) )
                remainingMilRating -= thisAlloc
                otSysAlloc += thisAlloc
            print "Targeted system %4d ( %10s )  has local threat %8d  ; existing military allocation %d and new allocation %8d"%(sid,  universe.getSystem(sid).name,  thrt, curAlloc,   thisAlloc)
        print "-----------------"
        print "Other Targeted Systems  under total threat: %d  -- total mil allocation-- existing: %d   ; new: %d"%(tototSysThreat,  totCurAlloc,  otSysAlloc )
        print "-----------------"

    otherTargetedSystemIDs = []
    fleetSuppliableSystemIDs = empire.fleetSupplyableSystemIDs
    for sysID in  AIstate.opponentSystemIDs:
        if sysID in fleetSuppliableSystemIDs:
            otherTargetedSystemIDs.append(sysID)
        else:
            for nID in  universe.getImmediateNeighbors(sysID,  empireID):
                if nID in fleetSuppliableSystemIDs:
                    otherTargetedSystemIDs.append(sysID)
                    break
        
    print "Blockade Targeted Systems :  %s"%(   [ "| %d %s |"%(sysID,  universe.getSystem(sysID).name)  for sysID in otherTargetedSystemIDs  ]  )
    print "-----------------"
    # for these, calc local threat only, no neighbor threat, but use a multiplier for fleet safety
    if len( otherTargetedSystemIDs ) > 0:
        otSysAlloc = 0
        otSysThreat = [  ( oSID,  threatBias +safetyFactor*(foAI.foAIstate.systemStatus.get(oSID, {}).get('fleetThreat', 0)+ foAI.foAIstate.systemStatus.get(oSID, {}).get('planetThreat', 0)  ))   for oSID in   otherTargetedSystemIDs      ]
        tototSysThreat = sum( [thrt for sid,  thrt in otSysThreat] )
        totCurAlloc = sum( [0.8*alreadyAssignedRating[sid] for sid,  thrt in otSysThreat] )
        for sid,  thrt in otSysThreat:
            curAlloc=0.8*alreadyAssignedRating[sid]
            thisAlloc=0
            if (thrt > curAlloc) and remainingMilRating > 1.5*(thrt-curAlloc)  and foAI.foAIstate.systemStatus.get(sid, {}).get('monsterThreat', 0) < 0.4*totMilRating:
                thisAlloc = min( min( int(0.99999 + (thrt-curAlloc)*1.5),  remainingMilRating ),  0.5*availMilRating)
                allocations.append(  (sid,  thisAlloc,  False,  10) )
                remainingMilRating -= thisAlloc
                otSysAlloc += thisAlloc
            print "Blockade Targeted system %4d ( %10s )  has local threat %8d  ; existing military allocation %d and new allocation %8d"%(sid,  universe.getSystem(sid).name,  thrt, curAlloc,   thisAlloc)
        print "-----------------"
        print "Blockade Targeted Systems  under total threat: %d  -- total mil allocation-- existing: %d   ; new: %d"%(tototSysThreat,  totCurAlloc,  otSysAlloc )
        print "-----------------"

    currentMilSystems = [sid for sid, alloc, takeAny, mm  in allocations ]
    interiorIDs = list( foAI.foAIstate.expInteriorSystemIDs)
    interiorTargets1 = [sid for sid in interiorIDs if (  ( sid not in currentMilSystems )) ]
    interiorTargets = [sid for sid in interiorIDs if ( (threatBias + foAI.foAIstate.systemStatus.get(sid, {}).get('fleetThreat', 0) >0.8*alreadyAssignedRating[sid] ) ) ]
    print ""
    print "Other Empire-Interior Systems :  %s"%(   [ "| %d %s |"%(sysID,  universe.getSystem(sysID).name)  for sysID in interiorTargets1  ]  )
    print "-----------------"
    # for these, calc fleet  threat only, no neighbor threat, but use a multiplier for fleet safety
    if len(interiorTargets) >0:
        otSysAlloc = 0
        otSysThreat = [  ( oSID,  threatBias +safetyFactor*foAI.foAIstate.systemStatus.get(oSID, {}).get('fleetThreat', 0)  )   for oSID in   interiorTargets      ]
        tototSysThreat = sum( [thrt for sid,  thrt in otSysThreat] )
        totCurAlloc = sum( [0.8*alreadyAssignedRating[sid] for sid,  thrt in otSysThreat] )
        for sid,  thrt in otSysThreat:
            curAlloc=0.8*alreadyAssignedRating[sid]
            thisAlloc=0
            if (thrt > curAlloc) and remainingMilRating > 0 :
                thisAlloc = min( min( int(0.99999 +( thrt-curAlloc)*1.5),  remainingMilRating ),  0.5*availMilRating)
                allocations.append(  (sid,  thisAlloc,  True,  1.2) )
                remainingMilRating -= thisAlloc
                otSysAlloc += thisAlloc
            print "Other interior system %4d ( %10s )  has local threat %8d  ; existing military allocation %d and new allocation %8d"%(sid,  universe.getSystem(sid).name,  thrt, curAlloc,   thisAlloc)
        print "-----------------"
        print "Other Interior Systems  under total threat: %d  -- total mil allocation-- existing: %d   ; new: %d"%(tototSysThreat,  totCurAlloc,  otSysAlloc )
        print "-----------------"
    else:
        print "-----------------"
        print "No Other Interior Systems  with fleet threat "
        print "-----------------"

    monsterDens=[]

    exploTargetIDs,  _ = ExplorationAI.getCurrentExplorationInfo(verbose=False)
    print ""
    print "Exploration-targeted Systems:  %s"%(   [ "| %d %s |"%(sysID,  universe.getSystem(sysID).name)  for sysID in exploTargetIDs  ]  )
    print "-----------------"
    # for these, calc fleet  threat only, no neighbor threat, but use a multiplier for fleet safety
    if len(exploTargetIDs) > 0:
        otSysAlloc = 0
        otSysThreat = [  ( oSID,  safetyFactor*(foAI.foAIstate.systemStatus.get(oSID, {}).get('fleetThreat', 0) + foAI.foAIstate.systemStatus.get(oSID, {}).get('planetThreat', 0) ))   for oSID in   exploTargetIDs      ]
        tototSysThreat = sum( [thrt for sid,  thrt in otSysThreat] )
        totCurAlloc = sum( [0.8*alreadyAssignedRating[sid] for sid,  thrt in otSysThreat] )
        if availMilRating <1125:
            maxMilRating = availMilRating
        else:
            maxMilRating = 0.5*availMilRating
        for sid,  thrt in otSysThreat:
            curAlloc=0.8*alreadyAssignedRating[sid]
            thisAlloc=0
            if (thrt > curAlloc) and remainingMilRating > 0:
                if foAI.foAIstate.systemStatus.get(sid, {}).get('monsterThreat', 0) > 2000:
                    monsterDens.append(sid)
                    continue  # consider dealing with big monsters later
                thisAlloc = min( min( int(0.99999 + (thrt-curAlloc)*1.5),  remainingMilRating ),  0.5*availMilRating)
                allocations.append(  (sid,  thisAlloc,  False,  2) )
                remainingMilRating -= thisAlloc
                otSysAlloc += thisAlloc
            print "Exploration-targeted  %4d ( %10s )  has local threat %8d  ; existing military allocation %d and new allocation %8d"%(sid,  universe.getSystem(sid).name,  thrt, curAlloc,   thisAlloc)
        print "-----------------"
        print "Exploration-targeted s  under total threat: %d  -- total mil allocation-- existing: %d   ; new: %d"%(tototSysThreat,  totCurAlloc,  otSysAlloc )
        print "-----------------"

    visibleSystemIDs = foAI.foAIstate.visInteriorSystemIDs.keys() + foAI.foAIstate. visBorderSystemIDs.keys()
    accessibleSystemIDs = [sysID for sysID in visibleSystemIDs if  universe.systemsConnected(sysID, homeSystemID, empireID) ]
    currentMilSystems = [sid for sid, alloc,  takeAny,  multiplier  in allocations if alloc > 0 ]
    borderTargets1 = [sid for sid in accessibleSystemIDs  if (  ( sid not in currentMilSystems )) ]
    borderTargets = [sid for sid in borderTargets1 if ( ( threatBias +foAI.foAIstate.systemStatus.get(sid, {}).get('fleetThreat', 0)  + foAI.foAIstate.systemStatus.get(sid, {}).get('planetThreat', 0) > 0.8*alreadyAssignedRating[sid] )) ]
    print ""
    print "Empire-Accessible Systems not yet allocated military:  %s"%(   [ "| %d %s |"%(sysID,  universe.getSystem(sysID) and universe.getSystem(sysID).name)  for sysID in borderTargets1  ]  )
    print "-----------------"
    # for these, calc fleet  threat only, no neighbor threat, but use a multiplier for fleet safety
    if len(borderTargets) > 0:
        otSysAlloc = 0
        otSysThreat = [  ( oSID,  threatBias +safetyFactor*(foAI.foAIstate.systemStatus.get(oSID, {}).get('fleetThreat', 0) + foAI.foAIstate.systemStatus.get(oSID, {}).get('planetThreat', 0))  )   for oSID in   borderTargets      ]
        tototSysThreat = sum( [thrt for sid,  thrt in otSysThreat] )
        totCurAlloc = sum( [0.8*alreadyAssignedRating[sid] for sid,  thrt in otSysThreat] )
        for sid,  thrt in otSysThreat:
            curAlloc=0.8*alreadyAssignedRating[sid]
            thisAlloc=0
            if (thrt > curAlloc) and remainingMilRating > 0:
                if foAI.foAIstate.systemStatus.get(sid, {}).get('monsterThreat', 0) > 2000:
                    if sid not in monsterDens:
                        monsterDens.append(sid)
                    continue  # consider dealing with big monsters later
                thisAlloc = min( min( int(0.99999 + (thrt-curAlloc)*1.5),  remainingMilRating ),  0.5*availMilRating)
                allocations.append(  (sid,  thisAlloc,  False, 5) )
                remainingMilRating -= thisAlloc
                otSysAlloc += thisAlloc
            print "Other Empire-Accessible system %4d ( %10s )  has local biased threat %8d  ; existing military allocation %d and new allocation %8d"%(sid,  universe.getSystem(sid).name,  thrt, curAlloc,   thisAlloc)
        print "-----------------"
        print "Other Empire-Accessible Systems  under total biased threat: %d  -- total mil allocation-- existing: %d   ; new: %d"%(tototSysThreat,  totCurAlloc,  otSysAlloc )
        print "-----------------"
    else:
        print "-----------------"
        print "No Other Empire-Accessible Systems  with biased local threat "
        print "-----------------"

    print ""
    print "Big-Monster Dens:  %s"%(   [ "| %d %s |"%(sysID,  universe.getSystem(sysID).name)  for sysID in monsterDens  ]  )
    print "-----------------"
    # for these, calc fleet  threat only, no neighbor threat, but use a multiplier for fleet safety
    if len(monsterDens) > 0:
        otSysAlloc = 0
        otSysThreat = [  ( oSID,  safetyFactor*(foAI.foAIstate.systemStatus.get(oSID, {}).get('fleetThreat', 0) + foAI.foAIstate.systemStatus.get(oSID, {}).get('planetThreat', 0) ) )   for oSID in   monsterDens      ]
        tototSysThreat = sum( [thrt for sid,  thrt in otSysThreat] )
        totCurAlloc = sum( [0.8*alreadyAssignedRating[sid] for sid,  thrt in otSysThreat] )
        for sid,  thrt in otSysThreat:
            curAlloc=0.8*alreadyAssignedRating[sid]
            thisAlloc=0
            if (thrt > curAlloc) and remainingMilRating > 2* thrt:
                thisAlloc = int(0.99999 + (thrt-curAlloc)*1.5)
                allocations.append(  (sid,  thisAlloc,  False) )
                remainingMilRating -= thisAlloc
                otSysAlloc += thisAlloc
            print "Monster Den  %4d ( %10s )  has local threat %8d  ; existing military allocation %d and new allocation %8d"%(sid,  universe.getSystem(sid).name,  thrt, curAlloc,   thisAlloc)
        print "-----------------"


    if remainingMilRating <=6:
        newAllocations = [ (sid,  alc,  alc,  ta) for (sid,  alc,  ta,  mm) in allocations ]
    else:
        totAlloc = sum( [alloc for sid,  alloc,  takeAny,  maxMul  in allocations ] )
        factor =(2.0* remainingMilRating ) / ( totAlloc  + 0.1)
        print "Remaining military strength allocation %d will be allocated  as %.1f %% surplus allocation to top current priorities"%(remainingMilRating,  100*factor)
        newAllocations = []
        for sid,  alloc,  takeAny,  maxMul in allocations:
            if remainingMilRating <= 0 :
                newAllocations.append(  ( sid, alloc,  alloc,  takeAny )  )
            else:
                thisAlloc =  int( max( maxMul-1,  factor )* alloc )
                newAllocations.append(  ( sid, alloc+thisAlloc, alloc,  takeAny )  )
                remainingMilRating -= thisAlloc

    MilitaryAllocations = newAllocations
    minMilAllocations = dict( [ (sid, alloc) for sid, alloc, takeAny,  mm in allocations   ]   )
    print "------------------------------\nFinal Military Allocations: %s \n-----------------------"%dict( [ (sid, alloc) for sid, alloc, minalloc,  takeAny in newAllocations   ]   )

    # export military systems for other AI modules
    AIstate.militarySystemIDs = [sid for sid, alloc,  minalloc,  takeAny  in newAllocations]

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

def sendMilitaryFleets(militaryFleetIDs, evaluatedSystems, missionType):
    "sends a list of military fleets to a list of system_value_pairs"
    
    if len(militaryFleetIDs)==0: return

    allMilitaryFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_MILITARY)
    AIstate.militaryFleetIDs = FleetUtilsAI.extractFleetIDsWithoutMissionTypes(allMilitaryFleetIDs)

    targets=[]
    for systemID_value_pair in evaluatedSystems: # evaluatedSystems is a dictionary
        targets.extend(  int(systemID_value_pair[1]) *[ systemID_value_pair[0] ]    )
    #currentFleetSizes= dict (  [ (fleetID, len( universe.getFleet(fleetID).shipIDs) )   for fleetID in allMilitaryFleetIDs ] )
    #newFleetSizes= dict (  [ (fleetID, len( universe.getFleet(fleetID).shipIDs) )   for fleetID in AIstate.militaryFleetIDs ] )

    if len(targets)==0: return

    if True:
        for fleetID in militaryFleetIDs:
            systemID = choice( targets )
            aiTarget = AITarget.AITarget(AITargetType.TARGET_SYSTEM, systemID)
            aiFleetMission = foAI.foAIstate.getAIFleetMission(fleetID)
            aiFleetMission.addAITarget(missionType, aiTarget)
        return
    else:

        i = 0
        for systemID_value_pair in evaluatedSystems: # evaluatedSystems is a dictionary
            if i >= len(militaryFleetIDs): return

            fleetID = militaryFleetIDs[i]
            systemID = systemID_value_pair[0]

            aiTarget = AITarget.AITarget(AITargetType.TARGET_SYSTEM, systemID)
            aiFleetMission = foAI.foAIstate.getAIFleetMission(fleetID)
            aiFleetMission.addAITarget(missionType, aiTarget)
            i = i + 1
            
        return

def assignMilitaryFleetsToSystems():
    # assign military fleets to military theater systems
    global MilitaryAllocations

    allMilitaryFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_MILITARY)
    if allMilitaryFleetIDs == []:
        MilitaryAllocations = []
        return
    #TODO: keep some continuity of missions
    AIstate.militaryFleetIDs = FleetUtilsAI.extractFleetIDsWithoutMissionTypes(allMilitaryFleetIDs)
    availMilFleetIDs = list( AIstate.militaryFleetIDs )

    #availMilFleetIDs =  list( allMilitaryFleetIDs)
    totMilRating = sum(  map(lambda x: foAI.foAIstate.getRating(x).get('overall', 0),  allMilitaryFleetIDs   )  )
    availMilRating = sum(  map(lambda x: foAI.foAIstate.getRating(x).get('overall', 0),  availMilFleetIDs   )  )
    print "=================================================="
    print "assigning military fleets"
    print "---------------------------------"
    remainingMilRating = availMilRating

    # get systems to defend
    universe = fo.getUniverse()

    availMilFleetIDs = set(availMilFleetIDs)
    for sysID,  alloc,  minalloc,   takeAny in MilitaryAllocations:
        foundFleets = []
        foundStats={}
        theseFleets = FleetUtilsAI.getFleetsForMission(1,  {'rating':alloc}, {'rating':minalloc},   foundStats,  "",  systemsToCheck=[sysID],  systemsChecked=[], fleetPoolSet=availMilFleetIDs,   fleetList=foundFleets,  verbose=False)
        if theseFleets == []:
            if foundFleets==[]  or  not ( FleetUtilsAI.statsMeetReqs( foundStats,  {'rating':minalloc}) or takeAny):
                print "NO available/suitable military  allocation for system %d ( %s ) -- requested allocation %8d, found available rating  %8d in fleets %s"%(sysID,  universe.getSystem(sysID).name,  minalloc,  foundStats.get('rating',  0),  foundFleets)
                availMilFleetIDs.update(foundFleets)
                continue
            else:
                theseFleets = foundFleets
                #rating = sum( map(lambda x: foAI.foAIstate.rateFleet(x),  foundFleets ) )
                ratings = map(lambda x: foAI.foAIstate.rateFleet(x),  foundFleets ) 
                rating = sum([fr.get('attack', 0) for fr in ratings]) * sum([fr.get('health', 0) for fr in ratings])
                if rating < minMilAllocations.get(sysID,  0):
                    print "PARTIAL  military  allocation for system %d ( %s ) -- requested allocation %8d  -- got %8d with fleets %s"%(sysID,  universe.getSystem(sysID).name,  minalloc,  rating,  theseFleets)
                else:
                    print "FULL MIN military  allocation for system %d ( %s ) -- requested allocation %8d -- got %8d with fleets %s "%(sysID,  universe.getSystem(sysID).name,  minMilAllocations.get(sysID, 0) ,  rating,  theseFleets)
        else:
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
    print "---------------------------------"

def assignMilitaryFleetsToSystems_Old():
    # assign military fleets to military theater systems

    sendMilitaryFleets(AIstate.militaryFleetIDs, AIstate.militarySystemIDs, AIFleetMissionType.FLEET_MISSION_MILITARY)
