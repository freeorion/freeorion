
# pylint: disable=line-too-long
""" The FreeOrionAI module contains the methods which can be made by the C AIInterface;
these methods in turn activate other portions of the python AI code"""

import os
import pickle                       # Python object serialization library
import sys
import traceback
from time import time
import random
from timing import main_timer, bucket_timer, resource_timer, init_timers


import freeOrionAIInterface as fo   # interface used to interact with FreeOrion AI client    # pylint: disable=import-error

#pylint: disable=relative-import
import AIstate
import ColonisationAI
import ExplorationAI
import FleetUtilsAI
import InvasionAI
import MilitaryAI
import PlanetUtilsAI
import PriorityAI
import ProductionAI
import ResearchAI
import ResourcesAI

using_statprof = False
try:
    import statprof
    #statprof.start()
    #using_statprof = True
except:
    pass


def UserString(label,  default=None): #this name left with C naming style for compatibility with translation assistance procedures  #pylint: disable=invalid-name
    """ a translation assistance tool is intended to search for this method to identify translatable strings"""
    table_string = fo.userString(label)
    if default is None:
        return table_string
    elif "ERROR: "+label in table_string:  #implement test for string lookup  not found error
        return default
    else:
        return table_string

_aggressions = {fo.aggression.beginner:"Beginner",  fo.aggression.turtle:"Turtle",  fo.aggression.cautious:"Cautious",  fo.aggression.typical:"Moderate",
             fo.aggression.aggressive:"Aggressive",  fo.aggression.maniacal:"Maniacal"}
_capitols = {fo.aggression.beginner:UserString("AI_CAPITOL_NAMES_BEGINNER", ""),  fo.aggression.turtle:UserString("AI_CAPITOL_NAMES_TURTLE", ""), fo.aggression.cautious:UserString("AI_CAPITOL_NAMES_CAUTIOUS", ""),
                    fo.aggression.typical:UserString("AI_CAPITOL_NAMES_TYPICAL", ""),  fo.aggression.aggressive:UserString("AI_CAPITOL_NAMES_AGGRESSIVE", ""),  fo.aggression.maniacal:UserString("AI_CAPITOL_NAMES_MANIACAL", "")}
# AIstate
foAIstate = None
_lastTurnTimestamp = 0


# called when Python AI starts, before any game new game starts or saved game is resumed
def initFreeOrionAI(): # pylint: disable=invalid-name
    """called by client to initialize AI """
    print "Initialized FreeOrion Python AI"
    print(sys.path)


# called when a new game is started (but not when a game is loaded).  should clear any pre-existing state
# and set up whatever is needed for AI to generate orders
def startNewGame(aggression=fo.aggression.aggressive): # pylint: disable=invalid-name
    """called by client at start of new game"""
    init_timers()
    print "New game started, AI Aggression level %d"% aggression

    # initialize AIstate
    global foAIstate
    foAIstate = AIstate.AIstate(aggression = aggression)
    foAIstate.sessionStartCleanup()
    print "Initialized foAIstate class"
    planet_id = PlanetUtilsAI.getCapital()
    planet = None
    universe = fo.getUniverse()
    if planet_id is not None and planet_id != -1:
        planet = universe.getPlanet(planet_id)
        new_name = random.choice(_capitols.get(aggression,  [""]).split('\n')).strip() + " " + planet.name
        print "Capitol City Names are: ",  _capitols
        print "This Capitol New name is ",  new_name
        res = fo.issueRenameOrder(planet_id,  new_name)
        print "Capitol Rename attempt result: %d; planet now named %s"% (res,  planet.name)


# called when client receives a load game message
def resumeLoadedGame(savedStateString): # pylint: disable=invalid-name
    """called by client to resume a loaded game"""
    init_timers()

    global foAIstate
    print "Resuming loaded game"
    try:
        #loading saved state
        foAIstate = pickle.loads(savedStateString)
        foAIstate.sessionStartCleanup()
    except:
        print "failed to parse saved state string"
        #assigning new state
        foAIstate = AIstate.AIstate(aggression=fo.aggression.aggressive)
        foAIstate.sessionStartCleanup()
        print "Error: exception triggered and caught:  ",  traceback.format_exc()


# called when the game is about to be saved, to let the Python AI know it should save any AI state
# information, such as plans or knowledge about the game from previous turns, in the state string so that
# they can be restored if the game is loaded
def prepareForSave(): # pylint: disable=invalid-name
    """called by client to preparing for game save by serializing state"""
    print "Preparing for game save by serializing state"

    # serialize (convert to string) global state dictionary and send to AI client to be stored in save file
    dumpStr = pickle.dumps(foAIstate)
    print "foAIstate pickled to string,  about to send to server"
    fo.setSaveStateString(dumpStr)


# called when this player receives a chat message.  senderID is the player who sent the message, and
# messageText is the text of the sent message
def handleChatMessage(senderID, messageText): # pylint: disable=invalid-name
    """called by client to handle chat messages"""
    print "Received chat message from " + str(senderID) + " that says: " + messageText + " - ignoring it"


