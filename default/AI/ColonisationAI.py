#get these declared first to help avoid import circularities
import freeOrionAIInterface as fo

import AIDependencies
import AITarget
import AIstate
import FleetUtilsAI
import FreeOrionAI as foAI
import PlanetUtilsAI
import ProductionAI 
import TechsListsAI
from EnumsAI import AIFleetMissionType, AIExplorableSystemType, AITargetType, AIFocusType
import EnumsAI


empireSpecies = {}
empireSpeciesSystems={}
empireColonizers = {}
empireShipBuilders={}
availableGrowthSpecials={}
empirePlanetsWithGrowthSpecials={}
activeGrowthSpecials={}
empireMetabolisms={}
annexableSystemIDs=set([])
annexableRing1=set([])
annexableRing2=set([])
annexableRing3=set([])
annexablePlanetIDs=set([])
curBestMilShipRating = 20
allColonyOpportunities = {}


environs =                  { str(fo.planetEnvironment.uninhabitable): 0,  str(fo.planetEnvironment.hostile): 1,  str(fo.planetEnvironment.poor): 2,  str(fo.planetEnvironment.adequate): 3,  str(fo.planetEnvironment.good):4 }
photoMap= { fo.starType.blue:3    , fo.starType.white:1.5  , fo.starType.red:-1 ,  fo.starType.neutron: -1 , fo.starType.blackHole: -10 , fo.starType.noStar: -10     }
#   mods per environ    uninhab   hostile    poor   adequate    good
popSizeModMap={
                            "env":               [ 0, -4, -2, 0,  3 ], 
                            "subHab":      [ 0,  1,  1,  1,  1 ], 
                            "symBio":       [ 0,  0,  1,  1,  1 ], 
                            "xenoGen":  [ 0,  1,  2,  2,  0 ], 
                            "xenoHyb":   [ 0,  2,  1,  0,  0 ], 
                            "cyborg":       [ 0,  2,  0,  0,  0 ], 
                            "ndim":            [ 0, 2,  2,  2,   2 ], 
                            "orbit":            [ 0,  1,  1,  1,  1 ], 
                            "gaia":             [ 0,  3,  3,  3,  3 ], 
                            }

def dictFromMap(map):
    return dict(  [  (el.key(),  el.data() ) for el in map ] )
def resetCAIGlobals():
    global curBestMilShipRating
    empireSpecies.clear()
    empireSpeciesSystems.clear()
    empireColonizers.clear()
    empireShipBuilders.clear()
    activeGrowthSpecials.clear()
    annexableSystemIDs.clear()
    annexableRing1.clear()
    annexableRing2.clear()
    annexableRing3.clear()
    annexablePlanetIDs.clear()
    curBestMilShipRating = 20
    allColonyOpportunities.clear()

