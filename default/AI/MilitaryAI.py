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

def getMilitaryFleets():
    "get armed military fleets"
    global MilitaryAllocations, totMilRating

    allMilitaryFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_MILITARY)
    if allMilitaryFleetIDs == []:
        MilitaryAllocations = []
        return
    #TODO: keep some continuity of missions
    #AIstate.militaryFleetIDs = FleetUtilsAI.extractFleetIDsWithoutMissionTypes(allMilitaryFleetIDs)

    milFleetIDs = list( allMilitaryFleetIDs)
    totMilRating = sum(  map(lambda x: foAI.foAIstate.getRating(x),  milFleetIDs   )  )
    print "=================================================="
    print "Total Military Rating: %d"%totMilRating
    print "---------------------------------"
    remainingMilRating = totMilRating
    allocations = []

    # get systems to defend
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
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


    capitalThreat = sum( foAI.foAIstate.systemStatus[capitalSysID].values() )
    if capitalThreat >0:
        allocations.append( ( capitalSysID,  2* capitalThreat,  True)  )
        remainingMilRating -= 2* capitalThreat
    print "Empire Capital System:   (%d) %s    -- threat : %d, military allocation %d"%(capitalSysID,  universe.getSystem(capitalSysID).name ,  capitalThreat,  min(totMilRating,  2*capitalThreat))
    print "-----------------"

    empirePlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    empireOccupiedSystemIDs = list( set(PlanetUtilsAI.getSystems(empirePlanetIDs))  - set([capitalSysID] )  )
    print "Empire-Occupied  Systems:  %s"%(   [ "| %d %s |"%(eoSysID,  universe.getSystem(eoSysID).name)  for eoSysID in empireOccupiedSystemIDs  ]  )
    print "-----------------"
    if len( empireOccupiedSystemIDs ) > 0:
        ocSysTotThreat = [  ( oSID,  sum( foAI.foAIstate.systemStatus[oSID].values() ) )  for oSID in   empireOccupiedSystemIDs      ]
        totocSysThreat = sum( [thrt for sid,  thrt in ocSysTotThreat] )
        allocationFactor = min(  1,  remainingMilRating / ( totocSysThreat +0.01 ) )
        ocSysAlloc = 0
        for sid,  thrt in ocSysTotThreat:
            if (thrt > 0) and remainingMilRating > 0:
                thisAlloc = min( min( int(0.99999 + thrt*allocationFactor ),  0.5*totMilRating) ,  remainingMilRating)
                allocations.append(  (sid,  thisAlloc,  True) )
                remainingMilRating -= thisAlloc
                ocSysAlloc += thisAlloc
        print "Provincial Empire-Occupied Sytems under total threat: %d  -- total mil allocation %d"%(totocSysThreat,  ocSysAlloc )
        print "-----------------"

    #TODO: split invasions, do native invasions highest priority
    otherTargetedSystemIDs = list(set(AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs + AIstate.invasionTargetedSystemIDs))
    print "Other Targeted Systems :  %s"%(   [ "| %d %s |"%(sysID,  universe.getSystem(sysID).name)  for sysID in otherTargetedSystemIDs  ]  )
    print "-----------------"
    # for these, calc local threat only, no neighbor threat, but use a multiplier for fleet safety
    if len( otherTargetedSystemIDs ) > 0:
        otSysAlloc = 0
        otSysThreat = [  ( oSID,  foAI.foAIstate.systemStatus[oSID]['fleetThreat']+ foAI.foAIstate.systemStatus[oSID]['planetThreat']  )   for oSID in   otherTargetedSystemIDs      ]
        tototSysThreat = sum( [thrt for sid,  thrt in otSysThreat] )
        for sid,  thrt in otSysThreat:
            if (thrt > 0) and remainingMilRating > 0:
                thisAlloc = min( min( int(0.99999 + thrt*1.5),  remainingMilRating ),  0.5*totMilRating)
                allocations.append(  (sid,  thisAlloc,  False) )
                remainingMilRating -= thisAlloc
                otSysAlloc += thisAlloc
                print "Targeted system %4d ( %10s )  has local threat %8d  and is allocated %8d military rating"%(sid,  universe.getSystem(sid).name,  thrt,  thisAlloc)
        print "-----------------"
        print "Other Targeted Systems  under total threat: %d  -- total mil allocation %d"%(tototSysThreat,  otSysAlloc )
        print "-----------------"

    currentMilSystems = [sid for sid, alloc, takeAny  in allocations ]
    interiorIDs = list( foAI.foAIstate.expInteriorSystemIDs)
    interiorTargets1 = [sid for sid in interiorIDs if (  ( sid not in currentMilSystems )) ]
    interiorTargets = [sid for sid in interiorIDs if ( ( foAI.foAIstate.systemStatus[sid]['fleetThreat'] >0 ) and   ( sid not in currentMilSystems )) ]
    print ""
    print "Other Empire-Interior Systems :  %s"%(   [ "| %d %s |"%(sysID,  universe.getSystem(sysID).name)  for sysID in interiorTargets1  ]  )
    print "-----------------"
    # for these, calc fleet  threat only, no neighbor threat, but use a multiplier for fleet safety
    if len(interiorTargets) >0:
        otSysAlloc = 0
        otSysThreat = [  ( oSID,  foAI.foAIstate.systemStatus[oSID]['fleetThreat']  )   for oSID in   interiorTargets      ]
        tototSysThreat = sum( [thrt for sid,  thrt in otSysThreat] )
        for sid,  thrt in otSysThreat:
            if (thrt > 0) and remainingMilRating > 0:
                thisAlloc = min( min( int(0.99999 + thrt*1.5),  remainingMilRating ),  0.5*totMilRating)
                allocations.append(  (sid,  thisAlloc,  True) )
                remainingMilRating -= thisAlloc
                otSysAlloc += thisAlloc
                print "Other Interior system %4d ( %10s )  has fleet threat %8d  and is allocated %8d military rating"%(sid,  universe.getSystem(sid).name,  thrt,  thisAlloc)
        print "-----------------"
        print "Other Interior Systems  under total threat: %d  -- total mil allocation %d"%(tototSysThreat,  otSysAlloc )
        print "-----------------"
    else:
        print "-----------------"
        print "No Other Interior Systems  with fleet threat "
        print "-----------------"

    currentMilSystems = [sid for sid, alloc,  takeAny  in allocations ]
    borderIDs = ExplorationAI.getBorderExploredSystemIDs()
    borderTargets1 = [sid for sid in borderIDs if (  ( sid not in currentMilSystems )) ]
    borderTargets = [sid for sid in borderIDs if ( ( foAI.foAIstate.systemStatus[sid]['fleetThreat']  + foAI.foAIstate.systemStatus[sid]['planetThreat'] >0 ) and   ( sid not in currentMilSystems )) ]
    print ""
    print "Other Empire-BorderSystems:  %s"%(   [ "| %d %s |"%(sysID,  universe.getSystem(sysID).name)  for sysID in borderTargets1  ]  )
    print "-----------------"
    # for these, calc fleet  threat only, no neighbor threat, but use a multiplier for fleet safety
    if len(borderTargets) > 0:
        otSysAlloc = 0
        otSysThreat = [  ( oSID,  foAI.foAIstate.systemStatus[oSID]['fleetThreat'] + foAI.foAIstate.systemStatus[oSID]['planetThreat']  )   for oSID in   borderTargets      ]
        tototSysThreat = sum( [thrt for sid,  thrt in otSysThreat] )
        for sid,  thrt in otSysThreat:
            if (thrt > 0) and remainingMilRating > 0:
                thisAlloc = min( min( int(0.99999 + thrt*1.5),  remainingMilRating ),  0.5*totMilRating)
                allocations.append(  (sid,  thisAlloc,  False) )
                remainingMilRating -= thisAlloc
                otSysAlloc += thisAlloc
                print "Other Border system %4d ( %10s )  has local threat %8d  and is allocated %8d military rating"%(sid,  universe.getSystem(sid).name,  thrt,  thisAlloc)
        print "-----------------"
        print "Other Border Systems  under total local threat: %d  -- total mil allocation %d"%(tototSysThreat,  otSysAlloc )
        print "-----------------"
    else:
        print "-----------------"
        print "No Other Border Systems  with local threat "
        print "-----------------"

    exploTargetIDs,  _ = ExplorationAI.getCurrentExplorationInfo(verbose=False)
    print ""
    print "Exploration-targeted Systems:  %s"%(   [ "| %d %s |"%(sysID,  universe.getSystem(sysID).name)  for sysID in exploTargetIDs  ]  )
    print "-----------------"
    # for these, calc fleet  threat only, no neighbor threat, but use a multiplier for fleet safety
    if len(exploTargetIDs) > 0:
        otSysAlloc = 0
        otSysThreat = [  ( oSID,  foAI.foAIstate.systemStatus[oSID]['fleetThreat'] + foAI.foAIstate.systemStatus[oSID]['planetThreat'] )   for oSID in   exploTargetIDs      ]
        tototSysThreat = sum( [thrt for sid,  thrt in otSysThreat] )
        if totMilRating <1025:
            maxMilRating = totMilRating
        else:
            maxMilRating = 0.5*totMilRating
        for sid,  thrt in otSysThreat:
            if (thrt > 0) and remainingMilRating > 0:
                thisAlloc = min( min( int(0.99999 + thrt*1.5),  remainingMilRating ),  0.5*totMilRating)
                allocations.append(  (sid,  thisAlloc,  False) )
                remainingMilRating -= thisAlloc
                otSysAlloc += thisAlloc
                print "Exploration-targeted System %4d ( %10s )  has local threat %8d  and is allocated %8d military rating"%(sid,  universe.getSystem(sid).name,  thrt,  thisAlloc)
        print "-----------------"
        print "Exploration-targeted Systems  under total local threat: %d  -- total mil allocation %d"%(tototSysThreat,  otSysAlloc )
        print "-----------------"

    if remainingMilRating <=6:
        newAllocations = [ (sid,  alc,  alc,  ta) for (sid,  alc,  ta) in allocations ]
    else:
        totAlloc = sum( [alloc for sid,  alloc,  takeAny  in allocations ] )
        factor =(2.0* remainingMilRating ) / ( totAlloc  + 0.1)
        print "Remaining military strength allocation %d will be allocated  as %.1f %% surplus allocation to top current priorities"%(remainingMilRating,  100*factor)
        newAllocations = []
        for sid,  alloc,  takeAny in allocations:
            if remainingMilRating <= 0 :
                newAllocations.append(  ( sid, alloc,  alloc,  takeAny )  )
            else:
                thisAlloc =  int( factor * alloc )
                newAllocations.append(  ( sid, alloc+thisAlloc, alloc,  takeAny )  )
                remainingMilRating -= thisAlloc

    MilitaryAllocations = newAllocations
    minMilAllocations = dict( [ (sid, alloc) for sid, alloc, takeAny in allocations   ]   )

    if False:  #keep old code for reference, for now
        fleetSupplyableSystemIDs = list(empire.fleetSupplyableSystemIDs)
        exploredSystemIDs = empire.exploredSystemIDs

        exploredPlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(exploredSystemIDs)
        allOwnedPlanetIDs = PlanetUtilsAI.getAllOwnedPlanetIDs(exploredPlanetIDs)
        allPopulatedSystemIDs = PlanetUtilsAI.getAllPopulatedSystemIDs(allOwnedPlanetIDs)
        print ""
        print "All Populated SystemIDs:             " + str(list(set(allPopulatedSystemIDs)))

        empireProvinceSystemIDs = list(set(empireOccupiedSystemIDs) - set([capitalSysID]))
        print "Empire Province SystemIDs:           " + str(empireProvinceSystemIDs)

        competitorSystemIDs = list(set(allPopulatedSystemIDs) - set(empireOccupiedSystemIDs))
        print "Competitor SystemIDs:                " + str(competitorSystemIDs)


        militaryTheaterSystemIDs = list(set(fleetSupplyableSystemIDs + empireOccupiedSystemIDs + competitorSystemIDs + otherTargetedSystemIDs))
        print "Military Theater SystemIDs:          " + str(militaryTheaterSystemIDs)

        allMilitaryTargetedSystemIDs = getMilitaryTargetedSystemIDs(universe.systemIDs, AIFleetMissionType.FLEET_MISSION_MILITARY, empireID)
        # export military targeted systems for other AI modules
        AIstate.militaryTargetedSystemIDs = allMilitaryTargetedSystemIDs
        print ""
        print "Military Targeted SystemIDs:         " + str(allMilitaryTargetedSystemIDs)

        militaryFleetIDs = allMilitaryFleetIDs
        if not militaryFleetIDs:
            print "Available Military Fleets:             0"
        else:
            print "Military FleetIDs:                   " + str(allMilitaryFleetIDs)

        numMilitaryFleets = len(FleetUtilsAI.extractFleetIDsWithoutMissionTypes(militaryFleetIDs))
        print "Military Fleets Without Missions:      " + str(numMilitaryFleets)

        evaluatedSystemIDs = list(set(militaryTheaterSystemIDs) - set(allMilitaryTargetedSystemIDs))
        # print "Evaluated SystemIDs:               " +str(evaluatedSystemIDs)

        evaluatedSystems = assignMilitaryValues(evaluatedSystemIDs, AIFleetMissionType.FLEET_MISSION_MILITARY, empireProvinceSystemIDs, otherTargetedSystemIDs, empire)

        sortedSystems = evaluatedSystems.items()
        sortedSystems.sort(lambda x, y: cmp(x[1], y[1]), reverse=True)

        print ""
        print "Military SystemIDs:"
        for evaluationPair in sortedSystems:
            print "    ID|Score: " + str(evaluationPair)

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
    #AIstate.militaryFleetIDs = FleetUtilsAI.extractFleetIDsWithoutMissionTypes(allMilitaryFleetIDs)
    #availMilFleetIDs = list( AIstate.militaryFleetIDs )

    availMilFleetIDs =  list( allMilitaryFleetIDs)
    totMilRating = sum(  map(lambda x: foAI.foAIstate.getRating(x),  availMilFleetIDs   )  )
    print "=================================================="
    print "assigning military fleets"
    print "---------------------------------"
    remainingMilRating = totMilRating

    # get systems to defend
    universe = fo.getUniverse()

    for sysID,  alloc,  minalloc,   takeAny in MilitaryAllocations:
        foundFleets = []
        foundRating=[0]
        theseFleets = FleetUtilsAI.getFleetsForMission(1,  alloc, minalloc,   foundRating,  "",  systemsToCheck=[sysID],  systemsChecked=[], fleetPool=availMilFleetIDs,   fleetList=foundFleets,  verbose=False)
        if theseFleets == []:
            if foundFleets==[]  or  (foundRating[0]<minalloc and not takeAny):
                print "NO available/suitable military  allocation for system %d ( %s ) -- requested allocation %8d"%(sysID,  universe.getSystem(sysID).name,  alloc)
                continue
            else:
                theseFleets = foundFleets
                rating = sum( map(lambda x: foAI.foAIstate.rateFleet(x),  foundFleets ) )
                if rating < minMilAllocations.get(sysID,  0):
                    print "PARTIAL  military  allocation for system %d ( %s ) -- requested allocation %8d  -- got %8d"%(sysID,  universe.getSystem(sysID).name,  alloc,  rating)
                else:
                    print "FULL MIN military  allocation for system %d ( %s ) -- requested allocation %8d"%(sysID,  universe.getSystem(sysID).name,  minMilAllocations.get(sysID, 0) )
        else:
            print "FULL+  military  allocation for system %d ( %s ) -- requested allocation %8d"%(sysID,  universe.getSystem(sysID).name,  alloc)
        aiTarget = AITarget.AITarget(AITargetType.TARGET_SYSTEM, sysID)
        for fleetID in theseFleets:
            fleet=universe.getFleet(fleetID)
            fo.issueAggressionOrder(fleetID,  True)
            aiFleetMission = foAI.foAIstate.getAIFleetMission(fleetID)
            aiFleetMission.clearAIFleetOrders()
            aiFleetMission.clearAITargets( (aiFleetMission.getAIMissionTypes() + [-1])[0] )
            aiFleetMission.addAITarget( AIFleetMissionType.FLEET_MISSION_MILITARY , aiTarget)
    print "---------------------------------"

def assignMilitaryFleetsToSystems_Old():
    # assign military fleets to military theater systems

    sendMilitaryFleets(AIstate.militaryFleetIDs, AIstate.militarySystemIDs, AIFleetMissionType.FLEET_MISSION_MILITARY)
