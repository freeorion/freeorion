"""The FreeOrionAI module contains the methods which can be made by the C AIInterface;
these methods in turn activate other portions of the python AI code."""
from common.configure_logging import redirect_logging_to_freeorion_logger, convenience_function_references_for_logger

# Logging is redirected before other imports so that import errors appear in log files.
redirect_logging_to_freeorion_logger()
(debug, info, warn, error, fatal) = convenience_function_references_for_logger()

import pickle  # Python object serialization library
import sys
import random

import freeOrionAIInterface as fo  # interface used to interact with FreeOrion AI client  # pylint: disable=import-error


from common.option_tools import parse_config
parse_config(fo.getOptionsDBOptionStr("ai-config"), fo.getUserConfigDir())

from freeorion_tools import patch_interface
patch_interface()

import AIstate
import ColonisationAI
import ExplorationAI
import DiplomaticCorp
import FleetUtilsAI
import InvasionAI
import MilitaryAI
import PlanetUtilsAI
import PriorityAI
import ProductionAI
import ResearchAI
import ResourcesAI
import TechsListsAI
from AIDependencies import INVALID_ID
from freeorion_tools import handle_debug_chat, AITimer, init_handlers
from common.listeners import listener
from character.character_module import Aggression
from character.character_strings_module import get_trait_name_aggression, possible_capitals

main_timer = AITimer('timer', write_log=True)
turn_timer = AITimer('bucket', write_log=True)

using_statprof = False
try:
    import statprof
    # statprof.start()
    # using_statprof = True
except ImportError:
    pass


user_dir = fo.getUserDataDir()
debug("Path to folder for user specific data: %s" % user_dir)
debug('Python paths %s' % sys.path)


# Mock to have proper inspection and autocomplete for this variable
class AIStateMock(AIstate.AIstate):
    def __init__(self):
        pass


# AIstate
foAIstate = AIStateMock()
diplomatic_corp = None


def startNewGame(aggression_input=fo.aggression.aggressive):  # pylint: disable=invalid-name
    """Called by client when a new game is started (but not when a game is loaded).
    Should clear any pre-existing state and set up whatever is needed for AI to generate orders."""
    empire = fo.getEmpire()
    if empire is None:
        print "This client has no empire. Ignoring new game start message."
        return

    if empire.eliminated:
        info("This empire has been eliminated. Ignoring new game start message.")
        return

    turn_timer.start("Server Processing")

    # initialize AIstate
    global foAIstate
    debug("Initializing foAIstate...")
    foAIstate = AIstate.AIstate(aggression_input)
    aggression_trait = foAIstate.character.get_trait(Aggression)
    debug("New game started, AI Aggression level %d (%s)" % (
        aggression_trait.key, get_trait_name_aggression(foAIstate.character)))
    foAIstate.session_start_cleanup()
    debug("Initialization of foAIstate complete!")
    debug("Trying to rename our homeworld...")
    planet_id = PlanetUtilsAI.get_capital()
    universe = fo.getUniverse()
    if planet_id is not None and planet_id != INVALID_ID:
        planet = universe.getPlanet(planet_id)
        new_name = " ".join([random.choice(possible_capitals(foAIstate.character)).strip(), planet.name])
        debug("    Renaming to %s..." % new_name)
        res = fo.issueRenameOrder(planet_id, new_name)
        debug("    Result: %d; Planet is now named %s" % (res, planet.name))

    diplomatic_corp_configs = {fo.aggression.beginner: DiplomaticCorp.BeginnerDiplomaticCorp,
                               fo.aggression.maniacal: DiplomaticCorp.ManiacalDiplomaticCorp}
    global diplomatic_corp
    diplomatic_corp = diplomatic_corp_configs.get(aggression_trait.key, DiplomaticCorp.DiplomaticCorp)()
    TechsListsAI.test_tech_integrity()


def resumeLoadedGame(saved_state_string):  # pylint: disable=invalid-name
    """Called by client to when resume a loaded game."""
    if fo.getEmpire() is None:
        print "This client has no empire. Doing nothing to resume loaded game."
        return

    if fo.getEmpire().eliminated:
        print "This empire has been eliminated. Ignoring resume loaded game."
        return
    turn_timer.start("Server Processing")

    global foAIstate
    print "Resuming loaded game"
    if not saved_state_string:
        error("AI given empty state-string to resume from; this is expected if the AI is assigned to an empire "
              "previously run by a human, but is otherwise an error. AI will be set to Aggressive.")
        foAIstate = AIstate.AIstate(fo.aggression.aggressive)
        foAIstate.session_start_cleanup()
    else:
        try:
            # loading saved state
            # pre load code
            foAIstate = pickle.loads(saved_state_string)
        except Exception as e:
            # assigning new state
            foAIstate = AIstate.AIstate(fo.aggression.aggressive)
            foAIstate.session_start_cleanup()
            error("Fail to load aiState from saved game: %s" % e, exc_info=True)

    aggression_trait = foAIstate.character.get_trait(Aggression)
    diplomatic_corp_configs = {fo.aggression.beginner: DiplomaticCorp.BeginnerDiplomaticCorp,
                               fo.aggression.maniacal: DiplomaticCorp.ManiacalDiplomaticCorp}
    global diplomatic_corp
    diplomatic_corp = diplomatic_corp_configs.get(aggression_trait.key, DiplomaticCorp.DiplomaticCorp)()
    TechsListsAI.test_tech_integrity()