def getColonyFleets():
    global  curBestMilShipRating
    
    curBestMilShipRating = ProductionAI.curBestMilShipRating()
    
    "get colony fleets"

    allColonyFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_COLONISATION)
    AIstate.colonyFleetIDs[:] = FleetUtilsAI.extractFleetIDsWithoutMissionTypes(allColonyFleetIDs)

    # get suppliable systems and planets
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    capitalID = PlanetUtilsAI.getCapital()
    #capitalID = empire.capitalID
    homeworld=None
    if capitalID:
        homeworld = universe.getPlanet(capitalID)
    if homeworld:
        speciesName = homeworld.speciesName
        homeworldName=homeworld.name
        homeSystemID = homeworld.systemID
    else:
        speciesName = ""
        homeworldName=" no remaining homeworld "
        homeSystemID = -1
    if not speciesName:
        speciesName = foAI.foAIstate.origSpeciesName
    species = fo.getSpecies(speciesName)
    if not species:
        print "**************************************************************************************"
        print "**************************************************************************************"
        print "Problem determining species for colonization planning: capitalID: %s,  homeworld %s  and species name %s"%(capitalID,  homeworldName,  speciesName)
    else:
        print "Plannning colonization for species name %s"%species.name

    fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
    fleetSupplyablePlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(fleetSupplyableSystemIDs)
    print ""
    print "    fleetSupplyableSystemIDs: " + str(list(fleetSupplyableSystemIDs))
    print "    fleetSupplyablePlanetIDs: " + str(fleetSupplyablePlanetIDs)
    print ""

    print "-------\nEmpire Obstructed Starlanes:"
    print  list(empire.obstructedStarlanes())


    annexableSystemIDs.clear()
    annexableRing1.clear()
    annexableRing2.clear()
    annexableRing3.clear()
    annexablePlanetIDs.clear()
    for sysID in empire.fleetSupplyableSystemIDs:
        annexableSystemIDs.add(sysID)
        for nID in  universe.getImmediateNeighbors(sysID,  empireID):
            annexableSystemIDs.add(nID)
            annexableRing1.add(nID)
    annexableRing1.difference_update(empire.fleetSupplyableSystemIDs)
    print "First Ring of annexable systems: ",  PlanetUtilsAI.sysNameIDs(annexableRing1)
    if empire.getTechStatus("CON_ORBITAL_CON") == fo.techStatus.complete:
        for sysID in list(annexableRing1):
            for nID in  universe.getImmediateNeighbors(sysID,  empireID):
                annexableRing2.add(nID)
        annexableRing2.difference_update(annexableSystemIDs)
        print "Second Ring of annexable systems: ",  PlanetUtilsAI.sysNameIDs(annexableRing2)
        annexableSystemIDs.update(annexableRing2)
        if foAI.foAIstate.aggression > fo.aggression.cautious:
            for sysID in list(annexableRing2):
                for nID in  universe.getImmediateNeighbors(sysID,  empireID):
                    annexableRing3.add(nID)
            annexableRing3.difference_update(annexableSystemIDs)
            print "Third Ring of annexable systems: ",  PlanetUtilsAI.sysNameIDs(annexableRing3)
            annexableSystemIDs.update(annexableRing3)
    annexablePlanetIDs.update( PlanetUtilsAI.getPlanetsInSystemsIDs(annexableSystemIDs))

    # get outpost and colonization planets
    
    exploredSystemIDs = foAI.foAIstate.getExplorableSystems(AIExplorableSystemType.EXPLORABLE_SYSTEM_EXPLORED)
    unExSysIDs = list(foAI.foAIstate.getExplorableSystems(AIExplorableSystemType.EXPLORABLE_SYSTEM_UNEXPLORED))
    unExSystems = map(universe.getSystem,  unExSysIDs)
    print "Unexplored Systems: %s " % [(sysID,  (sys and sys.name) or "name unknown") for sysID,  sys in zip( unExSysIDs,  unExSystems)]
    print "Explored SystemIDs: " + str(list(exploredSystemIDs))

    exploredPlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(exploredSystemIDs)
    print "Explored PlanetIDs: " + str(exploredPlanetIDs)
    print ""
    
    #visibleSystemIDs = foAI.foAIstate.visInteriorSystemIDs.keys() + foAI.foAIstate. visBorderSystemIDs.keys()
    #visiblePlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(visibleSystemIDs)
    #print "VisiblePlanets: %s "%[ (pid,  (universe.getPlanet(pid) and  universe.getPlanet(pid).name) or "unknown") for pid in  visiblePlanetIDs]
    #print ""
    
    #accessibleSystemIDs = [sysID for sysID in visibleSystemIDs if  universe.systemsConnected(sysID, homeSystemID, empireID) ]
    #acessiblePlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(accessibleSystemIDs)

    empireOwnedPlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    print "Empire Owned PlanetIDs:            " + str(empireOwnedPlanetIDs)
    
    #allOwnedPlanetIDs = PlanetUtilsAI.getAllOwnedPlanetIDs(exploredPlanetIDs) #working with Explored systems not all 'visible' because might not have a path to the latter
    allOwnedPlanetIDs = PlanetUtilsAI.getAllOwnedPlanetIDs(annexablePlanetIDs) #
    print "All annexable Owned or Populated PlanetIDs: " + str(set(allOwnedPlanetIDs)-set(empireOwnedPlanetIDs))

    #unOwnedPlanetIDs = list(set(exploredPlanetIDs) -set(allOwnedPlanetIDs))
    unOwnedPlanetIDs = list(set(annexablePlanetIDs) -set(allOwnedPlanetIDs))
    print "UnOwned annexable PlanetIDs:             " + str(PlanetUtilsAI.planetNameIDs(unOwnedPlanetIDs))
    
    empirePopCtrs = set( PlanetUtilsAI.getPopulatedPlanetIDs(  empireOwnedPlanetIDs) )
    empireOutpostIDs=set(empireOwnedPlanetIDs) - empirePopCtrs
    AIstate.popCtrIDs[:]=list(empirePopCtrs)
    AIstate.popCtrSystemIDs[:]=list(set(PlanetUtilsAI.getSystems(empirePopCtrs)))
    AIstate.outpostIDs[:]=list(empireOutpostIDs)
    AIstate.outpostSystemIDs[:]=list(set(PlanetUtilsAI.getSystems(empireOutpostIDs)))
    AIstate.colonizedSystems.clear()
    for pid in empireOwnedPlanetIDs:
        planet=universe.getPlanet(pid)
        if planet:
            AIstate.colonizedSystems.setdefault(planet.systemID,  []).append(pid)   # track these to plan Solar Generators and Singularity Generators, etc.
    AIstate.empireStars.clear()
    for sysID in AIstate.colonizedSystems:
        system = universe.getSystem(sysID)
        if system:
            AIstate.empireStars.setdefault(system.starType, []).append(sysID)

    claimedStars = {}
    for sType in AIstate.empireStars:
        claimedStars[sType] = list( AIstate.empireStars[sType] )
    for sysID in set( AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs):
        tSys = universe.getSystem(sysID)
        if not tSys: continue
        claimedStars.setdefault( tSys.starType, []).append(sysID)
    #foAI.foAIstate.misc['claimedStars'] = claimedStars
    
    
    oldPopCtrs=[]
    for specN in empireSpecies:
        oldPopCtrs.extend(empireSpecies[specN])
    oldEmpSpec = {}
    oldEmpSpec.update(empireSpecies)
    empireSpecies.clear()
    oldEmpCol= {}
    oldEmpCol.update(empireColonizers)
    empireColonizers.clear()
    empireShipBuilders.clear()
    empireMetabolisms.clear()
    availableGrowthSpecials.clear()
    activeGrowthSpecials.clear()
    empirePlanetsWithGrowthSpecials.clear()
    if empire.getTechStatus(TechsListsAI.exobotTechName) == fo.techStatus.complete:
        empireColonizers["SP_EXOBOT"]=[]# get it into colonizer list even if no colony yet
    empireSpeciesSystems.clear()
    
    for pID in empirePopCtrs:
        planet=universe.getPlanet(pID)
        if not planet:
            print "Error empire has apparently lost sight of former colony at planet %d but doesn't realize it"%pID
            continue
        pSpecName=planet.speciesName
        if pID not in oldPopCtrs:
            if  (AIFocusType.FOCUS_MINING in planet.availableFoci): 
                fo.issueChangeFocusOrder(pID, AIFocusType.FOCUS_MINING)
                print "Changing focus of newly acquired planet ID %d : %s  to mining "%(pID,  planet.name )
        empireSpecies[pSpecName] = empireSpecies.get(pSpecName,  [])+[pID]
    print "\n"+"Empire species roster:"
    for specName in empireSpecies:
        thisSpec=fo.getSpecies(specName)
        if thisSpec:
            thisMetab = [ tag for tag in thisSpec.tags if tag in  AIDependencies.metabolims]
            shipyards=[]
            for pID in empireSpecies[specName]:
                planet=universe.getPlanet(pID)
                if not planet: continue
                for metab in thisMetab:
                    #prev = empireMetabolisms.get(metab,  [0.0,  0.0] )
                    prev = empireMetabolisms.get(metab,  0.0 )
                    #prev[0] += planet.currentMeterValue(fo.meterType.population)
                    #prev[1] += planet.size
                    prev  += planet.size
                    empireMetabolisms[metab] = prev
                for special in planet.specials:
                    if special in AIDependencies.metabolimBoosts:
                        empirePlanetsWithGrowthSpecials.setdefault(pID,  []).append(special)
                        availableGrowthSpecials.setdefault(special,  []).append(pID)
                        if planet.focus == AIFocusType.FOCUS_GROWTH:
                            activeGrowthSpecials.setdefault(special,  []).append(pID)
                if  thisSpec.canProduceShips:
                    if "BLD_SHIPYARD_BASE" in [universe.getObject(bldg).buildingTypeName for bldg in planet.buildingIDs]:
                        shipyards.append(pID)
                empireSpeciesSystems.setdefault(planet.systemID,  {}).setdefault('pids', []).append(pID)
            if thisSpec.canProduceShips:
                empireShipBuilders[specName]=shipyards
                if thisSpec.canColonize:
                    empireColonizers[specName]=shipyards
            print "%s on planets %s; can%s colonize from %d shipyards; has tags %s"%(specName,  empireSpecies[specName],  ["not", ""][thisSpec.canColonize and thisSpec.canProduceShips], len(shipyards),  list(thisSpec.tags))
        else:
            print "Unable to retrieve info for Species named %s"%specName
    print""
    if empireSpecies!=oldEmpSpec:
        print "Old empire species: %s  ; new empire species: %s"%(oldEmpSpec,  empireSpecies)
    if empireColonizers!=oldEmpCol:
        print "Old empire colonizers: %s  ; new empire colonizers: %s"%(oldEmpCol,  empireColonizers)
    
    print 

    # export colony targeted systems for other AI modules
    colonyTargetedPlanetIDs = getColonyTargetedPlanetIDs(universe.planetIDs, AIFleetMissionType.FLEET_MISSION_COLONISATION, empireID)
    allColonyTargetedSystemIDs = PlanetUtilsAI.getSystems(colonyTargetedPlanetIDs)
    AIstate.colonyTargetedSystemIDs = allColonyTargetedSystemIDs
    print ""
    print "Colony Targeted SystemIDs:         " + str(AIstate.colonyTargetedSystemIDs)
    print "Colony Targeted PlanetIDs:         " + str(colonyTargetedPlanetIDs)

    colonyFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_COLONISATION)
    if not colonyFleetIDs:
        print "Available Colony Fleets:             0"
    else:
        print "Colony FleetIDs:                   " + str(FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_COLONISATION))

    numColonyFleets = len(FleetUtilsAI.extractFleetIDsWithoutMissionTypes(colonyFleetIDs))
    print "Colony Fleets Without Missions:      " + str(numColonyFleets)
    
    

    outpostTargetedPlanetIDs = getOutpostTargetedPlanetIDs(universe.planetIDs, AIFleetMissionType.FLEET_MISSION_OUTPOST, empireID)
    outpostTargetedPlanetIDs.extend( getOutpostTargetedPlanetIDs(universe.planetIDs, AIFleetMissionType.FLEET_MISSION_ORBITAL_OUTPOST, empireID))
    allOutpostTargetedSystemIDs = PlanetUtilsAI.getSystems(outpostTargetedPlanetIDs)

    # export outpost targeted systems for other AI modules
    AIstate.outpostTargetedSystemIDs = allOutpostTargetedSystemIDs
    print ""
    print "Outpost Targeted SystemIDs:        " + str(AIstate.outpostTargetedSystemIDs)
    print "Outpost Targeted PlanetIDs:        " + str(outpostTargetedPlanetIDs)

    outpostFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_OUTPOST)
    if not outpostFleetIDs:
        print "Available Outpost Fleets:            0"
    else:
        print "Outpost FleetIDs:                  " + str(FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_OUTPOST))

    numOutpostFleets = len(FleetUtilsAI.extractFleetIDsWithoutMissionTypes(outpostFleetIDs))
    print "Outpost Fleets Without Missions:     " + str(numOutpostFleets)
    
    availablePP = dict(  [ (tuple(el.key()),  el.data() ) for el in  empire.planetsWithAvailablePP ])  #keys are sets of ints; data is doubles
    availPP_BySys={}
    for pSet in availablePP:
        availPP_BySys.update(  [ (sysID, availablePP[pSet]) for sysID in  set(PlanetUtilsAI.getSystems( pSet))] )
    colonyCost=120*(1+ 0.06*len( list(AIstate.popCtrIDs) ))
    outpostCost=80*(1+ 0.06*len( list(AIstate.popCtrIDs) ))
    productionQueue = empire.productionQueue
    queuedBases=[]
    for queue_index  in range(0,  len(productionQueue)):
        element=productionQueue[queue_index]
        if element.buildType == EnumsAI.AIEmpireProductionTypes.BT_SHIP:
             if foAI.foAIstate.getShipRole(element.designID) in  [    EnumsAI.AIShipRoleType.SHIP_ROLE_BASE_OUTPOST ,  
                                                                                                                                        EnumsAI.AIShipRoleType.SHIP_ROLE_BASE_COLONISATION ] :
                 buildPlanet = universe.getPlanet(element.locationID)
                 queuedBases.append(buildPlanet.systemID)

    evaluatedColonyPlanetIDs = list(set(unOwnedPlanetIDs).union(empireOutpostIDs) - set(colonyTargetedPlanetIDs) )
    
    for pid in evaluatedColonyPlanetIDs:
        if pid in  foAI.foAIstate.qualifyingColonyBaseTargets: continue
        planet = universe.getPlanet(pid)
        if not planet: continue
        sysID = planet.systemID
        for pid2 in empireSpeciesSystems.get(sysID,  {}).get('pids', []):
            planet2 = universe.getPlanet(pid2)
            if not planet2: continue
            if planet2.speciesName  in empireColonizers:
                if  outpostCost <  12 * availPP_BySys.get(sysID,  0): #TODO: consider different ratio
                    system=universe.getSystem(sysID)
                    for pid3 in system.planetIDs:
                        if (pid3 not in empirePopCtrs):
                             foAI.foAIstate.qualifyingColonyBaseTargets.setdefault(pid3,  [pid2,  -1])
                    break
    
    # print "Evaluated Colony PlanetIDs:        " + str(evaluatedColonyPlanetIDs)

    reservedBaseTargets = foAI.foAIstate.qualifyingColonyBaseTargets.keys()
    for pid in (set(reservedBaseTargets) - set(outpostTargetedPlanetIDs)):
        if pid not in unOwnedPlanetIDs: continue
        if  foAI.foAIstate.qualifyingColonyBaseTargets[pid][1] != -1: continue  #already building for here
        loc = foAI.foAIstate.qualifyingColonyBaseTargets[pid][0]
        if 100 < evaluatePlanet(pid,  EnumsAI.AIFleetMissionType.FLEET_MISSION_OUTPOST,  fleetSupplyablePlanetIDs,  None,  empire, []): 
            bestShip,  colDesign,  buildChoices = ProductionAI.getBestShipInfo(EnumsAI.AIPriorityType.PRIORITY_PRODUCTION_ORBITAL_OUTPOST,  loc)
            if not bestShip:
                print "Error: no outpost base can be built at ",  PlanetUtilsAI.planetNameIDs([loc])
                continue
            #print "selecting  ",  PlanetUtilsAI.planetNameIDs([pid]),  " to build Orbital Defenses"
            retval  = fo.issueEnqueueShipProductionOrder(bestShip, loc)
            print "Enqueueing Outpost Base at %s for %s"%( PlanetUtilsAI.planetNameIDs([loc]),  PlanetUtilsAI.planetNameIDs([pid]))
            if retval !=0:
                foAI.foAIstate.qualifyingColonyBaseTargets[pid][1] = loc
                #res=fo.issueRequeueProductionOrder(productionQueue.size -1,  0) # move to front

    evaluatedOutpostPlanetIDs = list(set(unOwnedPlanetIDs) - set(outpostTargetedPlanetIDs)- set(colonyTargetedPlanetIDs) - set(reservedBaseTargets))
    
    # print "Evaluated Outpost PlanetIDs:       " + str(evaluatedOutpostPlanetIDs)

    evaluatedColonyPlanets = assignColonisationValues(evaluatedColonyPlanetIDs, AIFleetMissionType.FLEET_MISSION_COLONISATION, fleetSupplyablePlanetIDs, None, empire)
    allColonyOpportunities.clear()
    allColonyOpportunities.update(assignColonisationValues(evaluatedColonyPlanetIDs, AIFleetMissionType.FLEET_MISSION_COLONISATION, fleetSupplyablePlanetIDs, None, empire,  [], True))
    
    sortedPlanets = evaluatedColonyPlanets.items()
    sortedPlanets.sort(lambda x, y: cmp(x[1], y[1]), reverse=True)

    print ""
    print "Settleable Colony Planets (score,species) | ID | Name | Specials:"
    for ID, score in sortedPlanets:
        print "   %15s | %5s  | %s  | %s "%(score,  ID,  universe.getPlanet(ID).name ,  list(universe.getPlanet(ID).specials)) 
    print ""

    sortedPlanets = [(ID, score) for ID, score in sortedPlanets if score[0] > 0]
    # export planets for other AI modules
    foAI.foAIstate.colonisablePlanetIDs = sortedPlanets

    # get outpost fleets
    allOutpostFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_OUTPOST)
    AIstate.outpostFleetIDs = FleetUtilsAI.extractFleetIDsWithoutMissionTypes(allOutpostFleetIDs)

    evaluatedOutpostPlanets = assignColonisationValues(evaluatedOutpostPlanetIDs, AIFleetMissionType.FLEET_MISSION_OUTPOST, fleetSupplyablePlanetIDs, None, empire)
    #removeLowValuePlanets(evaluatedOutpostPlanets) 

    sortedOutposts = evaluatedOutpostPlanets.items()
    sortedOutposts.sort(lambda x, y: cmp(x[1], y[1]), reverse=True)

    print "Settleable Outpost PlanetIDs:"
    for ID, score in sortedOutposts:
        print "   %5s | %5s  | %s  | %s "%(score,  ID,  universe.getPlanet(ID).name ,  list(universe.getPlanet(ID).specials)) 
    print ""

    sortedOutposts = [(ID, score) for ID, score in sortedOutposts if score[0] > 0]
    # export outposts for other AI modules
    foAI.foAIstate.colonisableOutpostIDs = sortedOutposts

