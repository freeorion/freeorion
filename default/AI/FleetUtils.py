import freeOrionAIInterface as fo
import FreeOrionAI as foAI
import AIstate as gs

# def splitFleet(fleetID)

# def getShipIDWithRole(fleetID, shipRole)
# def getEmpireFleetIDs(empireID)
# def getFleetIDsByRole(fleetRole, AIgamestate)

# def fleetHasShipWithRole(fleetID, shipRole, AIgamestate)

# def extractFleetIDsWithoutMission(fleetIDs, AIgamestate)

# NOT YET DONE:
# def extractMovingFleetIDs
# def extractStationaryFleetIDs


def splitFleet(fleetID):
    "splits a fleet into its ships"

    universe = fo.getUniverse()
    empireID = fo.empireID()
    
    fleet = universe.getFleet(fleetID)

    if fleet == None: return
    if not fleet.whollyOwnedBy(empireID): return

    for shipID in fleet.shipIDs:
        if len(fleet.shipIDs) <= 1: break # don't use last ship => trouble
        fo.issueNewFleetOrder(str(shipID), shipID)


def fleetHasShipWithRole(fleetID, shipRole):
    "returns True if a ship with shipRole is in the fleet"

    universe = fo.getUniverse()
    fleet = universe.getFleet(fleetID)
    
    if fleet == None: return False

    for shipID in fleet.shipIDs:
        
        ship = universe.getShip(shipID)
        if (foAI.foAIstate.getShipRole(ship.design.id) == shipRole):
            return True

    return False


def getShipIDWithRole(fleetID, shipRole):
    "returns a ship with the specified role in the fleet"

    if not fleetHasShipWithRole(fleetID, shipRole):
        print "No ship with role " + shipRole + " found."
        return None

    universe = fo.getUniverse()
    fleet = universe.getFleet(fleetID)

    for shipID in fleet.shipIDs:

        ship = universe.getShip(shipID)
        if (foAI.foAIstate.getShipRole(ship.design.id) == shipRole):
            return shipID


def getEmpireFleetIDs():
    "returns all fleetIDs of an empire"
    
    universe = fo.getUniverse()
    empireID = fo.empireID()

    fleetIDs = universe.fleetIDs
    empireFleetIDs = []

    for fleetID in fleetIDs:

        fleet = universe.getFleet(fleetID)

        if (fleet == None): continue
        if (not fleet.whollyOwnedBy(empireID)): continue    

        empireFleetIDs = empireFleetIDs + [fleetID]

    return empireFleetIDs


def getEmpireFleetIDsByRole(fleetRole):
    "returns a list with fleetIDs that have the specified role"

    empireID = fo.empireID()

    fleetIDs = getEmpireFleetIDs()
    fleetIDsWithRole = []

    for fleetID in fleetIDs:

        if not (foAI.foAIstate.getFleetRole(fleetID) == fleetRole): continue
        
        fleetIDsWithRole.append(fleetID)

    return fleetIDsWithRole


def extractFleetIDsWithoutMission(fleetIDs):
    "extracts a list with fleetIDs that have no mission"

    fleetIDsWithoutMission = []

    for fleetID in fleetIDs:
        if not foAI.foAIstate.hasAnyMission(fleetID):
            fleetIDsWithoutMission.append(fleetID)

    return fleetIDsWithoutMission

# def extractMovingFleetIDs
# def extractStationaryFleetIDs


def assessFleetRole(fleetID):
    "counts the number of ShipRoles in a fleet and returns a corresponding fleetRole"
    # one colonyship should mean it's a colony fleet!
    
    universe = fo.getUniverse()
    
    shipRoles = {}
    for shipRole in gs.shipRoles: shipRoles[shipRole] = 0
    
    fleet = universe.getFleet(fleetID)

    # count shipRoles
    for shipID in fleet.shipIDs:

        ship = universe.getShip(shipID)
        role = foAI.foAIstate.getShipRole(ship.design.id)
        
        shipRoles[role] = shipRoles[role] +1

    # determine most common shipRole
    for shipRole in shipRoles:
        if shipRoles[shipRole] == max(shipRoles.values()):
            favouriteRole = shipRole

    # assign fleetrole
    if favouriteRole == "SR_EXPLORATION": return "MT_EXPLORATION"
    if favouriteRole == "SR_COLONISATION": return "MT_COLONISATION"
    if favouriteRole == "SR_ATTACK": return "MT_ATTACK"

    return gs.missionTypes[0]


def assessShipRole(shipID):
    "decides which role a ship has"
    # later: analyse parts

    universe = fo.getUniverse()
    ship = universe.getShip(shipID)
    
    if ship.design.name == "Scout": return "SR_EXPLORATION"
    if ship.design.name == "Colony Ship": return "SR_COLONISATION"
    if ship.design.name == "Mark I": return "SR_ATTACK"
    if ship.design.name == "Mark II": return "SR_ATTACK"
    if ship.design.name == "Mark III": return "SR_ATTACK"
    if ship.design.name == "Mark IV": return "SR_ATTACK"

    return gs.shipRoles[0]
