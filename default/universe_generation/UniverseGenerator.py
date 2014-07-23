import random

import foUniverseGenerator as fo

from starnames import name_star_systems
from galaxy import calc_star_system_positions
from starsystems import name_planets, generate_systems
from empires import generate_home_system_list, setup_empire
from natives import generate_natives
from monsters import generate_monsters
from specials import distribute_specials
from util import report_error, error_list
from statistics import log_planet_count_dist, log_planet_type_summary, log_species_summary, log_specials_summary


def error_report():
    """
    Can be called from C++ to retrieve a list of errors that occurred during universe generation
    """
    return error_list


def create_universe():
    """
    Main universe generation function invoked from C++ code
    """
    print "Python Universe Generator"

    # fetch universe and player setup data
    gsd = fo.get_galaxy_setup_data()
    psd_list = fo.get_player_setup_data()
    total_players = len(psd_list)

    # initialize RNG
    random.seed(gsd.seed)

    # make sure there are enough systems for the given number of players
    print "Universe creation requested with %d systems for %d players" % (gsd.size, total_players)
    size = max(gsd.size, (total_players * 3))
    if size > gsd.size:
        # gsd.size = size
        print "Too few systems for the requested number of players, number of systems adjusted accordingly"
    print "Creating universe with %d systems for %d players" % (size, total_players)

    # calculate star system positions
    system_positions = calc_star_system_positions(gsd.shape, size)
    size = len(system_positions)
    print gsd.shape, "Star system positions calculated, final number of systems:", size

    # generate and populate systems
    systems = generate_systems(system_positions, gsd)
    print len(systems), "systems generated and populated"

    # generate Starlanes
    fo.generate_starlanes(gsd.starlaneFrequency)
    print "Starlanes generated"

    print "Generate list of home systems..."
    home_systems = generate_home_system_list(total_players, systems)
    if not home_systems:
        err_msg = "Python create_universe: couldn't get any home systems, ABORTING!"
        report_error(err_msg)
        raise Exception(err_msg)
    print "...systems chosen:", home_systems

    # set up empires for each player
    for psd_entry, home_system in zip(psd_list, home_systems):
        empire = psd_entry.key()
        psd = psd_entry.data()
        if not setup_empire(empire, psd.empire_name, home_system, psd.starting_species, psd.player_name):
            report_error("Python create_universe: couldn't set up empire for player %s" % psd.player_name)

    # assign names to all star systems and their planets
    # this needs to be done after all systems have been generated and empire home systems have been set, as
    # only after all that is finished star types as well as planet sizes and types are fixed, and the naming
    # process depends on that
    print "Assign star system names"
    name_star_systems(systems)
    print "Set planet names"
    for system in systems:
        name_planets(system)

    print "Generating Natives"
    generate_natives(gsd.nativeFrequency, systems, home_systems)

    print "Generating Space Monsters"
    generate_monsters(gsd.monsterFrequency, systems)

    print "Distributing Starting Specials"
    distribute_specials(gsd.specialsFrequency, fo.get_all_objects())

    # finally, write some statistics to the log file
    print "############################################################"
    print "##             Universe generation statistics             ##"
    print "############################################################"
    log_planet_count_dist(systems)
    print "############################################################"
    log_planet_type_summary(systems)
    print "############################################################"
    log_species_summary()
    print "############################################################"
    log_specials_summary()
    print "############################################################"
    print "Python Universe Generator completed"

    # return true if no errors occurred, false otherwise
    return len(error_list) == 0