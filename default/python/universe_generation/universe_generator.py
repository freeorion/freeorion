from common.configure_logging import redirect_logging_to_freeorion_logger

# Logging is redirected before other imports so that import errors appear in log files.
redirect_logging_to_freeorion_logger()

import freeorion as fo
import random

import universe_statistics
from common.handlers import init_handlers
from common.listeners import listener
from common.option_tools import parse_config
from empires import compile_home_system_list, setup_empire
from fields import generate_fields
from galaxy import calc_star_system_positions
from monsters import generate_monsters
from natives import generate_natives
from specials import distribute_specials
from starnames import name_star_systems
from starsystems import generate_systems, name_planets
from teams import place_teams
from universe_tables import MAX_JUMPS_BETWEEN_SYSTEMS, MAX_STARLANE_LENGTH
from util import error_list, int_hash, report_error, seed_rng

parse_config(fo.get_options_db_option_str("ai-config"), fo.get_user_config_dir())
init_handlers(fo.get_options_db_option_str("ai-config"), None)


class PyGalaxySetupData:
    """
    Class used to store and manage a copy of the galaxy setup data provided by the FreeOrion interface.
    This data can then be modified when needed during the universe generation process, without having to
    change the original data structure.
    """

    def __init__(self, galaxy_setup_data):
        self.seed = galaxy_setup_data.seed
        self.size = galaxy_setup_data.size
        self.shape = galaxy_setup_data.shape
        self.age = galaxy_setup_data.age
        self.starlane_frequency = galaxy_setup_data.starlaneFrequency
        self.planet_density = galaxy_setup_data.planetDensity
        self.specials_frequency = galaxy_setup_data.specialsFrequency
        self.monster_frequency = galaxy_setup_data.monsterFrequency
        self.native_frequency = galaxy_setup_data.nativeFrequency
        self.max_ai_aggression = galaxy_setup_data.maxAIAggression
        self.game_uid = galaxy_setup_data.gameUID

    def dump(self):
        print("Galaxy Setup Data:")
        print("...Seed:", self.seed)
        print("...Size:", self.size)
        print("...Shape:", self.shape)
        print("...Age:", self.age)
        print("...Starlane Frequency:", self.starlane_frequency)
        print("...Planet Density:", self.planet_density)
        print("...Specials Frequency:", self.specials_frequency)
        print("...Monster Frequency:", self.monster_frequency)
        print("...Native Frequency:", self.native_frequency)
        print("...Max AI Aggression:", self.max_ai_aggression)
        print("...Game UID:", self.game_uid)


def error_report():
    """
    Can be called from C++ to retrieve a list of errors that occurred during universe generation
    """
    return error_list


