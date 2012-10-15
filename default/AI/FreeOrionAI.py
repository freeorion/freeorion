import freeOrionAIInterface as fo   # interface used to interact with FreeOrion AI client
import pickle                       # Python object serialization library
import AIstate
import FleetUtilsAI
import ExplorationAI
import ColonisationAI
import PriorityAI
import ResearchAI
import ProductionAI
import ResourcesAI
import InvasionAI
import MilitaryAI
import sys
import PlanetUtilsAI
from time import time
import os

# AIstate
foAIstate = None
__timerEntries= ["PriorityAI",  "ExplorationAI",  "ColonisationAI",  "InvasionAI",  "MilitaryAI",  "Gen_Fleet_Orders",  "Issue_Fleet_Orders",  
                        "ResearchAI",  "ProductionAI",  "ResourcesAI",  "Cleanup"]
__timerFile = None
__timerFileFmt = "%8d"+ (len(__timerEntries)*"\t %8d")

# called when Python AI starts, before any game new game starts or saved game is resumed
def initFreeOrionAI():
    print "Initialized FreeOrion Python AI"
    print(sys.path)

# called when a new game is started (but not when a game is loaded).  should clear any pre-existing state
# and set up whatever is needed for AI to generate orders
def startNewGame():
    global __timerFile
    print "New game started"

    # initialize AIstate
    global foAIstate
    foAIstate = AIstate.AIstate()
    print "Initialized foAIstate class"
    if __timerFile:
        __timerFile.close()
    if ResourcesAI.resourceTimerFile:
        ResourcesAI.resourceTimerFile.close()
    empireID = fo.getEmpire().empireID
    try:
        if os.path.exists("timers") and os.path.isdir("timers"):
            timerpath="timers"+os.path.sep+"timer_%02d.dat"%(empireID-1)
            __timerFile = open(timerpath,  'w')
            __timerFile.write("Turn\t" + "\t".join(__timerEntries) +'\n')
            if ResourcesAI.doResourceTiming:
                ResourcesAI.resourceTimerFile = open("timers"+os.path.sep+"resourceTimer_%2d.dat"%(empireID-1),  'w')
                ResourcesAI.resourceTimerFile.write("Turn\t"+ "\t".join(ResourcesAI.__timerEntries)+"\n")
            print "timer file saved at "+timerpath
    except:
        __timerFile=None
        ResourcesAI.resourceTimerFile  =None
        ResourcesAI.doResourceTiming = False

def splitNewFleets():
    "split any fleets (at creation, can have unplanned mix of ship roles)"

    print "Review of current Fleet Role/Mission records:"
    print "--------------------"
    print "Map of Roles keyed by Fleet ID: %s"%foAIstate.getFleetRolesMap()
    print "--------------------"
    print "Map of Missions  keyed by ID:"
    for item in  foAIstate.getFleetMissionsMap().items():
        print " %4d : %s "%item 
    print "--------------------"
    # TODO: check length of fleets for losses  or do in AIstat.__cleanRoles
    knownFleets= foAIstate.getFleetRolesMap().keys()
    splitableFleets=[]
    for fleetID in FleetUtilsAI.getEmpireFleetIDs(): 
        if fleetID  in  knownFleets: #not a new fleet
            continue
        else:
            splitableFleets.append(fleetID)
    if splitableFleets:
        print ("splitting new fleets")
        for fleetID in splitableFleets:
            print "\t splitting fleet ID %4d:"%fleetID
            newFleets = FleetUtilsAI.splitFleet(fleetID) # try splitting fleet
            # old fleet may have different role after split, later will be again identified
            #foAIstate.removeFleetRole(fleetID)  # in current system, orig new fleet will not yet have been assigned a role
            for newID in newFleets:
                fo.issueRenameOrder(newID,  "Fleet %5d"%newID) #to ease review of debugging logs
                foAIstate.getFleetRole(newID) #and mission?

def updateShipDesigns(): #
    "update ship design records"
    print ("Updating ship design records")
    shipIDs = []
    universe = fo.getUniverse()

    for fleetID in universe.fleetIDs:
        fleet = universe.getFleet(fleetID)
        if fleet:
            for shipID in fleet.shipIDs: 
                ship = universe.getShip(shipID)
                if ship and ship.design:
                    foAIstate.getShipRole(ship.design.id)
            # print str(ship.design.id) + ": " + str(shipRole)

def updateFleetsRoles():
    "updating fleet role records"
    print ("Updating fleet role records")
    # assign roles to fleets
    for fleetID in FleetUtilsAI.getEmpireFleetIDs(): 
        foAIstate.getFleetRole(fleetID) #force assessment if not previously known

