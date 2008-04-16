import freeOrionAIInterface as fo
import FreeOrionAI as foAI
import FleetUtils

# Evaluation uses only size and hospitality score
# later: specials, exploration, distance


# constants
minimalColoniseValue = 4 # minimal value a planet must have to be colonised; right now a size 2 terran planet


# globals
colonisablePlanets = []  # !!! move into AIstate


# main function
def generateColonisationOrders():
    
    empire = fo.getEmpire()
    empireID = fo.empireID()
    universe = fo.getUniverse()

    # get colonyships
    allColonyFleetIDs = FleetUtils.getEmpireFleetIDsByRole(empireID, "MT_COLONISATION")
    colonyFleetIDs = FleetUtils.extractFleetIDsWithoutMission(allColonyFleetIDs)

    removeInvalidMissions(universe)

    print "Colony Fleets: " + str(allColonyFleetIDs)
    print "Current colonise missions: " + str(foAI.foAIstate.getMissions("MT_COLONISATION"))
    
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

    # export planets for other AI modules
    global colonisablePlanets
    colonisablePlanets = sortedPlanets   # !!! move into AIstate?

    # send Colony Ships, colonise if at target system
    sendColonyShips(colonyFleetIDs, sortedPlanets, universe)
    coloniseTargetPlanets(universe)




def removeInvalidMissions(universe):
    "deletes invalid colonisation missions"

    print "Removing invalid colonisation missions:"

    missions = foAI.foAIstate.getMissions("MT_COLONISATION")

    for fleetID in missions:

        # fleet exists and has colony ship?
        fleet = universe.getFleet(fleetID)
                
        if (fleet == None) or (not FleetUtils.fleetHasShipWithRole(fleetID, "SR_COLONISATION")):
            foAI.foAIstate.removeMission("MT_COLONISATION", fleetID)
            continue

        # planet is uncolonized?
        mission = foAI.foAIstate.getMission("MT_COLONISATION", fleetID)
        planet = universe.getPlanet(mission[1])
        
        if (planet == None):
            foAI.foAIstate.removeMission("MT_COLONISATION", fleetID)
            continue

        if (not planet.unowned):
            foAI.foAIstate.removeMission("MT_COLONISATION", fleetID)
          

def getExploredSystemIDs(empire, universe):
    "retrieves all systems explored by the empire"
     
    systemIDs = []
    objectIDs = universe.allObjectIDs

    for objID in objectIDs:        
        system = universe.getSystem(objID)
        if (system == None): continue

        if (empire.hasExploredSystem(objID)): systemIDs.append(objID)
    
    return systemIDs


def getPlanetsInSystemsIDs(systemIDs, universe):
    "creates a list with all planets known to the empire"

    planetIDs = []
    objectIDs = universe.allObjectIDs

    for systemID in systemIDs:

        system = universe.getSystem(systemID)
        if (system == None): continue

        planetIDs.extend(system.planetIDs)
        
    return planetIDs


def removeAlreadyOwnedPlanetIDs(planetIDs, empireID, universe):
    "removes planets that already are being colonised or owned"

    coloniseMissions = foAI.foAIstate.getMissions("MT_COLONISATION")
    deletePlanets = []

    for planetID in planetIDs:

        planet = universe.getPlanet(planetID)

        # remove owned planets
        if (not planet.unowned):
            deletePlanets.append(planetID)
            continue
      
        # remove planets that are target of a mission
        for colonyFleetID in coloniseMissions:
            if planetID == coloniseMissions[colonyFleetID]: deletePlanets.append(planetID)
             
    for ID in deletePlanets:
        planetIDs.remove(ID)
        # print "removed planet " + str(ID)


def assignColonisationValues(planetIDs, universe):
    "creates a dictionary that takes planetIDs as key and their colonisation score as value"

    planetValues = {}

    for planetID in planetIDs:
        planetValues[planetID] = evaluatePlanet(planetID, universe)

    return planetValues


def evaluatePlanet(planetID, universe):
    "returns the colonisation value of a planet"

    planet = universe.getPlanet(planetID)
    if (planet == None): return 0
    
    return getPlanetHospitality(planetID, universe) * planet.size
    # planet size ranges from 1-5


def getPlanetHospitality(planetID, universe):
    "returns a value depending on the planet type"

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
    "removes all planets with a colonisation value < minimalColoniseValue"

    removeIDs = []

    for planetID in evaluatedPlanets.iterkeys():      
        if (evaluatedPlanets[planetID] < minimalColoniseValue):
            removeIDs.append(planetID)

    for ID in removeIDs: del evaluatedPlanets[ID]

  
def sendColonyShips(colonyFleetIDs, evaluatedPlanets, universe):
    "sends a list of colony ships to a list of planet_value_pairs"

    i = 0

    for planetID_value_pair in evaluatedPlanets:
        if i >= len(colonyFleetIDs): return

        colonyshipID = FleetUtils.getShipIDWithRole(colonyFleetIDs[i], "SR_COLONISATION")
        planetID = planetID_value_pair[0]
        planet = universe.getPlanet(planetID)
      
        print "SEND______________"
        print "Colony ShipID: " + str(colonyshipID)
        print "FleetID:       " + str(colonyFleetIDs[i])
        print "To PlanetID:   " + str(planetID)
        print "To SystemID:   " + str(planet.systemID)
      
        fo.issueFleetMoveOrder(colonyFleetIDs[i], planet.systemID)
        foAI.foAIstate.addMission("MT_COLONISATION", [colonyFleetIDs[i], planetID])

        i=i+1


def coloniseTargetPlanets(universe):
    "checks if a colonyship has arrived at its destination; colonises planet and removes mission"

    coloniseMissions = foAI.foAIstate.getMissions("MT_COLONISATION")

    for fleetID in coloniseMissions:
        
        # look if fleet has arrived at destination
        fleet = universe.getFleet(fleetID) 
        planet = universe.getPlanet(coloniseMissions[fleetID])

        if not(fleet.systemID == planet.systemID): continue

        # colonise and remove missions
        print "Colonizing planet ID " + str(coloniseMissions[fleetID]) + " with fleet " + str(fleetID)
        
        fo.issueColonizeOrder(FleetUtils.getShipIDWithRole(fleetID, "SR_COLONISATION"), coloniseMissions[fleetID])
        foAI.foAIstate.removeMission("MT_COLONISATION", fleetID)