def prepareForSave():  # pylint: disable=invalid-name
    """Called by client when the game is about to be saved, to let the Python AI know it should save any AI state
    information, such as plans or knowledge about the game from previous turns,
    in the state string so that they can be restored if the game is loaded."""
    empire = fo.getEmpire()
    if empire is None:
        print "This client has no empire. Doing nothing to prepare for save."
        return

    if empire.eliminated:
        info("This empire has been eliminated. Save info request")
        return

    info("Preparing for game save by serializing state")

    # serialize (convert to string) global state dictionary and send to AI client to be stored in save file
    try:
        dump_string = pickle.dumps(foAIstate)
        print "foAIstate pickled to string, about to send to server"
        fo.setSaveStateString(dump_string)
    except:
        error("foAIstate unable to pickle save-state string; "
              "the save file should be playable but the AI may have a different aggression.", exc_info=True)


def handleChatMessage(sender_id, message_text):  # pylint: disable=invalid-name
    """Called when this player receives a chat message. sender_id is the player who sent the message, and
    message_text is the text of the sent message."""
    empire = fo.getEmpire()
    if empire is None:
        print "This client has no empire. Doing nothing to handle chat message."
        return

    if empire.eliminated:
        debug("This empire has been eliminated. Ignoring chat message")
        return

    # debug("Received chat message from " + str(senderID) + " that says: " + messageText + " - ignoring it")
    # perhaps it is a debugging interaction
    if handle_debug_chat(sender_id, message_text):
        return
    if not diplomatic_corp:
        DiplomaticCorp.handle_pregame_chat(sender_id, message_text)
    else:
        diplomatic_corp.handle_midgame_chat(sender_id, message_text)


def handleDiplomaticMessage(message):  # pylint: disable=invalid-name
    """Called when this player receives a diplomatic message update from the server,
    such as if another player declares war, accepts peace, or cancels a proposed peace treaty."""
    empire = fo.getEmpire()
    if empire is None:
        print "This client has no empire. Doing nothing to handle diplomatic message."
        return

    if empire.eliminated:
        debug("This empire has been eliminated. Ignoring diplomatic message")
        return

    diplomatic_corp.handle_diplomatic_message(message)


def handleDiplomaticStatusUpdate(status_update):  # pylint: disable=invalid-name
    """Called when this player receives an update about the diplomatic status between players, which may
    or may not include this player."""
    empire = fo.getEmpire()
    if empire is None:
        print "This client has no empire. Doing nothing to handle diplomatic status message."
        return

    if empire.eliminated:
        debug("This empire has been eliminated. Ignoring diplomatic status update")
        return

    diplomatic_corp.handle_diplomatic_status_update(status_update)


