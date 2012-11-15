import freeOrionAIInterface as fo
import FreeOrionAI as foAI

def getCapital(): # if no current capital returns planet with biggest pop
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    empireID = empire.empireID
    capitalID = empire.capitalID
    homeworld = universe.getPlanet(capitalID)
    if homeworld:
        return capitalID
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
        if planet and (not planet.unowned) and planet.ownedBy(empireID):
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
