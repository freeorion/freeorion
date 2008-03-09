import freeOrionAIInterface as fo   # interface used to interact with FreeOrion AI client
import pickle                       # Python object serialization library
import ExplorationAI
import ColonisationAI


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
    
    turn = fo.currentTurn()

    print ""
    print "TURN: " + str(turn)

    # split all fleets
    empireID = fo.empireID()
    universe = fo.getUniverse()

    for fleetID in universe.fleetIDs:
        
        fleet = universe.getFleet(fleetID)
        if fleet == None: continue
        if not fleet.whollyOwnedBy(empireID): continue
        if fleet.numShips == 1: continue

        for shipID in fleet.shipIDs:
            if len(fleet.shipIDs) <= 1: break # don't use last ship
            fo.issueNewFleetOrder(str(shipID), shipID)

        # ISSUNG A NEW_FLEET ORDER ON A 1-SHIP FLEET WILL CAUSE TROUBLE!

    # call AI modules
    ExplorationAI.generateExplorationOrders()
    ColonisationAI.generateColonisationOrders()

    fo.doneTurn()