def getColonyTargetedPlanetIDs(planetIDs, missionType, empireID):
    "return list being settled with colony planets"

    universe = fo.getUniverse()
    colonyAIFleetMissions = foAI.foAIstate.getAIFleetMissionsWithAnyMissionTypes([missionType])

    colonyTargetedPlanets = []

    for planetID in planetIDs:
        planet = universe.getPlanet(planetID)
        # add planets that are target of a mission
        for colonyAIFleetMission in colonyAIFleetMissions:
            aiTarget = AITarget.AITarget(AITargetType.TARGET_PLANET, planetID)
            if colonyAIFleetMission.hasTarget(missionType, aiTarget):
                colonyTargetedPlanets.append(planetID)

    return colonyTargetedPlanets

def getOutpostTargetedPlanetIDs(planetIDs, missionType, empireID):
    "return list being settled with outposts planets"

    universe = fo.getUniverse()
    outpostAIFleetMissions = foAI.foAIstate.getAIFleetMissionsWithAnyMissionTypes([missionType])

    outpostTargetedPlanets = []

    for planetID in planetIDs:
        planet = universe.getPlanet(planetID)
        # add planets that are target of a mission
        for outpostAIFleetMission in outpostAIFleetMissions:
            aiTarget = AITarget.AITarget(AITargetType.TARGET_PLANET, planetID)
            if outpostAIFleetMission.hasTarget(missionType, aiTarget):
                outpostTargetedPlanets.append(planetID)

    return outpostTargetedPlanets

