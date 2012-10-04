import freeOrionAIInterface as fo
import FreeOrionAI as foAI
import AIstate
import FleetUtilsAI
from EnumsAI import AIFleetMissionType, AIExplorableSystemType, AITargetType
import AITarget
import PlanetUtilsAI

# makes these mapped to string version of values in case any sizes become reals instead of int
planetSIzes=            {   str(fo.planetSize.tiny): 1,     str(fo.planetSize.small): 2,    str(fo.planetSize.medium): 3,   str(fo.planetSize.large): 4,    str(fo.planetSize.huge): 5,  str(fo.planetSize.asteroids): 0,  str(fo.planetSize.gasGiant): 0 }
environs =                  { str(fo.planetEnvironment.uninhabitable): 0,  str(fo.planetEnvironment.hostile): 1,  str(fo.planetEnvironment.poor): 2,  str(fo.planetEnvironment.adequate): 3,  str(fo.planetEnvironment.good):4 }
#   mods per environ    uninhab   hostile    poor   adequate    good
popSizeModMap={
                            "env":          [ 0, -3, -1, 0,  3 ], 
                            "subHab":   [ 0,  1,  1,  1,  1 ], 
                            "symBio":   [ 0,  1,  1,  1,  0 ], 
                            "xenoGen": [ 0,  1,  1,  1,  1 ], 
                            "xenoHyb": [ 0,  1,  1,  1,  0 ], 
                            "cyborg":   [ 0,  1,  1,  1,  0 ], 
                            "ndim":       [ 0,15,15,15,15], 
                            "orbit":     [ 0,  5,  5,  5,  5 ], 
                            }


