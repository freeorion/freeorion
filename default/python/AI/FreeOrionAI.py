"""The FreeOrionAI module contains the methods which can be made by the C game client;
these methods in turn activate other portions of the python AI code."""
from functools import wraps
from logging import debug, error, fatal, info

from common.configure_logging import redirect_logging_to_freeorion_logger

# Logging is redirected before other imports so that import errors appear in log files.
redirect_logging_to_freeorion_logger()

import freeOrionAIInterface as fo
import random
import sys

from common.option_tools import parse_config

parse_config(fo.getOptionsDBOptionStr("ai-config"), fo.getUserConfigDir())

from freeorion_tools.fo_chat_handler import configure_debug_chat, process_chat_message
from freeorion_tools.patch_interface import patch_interface

patch_interface()

import ColonisationAI
import DiplomaticCorp
import ExplorationAI
import FleetUtilsAI
import InvasionAI
import MilitaryAI
import PlanetUtilsAI
import PolicyAI
import PriorityAI
import ProductionAI
import ResearchAI
import ResourcesAI
import ShipDesignAI
import TechsListsAI
from AIDependencies import INVALID_ID
from aistate_interface import create_new_aistate, get_aistate, load_aistate
from character.character_module import Aggression
from character.character_strings_module import (
    get_trait_name_aggression,
    possible_capitals,
)
from common.handlers import init_handlers
from common.listeners import listener
from empire.survey_universe import survey_universe
from expansion_plans.expansion_plans_implementation import initialise_expansion_plans
from freeorion_tools import chat_human
from freeorion_tools.timers import AITimer
from generate_orders import (
    empire_is_ok,
    greet_on_first_turn,
    print_existing_rules,
    print_starting_intro,
    replay_turn_after_load,
    set_game_turn_seed,
    update_resource_pool,
)

initialise_expansion_plans()

turn_timer = AITimer("full turn")

user_dir = fo.getUserDataDir()
debug("Path to folder for user specific data: %s" % user_dir)
debug("Python paths %s" % sys.path)


diplomatic_corp = None


def error_handler(func):
    """Decorator that logs any exception in decorated function, then re-raises"""

    @wraps(func)
    def _error_handler(*args, **kwargs):
        try:
            res = func(*args, **kwargs)
            return res
        except Exception as e:
            error("Exception %s occurred during %s", e, func.__name__, exc_info=True)
            raise

    return _error_handler


def _pre_game_start(empire_id):
    """
    Configuration that should be done before AI start operating.
    """
    aistate = get_aistate()
    aggression_trait = aistate.character.get_trait(Aggression)
    diplomatic_corp_configs = {
        fo.aggression.beginner: DiplomaticCorp.BeginnerDiplomaticCorp,
        fo.aggression.maniacal: DiplomaticCorp.ManiacalDiplomaticCorp,
    }
    global diplomatic_corp
    diplomatic_corp = diplomatic_corp_configs.get(aggression_trait.key, DiplomaticCorp.DiplomaticCorp)()
    TechsListsAI.test_tech_integrity()
    configure_debug_chat(empire_id)


def _choose_aggression():
    galaxy_setup_data = fo.getGalaxySetupData()
    aggression = int(galaxy_setup_data.maxAIAggression)

    if aggression > 0:
        rng = random.Random()
        galaxy_seed = hash(galaxy_setup_data.seed)
        empire_seed = 3 * hash(fo.getEmpire().name)
        rng.seed(galaxy_seed * empire_seed)

        random_number = rng.randint(0, 99)
        if random_number > 74:
            aggression -= 1

    return fo.aggression(aggression)


@error_handler
def startNewGame():  # pylint: disable=invalid-name
    """Called by client when a new game is started (but not when a game is loaded).
    Should clear any pre-existing state and set up whatever is needed for AI to generate orders."""
    empire = fo.getEmpire()
    if empire is None:
        fatal("This client has no empire. Ignoring new game start message.")
        return

    if empire.eliminated:
        info("This empire has been eliminated. Ignoring new game start message.")
        return
    # initialize AIstate
    debug("Initializing AI state...")
    aistate = create_new_aistate(_choose_aggression())
    aggression_trait = aistate.character.get_trait(Aggression)
    debug(
        "New game started, AI Aggression level %d (%s)"
        % (aggression_trait.key, get_trait_name_aggression(aistate.character))
    )
    aistate.session_start_cleanup()
    debug("Initialization of AI state complete!")
    debug("Trying to rename our homeworld...")
    planet_id = PlanetUtilsAI.get_capital()
    universe = fo.getUniverse()
    if planet_id != INVALID_ID:
        planet = universe.getPlanet(planet_id)
        new_name = " ".join([random.choice(possible_capitals(aistate.character)).strip(), planet.name])
        debug("    Renaming to %s..." % new_name)
        res = fo.issueRenameOrder(planet_id, new_name)
        debug("    Result: %d; Planet is now named %s" % (res, planet.name))
    _pre_game_start(empire.empireID)