def assignColonisationValues(planetIDs, missionType, fleetSupplyablePlanetIDs, species, empire,  detail=[],  returnAll=False): #TODO: clean up supplyable versus annexable
    "creates a dictionary that takes planetIDs as key and their colonisation score as value"
    origDetail = detail
    planetValues = {}
    if   (missionType == AIFleetMissionType.FLEET_MISSION_OUTPOST ):
        print "\n=========\nAssigning Outpost Values\n========="
        trySpecies = [ "" ]
    elif species is not None:
        print "\n=========\nAssigning Colony Values\n========="
        if isinstance(species,  str):
            trySpecies = [species]
        elif isinstance(species,  list):
            trySpecies = species
        else:
            trySpecies = [species.name]
    else:
        print "\n=========\nAssigning Colony Values\n========="
        trySpecies = list(  empireColonizers  )
    for planetID in planetIDs:
        pv = []
        for specName in trySpecies:
            detail = origDetail[:]
            pv.append( (evaluatePlanet(planetID, missionType, fleetSupplyablePlanetIDs, specName, empire,  detail),  specName,  list(detail)) )
        allSorted = sorted(pv,  reverse=True)
        best = allSorted[:1]
        if best!=[]:
            if returnAll:
                planetValues[planetID] = allSorted
            else:
                planetValues[planetID] = best[0][:2]
                print best[0][2]
    return planetValues