def getColonyFleets():
    "get colony fleets"

    allColonyFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_COLONISATION)
    AIstate.colonyFleetIDs = FleetUtilsAI.extractFleetIDsWithoutMissionTypes(allColonyFleetIDs)

    # get suppliable systems and planets
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    capitalID = PlanetUtilsAI.getCapital()
    #capitalID = empire.capitalID
    homeworld = universe.getPlanet(capitalID)
    if homeworld:
        speciesName = homeworld.speciesName
        homeworldName=homeworld.name
    else:
        speciesName = ""
        homeworldName=" no remaining homeworld "
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

    # get outpost and colonization planets
    exploredSystemIDs = foAI.foAIstate.getExplorableSystems(AIExplorableSystemType.EXPLORABLE_SYSTEM_EXPLORED)
    print "Unexplored SystemIDs: " + str(list(foAI.foAIstate.getExplorableSystems(AIExplorableSystemType.EXPLORABLE_SYSTEM_UNEXPLORED)))
    print "Explored SystemIDs: " + str(list(exploredSystemIDs))

    exploredPlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(exploredSystemIDs)
    print "Explored PlanetIDs: " + str(exploredPlanetIDs)
    print ""

    allOwnedPlanetIDs = PlanetUtilsAI.getAllOwnedPlanetIDs(exploredPlanetIDs)
    print "All Owned or Populated PlanetIDs: " + str(allOwnedPlanetIDs)

    empireOwnedPlanetIDs = PlanetUtilsAI.getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    print "Empire Owned PlanetIDs:            " + str(empireOwnedPlanetIDs)

    unOwnedPlanetIDs = list(set(exploredPlanetIDs) -set(allOwnedPlanetIDs))
    print "UnOwned PlanetIDs:             " + str(unOwnedPlanetIDs)
    
    empireOutpostIDs=set(empireOwnedPlanetIDs) - set( PlanetUtilsAI.getPopulatedPlanetIDs(  empireOwnedPlanetIDs) )

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

    print ""
    print "Outpost Targeted SystemIDs:        " + str(AIstate.outpostTargetedSystemIDs)
    outpostTargetedPlanetIDs = getOutpostTargetedPlanetIDs(universe.planetIDs, AIFleetMissionType.FLEET_MISSION_OUTPOST, empireID)
    allOutpostTargetedSystemIDs = PlanetUtilsAI.getSystems(outpostTargetedPlanetIDs)

    # export outpost targeted systems for other AI modules
    AIstate.outpostTargetedSystemIDs = allOutpostTargetedSystemIDs
    print "Outpost Targeted PlanetIDs:        " + str(outpostTargetedPlanetIDs)

    outpostFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_OUTPOST)
    if not outpostFleetIDs:
        print "Available Outpost Fleets:            0"
    else:
        print "Outpost FleetIDs:                  " + str(FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_OUTPOST))

    numOutpostFleets = len(FleetUtilsAI.extractFleetIDsWithoutMissionTypes(outpostFleetIDs))
    print "Outpost Fleets Without Missions:     " + str(numOutpostFleets)

    evaluatedColonyPlanetIDs = list(set(unOwnedPlanetIDs).union(empireOutpostIDs) - set(colonyTargetedPlanetIDs) )
    # print "Evaluated Colony PlanetIDs:        " + str(evaluatedColonyPlanetIDs)

    evaluatedOutpostPlanetIDs = list(set(unOwnedPlanetIDs) - set(outpostTargetedPlanetIDs))
    # print "Evaluated Outpost PlanetIDs:       " + str(evaluatedOutpostPlanetIDs)

    evaluatedColonyPlanets = assignColonisationValues(evaluatedColonyPlanetIDs, AIFleetMissionType.FLEET_MISSION_COLONISATION, fleetSupplyablePlanetIDs, species, empire)
    removeLowValuePlanets(evaluatedColonyPlanets)

    sortedPlanets = evaluatedColonyPlanets.items()
    sortedPlanets.sort(lambda x, y: cmp(x[1], y[1]), reverse=True)

    print ""
    print "Settleable Colony Planets score | ID | Name | Specials:"
    for ID, score in sortedPlanets:
        print "   %5s | %5s  | %s  | %s "%(score,  ID,  universe.getPlanet(ID).name ,  list(universe.getPlanet(ID).specials)) 
    print ""

    # export planets for other AI modules
    AIstate.colonisablePlanetIDs = sortedPlanets

    # get outpost fleets
    allOutpostFleetIDs = FleetUtilsAI.getEmpireFleetIDsByRole(AIFleetMissionType.FLEET_MISSION_OUTPOST)
    AIstate.outpostFleetIDs = FleetUtilsAI.extractFleetIDsWithoutMissionTypes(allOutpostFleetIDs)

    evaluatedOutpostPlanets = assignColonisationValues(evaluatedOutpostPlanetIDs, AIFleetMissionType.FLEET_MISSION_OUTPOST, fleetSupplyablePlanetIDs, species, empire)
    removeLowValuePlanets(evaluatedOutpostPlanets) #bad! lol, was preventing all mining outposts

    sortedOutposts = evaluatedOutpostPlanets.items()
    sortedOutposts.sort(lambda x, y: cmp(x[1], y[1]), reverse=True)

    print "Settleable Outpost PlanetIDs:"
    for ID, score in sortedOutposts:
        print "   %5s | %5s  | %s  | %s "%(score,  ID,  universe.getPlanet(ID).name ,  list(universe.getPlanet(ID).specials)) 
    print ""

    # export outposts for other AI modules
    AIstate.colonisableOutpostIDs = sortedOutposts

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

def assignColonyFleetsToColonise():
    # assign fleet targets to colonisable planets
    sendColonyShips(AIstate.colonyFleetIDs, AIstate.colonisablePlanetIDs, AIFleetMissionType.FLEET_MISSION_COLONISATION)

    # assign fleet targets to colonisable outposts
    sendColonyShips(AIstate.outpostFleetIDs, AIstate.colonisableOutpostIDs, AIFleetMissionType.FLEET_MISSION_OUTPOST)

def assignColonisationValues(planetIDs, missionType, fleetSupplyablePlanetIDs, species, empire):
    "creates a dictionary that takes planetIDs as key and their colonisation score as value"

    planetValues = {}

    for planetID in planetIDs:
        planetValues[planetID] = evaluatePlanet(planetID, missionType, fleetSupplyablePlanetIDs, species, empire)

    return planetValues

