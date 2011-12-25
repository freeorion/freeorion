import freeOrionAIInterface as fo
def getPlanetsInSystemsIDs(systemIDs):
    "return list of planets in systems"

    universe = fo.getUniverse()

    planetIDs = []

    for systemID in systemIDs:
        system = universe.getSystem(systemID)
        if system == None: continue

        planetIDs.extend(list(system.planetIDs)) # added list

    return planetIDs

def getOwnedPlanetsByEmpire(planetIDs, empireID):
    "return list of planets owned by empireID"

    universe = fo.getUniverse()

    result = []

    for planetID in planetIDs:
        planet = universe.getPlanet(planetID)
        if (not planet.unowned) and planet.ownedBy(empireID):
            result.append(planetID)

    return result

def getAllOwnedPlanetIDs(planetIDs):
    "return list of all owned planetIDs"

    universe = fo.getUniverse()

    allOwnedPlanetIDs = []

    for planetID in planetIDs:
        planet = universe.getPlanet(planetID)
        planetPopulation = planet.currentMeterValue(fo.meterType.population)
        if not planet.unowned or planetPopulation > 0:
            allOwnedPlanetIDs.append(planetID)

    return allOwnedPlanetIDs

def getCapitalID():
    "return planetID of empire capital"

    empire = fo.getEmpire()
    capitalID = empire.capitalID

    return capitalID