def evaluatePlanet(planetID, missionType, fleetSupplyablePlanetIDs, specName, empire,  detail = []):
    "returns the colonisation value of a planet"
    species=fo.getSpecies(specName or "") #in case None is passed as specName
    if detail != []:
        detail = []
    discountMultiplier = 20.0
    priorityScaling=1.0
    maxGGGs=1
    if empire.productionPoints <100:
        backupFactor = 0.0
    else:
        backupFactor = min(1.0,  (empire.productionPoints/200.0)**2 )
    
    universe = fo.getUniverse()
    claimedStars= foAI.foAIstate.misc.get('claimedStars',  {} )
    if claimedStars == {}:
        for sType in AIstate.empireStars:
            claimedStars[sType] = list( AIstate.empireStars[sType] )
        for sysID in set( AIstate.colonyTargetedSystemIDs + AIstate.outpostTargetedSystemIDs):
            tSys = universe.getSystem(sysID)
            if not tSys: continue
            claimedStars.setdefault( tSys.starType, []).append(sysID)
    
    empireResearchList = [element.tech for element in empire.researchQueue]
    planet = universe.getPlanet(planetID)
    if (planet == None): 
        VisMap = dictFromMap(universe.getVisibilityTurnsMap(planetID,  empire.empireID))
        print "Planet %d object not available; visMap: %s"%(planetID,  VisMap)
        return 0
    detail.append("%s : "%planet.name )
    system = universe.getSystem(planet.systemID)
    tagList=[]
    starBonus=0
    colonyStarBonus=0
    researchBonus=0
    growthVal = 0
    fixedInd = 0
    fixedRes = 0
    haveExistingPresence=False
    if  AIstate.colonizedSystems.get(planet.systemID,  [planetID]) != [planetID]: #if existing presence is target planet, don't count
        haveExistingPresence=True
    if species:
        tagList = list( species.tags )
    starPopMod=0
    if system:
        alreadyGotThisOne= planet.systemID in (AIstate.popCtrSystemIDs + AIstate.outpostSystemIDs) 
        if "PHOTOTROPHIC" in tagList:
            starPopMod = photoMap.get( system.starType,  0 )
            detail.append( "PHOTOTROPHIC popMod %.1f"%starPopMod    )
        if (empire.getTechStatus("PRO_SOL_ORB_GEN") == fo.techStatus.complete) or (  "PRO_SOL_ORB_GEN"  in empireResearchList[:8])  :    
            if system.starType in [fo.starType.blue, fo.starType.white]:
                if len (claimedStars.get(fo.starType.blue,  [])+claimedStars.get(fo.starType.white,  []))==0:
                    starBonus +=20* discountMultiplier
                    detail.append( "PRO_SOL_ORB_GEN BW  %.1f"%(20* discountMultiplier)    )
                elif   not alreadyGotThisOne:
                    starBonus +=10*discountMultiplier*backupFactor #still has extra value as an alternate location for solar generators
                    detail.append( "PRO_SOL_ORB_GEN BW Backup Location  %.1f"%(10* discountMultiplier *backupFactor)   )
                elif fo.currentTurn() > 100:  #lock up this whole system
                    pass
                    #starBonus += 5 #TODO: how much?
                    #detail.append( "PRO_SOL_ORB_GEN BW LockingDownSystem   %.1f"%5  )
            if system.starType in [fo.starType.yellow, fo.starType.orange]:
                if len (     claimedStars.get(fo.starType.blue,  [])+claimedStars.get(fo.starType.white,  [])+
                                    claimedStars.get(fo.starType.yellow,  [])+claimedStars.get(fo.starType.orange,  []))==0:
                    starBonus +=10* discountMultiplier
                    detail.append( "PRO_SOL_ORB_GEN YO  %.1f"%(10* discountMultiplier ))
                else:
                    pass
                    #starBonus +=2 #still has extra value as an alternate location for solar generators
                    #detail.append( "PRO_SOL_ORB_GEN YO Backup  %.1f"%2 )
        if system.starType in [fo.starType.blackHole] and fo.currentTurn() > 100:
            if not alreadyGotThisOne:
                starBonus +=10*discountMultiplier*backupFactor #whether have tech yet or not, assign some base value
                detail.append( "Black Hole %.1f"%(10* discountMultiplier*backupFactor)    )
            else:
                starBonus += 5*discountMultiplier*backupFactor
                detail.append( "Black Hole Backup %.1f"%(5* discountMultiplier*backupFactor )   )
        if (empire.getTechStatus("PRO_SINGULAR_GEN") == fo.techStatus.complete) or (  "PRO_SINGULAR_GEN"  in empireResearchList[:8])  :    
            if system.starType in [fo.starType.blackHole] :
                if len (claimedStars.get(fo.starType.blackHole,  []))==0:
                    starBonus +=200*discountMultiplier #pretty rare planets, good for generator
                    detail.append( "PRO_SINGULAR_GEN %.1f"%(200* discountMultiplier  )  )
                elif  planet.systemID not in claimedStars.get(fo.starType.blackHole,  []):
                    starBonus +=100*discountMultiplier*backupFactor #still has extra value as an alternate location for generators & for blocking enemies generators
                    detail.append( "PRO_SINGULAR_GEN Backup %.1f"%(100* discountMultiplier*backupFactor  )  )
            elif system.starType in [fo.starType.red] and ( len (claimedStars.get(fo.starType.blackHole,  [])) )==0:
                rfactor = (1.0+len (claimedStars.get(fo.starType.red,  [])))**(-2)
                starBonus +=40*discountMultiplier*backupFactor*rfactor # can be used for artificial black hole
                detail.append( "Red Star for Art Black Hole  %.1f"%(20* discountMultiplier*backupFactor*rfactor  )  )
        if (empire.getTechStatus("PRO_NEUTRONIUM_EXTRACTION") == fo.techStatus.complete) or (  "PRO_NEUTRONIUM_EXTRACTION"  in empireResearchList[:8])  :    
            if system.starType in [fo.starType.neutron]:
                if len (claimedStars.get(fo.starType.neutron,  []))==0:
                    starBonus +=80*discountMultiplier #pretty rare planets, good for armor
                    detail.append( "PRO_NEUTRONIUM_EXTRACTION  %.1f"%(80* discountMultiplier  )  )
                else:
                    starBonus +=20*discountMultiplier*backupFactor #still has extra value as an alternate location for generators & for bnlocking enemies generators
                    detail.append( "PRO_NEUTRONIUM_EXTRACTION Backup  %.1f"%(20* discountMultiplier*backupFactor  )  )
        if (empire.getTechStatus("SHP_ENRG_BOUND_MAN") == fo.techStatus.complete) or (  "SHP_ENRG_BOUND_MAN"  in empireResearchList[:6])  :    
            if system.starType in [fo.starType.blackHole,  fo.starType.blue] :
                if len (claimedStars.get(fo.starType.blackHole,  [])  +  claimedStars.get(fo.starType.blue,  []) )    ==0:
                    colonyStarBonus +=100*discountMultiplier #pretty rare planets, good for generator
                    detail.append( "SHP_ENRG_BOUND_MAN  %.1f"%(100* discountMultiplier  )  )
                elif  planet.systemID not in (claimedStars.get(fo.starType.blackHole,  [])  +  claimedStars.get(fo.starType.blue,  []) ):
                    colonyStarBonus +=50*discountMultiplier*backupFactor #still has extra value as an alternate location for generators & for bnlocking enemies generators
                    detail.append( "SHP_ENRG_BOUND_MAN Backup  %.1f"%(50* discountMultiplier*backupFactor  )  )
    retval = starBonus
    
    planetSpecials = list(planet.specials)
    if "ECCENTRIC_ORBIT_SPECIAL" in planet.specials:
        fixedRes += discountMultiplier*2*3
        detail.append( "ECCENTRIC_ORBIT_SPECIAL  %.1f"%(discountMultiplier*2*3  )  )
        
    if ( "ANCIENT_RUINS_SPECIAL" in planet.specials ): #TODO: add value for depleted ancient ruins
        retval += discountMultiplier*20
        detail.append("Undepleted Ruins %.1f"%discountMultiplier*20)
        
    if   (missionType == AIFleetMissionType.FLEET_MISSION_OUTPOST ):
        for special in planetSpecials:
            if "_NEST_" in special:
                retval+=5*discountMultiplier*backupFactor # get an outpost on the nest quick
                detail.append( "%s  %.1f"%(special,  discountMultiplier*5*backupFactor  )  )
        if   ( planet.size  ==  fo.planetSize.asteroids ): 
            if (empire.getTechStatus("PRO_MICROGRAV_MAN") == fo.techStatus.complete ): 
                if system:
                    astVal=0
                    for pid in system.planetIDs:
                        otherPlanet=universe.getPlanet(pid)
                        if otherPlanet.size == fo.planetSize.asteroids:
                            if pid==planetID:
                                continue
                            elif pid < planetID:
                                astVal=0
                                break
                        elif otherPlanet.size!= fo.planetSize.gasGiant and otherPlanet.owner==empire.empireID and otherPlanet.speciesName!="":
                            astVal+=5 * discountMultiplier
                    retval += astVal
                    if astVal >0:
                        detail.append( "AsteroidMining %.1f"%(astVal  )  )
            if (empire.getTechStatus("SHP_ASTEROID_HULLS") == fo.techStatus.complete ) or (  "SHP_ASTEROID_HULLS"  in empireResearchList[:3]) :
                if system:
                    astVal=0
                    for pid in system.planetIDs:
                        otherPlanet=universe.getPlanet(pid)
                        if otherPlanet.size == fo.planetSize.asteroids:
                            if pid==planetID:
                                continue
                            elif pid < planetID:
                                astVal=0
                                break
                        elif otherPlanet.size!= fo.planetSize.gasGiant and otherPlanet.owner==empire.empireID and otherPlanet.speciesName!="":
                            otherSpecies = fo.getSpecies(otherPlanet.speciesName)
                            if otherSpecies and otherSpecies.canProduceShips:
                                astVal+=20 * discountMultiplier
                    retval += astVal
                    if astVal >0:
                        detail.append( "AsteroidShipBuilding %.1f"%(astVal  )  )
        if  ( ( planet.size  ==  fo.planetSize.gasGiant ) and  ( (empire.getTechStatus("PRO_ORBITAL_GEN") == fo.techStatus.complete ) or (  "PRO_ORBITAL_GEN"  in empireResearchList[:3]) )):
            if system:
                GGList=[]
                orbGenVal=0
                GGDetail=[]
                for pid in system.planetIDs:
                    otherPlanet=universe.getPlanet(pid)
                    if otherPlanet.size== fo.planetSize.gasGiant:
                        GGList.append(pid)
                    if  pid!=planetID and otherPlanet.owner==empire.empireID and (AIFocusType.FOCUS_INDUSTRY  in list(otherPlanet.availableFoci)+[otherPlanet.focus]):
                        orbGenVal+=2*10*discountMultiplier
                        GGDetail.append( "GGG  for %s  %.1f"%(otherPlanet.name,    discountMultiplier*10  )  )
                if planetID in sorted(GGList)[:maxGGGs]:
                    retval += orbGenVal
                    detail.extend( GGDetail )
                else:
                    detail.append( "Won't GGG")
        thrtFactor = 1.0
        if ( foAI.foAIstate.systemStatus.get(planet.systemID,  {}).get('fleetThreat', 0)  + foAI.foAIstate.systemStatus.get(planet.systemID,  {}).get('monsterThreat', 0) )> 2*curBestMilShipRating:
            thrtFactor = 0.5
            retval *=thrtFactor
            detail.append( "threat reducing value" )
        if haveExistingPresence:
            detail.append("multiplanet presence")
            if thrtFactor < 1.0:
                retval = (retval/thrtFactor) * (0.5 + 0.5*thrtFactor) #mitigate threat
            retval *=1.5
        return int(retval)
    else: #colonization mission
        if not species:
            return 0
        retval += fixedRes
        retval += colonyStarBonus
        asteroidBonus=0
        gasGiantBonus=0
        GGPresent=False
        miningBonus=0
        perGGG=2*10*discountMultiplier
        planetSize = planet.size
        if system and AIFocusType.FOCUS_INDUSTRY in species.foci:
            gotAsteroids=False
            for pid  in [id for id in system.planetIDs if id != planetID]:
                p2 = universe.getPlanet(pid)
                if p2:
                    if p2.size== fo.planetSize.asteroids and not gotAsteroids :
                        gotAsteroids = True
                        if ( (empire.getTechStatus("PRO_MICROGRAV_MAN") == fo.techStatus.complete ) or (  "PRO_MICROGRAV_MAN"  in empireResearchList[:3]) ):
                            asteroidBonus = 2*5*discountMultiplier
                            detail.append( "Asteroid mining from %s  %.1f"%(p2.name,    2*discountMultiplier*5  )  )
                        if (empire.getTechStatus("SHP_ASTEROID_HULLS") == fo.techStatus.complete ) or (  "SHP_ASTEROID_HULLS"  in empireResearchList[:3]) :
                            if species and species.canProduceShips:
                                asteroidBonus += 20*discountMultiplier
                                detail.append( "Asteroid ShipBuilding from %s  %.1f"%(p2.name,    2*discountMultiplier*20  )  )
                    if p2.size== fo.planetSize.gasGiant :
                        GGPresent=True
                        if ( (empire.getTechStatus("PRO_ORBITAL_GEN") == fo.techStatus.complete ) or (  "PRO_ORBITAL_GEN"  in empireResearchList[:3]) ):
                            gasGiantBonus += perGGG
                            detail.append( "GGG  from %s  %.1f"%(p2.name,    perGGG  )  )
        gasGiantBonus = min( gasGiantBonus,  maxGGGs * perGGG )
        if   (planet.size==fo.planetSize.gasGiant):
            if not (species and species.name  ==  "SP_SUPER_TEST"): 
                detail.append("Can't Settle GG" )
                return 0
            else:
                planetEnv = fo.planetEnvironment.adequate#I think
                planetSize=6 #I think
        elif ( planet.size  ==  fo.planetSize.asteroids ):
            planetSize=3 #I think
            if  not species or (species.name not in [  "SP_EXOBOT", "SP_SUPER_TEST"  ]):
                detail.append( "Can't settle Asteroids" )
                return 0
            elif species.name == "SP_EXOBOT":
                planetEnv =fo.planetEnvironment.poor
            elif species.name == "SP_SUPER_TEST":
                planetEnv = fo.planetEnvironment.adequate#I think
            else:
                return 0
        else:
            planetEnv  = environs[ str(species.getPlanetEnvironment(planet.type)) ]
        if planetEnv==0:
            return -9999
        popSizeMod=0
        conditionalPopSizeMod=0
        popSizeMod += popSizeModMap["env"][planetEnv]
        detail.append("EnvironPopSizeMod(%d)"%popSizeMod)
        if "SELF_SUSTAINING" in tagList:
            popSizeMod*=2
            detail.append("SelfSustaining_PSM(2)" )
        if "PHOTOTROPHIC" in tagList:
            popSizeMod += starPopMod
            detail.append("Phototropic Star Bonus_PSM(%0.1f)"%starPopMod)
        if (empire.getTechStatus("GRO_SUBTER_HAB") == fo.techStatus.complete)  or "TUNNELS_SPECIAL" in planetSpecials:    
            if "TECTONIC_INSTABILITY_SPECIAL" not in planetSpecials:
                conditionalPopSizeMod += popSizeModMap["subHab"][planetEnv]
                if "TUNNELS_SPECIAL" in planetSpecials:
                    T_reason="Tunnels_PSM(%d)"
                else:
                    T_reason="Sub_Hab_PSM(%d)"
                detail.append(T_reason%popSizeModMap["subHab"][planetEnv])
        for gTech,  gKey in [ ("GRO_SYMBIOTIC_BIO", "symBio"), 
                                                        ("GRO_XENO_GENETICS", "xenoGen"), 
                                                        ("GRO_XENO_HYBRID", "xenoHyb"), 
                                                        ("GRO_CYBORG", "cyborg"), 
                                                        ("CON_NDIM_STRUC", "ndim"), 
                                                        ("CON_ORBITAL_HAB", "orbit") ]:
            if empire.getTechStatus(gTech) == fo.techStatus.complete:
                popSizeMod += popSizeModMap[gKey][planetEnv]
                detail.append("%s_PSM(%d)"%(gKey,  popSizeModMap[gKey][planetEnv]))

        if "GAIA_SPECIAL" in planet.specials:
            popSizeMod += 3
            detail.append("Gaia_PSM(3)")

        for special in [ "SLOW_ROTATION_SPECIAL",  "SOLID_CORE_SPECIAL"] :
            if special in planetSpecials:
                popSizeMod -= 1
                detail.append("%s_PSM(-1)"%special)

        applicableBoosts=set([])
        for thisTag in [ tag for tag in tagList if tag in  AIDependencies.metabolims]:
            metabBoosts= AIDependencies.metabolimBoostMap.get(thisTag,  [])
            if popSizeMod > 0:
                for key in activeGrowthSpecials.keys():
                    if  ( len(activeGrowthSpecials[key])>0 ) and ( key in metabBoosts ):
                        applicableBoosts.add(key)
                        detail.append("%s boost active"%key)
            for boost in metabBoosts:
                if boost in planetSpecials:
                    applicableBoosts.add(boost)
                    detail.append("%s boost present"%boost)
        
        nBoosts = len(applicableBoosts)
        if nBoosts:
            popSizeMod += nBoosts
            detail.append("boosts_PSM(%d from %s)"%(nBoosts, applicableBoosts))
        if popSizeMod > 0:
            popSizeMod += conditionalPopSizeMod

        popSize = planetSize * popSizeMod
        detail.append("baseMaxPop size*psm %d * %d = %d"%(planetSize,  popSizeMod,  popSize) )

        if "DIM_RIFT_MASTER_SPECIAL" in planet.specials:
            popSize -= 4
            detail.append("DIM_RIFT_MASTER_SPECIAL(maxPop-4)")
        detail.append("maxPop %.1f"%popSize)

        for special in [ "MINERALS_SPECIAL",  "CRYSTALS_SPECIAL",  "METALOIDS_SPECIAL"] : 
            if special in planetSpecials:
                miningBonus+=1
        
        proSingVal = [0, 4][(len( claimedStars.get(fo.starType.blackHole,  [])) > 0)]
        basePopInd=0.2
        indMult=1
        indTechMap={    "GRO_ENERGY_META":  0.5, 
                                            "PRO_ROBOTIC_PROD":0.4, 
                                            "PRO_FUSION_GEN":       1.0, 
                                            "PRO_INDUSTRY_CENTER_I": 1, 
                                            "PRO_INDUSTRY_CENTER_II":1, 
                                            "PRO_INDUSTRY_CENTER_III":1, 
                                            "PRO_SOL_ORB_GEN":  2.0,   #assumes will build a gen at a blue/white star
                                            "PRO_SINGULAR_GEN": proSingVal, 
                                            }
                                            
        for tech in indTechMap:
            if (empire.getTechStatus(tech) == fo.techStatus.complete):
                indMult += indTechMap[tech]
        indVal = 0
        if (empire.getTechStatus("PRO_SENTIENT_AUTOMATION") == fo.techStatus.complete):
            fixedInd += discountMultiplier * 5
        if AIFocusType.FOCUS_INDUSTRY  in species.foci:
            indVal += discountMultiplier * basePopInd * popSize*miningBonus
            indVal += discountMultiplier * basePopInd * popSize * indMult
        detail.append("indVal %.1f"%indVal)
        # used to give preference to closest worlds
        empireID = empire.empireID
        capitalID = PlanetUtilsAI.getCapital()
        homeworld = universe.getPlanet(capitalID)
        if homeworld:
            homeSystemID = homeworld.systemID
            evalSystemID = planet.systemID
            leastJumpsPath = len(universe.leastJumpsPath(homeSystemID, evalSystemID, empireID))
            distanceFactor = 1.001 / (leastJumpsPath + 1)
        else:
            distanceFactor = 0

        for special in [ spec for spec in  planetSpecials if spec in AIDependencies.metabolimBoosts]:
            gbonus =  discountMultiplier  * basePopInd  * indMult * empireMetabolisms.get( AIDependencies.metabolimBoosts[special] , 0)#  due to growth applicability to other planets
            growthVal += gbonus
            detail.append( "Bonus for %s: %.1f"%(special,  gbonus))

        basePopRes = 0.2 #will also be doubling value of research, below
        if AIFocusType.FOCUS_RESEARCH in species.foci:
            researchBonus += discountMultiplier*2*basePopRes*popSize
            if ( "ANCIENT_RUINS_SPECIAL" in planet.specials )  or ( "ANCIENT_RUINS_DEPLETED_SPECIAL" in planet.specials ):
                researchBonus += discountMultiplier*2*basePopRes*popSize*5
                detail.append("Ruins Research")
            if "COMPUTRONIUM_SPECIAL" in planet.specials:
                researchBonus += discountMultiplier*2*10    #TODO: do actual calc
                detail.append("COMPUTRONIUM_SPECIAL")

        if popSize <= 0:
            detail.append("Non-positive population projection for species '%s',  so no colonization value"%(species and species.name))
            return 0

        retval  += max(indVal+asteroidBonus+gasGiantBonus,  researchBonus,  growthVal)+fixedInd + fixedRes
        if planet.systemID in annexableRing1:
            retval += 10
        elif planet.systemID in annexableRing2:
            retval += 20
        elif planet.systemID in annexableRing3:
            retval += 10
        
        retval *= priorityScaling
        thrtRatio = (foAI.foAIstate.systemStatus.get(planet.systemID,  {}).get('fleetThreat', 0)+foAI.foAIstate.systemStatus.get(planet.systemID,  {}).get('monsterThreat', 0)+0.2*foAI.foAIstate.systemStatus.get(planet.systemID,  {}).get('neighborThreat', 0)) / float(curBestMilShipRating)
        if False:
            if thrtRatio > 4:
                retval = 0.3*retval 
            elif thrtRatio >= 2:
                retval = 0.7* retval
            elif thrtRatio > 0:
                retval = 0.85* retval
                
        thrtFactor = 1.0
        if thrtRatio > 1:
            detail.append("threat reducing value")
            thrtFactor = 0.85
            retval  *= thrtFactor
            
        if haveExistingPresence:
            detail.append("multiplanet presence")
            if thrtFactor < 1.0:
                retval = (retval/thrtFactor) * (0.5 + 0.5*thrtFactor) #mitigate threat
            retval *=1.5

    return retval