# called when this player recives a diplomatic message update from the server, such as if another player
# declares war, accepts peace, or cancels a proposed peace treaty.
def handleDiplomaticMessage(message): # pylint: disable=invalid-name
    """called by client to handle diplomatic messages"""
    print "Received diplomatic " + str(message.type) + " message from empire " + str(message.sender) + " to empire " + str(message.recipient)
    print "my empire id: " + str(fo.empireID())
    if message.type == fo.diplomaticMessageType.peaceProposal and message.recipient == fo.empireID():
        replySender = message.recipient
        replyRecipient = message.sender
        proposalSenderPlayer = fo.empirePlayerID(message.sender)
        fo.sendChatMessage(proposalSenderPlayer,  "So,  the Terran Hairless Plains Ape advising your empire wishes to scratch its belly for a while?")
        if (  (foAIstate.aggression==fo.aggression.beginner )  or
                (foAIstate.aggression!=fo.aggression.maniacal ) and (  random.random() < 1.0/ (((foAIstate.aggression +0.01)*fo.currentTurn()/2)**0.5)  )):
            fo.sendChatMessage(proposalSenderPlayer,  "OK, Peace offer accepted.")
            reply = fo.diplomaticMessage(replySender, replyRecipient, fo.diplomaticMessageType.acceptProposal)
            print "Sending diplomatic message to empire " + str(replyRecipient) + " of type " + str(reply.type)
            fo.sendDiplomaticMessage(reply)
        else:
            fo.sendChatMessage(proposalSenderPlayer,  "Maybe later.  We are currently getting busy  with Experimental Test Subject yo-Ma-ma.")


# called when this player receives and update about the diplomatic status between players, which may
# or may not include this player.
def handleDiplomaticStatusUpdate(statusUpdate): # pylint: disable=invalid-name
    """called by client to handle diplomatic status updates"""
    print "Received diplomatic status update to " + str (statusUpdate.status) + " about empire " + str(statusUpdate.empire1) + " and empire " + str(statusUpdate.empire2)


# called once per turn to tell the Python AI to generate and issue orders to control its empire.
# at end of this function, fo.doneTurn() should be called to indicate to the client that orders are finished
# and can be sent to the server for processing.
def generateOrders(): # pylint: disable=invalid-name
    """called by client to get the AI's orders for the turn"""
    global _lastTurnTimestamp
    universe = fo.getUniverse()
    turnStartTime = time() #starting AI timer here, to be sure AI doesn't get blame for any  lags in server being able to provide the Universe object
    empire = fo.getEmpire()
    planetID = PlanetUtilsAI.getCapital()
    # set the random seed (based on galaxy seed, empire ID and current turn)
    # for game-reload consistency 
    random_seed = str(fo.getGalaxySetupData().seed) + "%03d%05d"%(fo.empireID(),  fo.currentTurn())
    random.seed(random_seed)
    planet = None
    if planetID is not None:
        planet = universe.getPlanet(planetID)
    print "***************************************************************************"
    print "***************************************************************************"
    print ("Generating Orders")
    res_idx = ResearchAI.get_research_index()
    print "EmpireID:    " + str(empire.empireID) + " Name: " + empire.name+ "_"+str(empire.empireID) +"_pid:"+str(fo.playerID())+"_"+fo.playerName()+"_"+("RIdx_%d"%res_idx)+"_"+_aggressions.get(foAIstate.aggression,  "?") + " Turn: " + str(fo.currentTurn())
    empireColor = empire.colour
    print "EmpireColors: %d %d %d %d"% (empireColor.r,  empireColor.g,  empireColor.b,  empireColor.a)
    if planet:
        print "CapitalID: " + str(planetID) + " Name: " + planet.name + " Species: " + planet.speciesName
    else:
        print "CapitalID: None Currently      Name: None     Species: None "
    print "***************************************************************************"
    print "***************************************************************************"

    if fo.currentTurn() == 1:
        declareWarOnAll()

    # turn cleanup !!! this was formerly done at start of every turn -- not sure why
    splitNewFleets()

    #updateShipDesigns()   #should not be needed anymore;
    #updateFleetsRoles()

    foAIstate.refresh() #checks exploration border & clears roles/missions of missing fleets & updates fleet locs & threats
    foAIstate.reportSystemThreats()
    # ...missions
    # ...demands/priorities

    print("Calling AI Modules")

    # call AI modules
    timer = [time()]
    try:
        PriorityAI.calculatePriorities()
    except:
        print "Error: exception triggered and caught:  ",  traceback.format_exc() # try traceback.print_exc()
    timer.append( time()  )
    try:
        ExplorationAI.assignScoutsToExploreSystems()
    except:
        print "Error: exception triggered and caught:  ",  traceback.format_exc()
    timer.append( time()  )
    try:
        ColonisationAI.assignColonyFleetsToColonise()
    except:
        print "Error: exception triggered and caught:  ",  traceback.format_exc()
    timer.append( time()  )
    try:
        InvasionAI.assignInvasionFleetsToInvade()
    except:
        print "Error: exception triggered and caught:  ",  traceback.format_exc()
    timer.append( time()  )
    try:
        MilitaryAI.assignMilitaryFleetsToSystems()
    except:
        print "Error: exception triggered and caught:  ",  traceback.format_exc()
    timer.append( time()  )
    try:
        FleetUtilsAI.generateAIFleetOrdersForAIFleetMissions()
    except:
        print "Error: exception triggered and caught:  ",  traceback.format_exc()
    timer.append( time()  )
    try:
        FleetUtilsAI.issueAIFleetOrdersForAIFleetMissions()
    except:
        print "Error: exception triggered and caught:  ",  traceback.format_exc()
    timer.append( time()  )
    try:
        ResearchAI.generateResearchOrders()
    except:
        print "Error: exception triggered and caught:  ",  traceback.format_exc()
    timer.append( time()  )
    try:
        ProductionAI.generateProductionOrders()
    except:
        print "Error: exception triggered and caught:  ",  traceback.format_exc()
    timer.append( time()  )
    try:
        ResourcesAI.generateResourcesOrders()
    except:
        print "Error: exception triggered and caught:  ",  traceback.format_exc()
    timer.append( time()  )
    try:
        foAIstate.afterTurnCleanup()
    except:
        print "Error: exception triggered and caught:  ",  traceback.format_exc()
    timer.append( time()  )
    times = [timer[i] - timer[i-1] for i in range(1,  len(timer))]
    turnEndTime = time()
    main_timer.add_time(fo.currentTurn(), times)
    bucket_timer.add_time(fo.currentTurn(), (turnStartTime - _lastTurnTimestamp, turnEndTime-turnStartTime))

    try:
        fo.doneTurn()
    except:
        print "Error: exception triggered and caught:  ",  traceback.format_exc()

    if using_statprof:
        try:
            statprof.stop()
            statprof.display()
            statprof.start()
        except:
            pass
