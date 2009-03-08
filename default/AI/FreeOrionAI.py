import freeOrionAIInterface as fo   # interface used to interact with FreeOrion AI client
import pickle                       # Python object serialization library
import AIstate
import FleetUtilsAI
import ExplorationAI
import ColonisationAI
# import TacticsAI
import PriorityAI
# import ProductionAI

# AIstate
foAIstate = None


# called when Python AI starts, before any game new game starts or saved game is resumed
def initFreeOrionAI():
    print "Initialized FreeOrion Python AI"


# called when a new game is started (but not when a game is loaded).  should clear any pre-existing state
# and set up whatever is needed for AI to generate orders
def startNewGame():
    print "New game started"

    empireID = fo.empireID()
    universe = fo.getUniverse()

    # initialize AIstate
    global foAIstate
    foAIstate = AIstate.AIstate()
    print "Initialized foAIstate class"

    # split all fleets
    for fleetID in universe.fleetIDs: FleetUtilsAI.splitFleet(fleetID)

    # assign roles to ships
    shipIDs = []

    for fleetID in universe.fleetIDs:
        fleet = universe.getFleet(fleetID)

        if not fleet.whollyOwnedBy(empireID): continue
        for ID in fleet.shipIDs: shipIDs = shipIDs + [ID]

    for shipID in shipIDs:
        ship = universe.getShip(shipID)
        foAIstate.addShipRole(ship.design.id, FleetUtilsAI.assessShipRole(shipID))
        # print str(ship.design.id) + ": " + FleetUtilsAI.assessShipRole(shipID)

    # assign roles to fleets
    for fleetID in universe.fleetIDs:
        if not fleet.whollyOwnedBy(empireID): continue
        foAIstate.addFleetRole(fleetID, FleetUtilsAI.assessFleetRole(fleetID))
        # print str(fleetID) + ": " + FleetUtilsAIAIAI.assessFleetRole(fleetID)


# called when client receives a load game message
def resumeLoadedGame(savedStateString):
    global foAIstate
    print "Resuming loaded game"
    try:
        #loading saved state
        foAIstate = pickle.loads(savedStateString)
    except:
        print "failed to parse saved state string"
        #assigning new state
        foAIstate = AIstate.AIstate()


# called when the game is about to be saved, to let the Python AI know it should save any AI state
# information, such as plans or knowledge about the game from previous turns, in the state string so that
# they can be restored if the game is loaded
def prepareForSave():
    print "Preparing for game save by serializing state"

    # serialize (convert to string) global state dictionary and send to AI client to be stored in save file
    fo.setSaveStateString(pickle.dumps(foAIstate))


# called when this player receives a chat message.  senderID is the player who sent the message, and
# messageText is the text of the sent message
def handleChatMessage(senderID, messageText):
    print "Received chat message from " + str(senderID) + " that says: " + messageText + " - ignoring it"


# called once per turn to tell the Python AI to generate and issue orders to control its empire.
# at end of this function, fo.doneTurn() should be called to indicate to the client that orders are finished
# and can be sent to the server for processing.
def generateOrders():

    print ""
    print "TURN: " + str(fo.currentTurn())

    # turn cleanup
    foAIstate.clean(ExplorationAI.getHomeSystemID(), FleetUtilsAI.getEmpireFleetIDs())
    # ...missions
    # ...demands/priorities

    # call AI modules
    PriorityAI.calculatePriorities()
    
    ExplorationAI.assignScoutsToExploreSystems()
    ColonisationAI.assignColonyFleetsToColonise()
    # ProductionAI.generateProductionOrders()
    # TacticsAI.generateTacticOrders()
    FleetUtilsAI.generateAIFleetOrdersForAIFleetMissions()
    FleetUtilsAI.issueAIFleetOrdersForAIFleetMissions()

    fo.doneTurn()
