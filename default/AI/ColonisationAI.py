import freeOrionAIInterface as fo

# Evaluation uses only size and hospitality score
# later: specials, exploration, distance


# constants
minimalColoniseValue = 5 # minimal value a planet must have to be colonised; right now a size 3 terran planet


# globals
coloniseMissions = {}


# main function
def generateColonisationOrders():
    print "Generating Colonisation Orders"
    
    empire = fo.getEmpire()
    empireID = fo.empireID()
    universe = fo.getUniverse()

    # get colonyships
    colonyshipIDs = getEmpireColonyshipIDs(empireID, universe)

    global coloniseMissions
    print "Current colonise missions" + str(coloniseMissions)

    removeColonyshipsWithMission(colonyshipIDs, universe)
    removeInvalidMissions(universe)
    
    print "Available colony ships: " + str(colonyshipIDs)

    # get planets
    systemIDs = getExploredSystemIDs(empire, universe)
    planetIDs = getPlanetsInSystemsIDs(systemIDs, universe)

    removeAlreadyOwnedPlanetIDs(planetIDs, empireID, universe)

    evaluatedPlanets = assignColonisationValues(planetIDs, universe)
    removeLowValuePlanets(evaluatedPlanets)

    sortedPlanets = evaluatedPlanets.items()
    sortedPlanets.sort(lambda x,y: cmp(x[1],y[1]), reverse=True)

    print "Colonisable planets:"
    for evaluationPair in sortedPlanets: print "ID|Score: " + str(evaluationPair)

    # send Colony Ships, colonise
    sendColonyShips(colonyshipIDs, sortedPlanets, universe)
    coloniseTargetPlanets(universe)




# Helper functions

def getEmpireColonyshipIDs(empireID, universe):
# returns the IDs of all colonyships of an empire
            
    shipIDs = []
    objectIDs = universe.allObjectIDs

    for objID in objectIDs:

        ship = universe.getShip(objID)

        if (ship == None): continue                     
        if (not ship.whollyOwnedBy(empireID)): continue 
        if (not ship.design.name == 'Colony Ship'): continue

        shipIDs.append(objID)

    return shipIDs


def coloniseTargetPlanets(universe):
# checks if a colonyship has arrived at its destination; colonises planet
# and removes mission

    global coloniseMissions
    removeMissions = []

    for colonyshipID in coloniseMissions:
        
        # look if colony ship has arrived at destination
        colonyship = universe.getShip(colonyshipID) 
        if (colonyship == None): continue

        planet = universe.getPlanet(coloniseMissions[colonyshipID])
        if (planet == None): continue

        if not(colonyship.systemID == planet.systemID): continue

        # colonise and remove missions
        print "Colonizing planet " + str(coloniseMissions[colonyshipID])
        fo.issueColonizeOrder(colonyshipID, coloniseMissions[colonyshipID])
        removeMissions.append(colonyshipID)

    # remove completed missions
    for ID in removeMissions: del coloniseMissions[ID]


def removeColonyshipsWithMission(colonyshipIDs, universe):
# removes colony ships that already have a mission from list of
# available colonyships

    global coloniseMissions

    for shipID in coloniseMissions:
        if shipID in colonyshipIDs: colonyshipIDs.remove(shipID)


def removeInvalidMissions(universe):
# deletes missions if either the colony ship is lost or the planet has been
# colonised by someone else

    global coloniseMissions
    deleteMissions = []

    for shipID in coloniseMissions:
        ship = universe.getShip(shipID)
        if ship == None: deleteMissions.append(shipID)
        if ship.design.name != "Colony Ship": deleteMissions.append(shipID)
        # could check for empire

        planet = universe.getPlanet(coloniseMissions[shipID])
        if planet == None: deleteMissions.append(shipID)
        if not planet.unowned: deleteMissions.append(shipID)

    for shipID in deleteMissions:
        print "remove invalid mission :" + str(shipID) + "/" + str(coloniseMissions[shipID])
        del coloniseMissions[shipID]
          