def removeLowValuePlanets(evaluatedPlanets):
    "removes all planets with a colonisation value < minimalColoniseValue"

    removeIDs = []
    minVal = AIstate.minimalColoniseValue
    if foAI.foAIstate.aggression <fo.aggression.typical:
        minVal *= 3

    # print ":: min:" + str(AIstate.minimalColoniseValue)
    for planetID in evaluatedPlanets.iterkeys():
        #print ":: eval:" + str(planetID) + " val:" + str(evaluatedPlanets[planetID])
        if (evaluatedPlanets[planetID][0] < minVal):
            removeIDs.append(planetID)
    #print "removing ",  removeIDs
    for ID in removeIDs: del evaluatedPlanets[ID]

def assignColonyFleetsToColonise():
    
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    fleetSupplyableSystemIDs = empire.fleetSupplyableSystemIDs
    fleetSupplyablePlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(fleetSupplyableSystemIDs)
    
    allOutpostBaseFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_ORBITAL_OUTPOST)
    availOutpostBaseFleetIDs = FleetUtilsAI.extractFleetIDsWithoutMissionTypes(allOutpostBaseFleetIDs)
    for fid in availOutpostBaseFleetIDs:
        fleet = universe.getFleet(fid)
        if not fleet: continue
        sysID = fleet.systemID
        system = universe.getSystem(sysID)
        availPlanets = set(system.planetIDs).intersection(set( foAI.foAIstate.qualifyingColonyBaseTargets.keys()))
        targets = [pid for pid in availPlanets if foAI.foAIstate.qualifyingColonyBaseTargets[pid][1] != -1 ]
        if not targets: 
            print "Error found no valid target for outpost base in system %s (%d)"%(system.name,  sysID)
            continue
        
        targetID=-1
        bestScore=-1
        for pid,  rating in assignColonisationValues(targets, AIFleetMissionType.FLEET_MISSION_OUTPOST, fleetSupplyablePlanetIDs, None, empire).items():
            if rating[0]>bestScore:
                bestScore = rating[0]
                targetID = pid
        foAI.foAIstate.qualifyingColonyBaseTargets[targetID][1] = -1 #TODO: should probably delete
        aiTarget = AITarget.AITarget(AITargetType.TARGET_PLANET, targetID)
        aiFleetMission = foAI.foAIstate.getAIFleetMission(fid)
        aiFleetMission.addAITarget(AIFleetMissionType.FLEET_MISSION_ORBITAL_OUTPOST, aiTarget)
    
    # assign fleet targets to colonisable planets
    sendColonyShips(AIstate.colonyFleetIDs, foAI.foAIstate.colonisablePlanetIDs, AIFleetMissionType.FLEET_MISSION_COLONISATION)

    # assign fleet targets to colonisable outposts
    sendColonyShips(AIstate.outpostFleetIDs, foAI.foAIstate.colonisableOutpostIDs, AIFleetMissionType.FLEET_MISSION_OUTPOST)