@listener
def create_universe(psd_map):  # noqa: C901
    """
    Main universe generation function invoked from C++ code.
    """
    print("Python Universe Generator")

    # fetch universe and player setup data
    gsd = PyGalaxySetupData(fo.get_galaxy_setup_data())
    gsd.dump()
    total_players = len(psd_map)

    # initialize RNG
    h = int_hash(gsd.seed.encode("utf-8"))
    print("Using hashed seed", h)
    seed_rng(h)
    seed_pool = [random.random() for _ in range(100)]
    print("Seed pool:", seed_pool)

    # make sure there are enough systems for the given number of players
    print("Universe creation requested with %d systems for %d players" % (gsd.size, total_players))
    min_size = total_players * 3
    if min_size > gsd.size:
        gsd.size = min_size
        print("Too few systems for the requested number of players, number of systems adjusted accordingly")
    print("Creating universe with %d systems for %d players" % (gsd.size, total_players))

    # calculate star system positions
    seed_rng(seed_pool.pop())
    system_positions = calc_star_system_positions(gsd)
    size = len(system_positions)
    print(gsd.shape, "Star system positions calculated, final number of systems:", size)

    # generate and populate systems
    seed_rng(seed_pool.pop())
    systems = generate_systems(system_positions, gsd)
    print(len(systems), "systems generated and populated")

    # generate Starlanes
    seed_rng(seed_pool.pop())
    fo.generate_starlanes(MAX_JUMPS_BETWEEN_SYSTEMS[gsd.starlane_frequency], MAX_STARLANE_LENGTH)
    print("Starlanes generated")

    print("Compile list of home systems...")
    seed_rng(seed_pool.pop())
    home_systems = compile_home_system_list(total_players, systems, gsd)
    if not home_systems:
        err_msg = "Python create_universe: couldn't get any home systems, ABORTING!"
        report_error(err_msg)
        raise Exception(err_msg)
    print("Home systems:", home_systems)

    teams = {}
    for psd in psd_map.values():
        if psd.starting_team >= 0:
            teams[psd.starting_team] = teams.get(psd.starting_team, 0) + 1
    teams = {k: v for k, v in teams.items() if v > 1}
    print("Teams: ", teams)

    seed_rng(seed_pool.pop())
    if teams:
        psds = list(psd_map.items())
        hs = list(home_systems)
        for home_system, team in place_teams(hs, systems, teams):
            hs.remove(home_system)
            psds_new = list()
            placed = False
            for empire, psd in psds:
                if placed or psd.starting_team != team:
                    psds_new.append((empire, psd))
                else:
                    placed = True
                    if not setup_empire(
                        empire, psd.empire_name, home_system, psd.starting_species, psd.player_name, gsd
                    ):
                        report_error("Python create_universe: couldn't set up empire for player %s" % psd.player_name)
            if not placed:
                report_error("Python create_universe: couldn't set up empire for team %d" % team)
            psds = psds_new
        # place leftovers
        for (empire, psd), home_system in zip(psds, hs):
            if not setup_empire(empire, psd.empire_name, home_system, psd.starting_species, psd.player_name, gsd):
                report_error("Python create_universe: couldn't set up empire for player %s" % psd.player_name)
    else:
        # set up empires for each player
        for empire, psd, home_system in zip(psd_map.keys(), psd_map.values(), home_systems):
            if not setup_empire(empire, psd.empire_name, home_system, psd.starting_species, psd.player_name, gsd):
                report_error("Python create_universe: couldn't set up empire for player %s" % psd.player_name)

    # assign names to all star systems and their planets
    # this needs to be done after all systems have been generated and empire home systems have been set, as
    # only after all that is finished star types as well as planet sizes and types are fixed, and the naming
    # process depends on that
    print("Assign star system names")
    seed_rng(seed_pool.pop())
    name_star_systems(systems)
    print("Set planet names")
    for system in systems:
        name_planets(system)

    print("Generating stationary fields in systems")
    seed_rng(seed_pool.pop())
    generate_fields(systems)

    print("Generating Natives")
    seed_rng(seed_pool.pop())
    generate_natives(gsd.native_frequency, systems, home_systems)

    print("Generating Space Monsters")
    seed_rng(seed_pool.pop())
    generate_monsters(gsd.monster_frequency, systems)

    print("Distributing Starting Specials")
    seed_rng(seed_pool.pop())
    distribute_specials(gsd.specials_frequency, fo.get_all_objects())

    # finally, write some statistics to the log file
    print("############################################################")
    print("##             Universe generation statistics             ##")
    print("############################################################")
    universe_statistics.log_planet_count_dist(systems)
    print("############################################################")
    universe_statistics.log_planet_type_summary(systems)
    print("############################################################")
    universe_statistics.log_species_summary(gsd.native_frequency)
    print("############################################################")
    universe_statistics.log_monsters_summary(gsd.monster_frequency)
    print("############################################################")
    universe_statistics.log_specials_summary()
    print("############################################################")
    universe_statistics.log_systems()
    universe_statistics.log_planets()

    if error_list:
        print("Python Universe Generator completed with errors")
        return False
    else:
        print("Python Universe Generator completed successfully")
        return True