#
#The following methods should probably be moved to the AIstate module, to keep this module more focused on implementing required interface


def splitNewFleets(): # pylint: disable=invalid-name
    """split any new fleets (at new game creation, can have unplanned mix of ship roles)"""

    print "Review of current Fleet Role/Mission records:"
    print "--------------------"
    print "Map of Roles keyed by Fleet ID: %s"% foAIstate.getFleetRolesMap()
    print "--------------------"
    print "Map of Missions  keyed by ID:"
    for item in  foAIstate.getFleetMissionsMap().items():
        print " %4d : %s "% item
    print "--------------------"
    # TODO: check length of fleets for losses  or do in AIstat.__cleanRoles
    knownFleets = foAIstate.getFleetRolesMap().keys()
    foAIstate.newlySplitFleets.clear()
    splitableFleets = []
    for fleetID in FleetUtilsAI.getEmpireFleetIDs():
        if fleetID  in  knownFleets: #not a new fleet
            continue
        else:
            splitableFleets.append(fleetID)
    if splitableFleets:
        universe = fo.getUniverse()
        print ("splitting new fleets")
        for fleetID in splitableFleets:
            fleet = universe.getFleet(fleetID)
            if not fleet:
                print "Error splittting new fleets; resulting fleet ID %d  appears to not exist"% fleetID
                continue
            fleetLen = len(list(fleet.shipIDs))
            if fleetLen == 1:
                continue
            newFleets = FleetUtilsAI.splitFleet(fleetID) # try splitting fleet
            print "\t from splitting fleet ID %4d  with %d ships, got %d new fleets:"% (fleetID,  fleetLen,  len(newFleets))
            # old fleet may have different role after split, later will be again identified
            #foAIstate.removeFleetRole(fleetID)  # in current system, orig new fleet will not yet have been assigned a role


def updateShipDesigns(): # pylint: disable=invalid-name
    """update ship design records"""
    print ("Updating ship design records")
    universe = fo.getUniverse()

    for fleetID in universe.fleetIDs:
        fleet = universe.getFleet(fleetID)
        if fleet:
            for shipID in fleet.shipIDs:
                ship = universe.getShip(shipID)
                if ship and ship.design:
                    foAIstate.getShipRole(ship.design.id)
            # print str(ship.design.id) + ": " + str(shipRole)


def updateFleetsRoles(): # pylint: disable=invalid-name
    """updating fleet role records"""
    print ("Updating fleet role records")
    # assign roles to fleets
    for fleetID in FleetUtilsAI.getEmpireFleetIDs():
        foAIstate.getFleetRole(fleetID) #force assessment if not previously known


def declareWarOnAll(): # pylint: disable=invalid-name
    """used to declare war on all other empires (at start of game)"""
    my_emp_id = fo.empireID()
    for emp_id in fo.allEmpireIDs():
        if emp_id != my_emp_id:
            msg = fo.diplomaticMessage(my_emp_id, emp_id, fo.diplomaticMessageType.warDeclaration)
            fo.sendDiplomaticMessage(msg)
