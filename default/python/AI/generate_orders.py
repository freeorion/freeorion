"""
Various actions for the order generation.
"""
import freeOrionAIInterface as fo
import random
from functools import wraps
from logging import debug, error, fatal, info

import PlanetUtilsAI
from AIDependencies import INVALID_ID
from aistate_interface import get_aistate

# This import initializes the aggresion module and we need to import it to make module work
from character.character_module import Aggression  # noqa F401
from character.character_strings_module import get_trait_name_aggression
from common.option_tools import check_bool, get_option_dict
from freeorion_tools.statistics import stats


def _once(function):
    """
    Run once per runtime.
    """
    executed = False

    @wraps(function)
    def wrapper():
        nonlocal executed
        if executed:
            return
        else:
            executed = True
            function()

    return wrapper


@_once
def print_existing_rules():
    try:
        rules = fo.getGameRules()
        debug("Defined game rules:")
        for rule_name, rule_value in rules.getRulesAsStrings().items():
            debug("%s: %s", rule_name, rule_value)
        debug("Rule RULE_NUM_COMBAT_ROUNDS value: " + str(rules.getInt("RULE_NUM_COMBAT_ROUNDS")))
    except Exception as e:
        error("Exception %s when trying to get game rules" % e, exc_info=True)


def empire_is_ok():
    """
    If nothing can be ordered anyway, exit early.
    Note that there is no need to update meters etc. in this case.

    Return False if we should stop order generation.
    """
    empire = fo.getEmpire()
    if empire is None:
        fatal("This client has no empire. Aborting order generation.")
        return False
    elif empire.eliminated:
        info("This empire has been eliminated. Aborting order generation.")
        return False
    return True


def update_resource_pool():
    info("Meter / Resource Pool updating...")
    fo.initMeterEstimatesDiscrepancies()
    fo.updateMeterEstimates(False)
    fo.updateResourcePools()


def set_game_turn_seed():
    """
    Set the random seed for game-reload consistency.

    Seed is based on galaxy seed, empire name and current turn.
    """
    turn = fo.currentTurn()
    random_seed = str(fo.getGalaxySetupData().seed) + "%05d%s" % (turn, fo.getEmpire().name)
    random.seed(random_seed)


def _dump_empire_info():
    empire = fo.getEmpire()
    turn = fo.currentTurn()

    # TODO: It would be nice to decouple char and AI state.
    #       Character is immutable and ai state is mutable.
    #       Dependency on immutable object is easier to manage
    research_index = get_aistate().character.get_research_index()
    aggression_name = get_trait_name_aggression(get_aistate().character)

    name_parts = (
        empire.name,
        empire.empireID,
        "pid",
        fo.playerID(),
        fo.playerName(),
        "RIdx",
        research_index,
        aggression_name.capitalize(),
    )
    empire_name = "_".join(str(part) for part in name_parts)
    stats.empire(empire.empireID, empire_name, turn)
    stats.empire_color(*empire.colour)


def greet_on_first_turn(diplomatic_corp):
    if fo.currentTurn() != 1:
        return

    human_player = fo.empirePlayerID(1)
    greet = diplomatic_corp.get_first_turn_greet_message()
    trait_name = get_trait_name_aggression(get_aistate().character)
    fo.sendChatMessage(human_player, f"{fo.getEmpire().name} ({trait_name}): [[{greet}]]")


def replay_turn_after_load():
    """
    When loading a savegame, the AI will already have issued orders for this turn.
    To avoid duplicate orders, generally try not to replay turns. However, for debugging
    purposes it is often useful to replay the turn and observe varying results after
    code changes. Set the replay_after_load flag in the AI config to let the AI issue
    new orders after a game load. Note that the orders from the original savegame are
    still being issued and the AIstate was saved after those orders were issued.

    Return True is we should abort order generation.
    """
    replay_option_is_set = check_bool(get_option_dict().get("replay_turn_after_load", "False"))
    turn_is_already_played = fo.currentTurn() == get_aistate().last_turn_played

    # TODO: Consider adding an option to clear AI orders after load (must save AIstate at turn start then)
    if turn_is_already_played:
        info("The AIstate indicates that this turn was already played.")
        if replay_option_is_set:
            info("Issuing new orders anyway.")
            return False
        else:
            info("Aborting new order generation. Orders from savegame will still be issued.")
            return True
    else:
        return False


def print_starting_intro():
    debug("\n\n\n" + "=" * 20)
    debug(f"Starting turn {fo.currentTurn()}")
    debug("=" * 20 + "\n")

    debug("***************************************************************************")
    debug("*******  Log info for AI progress chart script. Do not modify.   **********")
    debug("Generating Orders")
    _dump_empire_info()
    _print_empire_capital()
    stats.ship_count(get_aistate().shipCount)


def _print_empire_capital():
    planet_id = PlanetUtilsAI.get_capital()
    if planet_id is not None and planet_id != INVALID_ID:
        planet = fo.getUniverse().getPlanet(planet_id)
        stats.capital(planet_id, planet.name, planet.speciesName)
    else:
        stats.capital(None, None, None)
    debug("***************************************************************************")
    debug("***************************************************************************")