@error_handler
def resumeLoadedGame(saved_state_string):  # pylint: disable=invalid-name
    """Called by client to when resume a loaded game."""
    debug("Resuming loaded game")

    if saved_state_string == "NO_STATE_YET" and fo.currentTurn() == 1:
        info("AI given uninitialized state-string to resume from on turn 1.")
        info(
            "Assuming post-universe-generation autosave before any orders were sent "
            "and behaving as if a new game was started."
        )
        return startNewGame()

    if fo.getEmpire() is None:
        fatal("This client has no empire. Doing nothing to resume loaded game.")
        return

    if fo.getEmpire().eliminated:
        info("This empire has been eliminated. Ignoring resume loaded game.")
        return

    aistate = None
    if saved_state_string == "NOT_SET_BY_CLIENT_TYPE":
        info("AI assigned to empire previously run by human.")
        chat_human("We have been assigned an empire previously run by a human player. We can manage this.")
    elif saved_state_string == "":
        error(
            "AI given empty state-string to resume from. "
            "AI can continue but behaviour may be different from the previous session."
        )
    else:
        try:
            # loading saved state
            aistate = load_aistate(saved_state_string)
        except Exception as e:
            error(
                "Failed to load the AIstate from the savegame: %s"
                " AI can continue but behaviour may be different from the previous session.",
                e,
                exc_info=True,
            )
    if aistate is None:
        info("Creating new ai state due to failed load.")
        aistate = create_new_aistate(_choose_aggression())
    aistate.session_start_cleanup()
    _pre_game_start(fo.getEmpire().empireID)


@error_handler
def prepareForSave():  # pylint: disable=invalid-name
    """Called by client when the game is about to be saved, to let the Python AI know it should save any AI state
    information, such as plans or knowledge about the game from previous turns,
    in the state string so that they can be restored if the game is loaded."""
    empire = fo.getEmpire()
    if empire is None:
        fatal("This client has no empire. Doing nothing to prepare for save.")
        return

    if empire.eliminated:
        info("This empire has been eliminated. Save info request")
        return

    info("Preparing for game save by serializing state")

    # serialize (convert to string) global state dictionary and send to AI client to be stored in save file
    import savegame_codec

    try:
        dump_string = savegame_codec.build_savegame_string()
        fo.setSaveStateString(dump_string)
    except Exception as e:
        error(
            "Failed to encode the AIstate as save-state string. "
            "The resulting save file should be playable but the AI "
            "may have a different aggression. The error raised was: %s" % e,
            exc_info=True,
        )


@error_handler
def handleChatMessage(sender_id, message_text):  # pylint: disable=invalid-name
    """Called when this player receives a chat message. sender_id is the player who sent the message, and
    message_text is the text of the sent message."""
    empire = fo.getEmpire()
    if empire is None:
        fatal("This client has no empire. Doing nothing to handle chat message.")
        return

    if empire.eliminated:
        debug("This empire has been eliminated. Ignoring chat message")
        return
    process_chat_message(sender_id, message_text, diplomatic_corp)


@error_handler
def handleDiplomaticMessage(message):  # pylint: disable=invalid-name
    """Called when this player receives a diplomatic message update from the server,
    such as if another player declares war, accepts peace, or cancels a proposed peace treaty."""
    empire = fo.getEmpire()
    if empire is None:
        fatal("This client has no empire. Doing nothing to handle diplomatic message.")
        return

    if empire.eliminated:
        debug("This empire has been eliminated. Ignoring diplomatic message")
        return

    diplomatic_corp.handle_diplomatic_message(message)


@error_handler
def handleDiplomaticStatusUpdate(status_update):  # pylint: disable=invalid-name
    """Called when this player receives an update about the diplomatic status between players, which may
    or may not include this player."""
    empire = fo.getEmpire()
    if empire is None:
        fatal("This client has no empire. Doing nothing to handle diplomatic status message.")
        return

    if empire.eliminated:
        debug("This empire has been eliminated. Ignoring diplomatic status update")
        return

    diplomatic_corp.handle_diplomatic_status_update(status_update)


generate_order_timer = AITimer("generate orders")


@error_handler
@listener
def generateOrders():  # pylint: disable=invalid-name
    """
    Called once per turn to tell the Python AI to generate
    and issue orders, i.e. to control its empire.

    After leaving this function, the AI's turn will be finished
    and its orders will be sent to the server.
    """
    turn_timer.start("AI planning")
    print_existing_rules()

    if not empire_is_ok():
        return

    DiplomaticCorp.check_gang_up()

    set_game_turn_seed()

    generate_order_timer.start("Update states on server")
    # This code block is required for correct AI work.
    update_resource_pool()

    generate_order_timer.start("Prepare each turn data")
    # results of this function are needed in many places...

    greet_on_first_turn(diplomatic_corp)

    if replay_turn_after_load():
        return

    survey_universe()

    print_starting_intro()

    aistate = get_aistate()
    aistate.prepare_for_new_turn()
    debug("Calling AI Modules")
    # call AI modules
    action_list = [
        ShipDesignAI.Cache.update_for_new_turn,
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
        PolicyAI.generate_policy_orders,
    ]

    for action in action_list:
        try:
            generate_order_timer.start(action.__name__)
            action()
            generate_order_timer.stop()
        except Exception as e:
            error(f"Exception {e} while trying to {action.__name__}", exc_info=True)

    aistate.last_turn_played = fo.currentTurn()
    generate_order_timer.stop_print_and_clear()
    turn_timer.stop_print_and_clear()
    turn_timer.start("Time between AI turn")


init_handlers(fo.getOptionsDBOptionStr("ai-config"), fo.getAIDir())
