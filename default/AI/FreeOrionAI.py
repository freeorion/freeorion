import freeOrionAIInterface as fo
import otherStuff

i = 0

# called when Python AI starts
def initFreeOrionAI():
    print "Initialized FreeOrion Python AI"


# called once per turn
def generateOrders():
    print "Generating Orders"

    global i
    i = i + 1
    otherStuff.doStuff(i)

    empire = fo.getEmpire()
    empireID = fo.empireID()
    universe = fo.getUniverse()

    # get stationary fleets
    fleetIDs = getEmpireStationaryFleetIDs(empireID)
    print "fleetIDs: " + str(fleetIDs)


    for fleet_id in fleetIDs:
        fleet = universe.getFleet(fleet_id)
        if (fleet == None): continue

        print "Fleet: " + str(fleet_id)

        startSystemID = fleet.systemID
        if (startSystemID == universe.invalidObjectID): continue

        print "in system: " + str(startSystemID)

        systemIDs = getExplorableSystemIDs(startSystemID, empireID)

        print "can explore: " + str(systemIDs)

        if (len(systemIDs) > 0):
            destinationID = systemIDs[0]
            fo.issueFleetMoveOrder(fleet_id, destinationID)


    # get all object IDs
    objectIDs = universe.allObjectIDs

    # Testing: list planet specials, owners; rename a planet this player owns, etc.
    for objectID in objectIDs:
        planet = universe.getPlanet(objectID)
        if (planet == None): continue

        print "planet: " + str(planet.name)

        if (not planet.whollyOwnedBy(empireID)): continue

        fo.issueRenameOrder(objectID, "RENAMED PLANET!!!")

        ons = planet.owners
        print ons

        spl = planet.specials
        print spl


    # colonize?!
    colonyShipID = universe.invalidObjectID
    for shipID in objectIDs:
        ship = universe.getShip(shipID)
        if (ship == None): continue
        print "ship id: " + str(shipID)

        if (ship.systemID == universe.invalidObjectID): continue
        if (not ship.whollyOwnedBy(empireID)): continue
        print "owned by me!"
        if (ship.design.name != "Colony Ship"): continue
        print "with design name Colony Ship, in system with id " + str(ship.systemID)

        colonyShipSystemID = ship.systemID
        colonyShipID = shipID

        break

    if (colonyShipID != universe.invalidObjectID):
        for planetID in objectIDs:
            planet = universe.getPlanet(planetID)
            if (planet == None): continue
            if (not planet.unowned): continue
            if (planet.type != fo.planetType.terran): continue
            if (planet.systemID != colonyShipSystemID): continue

            print "colonizing " + planet.name
            fo.issueColonizeOrder(colonyShipID, planetID)

    fo.doneTurn()


# returns list of systems ids known of by but not explored by empireID,
# that a ship located in startSystemID could reach via starlanes
def getExplorableSystemIDs(startSystemID, empireID):
    print "getExplorableSystemIDs"
    universe = fo.getUniverse()
    objectIDs = universe.allObjectIDs
    empire = fo.getEmpire(empireID)

    systemIDs = []


    for objectID in objectIDs:
        print "getExplorableSystemIDs object id: " + str(objectID)

        system = universe.getSystem(objectID)
        if (system == None): continue

        print "...is a system"

        if (empire.hasExploredSystem(objectID)): continue

        print "...not explored"

        if (not universe.systemsConnected(objectID, startSystemID, empireID)): continue

        print "...is connected to start system " + str(startSystemID)

        systemIDs = systemIDs + [objectID]

    return systemIDs


# returns list of staitionary fleet ids owned by empireID
def getEmpireStationaryFleetIDs(empireID):
    print "getEmpireStationaryFleetIDs"
    universe = fo.getUniverse()
    objectIDs = universe.allObjectIDs

    fleetIDs = []

    for objectID in objectIDs:
        fleet = universe.getFleet(objectID)
        if (fleet == None): continue

        if (not fleet.whollyOwnedBy(empireID)): continue

        if (fleet.nextSystemID != universe.invalidObjectID): continue

        fleetIDs = fleetIDs + [objectID]

    return fleetIDs