@listener
def generateOrders():  # pylint: disable=invalid-name
    """Called once per turn to tell the Python AI to generate and issue orders to control its empire.
    at end of this function, fo.doneTurn() should be called to indicate to the client that orders are finished
    and can be sent to the server for processing."""

    rules = fo.getGameRules()
    print "Defined game rules:"
    for rule in rules.getRulesAsStrings:
        print "Name: " + rule.name + "  value: " + str(rule.value)
    print "Rule RULE_NUM_COMBAT_ROUNDS value: " + str(rules.getInt("RULE_NUM_COMBAT_ROUNDS"))

    empire = fo.getEmpire()
    if empire is None:
        print "This client has no empire. Doing nothing to generate orders."
        try:
            # early abort if no empire. no need to do meter calculations
            # on last-seen gamestate if nothing can be ordered anyway...
            #
            # note that doneTurn() is issued on behalf of the client network
            # id, not the empire id, so not having a correct empire id does
            # not invalidate doneTurn()
            fo.doneTurn()
        except Exception as e:
            error("Exception %s in doneTurn() on non-existent empire" % e, exc_info=True)
        return

    if empire.eliminated:
        debug("This empire has been eliminated. Aborting order generation")
        try:
            # early abort if already eliminated. no need to do meter calculations
            # on last-seen gamestate if nothing can be ordered anyway...
            fo.doneTurn()
        except Exception as e:
            error("Exception %s while trying doneTurn() on eliminated empire" % e, exc_info=True)
        return

    # This code block is required for correct AI work.
    info("Meter / Resource Pool updating...")
    fo.initMeterEstimatesDiscrepancies()
    fo.updateMeterEstimates(False)
    fo.updateResourcePools()

    turn = fo.currentTurn()
    turn_uid = foAIstate.set_turn_uid()
    debug("\n\n\n" + "=" * 20)
    debug("Starting turn %s (%s) of game: %s" % (turn, turn_uid, foAIstate.uid))
    debug("=" * 20 + "\n")

    turn_timer.start("AI planning")
    # set the random seed (based on galaxy seed, empire name and current turn)
    # for game-reload consistency.
    random_seed = str(fo.getGalaxySetupData().seed) + "%05d%s" % (turn, fo.getEmpire().name)
    random.seed(random_seed)

    universe = fo.getUniverse()
    empire = fo.getEmpire()
    planet_id = PlanetUtilsAI.get_capital()
    planet = None
    if planet_id is not None:
        planet = universe.getPlanet(planet_id)
    aggression_name = get_trait_name_aggression(foAIstate.character)
    print "***************************************************************************"
    print "*******  Log info for AI progress chart script. Do not modify.   **********"
    print ("Generating Orders")
    print ("EmpireID: {empire.empireID}"
           " Name: {empire.name}_{empire.empireID}_pid:{p_id}_{p_name}RIdx_{res_idx}_{aggression}"
           " Turn: {turn}").format(empire=empire, p_id=fo.playerID(), p_name=fo.playerName(),
                                   res_idx=ResearchAI.get_research_index(), turn=turn,
                                   aggression=aggression_name.capitalize())
    print "EmpireColors: {0.colour.r} {0.colour.g} {0.colour.b} {0.colour.a}".format(empire)
    if planet:
        print "CapitalID: " + str(planet_id) + " Name: " + planet.name + " Species: " + planet.speciesName
    else:
        print "CapitalID: None Currently Name: None Species: None "
    print "***************************************************************************"
    print "***************************************************************************"

    if turn == 1:
        declare_war_on_all()
        human_player = fo.empirePlayerID(1)
        greet = diplomatic_corp.get_first_turn_greet_message()
        fo.sendChatMessage(human_player,
                           '%s (%s): [[%s]]' % (empire.name, get_trait_name_aggression(foAIstate.character), greet))

    foAIstate.prepare_for_new_turn()
    debug("Calling AI Modules")
    # call AI modules
    action_list = [ColonisationAI.survey_universe,
                   ProductionAI.find_best_designs_this_turn,
                   PriorityAI.calculate_priorities,
                   ExplorationAI.assign_scouts_to_explore_systems,
                   ColonisationAI.assign_colony_fleets_to_colonise,
                   InvasionAI.assign_invasion_fleets_to_invade,
                   MilitaryAI.assign_military_fleets_to_systems,
                   FleetUtilsAI.generate_fleet_orders_for_fleet_missions,
                   FleetUtilsAI.issue_fleet_orders_for_fleet_missions,
                   ResearchAI.generate_research_orders,
                   ProductionAI.generate_production_orders,
                   ResourcesAI.generate_resources_orders,
                   ]

    for action in action_list:
        try:
            main_timer.start(action.__name__)
            action()
            main_timer.stop()
        except Exception as e:
            error("Exception %s while trying to %s" % (e, action.__name__), exc_info=True)
    main_timer.stop_print_and_clear()
    turn_timer.stop_print_and_clear()
    turn_timer.start("Server_Processing")

    try:
        fo.doneTurn()
    except Exception as e:
        error("Exception %s while trying doneTurn()" % e, exc_info=True)  # TODO move it to cycle above

    if using_statprof:
        try:
            statprof.stop()
            statprof.display()
            statprof.start()
        except:
            pass


# The following methods should probably be moved to the AIstate module,
# to keep this module more focused on implementing required interface
def declare_war_on_all():  # pylint: disable=invalid-name
    """Used to declare war on all other empires (at start of game)"""
    my_emp_id = fo.empireID()
    for emp_id in fo.allEmpireIDs():
        if emp_id != my_emp_id:
            msg = fo.diplomaticMessage(my_emp_id, emp_id, fo.diplomaticMessageType.warDeclaration)
            fo.sendDiplomaticMessage(msg)


init_handlers(fo.getOptionsDBOptionStr("ai-config"), fo.getAIDir())