def sendColonyShips(colonyFleetIDs, evaluatedPlanets, missionType):
    "sends a list of colony ships to a list of planet_value_pairs"
    fleetPool = colonyFleetIDs[:]
    tryAll=False
    if   (missionType == AIFleetMissionType.FLEET_MISSION_OUTPOST ):
        cost = 20+AIDependencies.outpostPodCost * ( 1 + len(AIstate.popCtrIDs)*AIDependencies.colonyPodUpkeep )
    else:
        tryAll=True
        cost = 20+AIDependencies.colonyPodCost * ( 1 + len(AIstate.popCtrIDs)*AIDependencies.colonyPodUpkeep )
        if fo.currentTurn() < 50:
            cost *= 0.4  #will be making fast tech progress so value is underestimated
        elif fo.currentTurn() < 80:
            cost *= 0.8  #will be making fast-ish tech progress so value is underestimated

    potentialTargets =   [  (pid, (score, specName)  )  for (pid,  (score, specName) ) in  evaluatedPlanets if score > (0.8 * cost) ]

    print "colony/outpost  ship matching -- fleets  %s to planets %s"%( fleetPool,  evaluatedPlanets)
    
    if tryAll:
        print "trying best matches to current colony ships"
        bestScores= dict(evaluatedPlanets)
        potentialTargets = []
        for pid,  ratings in allColonyOpportunities.items():
            for rating in ratings:
                if rating[0] >= 0.75 * bestScores.get(pid,  [9999])[0]:
                    potentialTargets.append( (pid,  rating ) )
        potentialTargets.sort(lambda x, y: cmp(x[1], y[1]), reverse=True)
    
    #adding a lot of checking here because have been getting mysterious  exception, after too many recursions to get info
    fleetPool=set(fleetPool)
    universe=fo.getUniverse()
    empireID=fo.empireID()
    destroyedObjIDs = universe.destroyedObjectIDs(empireID)
    for fid in fleetPool:
        fleet = universe.getFleet(fid)
        if not fleet  or fleet.empty:
            print "Error: bad fleet ( ID %d ) given to colonization routine; will be skipped"%fid
            fleetPool.remove(fid)
            continue
        reportStr="Fleet ID (%d): %d ships; species: "%(fid,  fleet.numShips)
        for sid in fleet.shipIDs:
            ship = universe.getShip(sid)
            if not ship:
                reportStr += "NoShip, "
            else:
                reportStr += "%s,  "%ship.speciesName
        print reportStr
    print
    alreadyTargeted = []
    #for planetID_value_pair in evaluatedPlanets:
    while (len(fleetPool) > 0 ) and ( len(potentialTargets) >0):
        thisTarget = potentialTargets.pop(0)
        if thisTarget in alreadyTargeted:
            continue
        thisScore=thisTarget[1][0]
        thisPlanetID=thisTarget[0]
        if thisPlanetID in alreadyTargeted:
            continue
        thisPlanet = universe.getPlanet(thisPlanetID)
        print "checking pool %s against target %s  current owner %s  targetSpec %s"%(fleetPool,  thisPlanet.name,  thisPlanet.owner,  thisTarget)
        thisSysID = thisPlanet.systemID
        if (foAI.foAIstate.systemStatus.setdefault(thisSysID, {}).setdefault('monsterThreat', 0) > 2000) or (fo.currentTurn() <20  and foAI.foAIstate.systemStatus[thisSysID]['monsterThreat'] > 200):
            print "Skipping colonization of system %s due to Big Monster,  threat %d"%(PlanetUtilsAI.sysNameIDs([thisSysID]),  foAI.foAIstate.systemStatus[thisSysID]['monsterThreat'])
            alreadyTargeted.append( thisPlanetID )
            continue
        thisSpec=thisTarget[1][1]
        foundFleets=[]
        try:
            thisFleetList = FleetUtilsAI.getFleetsForMission(nships=1,  targetStats={},  minStats={},  curStats={},  species=thisSpec,  systemsToCheck=[thisSysID],  systemsChecked=[], 
                                                         fleetPoolSet = fleetPool,   fleetList=foundFleets,  triedFleets=set([]),  verbose=False)
        except:
            continue
        if thisFleetList==[]:
            fleetPool.update(foundFleets)#just to be safe
            continue #must have no compatible colony/outpost ships 
        fleetID = thisFleetList[0]
        alreadyTargeted.append( thisPlanetID )

        aiTarget = AITarget.AITarget(AITargetType.TARGET_PLANET, thisPlanetID)
        aiFleetMission = foAI.foAIstate.getAIFleetMission(fleetID)
        aiFleetMission.addAITarget(missionType, aiTarget)
