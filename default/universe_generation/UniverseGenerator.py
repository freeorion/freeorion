import random

import foUniverseGenerator as fo

import starnames
import galaxy
import starsystems
import empires
import natives
import monsters
import specials
import statistics


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
    size = max(gsd.size, (total_players * 3))
    if size > gsd.size:
        # gsd.size = size
        print "Too few systems for the requested number of players, number of systems adjusted accordingly"
    print "Creating universe with %d systems for %d players" % (size, total_players)

    # calculate star system positions
    system_positions = galaxy.calc_star_system_positions(gsd.shape, size)
    size = len(system_positions)
    print gsd.shape, "Star system positions calculated, final number of systems:", size

    # generate and populate systems
    systems = starsystems.generate_systems(system_positions, gsd)
    print len(systems), "systems generated and populated"

    # generate Starlanes
    fo.generate_starlanes(gsd.starlaneFrequency)
    print "Starlanes generated"

    print "Generate list of home systems..."
    home_systems = empires.generate_home_system_list(total_players, systems)
    print "...systems chosen:", home_systems

    # set up empires for each player
    for psd_entry, home_system in zip(psd_list, home_systems):
        empire = psd_entry.key()
        psd = psd_entry.data()
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

    print "Generating Natives"
    natives.generate_natives(gsd.nativeFrequency, systems, home_systems)

    print "Generating Space Monsters"
    monsters.generate_monsters(gsd.monsterFrequency, systems)

    print "Distributing Starting Specials"
    specials.distribute_specials(gsd.specialsFrequency, fo.get_all_objects())

    # finally, write some statistics to the log file
    print "##############################"
    print "Universe generation statistics"
    print "##############################"
    statistics.log_planet_count_dist(systems)
    statistics.log_planet_type_summary(systems)
    statistics.log_species_summary()
    statistics.log_specials_summary()
    print "Python Universe Generator completed"