def evaluatePlanet(planetID, missionType, fleetSupplyablePlanetIDs, species, empire):
    "returns the colonisation value of a planet"
    # TODO: in planet evaluation consider specials and distance
    universe = fo.getUniverse()
    planet = universe.getPlanet(planetID)
    if (planet == None): return 0
    planetSpecials = list(planet.specials)
    if   (missionType == AIFleetMissionType.FLEET_MISSION_COLONISATION ) and ( planet.size  ==  fo.planetSize.asteroids ) or (planet.size==fo.planetSize.gasGiant): 
        return 0   # currently not supporting species inhabiting asteroids
    elif   (missionType == AIFleetMissionType.FLEET_MISSION_OUTPOST ):
        retval = 0
        if  ( ( planet.size  ==  fo.planetSize.asteroids ) and  (empire.getTechStatus("PRO_ASTEROID_MINE") == fo.techStatus.complete ) ): 
                retval= 10   # asteroid mining is great return
        for special in [ "MINERALS_SPECIAL",  "CRYSTALS_SPECIAL",  "METALOIDS_SPECIAL"] :
            if special in planetSpecials:
                retval = 25
        return retval
            
    planetEnv  = environs[ str(species.getPlanetEnvironment(planet.type)) ]

    popSizeMod=0
    popSizeMod += popSizeModMap["env"][planetEnv]
    if empire.getTechStatus("GRO_SUBTER_HAB") == fo.techStatus.complete:    
        popSizeMod += popSizeModMap["subHab"][planetEnv]
    if empire.getTechStatus("GRO_SYMBIOTIC_BIO") == fo.techStatus.complete:
        popSizeMod += popSizeModMap["symBio"][planetEnv]
    if empire.getTechStatus("GRO_XENO_GENETICS") == fo.techStatus.complete:
        popSizeMod += popSizeModMap["xenoGen"][planetEnv]
    if empire.getTechStatus("GRO_XENO_HYBRID") == fo.techStatus.complete:
        popSizeMod += popSizeModMap["xenoHyb"][planetEnv]
    if empire.getTechStatus("GRO_CYBORG") == fo.techStatus.complete:
        popSizeMod += popSizeModMap["cyborg"][planetEnv]
    
    for special in [ "SLOW_ROTATION_SPECIAL",  "SOLID_CORE_SPECIAL"] :
        if special in planetSpecials:
            popSizeMod -= 1
    
    #have to use these namelists since species tags don't seem available to AI currently
    for special, namelist in [ ("PROBIOTIC_SPECIAL",  ["SP_HUMAN",  "SP_SCYLIOR",  "SP_GYISACHE",  "SP_HHHOH",  "SP_EAXAW"]),
                                                       ("FRUIT_SPECIAL",  ["SP_HUMAN",  "SP_SCYLIOR",  "SP_GYISACHE",  "SP_HHHOH",  "SP_EAXAW",  "SP_TRITH"]),
                                                       ("SPICE_SPECIAL",  ["SP_HUMAN",  "SP_SCYLIOR",  "SP_GYISACHE",  "SP_HHHOH",  "SP_EAXAW"]),
                                                       ("MONOPOLE_SPECIAL",  ["SP_CRAY"]),
                                                       ("SUPERCONDUCTOR_SPECIAL",  ["SP_CRAY"]),
                                                       ("POSITRONIUM_SPECIAL",  ["SP_CRAY"]),
                                                       ("MINERALS_SPECIAL",  ["SP_GEORGE",  "SP_EGASSEM"]),
                                                       ("METALOIDS_SPECIAL",  ["SP_GEORGE",  "SP_EGASSEM"]),
                                                 ]:
        if special in planetSpecials:
            if  species.name in namelist:
                popSizeMod += 1
            #    print "planet %s had special %s that triggers pop mod for species %s"%(planet.name,  special,  species.name)
            #else:
            #    print "planet %s had special %s without pop mod for species %s"%(planet.name,  special,  species.name)
        
    if "GAIA_SPECIAL" in planet.specials:
        popSizeMod += 3

    popSize = planet.size * popSizeMod
    if empire.getTechStatus("CON_NDIM_STRUC") == fo.techStatus.complete:
        popSize += popSizeModMap["ndim"][planetEnv]
    if empire.getTechStatus("CON_ORBITAL_HAB") == fo.techStatus.complete:
        popSize += popSizeModMap["orbit"][planetEnv]

    if "DIM_RIFT_MASTER_SPECIAL" in planet.specials:
        popSize -= 4


    # give preference to closest worlds
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

    # print ">>> evaluatePlanet ID:" + str(planetID) + "/" + str(planet.type) + "/" + str(planet.size) + "/" + str(leastJumpsPath) + "/" + str(distanceFactor)
    if missionType == AIFleetMissionType.FLEET_MISSION_COLONISATION:
        # planet size ranges from 1-5
        if (planetID in fleetSupplyablePlanetIDs):
            return popSize + distanceFactor
            #return getPlanetHospitality(planetID, species) * planet.size + distanceFactor
        else:
            return popSize - distanceFactor
            #return getPlanetHospitality(planetID, species) * planet.size - distanceFactor
    elif missionType == AIFleetMissionType.FLEET_MISSION_OUTPOST:
        planetEnvironment = species.getPlanetEnvironment(planet.type)
        if planetEnvironment == fo.planetEnvironment.uninhabitable:
            # prevent outposts from being built when they cannot get food
            if (planetID in fleetSupplyablePlanetIDs):
                return AIstate.minimalColoniseValue + distanceFactor
            elif (str("GRO_ORBIT_FARMING") in empire.availableTechs):
                return AIstate.minimalColoniseValue + distanceFactor
            else:
                return AIstate.minimalColoniseValue - distanceFactor

