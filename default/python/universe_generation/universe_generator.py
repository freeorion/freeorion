import random

import freeorion as fo

from starnames import name_star_systems
from galaxy import calc_star_system_positions
from starsystems import name_planets, generate_systems
from empires import compile_home_system_list, setup_empire
from natives import generate_natives
from monsters import generate_monsters
from specials import distribute_specials
from util import seed_rng, report_error, error_list
import statistics


def error_report():
    """
    Can be called from C++ to retrieve a list of errors that occurred during universe generation
    """
    return error_list


def create_universe(psd_map):
    """
    Main universe generation function invoked from C++ code.
    """
    print "Python Universe Generator"

    # fetch universe and player setup data
    gsd = fo.get_galaxy_setup_data()
    total_players = len(psd_map)

    # initialize RNG
    seed_rng(gsd.seed)
    seed_pool = [random.random() for _ in range(100)]

    # make sure there are enough systems for the given number of players
    print "Universe creation requested with %d systems for %d players" % (gsd.size, total_players)
    size = max(gsd.size, (total_players * 3))
    if size > gsd.size:
        # gsd.size = size
        print "Too few systems for the requested number of players, number of systems adjusted accordingly"
    print "Creating universe with %d systems for %d players" % (size, total_players)

    # calculate star system positions
    seed_rng(seed_pool.pop())
    system_positions = calc_star_system_positions(gsd.shape, size)
    size = len(system_positions)
    print gsd.shape, "Star system positions calculated, final number of systems:", size

    # generate and populate systems
    seed_rng(seed_pool.pop())
    systems = generate_systems(system_positions, gsd)
    print len(systems), "systems generated and populated"

    # generate Starlanes
    seed_rng(seed_pool.pop())
    fo.generate_starlanes(gsd.starlaneFrequency)
    print "Starlanes generated"

    print "Compile list of home systems..."
    seed_rng(seed_pool.pop())
    home_systems = compile_home_system_list(total_players, systems)
    if not home_systems:
        err_msg = "Python create_universe: couldn't get any home systems, ABORTING!"
        report_error(err_msg)
        raise Exception(err_msg)
    print "Home systems:", home_systems

    # set up empires for each player
    seed_rng(seed_pool.pop())
    for empire, psd, home_system in zip(psd_map.keys(), psd_map.values(), home_systems):
        if not setup_empire(empire, psd.empire_name, home_system, psd.starting_species, psd.player_name):
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

    print "Generating Natives"
    seed_rng(seed_pool.pop())
    generate_natives(gsd.nativeFrequency, systems, home_systems)

    print "Generating Space Monsters"
    seed_rng(seed_pool.pop())
    generate_monsters(gsd.monsterFrequency, systems)

    print "Distributing Starting Specials"
    seed_rng(seed_pool.pop())
    distribute_specials(gsd.specialsFrequency, fo.get_all_objects())

    # finally, write some statistics to the log file
    print "############################################################"
    print "##             Universe generation statistics             ##"
    print "############################################################"
    statistics.log_planet_count_dist(systems)
    print "############################################################"
    statistics.log_planet_type_summary(systems)
    print "############################################################"
    statistics.log_species_summary()
    print "############################################################"
    statistics.log_monsters_summary()
    print "############################################################"
    statistics.log_specials_summary()
    print "############################################################"

    if error_list:
        print "Python Universe Generator completed with errors"
        return False
    else:
        print "Python Universe Generator completed successfully"
        return True
