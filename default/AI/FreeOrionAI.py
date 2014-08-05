
# pylint: disable=line-too-long
""" The FreeOrionAI module contains the methods which can be made by the C AIInterface;
these methods in turn activate other portions of the python AI code"""

import pickle                       # Python object serialization library
import sys
import random
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
from tools import UserString
from debug_tools import chat_on_error, print_error
from timing import Timer

main_timer = Timer('timer', write_log=True)
turn_timer = Timer('bucket', write_log=True)

using_statprof = False
try:
    import statprof
    #statprof.start()
    #using_statprof = True
except:
    pass


_aggressions = {fo.aggression.beginner:"Beginner",  fo.aggression.turtle:"Turtle",  fo.aggression.cautious:"Cautious",  fo.aggression.typical:"Moderate",
             fo.aggression.aggressive:"Aggressive",  fo.aggression.maniacal:"Maniacal"}
_capitols = {fo.aggression.beginner:UserString("AI_CAPITOL_NAMES_BEGINNER", ""),  fo.aggression.turtle:UserString("AI_CAPITOL_NAMES_TURTLE", ""), fo.aggression.cautious:UserString("AI_CAPITOL_NAMES_CAUTIOUS", ""),
                    fo.aggression.typical:UserString("AI_CAPITOL_NAMES_TYPICAL", ""),  fo.aggression.aggressive:UserString("AI_CAPITOL_NAMES_AGGRESSIVE", ""),  fo.aggression.maniacal:UserString("AI_CAPITOL_NAMES_MANIACAL", "")}
# AIstate
foAIstate = None


# called when Python AI starts, before any game new game starts or saved game is resumed
def initFreeOrionAI(): # pylint: disable=invalid-name
    """called by client to initialize AI """
    print "Initialized FreeOrion Python AI"
    print(sys.path)


# called when a new game is started (but not when a game is loaded).  should clear any pre-existing state
# and set up whatever is needed for AI to generate orders
@chat_on_error
def startNewGame(aggression=fo.aggression.aggressive): # pylint: disable=invalid-name
    """called by client at start of new game"""
    turn_timer.start("Server Processing")
    print "New game started, AI Aggression level %d"% aggression

    # initialize AIstate
    global foAIstate
    foAIstate = AIstate.AIstate(aggression = aggression)
    foAIstate.session_start_cleanup()
    print "Initialized foAIstate class"
    planet_id = PlanetUtilsAI.get_capital()
    planet = None
    universe = fo.getUniverse()
    if planet_id is not None and planet_id != -1:
        planet = universe.getPlanet(planet_id)
        new_name = random.choice(_capitols.get(aggression,  "").split('\n')).strip() + " " + planet.name
        print "Capitol City Names are: ",  _capitols
        print "This Capitol New name is ",  new_name
        res = fo.issueRenameOrder(planet_id,  new_name)
        print "Capitol Rename attempt result: %d; planet now named %s"% (res,  planet.name)


# called when client receives a load game message
@chat_on_error
def resumeLoadedGame(savedStateString): # pylint: disable=invalid-name
    """called by client to resume a loaded game"""
    turn_timer.start("Server Processing")

    global foAIstate
    print "Resuming loaded game"
    try:
        #loading saved state
        foAIstate = pickle.loads(savedStateString)
        foAIstate.session_start_cleanup()
    except:
        print "failed to parse saved state string"
        #assigning new state
        foAIstate = AIstate.AIstate(aggression=fo.aggression.aggressive)
        foAIstate.session_start_cleanup()
        print_error("Fail to load aiState form saved game")


# called when the game is about to be saved, to let the Python AI know it should save any AI state
# information, such as plans or knowledge about the game from previous turns, in the state string so that
# they can be restored if the game is loaded
@chat_on_error
def prepareForSave(): # pylint: disable=invalid-name
    """called by client to preparing for game save by serializing state"""
    print "Preparing for game save by serializing state"

    # serialize (convert to string) global state dictionary and send to AI client to be stored in save file
    dumpStr = pickle.dumps(foAIstate)
    print "foAIstate pickled to string,  about to send to server"
    fo.setSaveStateString(dumpStr)


# called when this player receives a chat message.  senderID is the player who sent the message, and
# messageText is the text of the sent message
@chat_on_error
def handleChatMessage(senderID, messageText): # pylint: disable=invalid-name
    """called by client to handle chat messages"""
    print "Received chat message from " + str(senderID) + " that says: " + messageText + " - ignoring it"


# called when this player recives a diplomatic message update from the server, such as if another player
# declares war, accepts peace, or cancels a proposed peace treaty.
@chat_on_error
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
@chat_on_error
def handleDiplomaticStatusUpdate(statusUpdate): # pylint: disable=invalid-name
    """called by client to handle diplomatic status updates"""
    print "Received diplomatic status update to " + str (statusUpdate.status) + " about empire " + str(statusUpdate.empire1) + " and empire " + str(statusUpdate.empire2)


# called once per turn to tell the Python AI to generate and issue orders to control its empire.
# at end of this function, fo.doneTurn() should be called to indicate to the client that orders are finished
# and can be sent to the server for processing.
@chat_on_error
def generateOrders(): # pylint: disable=invalid-name
    """called by client to get the AI's orders for the turn"""
    turn_timer.start("AI planning")
    universe = fo.getUniverse()
    empire = fo.getEmpire()
    planetID = PlanetUtilsAI.get_capital()
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
        human_player = fo.empirePlayerID(1)
        fo.sendChatMessage(human_player,  "%s Empire, Aggression: %s"%(empire.name, _aggressions.get(foAIstate.aggression,  "?")))

    # turn cleanup !!! this was formerly done at start of every turn -- not sure why
    foAIstate.split_new_fleets()

    foAIstate.refresh() #checks exploration border & clears roles/missions of missing fleets & updates fleet locs & threats
    foAIstate.report_system_threats()
    # ...missions
    # ...demands/priorities
    print("Calling AI Modules")
    # call AI modules
    action_list = [PriorityAI.calculate_priorities,
                   ExplorationAI.assign_scouts_to_explore_systems,
                   ColonisationAI.assign_colony_fleets_to_colonise,
                   InvasionAI.assign_invasion_fleets_to_invade,
                   MilitaryAI.assign_military_fleets_to_systems,
                   FleetUtilsAI.generate_fleet_orders_for_fleet_missions,
                   FleetUtilsAI.issue_fleet_orders_for_fleet_missions,
                   ResearchAI.generate_research_orders,
                   ProductionAI.generateProductionOrders,
                   ResourcesAI.generate_resources_orders,
                   foAIstate.after_turn_cleanup,
                   ]

    for action in action_list:
        try:
            main_timer.start(action.__name__)
            action()
            main_timer.stop()
        except Exception as e:
            print_error(e, location=action.__name__)
    main_timer.end()
    turn_timer.end()
    turn_timer.start("Server_Processing")

    try:
        fo.doneTurn()
    except Exception as e:
        print_error(e)  # TODO move it to cycle above

    if using_statprof:
        try:
            statprof.stop()
            statprof.display()
            statprof.start()
        except:
            pass


#The following methods should probably be moved to the AIstate module, to keep this module more focused on implementing required interface
def declareWarOnAll(): # pylint: disable=invalid-name
    """used to declare war on all other empires (at start of game)"""
    my_emp_id = fo.empireID()
    for emp_id in fo.allEmpireIDs():
        if emp_id != my_emp_id:
            msg = fo.diplomaticMessage(my_emp_id, emp_id, fo.diplomaticMessageType.warDeclaration)
            fo.sendDiplomaticMessage(msg)