def getExploredSystemIDs(empire, universe):
# retrieves all explored systems
     
    systemIDs = []
    objectIDs = universe.allObjectIDs

    for objID in objectIDs:        
        system = universe.getSystem(objID)
        if (system == None): continue

        if (empire.hasExploredSystem(objID)): systemIDs.append(objID)
    
    return systemIDs


def getPlanetsInSystemsIDs(systemIDs, universe):
# creates a list with all known planets

    planetIDs = []
    objectIDs = universe.allObjectIDs

    for systemID in systemIDs:

        system = universe.getSystem(systemID)
        if (system == None): continue

        # complicated code below, one line code would be:
        # planetIDs.extend(system.planetIDs)
        for object_id in objectIDs:
            planet = universe.getPlanet(object_id)
            if (planet == None): continue
            if (planet.systemID != systemID): continue

            planetIDs.append(object_id)

    return planetIDs


def removeAlreadyOwnedPlanetIDs(planetIDs, empireID, universe):
# removes planets...
# - that are already colonised (eg. home world)
# - that an own colony ship is en route to

    global coloniseMissions

    for planetID in planetIDs:

        planet = universe.getPlanet(planetID)
        deletePlanet = False

        if not (planet.unowned): deletePlanet = True
      
         # remove planets that are target of a mission of your own colony ship
        for colonyshipID in coloniseMissions:
            if planetID != coloniseMissions[colonyshipID]: continue
          
            colonyship = universe.getShip(colonyshipID)
            if colonyship.whollyOwnedBy(empireID): deletePlanet = True
             
        if deletePlanet:
            planetIDs.remove(planetID)
            # print "removed planet " + str(planetID)

    return planetIDs


def assignColonisationValues(planetIDs, universe):
# creates a dictionary that takes planetIDs as key and their colonisation
# score as value

    planetValues = {}

    for planetID in planetIDs:
        planetValues[planetID] = evaluatePlanet(planetID, universe)

    return planetValues


def evaluatePlanet(planetID, universe):
# returns the colonisation value of a planet

    planet = universe.getPlanet(planetID)
    if (planet == None): return 0
    
    return getPlanetHospitality(planetID, universe) * planet.size
    # planet size ranges from 1-5


def getPlanetHospitality(planetID, universe):
# returns a value depending on the planet type

    planet = universe.getPlanet(planetID)
    if planet == None: return 0

    # should be reworked with races
    if planet.type == fo.planetType.terran: return 2
    if planet.type == fo.planetType.ocean: return 1
    if planet.type == fo.planetType.desert: return 1
    if planet.type == fo.planetType.tundra: return 0.5
    if planet.type == fo.planetType.swamp: return 0.5

    return 0
    

def removeLowValuePlanets(evaluatedPlanets):
# removes all planets with a colonisation value < minimalColoniseValue

    removeIDs = []

    for planetID in evaluatedPlanets.iterkeys():      
        if (evaluatedPlanets[planetID] < minimalColoniseValue):
            removeIDs.append(planetID)

    for ID in removeIDs: del evaluatedPlanets[ID]

  
def sendColonyShips(colonyshipIDs, evaluatedPlanets, universe):
# sends a list of colony ships to a list of planet_value_pairs

    i = 0
    global coloniseMissions

    for planetID_value_pair in evaluatedPlanets:
        if i >= len(colonyshipIDs): return

        colonyship = universe.getShip(colonyshipIDs[i])
        planet = universe.getPlanet(planetID_value_pair[0])
      
        print "SEND______________"
        print "Colony ShipID: " + str(colonyshipIDs[i])
        print "FleetID:       " + str(colonyship.fleetID)
        print "To PlanetID:   " + str(planetID_value_pair[0])
        print "To SystemID:   " + str(planet.systemID)
      
        fo.issueFleetMoveOrder(colonyship.fleetID, planet.systemID)
        coloniseMissions[colonyshipIDs[i]] = planetID_value_pair[0]

        i=i+1


  