def getPlanetHospitality(planetID, species):
    "returns a value depending on the planet type"

    universe = fo.getUniverse()

    planet = universe.getPlanet(planetID)
    if planet == None: return 0

    planetEnvironment = species.getPlanetEnvironment(planet.type)
    # print ":: planet:" + str(planetID) + " type:" + str(planet.type) + " size:" + str(planet.size) + " env:" + str(planetEnvironment)

    # reworked with races
    if planetEnvironment == fo.planetEnvironment.good: return 2.75
    if planetEnvironment == fo.planetEnvironment.adequate: return 1
    if planetEnvironment == fo.planetEnvironment.poor: return 0.5
    if planetEnvironment == fo.planetEnvironment.hostile: return 0.25
    if planetEnvironment == fo.planetEnvironment.uninhabitable: return 0.1

    return 0

def removeLowValuePlanets(evaluatedPlanets):
    "removes all planets with a colonisation value < minimalColoniseValue"

    removeIDs = []

    # print ":: min:" + str(AIstate.minimalColoniseValue)
    for planetID in evaluatedPlanets.iterkeys():
        # print ":: eval:" + str(planetID) + " val:" + str(evaluatedPlanets[planetID])
        if (evaluatedPlanets[planetID] < AIstate.minimalColoniseValue):
            removeIDs.append(planetID)

    for ID in removeIDs: del evaluatedPlanets[ID]

def sendColonyShips(colonyFleetIDs, evaluatedPlanets, missionType):
    "sends a list of colony ships to a list of planet_value_pairs"

    i = 0
    print "colony/outpost  ship matching -- fleets  %s to planets %s"%( colonyFleetIDs,  evaluatedPlanets)
    for planetID_value_pair in evaluatedPlanets:
        if i >= len(colonyFleetIDs): return

        fleetID = colonyFleetIDs[i]
        planetID = planetID_value_pair[0]

        aiTarget = AITarget.AITarget(AITargetType.TARGET_PLANET, planetID)
        aiFleetMission = foAI.foAIstate.getAIFleetMission(fleetID)
        aiFleetMission.addAITarget(missionType, aiTarget)

        i = i + 1
