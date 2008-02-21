import freeOrionAIInterface as fo   # interface used to interact with FreeOrion AI client
import pickle                       # Python object serialization library


# global variable for test purposes.  code in generateOrders demonstrates that this value persists between
# calls from c++ to functions defined in this module
i = 0

# global Python dict in which AI state information can be stored.  data will persist between calls to
# generate orders.  a dict can be serialized to a string, which can be stored in a saved game.  the
# string will then be returned when the saved game is loaded, and can be deserialized to restore AI
# state from before the save.
d = {}


# called when Python AI starts, before any game new game starts or saved game is resumed
def initFreeOrionAI():
    global i
    print "Initialized FreeOrion Python AI"
    i = 20


# called when a new game is started (but not when a game is loaded).  should clear any pre-existing state
# and set up whatever is needed for AI to generate orders
def startNewGame():
    global d
    print "New game started"
    d = {"dict_key" : "dict_value"}


# called when client receives a load game message
def resumeLoadedGame(savedStateString):
    global d
    print "Resuming loaded game"
    try:
        d = pickle.loads(savedStateString)
    except:
        print "failed to parse saved state string"
        d = {}


# called when the game is about to be saved, to let the Python AI know it should save any AI state
# information, such as plans or knowledge about the game from previous turns, in the state string so that 
# they can be restored if the game is loaded
def prepareForSave():
    global d
    print "Preparing for game save by serializing state"

    # serialize (convert to string) global state dictionary and send to AI client to be stored in save file
    fo.setSaveStateString(pickle.dumps(d))


# called when this player receives a chat message.  senderID is the player who sent the message, and 
# messageText is the text of the sent message
def handleChatMessage(senderID, messageText):
    print "Received chat message - ignoring it"


# called once per turn to tell the Python AI to generate and issue orders to control its empire.
# at end of this function, fo.doneTurn() should be called to indicate to the client that orders are finished
# and can be sent to the server for processing.
def generateOrders():
    print "Generating Orders"

    global i
    global d

    # demonstrate persistant data that can be modified each time a Python function is called from C++
    i = i + 1
    print "data persistent between turns: " + str(i)

    # diplay current state of global dictionary
    print "global dict: " + str(d)
    d[i] = fo.currentTurn()


    # retreive objects from freeOrionAIInterface that can be used to access the gamestate
    empire = fo.getEmpire()
    empireID = fo.empireID()
    universe = fo.getUniverse()

    researchQueue = empire.researchQueue

    print "initial techs on queue: " + str(len(researchQueue))

    fo.issueEnqueueTechOrder("LRN_PHYS_BRAIN", -1)
    fo.issueEnqueueTechOrder("LRN_ALGO_ELEGANCE", -1)
    fo.issueEnqueueTechOrder("GRO_MEDICAL_PATH", -1)

    print "final techs on queue: " + str(len(researchQueue))
    for element in researchQueue:
        print element.tech.name

    #print "tech 2: " + str(researchQueue[2].tech.name) # this works on turn one, but will crash on turn two because the tech will have been researched.  left here, commented, full illustrative purposes

    print "tech LRN_ALGO_ELEGANCE on queue? " + str(researchQueue.inQueue("LRN_ALGO_ELEGANCE"))

    #fo.issueEnqueueBuildingProductionOrder("BLD_BIOTERROR_LAB", empire.capitolID)
    availableShipDesignIDs = empire.availableShipDesigns
    for id in availableShipDesignIDs:
        print "available design id: " + str(id)
        fo.issueEnqueueShipProductionOrder(id, empire.capitolID)

    productionQueue = empire.productionQueue

    print "items on production queue:"
    for element in productionQueue:
        print "name: " + element.name + " id: " + str(element.designID) + " turns left: " + str(element.turnsLeft)

    #print "production item 2: " + productionQueue[2].name

    # get stationary fleets
    fleetIDs = getEmpireStationaryFleetIDs(empireID)
    print "fleetIDs: " + str(fleetIDs)

    # order stationary fleets to explore
    for fleet_id in fleetIDs:
        fleet = universe.getFleet(fleet_id)
        if (fleet == None): continue

        print "fleet " + fleet.name + " ships:"
        for shipID in fleet.shipIDs:
            print "shipID: " + str(shipID)

        startSystemID = fleet.systemID
        if (startSystemID == universe.invalidObjectID): continue

        systemIDs = getExplorableSystemIDs(startSystemID, empireID)

        if (len(systemIDs) > 0):
            destinationID = systemIDs[0]
            fo.issueFleetMoveOrder(fleet_id, destinationID)


    # get id numbers of all objects in the universe known to this player
    objectIDs = universe.allObjectIDs

    # Testing: list planet specials, owners; rename a planet this player owns, etc.
    for objectID in objectIDs:
        planet = universe.getPlanet(objectID)
        if (planet == None): continue

        if (not planet.whollyOwnedBy(empireID)): continue

        #fo.issueRenameOrder(objectID, "RENAMED PLANET!!!")

        print "this empire owns planet: " + str(planet.name)

        #print planet.owners
        #print planet.specials


    # colonize?!
    colonyShipID = universe.invalidObjectID
    for shipID in objectIDs:
        ship = universe.getShip(shipID)
        if (ship == None): continue

        if (ship.systemID == universe.invalidObjectID): continue
        if (not ship.whollyOwnedBy(empireID)): continue
        print "ship id: " + str(shipID)
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
        system = universe.getSystem(objectID)
        if (system == None): continue
        if (empire.hasExploredSystem(objectID)): continue
        if (not universe.systemsConnected(objectID, startSystemID, empireID)): continue
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

