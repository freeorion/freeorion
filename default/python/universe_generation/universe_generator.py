import random

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
import statistics


# tuples of galaxy setup options to pick from randomly
low_to_high =  (fo.galaxySetupOption.low, fo.galaxySetupOption.medium, fo.galaxySetupOption.high)
none_to_high = (fo.galaxySetupOption.low, fo.galaxySetupOption.medium, fo.galaxySetupOption.high, fo.galaxySetupOption.none)

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
        self.__age = galaxy_setup_data.age
        self.__starlane_frequency = galaxy_setup_data.starlaneFrequency
        self.__planet_density = galaxy_setup_data.planetDensity
        self.__specials_frequency = galaxy_setup_data.specialsFrequency
        self.__monster_frequency = galaxy_setup_data.monsterFrequency
        self.__native_frequency = galaxy_setup_data.nativeFrequency
        self.max_ai_aggression = galaxy_setup_data.maxAIAggression
    
    def age(self):
        #print("stored age: " + str(self.__age))
        if (self.__age != fo.galaxySetupOption.random):
            return self.__age
        return random.choice(low_to_high)

    def starlaneFrequency(self):
        #print("stored starlaneFrequency: " + str(self.__starlane_frequency))
        if (self.__starlane_frequency != fo.galaxySetupOption.random):
            return self.__starlane_frequency
        return random.choice(low_to_high)

    def planetDensity(self):
        #print("stored planetDensity: " + str(self.__planet_density))
        if (self.__planet_density != fo.galaxySetupOption.random):
            return self.__planet_density
        return random.choice(low_to_high)

    def specialsFrequency(self):
        #print("stored specialsFrequency: " + str(self.__specials_frequency))
        if (self.__specials_frequency != fo.galaxySetupOption.random):
            return self.__specials_frequency
        return random.choice(none_to_high)

    def monsterFrequency(self):
        #print("stored monsterFrequency: " + str(self.__monster_frequency))
        if (self.__monster_frequency != fo.galaxySetupOption.random):
            return self.__monster_frequency
        return random.choice(none_to_high)

    def nativeFrequency(self):
        #print("stored nativeFrequency: " + str(self.__native_frequency))
        if (self.__native_frequency != fo.galaxySetupOption.random):
            return self.__native_frequency
        return random.choice(none_to_high)


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
    gsd = PyGalaxySetupData(fo.get_galaxy_setup_data())
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
    fo.generate_starlanes(MAX_JUMPS_BETWEEN_SYSTEMS[gsd.starlaneFrequency()], MAX_STARLANE_LENGTH)
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

    print "Generating stationary fields in systems"
    seed_rng(seed_pool.pop())
    generate_fields(systems)

    print "Generating Natives"
    seed_rng(seed_pool.pop())
    generate_natives(gsd.nativeFrequency(), systems, home_systems)

    print "Generating Space Monsters"
    seed_rng(seed_pool.pop())
    generate_monsters(gsd.monsterFrequency(), systems)

    print "Distributing Starting Specials"
    seed_rng(seed_pool.pop())
    distribute_specials(gsd.specialsFrequency(), fo.get_all_objects())

    # finally, write some statistics to the log file
    print "############################################################"
    print "##             Universe generation statistics             ##"
    print "############################################################"
    statistics.log_planet_count_dist(systems)
    print "############################################################"
    statistics.log_planet_type_summary(systems)
    print "############################################################"
    statistics.log_species_summary(gsd.nativeFrequency())
    print "############################################################"
    statistics.log_monsters_summary(gsd.monsterFrequency())
    print "############################################################"
    statistics.log_specials_summary()
    print "############################################################"

    if error_list:
        print "Python Universe Generator completed with errors"
        return False
    else:
        print "Python Universe Generator completed successfully"
        return True
