from common.configure_logging import redirect_logging_to_freeorion_logger

# Logging is redirected before other imports so that import errors appear in log files.
redirect_logging_to_freeorion_logger()

import os.path
import random
import re
import sys

import freeorion as fo

from starnames import name_star_systems
from galaxy import calc_star_system_positions
from starsystems import name_planets, generate_systems
from empires import compile_home_system_list, setup_empire
from fields import generate_fields
from natives import generate_natives
from monsters import generate_monsters
from specials import distribute_specials
from util import int_hash, seed_rng, report_error, error_list
from universe_tables import MAX_JUMPS_BETWEEN_SYSTEMS, MAX_STARLANE_LENGTH
import universe_statistics

from common.handlers import init_handlers
from common.listeners import listener
from common.option_tools import parse_config
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
        self.starting_era = galaxy_setup_data.startingEra
        self.max_ai_aggression = galaxy_setup_data.maxAIAggression
        self.game_uid = galaxy_setup_data.gameUID

    def dump(self):
        print "Galaxy Setup Data:"
        print "...Seed:", self.seed
        print "...Size:", self.size
        print "...Shape:", self.shape
        print "...Age:", self.age
        print "...Starlane Frequency:", self.starlane_frequency
        print "...Planet Density:", self.planet_density
        print "...Specials Frequency:", self.specials_frequency
        print "...Monster Frequency:", self.monster_frequency
        print "...Native Frequency:", self.native_frequency
        print "...StartingEra:", self.starting_era
        print "...Max AI Aggression:", self.max_ai_aggression
        print "...Game UID:", self.game_uid


def error_report():
    """
    Can be called from C++ to retrieve a list of errors that occurred during universe generation
    """
    return error_list


def read_starting_unlocks(starting_era):
    starting_unlocks = {
        "buildings": [],
        "items": [],
        "fleets": [],
    }

    unlock_dir = os.path.join(fo.get_resource_dir(), "scripting",
                              "starting_unlocks")
    if starting_era == fo.startingEra.prewarp:
        buildings_inf = os.path.join(unlock_dir, "buildings-prewarp.inf")
        items_inf = os.path.join(unlock_dir, "items-prewarp.inf")
    else:
        buildings_inf = os.path.join(unlock_dir, "buildings.inf")
        items_inf = os.path.join(unlock_dir, "items.inf")
        fleets_inf = os.path.join(unlock_dir, "fleets.inf")

    comment_expr = re.compile(r'\s*(?://|$)')
    try:
        f = open(buildings_inf)
    except IOError:
        print >> sys.stderr, "Cannot read", buildings_inf
    else:
        expr = re.compile(r'.*Building\s+name\s+=\s+"(.+)"')
        for line in f:
            if comment_expr.match(line):
                continue
            match = expr.match(line)
            if match is None:
                print >> sys.stderr, "buildings.inf parse error:", line
            else:
                starting_unlocks["buildings"].append(match.group(1))
        f.close()

    try:
        f = open(items_inf)
    except IOError:
        print >> sys.stderr, "Cannot read", items_inf
    else:
        expr = re.compile(r'.*Item\s+type\s+=\s+(.+?)\s+name\s+=\s+"(.+)"')
        for line in f:
            if comment_expr.match(line):
                continue
            match = expr.match(line)
            if match is None:
                print >> sys.stderr, "items.inf parse error:", line
            else:
                starting_unlocks["items"].append([match.group(1),
                                                  match.group(2)])
        f.close()

    if starting_era == fo.startingEra.prewarp:
        return starting_unlocks

    try:
        f = open(fleets_inf)
    except IOError:
        print >> sys.stderr, "Cannot read", fleets_inf
    else:
        name_expr = re.compile(r'.*name\s+=\s+"(.+)"')
        ship_expr = re.compile(r'.*?"(.+)"')
        fleet = None
        for line in f:
            match = name_expr.match(line)
            if match is not None:
                if fleet is not None:
                    if len(fleet) > 1:
                        starting_unlocks["fleets"].append(fleet)
                    else:
                        print >> sys.stderr, \
                            "fleets.inf parse error: empty fleet", fleet[0]
                fleet = [match.group(1)]
                continue
            match = ship_expr.match(line)
            if match is None:
                continue
            if fleet is None:
                print >> sys.stderr, \
                    "fleets.inf parse error: ship outside fleet", line
            else:
                fleet.append(match.group(1))
        if fleet is not None:
            if len(fleet) > 1:
                starting_unlocks["fleets"].append(fleet)
            else:
                print >> sys.stderr, \
                    "fleets.inf parse error: empty fleet", fleet[0]
        f.close()
    return starting_unlocks


