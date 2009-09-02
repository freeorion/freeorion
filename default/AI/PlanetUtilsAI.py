import freeOrionAIInterface as fo

def getPlanetsInSystemsIDs(systemIDs):
    "creates a list with all planets known to the empire"

    universe = fo.getUniverse()

    planetIDs = []

    for systemID in systemIDs:

        system = universe.getSystem(systemID)
        if (system == None): continue

        planetIDs.extend(system.planetIDs)

    return planetIDs

def getOwnedPlanetsByEmpire(planetIDs, empireID):
    "return list of planets owned by empireID"

    result = []

    universe = fo.getUniverse()
    for planetID in planetIDs:
        planet = universe.getPlanet(planetID)
        if (not planet.unowned) and planet.ownedBy(empireID):
            result.append(planetID)

    return result