# called when client receives a load game message
def resumeLoadedGame(savedStateString):
    global foAIstate
    global __timerFile
    print "Resuming loaded game"
    try:
        #loading saved state
        foAIstate = pickle.loads(savedStateString)
    except:
        print "failed to parse saved state string"
        #assigning new state
        foAIstate = AIstate.AIstate()
    if __timerFile:
        __timerFile.close()
    if ResourcesAI.resourceTimerFile:
        ResourcesAI.resourceTimerFile.close()
    empireID = fo.getEmpire().empireID
    try:
        if os.path.exists("timers") and os.path.isdir("timers"):
            timerpath="timers"+os.path.sep+"timer_%02d.dat"%(empireID-1)
            __timerFile = open(timerpath,  'w')
            __timerFile.write("Turn\t" + "\t".join(__timerEntries) +'\n')
            if ResourcesAI.doResourceTiming:
                ResourcesAI.resourceTimerFile = open("timers"+os.path.sep+"resourceTimer_%2d.dat"%(empireID-1),  'w')
                ResourcesAI.resourceTimerFile.write("Turn\t"+ "\t".join(ResourcesAI.timerEntries)+"\n")
            print "timer file saved at "+timerpath
    except:
        __timerFile=None
        ResourcesAI.resourceTimerFile  =None
        ResourcesAI.doResourceTiming = False

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

# called when this player recives a diplomatic message update from the server, such as if another player
# declares war, accepts peace, or cancels a proposed peace treaty.
def handleDiplomaticMessage(message):
    print "Received diplomatic " + str(message.type) + " message from empire " + str(message.sender) + " to empire " + str(message.recipient)
    print "my empire id: " + str(fo.empireID())
    if (message.type == fo.diplomaticMessageType.peaceProposal and message.recipient == fo.empireID()):
        replySender = message.recipient
        replyRecipient = message.sender
        reply = fo.diplomaticMessage(replySender, replyRecipient, fo.diplomaticMessageType.acceptProposal)
        print "Sending diplomatic message to empire " + str(replyRecipient) + " of type " + str(reply.type)
        fo.sendDiplomaticMessage(reply)

# called when this player receives and update about the diplomatic status between players, which may
# or may not include this player.
def handleDiplomaticStatusUpdate(statusUpdate):
    print "Received diplomatic status update to " + str (statusUpdate.status) + " about empire " + str(statusUpdate.empire1) + " and empire " + str(statusUpdate.empire2)

def declareWarOnAll():
    my_emp_id = fo.empireID()
    for emp_id in fo.allEmpireIDs():
        if emp_id != my_emp_id:
            msg = fo.diplomaticMessage(my_emp_id, emp_id, fo.diplomaticMessageType.warDeclaration)
            fo.sendDiplomaticMessage(msg)

# called once per turn to tell the Python AI to generate and issue orders to control its empire.
# at end of this function, fo.doneTurn() should be called to indicate to the client that orders are finished
# and can be sent to the server for processing.
def generateOrders():
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    planetID = PlanetUtilsAI.getCapital()
    planet = universe.getPlanet(planetID)
    print "***************************************************************************"
    print "***************************************************************************"
    print ("Generating Orders")
    print "EmpireID:    " + str(empire.empireID) + " Name: " + empire.name + " Turn: " + str(fo.currentTurn())
    if planet: 
        print "CapitalID: " + str(planetID) + " Name: " + planet.name + " Species: " + planet.speciesName 
    else:
        print "No current capital"
    print "***************************************************************************"
    print "***************************************************************************"
    
    if fo.currentTurn() == 1:
        declareWarOnAll()

    # turn cleanup !!! this was formerly done at start of every turn -- not sure why
    splitNewFleets()

    #updateShipDesigns()   #should not be needed anymore;
    #updateFleetsRoles()
    
    foAIstate.clean() #checks exploration border & clears roles/missions of missing fleets & updates fleet locs
    foAIstate.reportSystemThreats()
    # ...missions
    # ...demands/priorities

    print("Calling AI Modules")

    # call AI modules
    timer=[time()]
    PriorityAI.calculatePriorities()
    timer.append( time()  )
    ExplorationAI.assignScoutsToExploreSystems()
    timer.append( time()  )
    ColonisationAI.assignColonyFleetsToColonise()
    timer.append( time()  )
    InvasionAI.assignInvasionFleetsToInvade()
    timer.append( time()  )
    MilitaryAI.assignMilitaryFleetsToSystems()
    timer.append( time()  )
    FleetUtilsAI.generateAIFleetOrdersForAIFleetMissions()
    timer.append( time()  )
    FleetUtilsAI.issueAIFleetOrdersForAIFleetMissions()
    timer.append( time()  )
    ResearchAI.generateResearchOrders()
    timer.append( time()  )
    ProductionAI.generateProductionOrders()
    timer.append( time()  )
    ResourcesAI.generateResourcesOrders()    
    timer.append( time()  )
    foAIstate.afterTurnCleanup()
    timer.append( time()  )
    times = [timer[i] - timer[i-1] for i in range(1,  len(timer) ) ]
    timeFmt = "%30s: %8d msec  "
    print "AI Module Time Requirements:"
    for mod,  modTime in zip(__timerEntries,  times):
        print timeFmt%((30*' '+mod)[-30:],  int(1000*modTime))
    if __timerFile:
        __timerFile.write(  __timerFileFmt%tuple( [ fo.currentTurn() ]+map(lambda x: int(1000*x),  times )) +'\n')
        __timerFile.flush()
    fo.doneTurn()
