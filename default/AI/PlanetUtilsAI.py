import freeOrionAIInterface as fo
import FreeOrionAI as foAI

def sysNameIDs(sysIDs):
    universe = fo.getUniverse()
    res=[]
    for sysID in sysIDs:
        sys = universe.getSystem(sysID)
        if sys:
            res.append( "%s:%d"%(sys.name, sysID ) )
        else:
            res.append("unkown:%d"%sysID )
    return res

def planetNameIDs(planetIDs):
    universe = fo.getUniverse()
    res=[]
    for pid in planetIDs:
        planet = universe.getSystem(pid)
        if planet:
            res.append( "%s:%d"%(planet.name, pid ) )
        else:
            res.append("unkown:%d"%pid )
    return res

def getCapital(): # if no current capital returns planet with biggest pop
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    if empire == None:
        print "Danger Danger! FO can't find an empire for me!!!!"
        return None
    empireID = empire.empireID
    capitalID = empire.capitalID
    homeworld = universe.getPlanet(capitalID)
    if homeworld:
        if homeworld.owner==empireID:
            return capitalID
        else:
            print "Nominal Capitol %s does not appear to be owned by empire %d  %s"%(homeworld.name,  empireID,  empire.name)
    #exploredSystemIDs = empire.exploredSystemIDs
    #exploredPlanetIDs = PlanetUtilsAI.getPlanetsInSystemsIDs(exploredSystemIDs)
    empireOwnedPlanetIDs = getOwnedPlanetsByEmpire(universe.planetIDs, empireID)
    peopledPlanets = getPopulatedPlanetIDs(  empireOwnedPlanetIDs)
    if not peopledPlanets:
        if empireOwnedPlanetIDs:
            return empireOwnedPlanetIDs[0]
        else:
            return None
    popMap = []
    for planetID in peopledPlanets:
        popMap.append( ( universe.getPlanet(planetID).currentMeterValue(fo.meterType.population) ,  planetID) )
    popMap.sort()
    return popMap[-1][-1]

def getCapitalSysID(): 
    capID = getCapital()
    if capID is None:
        return -1
    else:
        return fo.getUniverse().getPlanet(capID).systemID


def getPlanetsInSystemsIDs(systemIDs):
    "return list of planets in systems"

    universe = fo.getUniverse()
    planetIDs = []

    for systemID in systemIDs:
        thesePlanets=set(foAI.foAIstate.systemStatus.get(systemID, {}).get('planets', {}).keys())
        system = universe.getSystem(systemID)
        if system != None: 
            thesePlanets.update(list(system.planetIDs))
        planetIDs.extend(list(thesePlanets)) # added list

    return planetIDs

def getOwnedPlanetsByEmpire(planetIDs, empireID):
    "return list of planets owned by empireID"

    universe = fo.getUniverse()
    result = []

    for planetID in planetIDs:
        planet = universe.getPlanet(planetID)
        #even if our universe says we own it, if we can't see it we must have lost it
        if planet and (not planet.unowned) and planet.ownedBy(empireID) and (universe.getVisibility(planetID,  empireID) >= fo.visibility.partial):  
            result.append(planetID)

    return result
    

def getTypePlanetEmpireOwned(planetType):
    "return list of specific type planets owned by empireID"

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    ownedPlanetIDs = getOwnedPlanetsByEmpire(universe.planetIDs, empireID)

    ownedTypePlanetIDs = []

    for planetID in ownedPlanetIDs:
        planet = universe.getPlanet(planetID)
        if planet.type == planetType:
            ownedTypePlanetIDs.append(planetID)

    return ownedTypePlanetIDs

def getAllOwnedPlanetIDs(planetIDs):
    "return list of all owned and populated planetIDs"

    universe = fo.getUniverse()
    allOwnedPlanetIDs = []

    for planetID in planetIDs:
        planet = universe.getPlanet(planetID)
        if planet:
            planetPopulation = planet.currentMeterValue(fo.meterType.population)
            if not planet.unowned or planetPopulation > 0:
                allOwnedPlanetIDs.append(planetID)

    return allOwnedPlanetIDs
    
def getPopulatedPlanetIDs(planetIDs):
    universe = fo.getUniverse()
    pops=[]
    for planetID in planetIDs:
        planet = universe.getPlanet(planetID)
        if planet.currentMeterValue(fo.meterType.population) >0:
            pops.append(planetID)
    return pops

def getAllPopulatedSystemIDs(planetIDs):
    "return list of all populated systemIDs"

    universe = fo.getUniverse()
    allPopulatedSystemIDs = []
    popPlanets=getPopulatedPlanetIDs(planetIDs)
    return [universe.getPlanet(planetID).systemID for planetID in popPlanets ]

def getSystems(planetIDs):
    "return list of systems containing planetIDs"

    universe = fo.getUniverse()
    systemIDs = []

    for planetID in planetIDs:
        planet = universe.getPlanet(planetID)
        systemID = planet.systemID

        systemIDs.append(systemID)

    return systemIDs
