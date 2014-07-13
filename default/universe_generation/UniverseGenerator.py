import random

import foUniverseGenerator as fo

import starnames
import galaxy
import starsystems
import empires
import statistics


def adjust_universe_size(size, total_players):
    """
    This function checks if there are enough systems to give all
    players adequately-separated homeworlds, and increases the
    number of systems accordingly if not
    """
    min_sys = total_players * 3
    if size < min_sys:
        return min_sys
    else:
        return size


def create_universe():
    print "Python Universe Generator"

    # fetch universe and player setup data
    gsd = fo.get_galaxy_setup_data()
    psd_list = fo.get_player_setup_data()
    total_players = len(psd_list)

    # initialize RNG
    random.seed(gsd.seed)

    # make sure there are enough systems for the given number of players
    print "Universe creation requested with %d systems for %d players" % (gsd.size, total_players)
    new_size = adjust_universe_size(gsd.size, total_players)
    if new_size > gsd.size:
        gsd.size = new_size
        print "Too few systems for the requested number of players, number of systems adjusted accordingly"
    print "Creating universe with %d systems for %d players" % (gsd.size, total_players)

    # get typical width for universe based on number of systems
    width = fo.calc_typical_universe_width(gsd.size)
    fo.set_universe_width(width)
    print "Set universe width to", width

    # calculate star system positions
    system_positions = galaxy.calc_star_system_positions(gsd.shape, gsd.size, width)
    gsd.size = len(system_positions)
    print gsd.shape, "Star system positions calculated, final number of systems:", gsd.size

    # generate and populate systems
    systems = starsystems.generate_systems(system_positions, gsd)
    print len(systems), "systems generated and populated"

    # generate Starlanes
    fo.generate_starlanes(gsd.starlaneFrequency)
    print "Starlanes generated"

    print "Generate list of home systems..."
    home_systems = empires.generate_home_system_list(total_players, systems)
    print "...systems choosen:", home_systems

    # set up empires for each player
    for psd_entry in psd_list:
        empire = psd_entry.key()
        psd = psd_entry.data()
        home_system = home_systems.pop()
        empires.setup_empire(empire, psd.empire_name, home_system, psd.starting_species, psd.player_name)

    # assign names to all star systems and their planets
    # this needs to be done after all systems have been generated and empire home systems have been set, as
    # only after all that is finished star types as well as planet sizes and types are fixed, and the naming
    # process depends on that
    print "Assign star system names"
    starnames.name_star_systems(systems)
    print "Set planet names"
    for system in systems:
        starsystems.name_planets(system)

    # finally, write some statistics to the log file
    print
    print "##############################"
    print "Universe generation statistics"
    print "##############################"
    print
    statistics.log_planet_count_dist(systems)
    print
    statistics.log_planet_type_summary(systems)
    print
    print "Python Universe Generator completed"