@listener
def create_universe(psd_map):
    """
    Main universe generation function invoked from C++ code.
    """
    print "Python Universe Generator"

    # fetch universe and player setup data
    gsd = PyGalaxySetupData(fo.get_galaxy_setup_data())
    gsd.dump()
    total_players = len(psd_map)

    # initialize RNG
    h = int_hash(gsd.seed)
    print "Using hashed seed", h
    seed_rng(h)
    seed_pool = [random.random() for _ in range(100)]
    print "Seed pool:", seed_pool

    # make sure there are enough systems for the given number of players
    print "Universe creation requested with %d systems for %d players" % (gsd.size, total_players)
    min_size = total_players * 3
    if min_size > gsd.size:
        gsd.size = min_size
        print "Too few systems for the requested number of players, number of systems adjusted accordingly"
    print "Creating universe with %d systems for %d players" % (gsd.size, total_players)

    # calculate star system positions
    seed_rng(seed_pool.pop())
    system_positions = calc_star_system_positions(gsd)
    size = len(system_positions)
    print gsd.shape, "Star system positions calculated, final number of systems:", size

    # generate and populate systems
    seed_rng(seed_pool.pop())
    systems = generate_systems(system_positions, gsd)
    print len(systems), "systems generated and populated"

    # generate Starlanes
    seed_rng(seed_pool.pop())
    fo.generate_starlanes(MAX_JUMPS_BETWEEN_SYSTEMS[gsd.starlane_frequency], MAX_STARLANE_LENGTH)
    print "Starlanes generated"

    print "Compile list of home systems..."
    seed_rng(seed_pool.pop())
    home_systems = compile_home_system_list(total_players, systems, gsd)
    if not home_systems:
        err_msg = "Python create_universe: couldn't get any home systems, ABORTING!"
        report_error(err_msg)
        raise Exception(err_msg)
    print "Home systems:", home_systems

    # set up empires for each player
    starting_unlocks = read_starting_unlocks(gsd.starting_era)
    seed_rng(seed_pool.pop())
    for empire, psd, home_system in zip(psd_map.keys(), psd_map.values(), home_systems):
        if not setup_empire(empire, psd.empire_name, home_system, psd.starting_species, psd.player_name, starting_unlocks):
            report_error("Python create_universe: couldn't set up empire for player %s" % psd.player_name)

    # assign names to all star systems and their planets
    # this needs to be done after all systems have been generated and empire home systems have been set, as
    # only after all that is finished star types as well as planet sizes and types are fixed, and the naming
    # process depends on that
    print "Assign star system names"
    seed_rng(seed_pool.pop())
    name_star_systems(systems)
    print "Set planet names"
    for system in systems:
        name_planets(system)

    print "Generating stationary fields in systems"
    seed_rng(seed_pool.pop())
    generate_fields(systems)

    print "Generating Natives"
    seed_rng(seed_pool.pop())
    generate_natives(gsd.native_frequency, systems, home_systems)

    print "Generating Space Monsters"
    seed_rng(seed_pool.pop())
    generate_monsters(gsd.monster_frequency, systems)

    print "Distributing Starting Specials"
    seed_rng(seed_pool.pop())
    distribute_specials(gsd.specials_frequency, fo.get_all_objects())

    # finally, write some statistics to the log file
    print "############################################################"
    print "##             Universe generation statistics             ##"
    print "############################################################"
    universe_statistics.log_planet_count_dist(systems)
    print "############################################################"
    universe_statistics.log_planet_type_summary(systems)
    print "############################################################"
    universe_statistics.log_species_summary(gsd.native_frequency)
    print "############################################################"
    universe_statistics.log_monsters_summary(gsd.monster_frequency)
    print "############################################################"
    universe_statistics.log_specials_summary()
    print "############################################################"
    universe_statistics.log_systems()
    universe_statistics.log_planets()

    if error_list:
        print "Python Universe Generator completed with errors"
        return False
    else:
        print "Python Universe Generator completed successfully"
        return True
