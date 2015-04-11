import os.path
import random

import freeorion as fo

import names
import starsystems
import planets
import util
import statistics


def get_empire_name_generator():
    """
    String generator, return random empire name from string list,
    if string list is empty generate random name.
    """
    empire_names = names.get_name_list("EMPIRE_NAMES")
    random.shuffle(empire_names)
    while True:
        if empire_names:
            yield empire_names.pop()
        else:
            yield names.random_name(5)


# generate names for empires, use next(empire_name_generator) to get next name.
empire_name_generator = get_empire_name_generator()


def get_starting_species_pool():
    """
    Empire species pool generator, return random empire species and ensure somewhat even distribution
    """
    # fill the initial pool with two sets of all playable species
    # this way we have somewhat, but not absolutely strict even distribution of starting species at least when there
    # is only a few number of players (some species can occur twice at max while others not at all)
    pool = fo.get_playable_species() * 2

    # randomize order in initial pool so we don't get the same species all the time
    random.shuffle(pool)
    # generator loop
    while True:
        # if our pool is exhausted (because we have more players than species instances in our initial pool)
        # refill the pool with one set of all playable species
        if not pool:
            pool = fo.get_playable_species()
            # again, randomize order in refilled pool so we don't get the same species all the time
            random.shuffle(pool)
        # pick and return next species, and remove it from our pool
        yield pool.pop()


# generates starting species for empires, use next(starting_species_pool) to get next species
starting_species_pool = get_starting_species_pool()


def find_systems_with_min_jumps_between(num_systems, systems_pool, min_jumps):
    """
    Find requested number of systems out of a pool that are at least a specified number of jumps apart.
    """
    # make several tries to get the requested number of systems
    attempts = min(100, len(systems_pool))
    while attempts > 0:
        attempts -= 1
        # shuffle our pool of systems so each try the candidates are tried in different order
        # (otherwise each try would yield the same result, which would make trying several times kind of pointless...)
        random.shuffle(systems_pool)
        # try to find systems that meet our condition until we either have the requested number
        # or we have tried all systems in our pool
        accepted = []
        for candidate in systems_pool:
            # check if our candidate is at least min_jumps away from all other systems we already found
            if all(fo.jump_distance(candidate, system) >= min_jumps for system in accepted):
                # if yes, add the candidate to the list of accepted systems
                accepted.append(candidate)
                # if we have the requested number of systems, we can stop and return the systems we found
                if len(accepted) >= num_systems:
                    return accepted
    # all tries failed, return an empty list to indicate failure
    return []


def compile_home_system_list(num_home_systems, systems):
    """
    Compiles a list with a requested number of home systems.
    """

    # if the list of systems to choose home systems from is empty, report an error and return empty list
    if not systems:
        util.report_error("Python generate_home_system_list: no systems to choose from")
        return []

    # calculate an initial minimal number of jumps that the home systems should be apart,
    # based on the total number of systems to choose from and the requested number of home systems
    min_jumps = max(int(float(len(systems)) / float(num_home_systems * 2)), 5)
    # try to find the home systems, decrease the min jumps until enough systems can be found, or the min jump distance
    # gets reduced to 0 (meaning we don't have enough systems to choose from at all)
    while min_jumps > 0:
        print "Trying to find", num_home_systems, "home systems that are at least", min_jumps, "jumps apart"
        # try to find home systems...
        home_systems = find_systems_with_min_jumps_between(num_home_systems, systems, min_jumps)
        # ...check if we got enough...
        if len(home_systems) >= num_home_systems:
            # ...yes, we got what we need, so let's break out of the loop
            break
        print "Home system min jump conflict: %d systems and %d empires, tried %d min jump and failed"\
              % (len(systems), num_home_systems, min_jumps)
        # ...no, decrease the min jump distance and try again
        min_jumps -= 1

    # check if the loop above delivered a list with enough home systems, or if it exited because the min jump distance
    # has been decreased to 0 without finding enough systems
    # in that case, our galaxy obviously is too crowded, report an error and return an empty list
    if len(home_systems) < num_home_systems:
        util.report_error("Python generate_home_system_list: requested %d homeworlds in a galaxy with %d systems"
                          % (num_home_systems, len(systems)))
        return []

    # make sure all our home systems have a "real" star (that is, a star that is not a neutron star, black hole,
    # or even no star at all) and at least one planet in it
    for home_system in home_systems:
        # if this home system has no "real" star, change star type to a randomly selected "real" star
        if fo.sys_get_star_type(home_system) not in starsystems.star_types_real:
            star_type = random.choice(starsystems.star_types_real)
            print "Home system", home_system, "has star type", fo.sys_get_star_type(home_system),\
                  ", changing that to", star_type
            fo.sys_set_star_type(home_system, star_type)

        # if this home system has no planets, create one in a random orbit
        # we take random values for type and size, as these will be set to suitable values later
        if not fo.sys_get_planets(home_system):
            print "Home system", home_system, "has no planets, adding one"
            planet = fo.create_planet(random.choice(planets.planet_sizes_real),
                                      random.choice(planets.planet_types_real),
                                      home_system, random.randint(0, fo.sys_get_num_orbits(home_system) - 1), "")
            # if we couldn't create the planet, report an error and return an empty list
            if planet == fo.invalid_object():
                util.report_error("Python generate_home_system_list: couldn't create planet in home system")
                return []

    return home_systems


def setup_empire(empire, empire_name, home_system, starting_species, player_name):
    """
    Sets up various aspects of an empire, like empire name, homeworld, etc.
    """

    # set empire name, if no one is given, pick one randomly
    if not empire_name:
        print "No empire name set for player", player_name, ", picking one randomly"
        empire_name = next(empire_name_generator)
    fo.empire_set_name(empire, empire_name)
    print "Empire name for player", player_name, "is", empire_name

    # check starting species, if no one is given, pick one randomly
    if not starting_species:
        print "No starting species set for player", player_name, ", picking one randomly"
        starting_species = next(starting_species_pool)
    print "Starting species for player", player_name, "is", starting_species
    statistics.empire_species[starting_species] += 1

    # pick a planet from the specified home system as homeworld
    planet_list = fo.sys_get_planets(home_system)
    # if the system is empty, report an error and return false, indicating failure
    if not planet_list:
        util.report_error("Python setup_empire: got home system with no planets")
        return False
    homeworld = random.choice(planet_list)

    # set selected planet as empire homeworld with selected starting species
    fo.empire_set_homeworld(empire, homeworld, starting_species)

    # set homeworld focus
    # check if the preferred focus for the starting species is among
    # the foci available on the homeworld planet
    available_foci = fo.planet_available_foci(homeworld)
    preferred_focus = fo.species_preferred_focus(starting_species)
    if preferred_focus in available_foci:
        # if yes, set the homeworld focus to the preferred focus
        print "Player", player_name, ": setting preferred focus", preferred_focus, "on homeworld"
        fo.planet_set_focus(homeworld, preferred_focus)
    elif len(available_foci) > 0:
        # if no, and there is at least one available focus,
        # just take the first of the list
        if preferred_focus == "":
            print "Player", player_name, ": starting species", starting_species, "has no preferred focus, using",\
                  available_foci[0], "instead"
        else:
            print "Player", player_name, ": preferred focus", preferred_focus, "for starting species",\
                  starting_species, "not available on homeworld, using", available_foci[0], "instead"
        fo.planet_set_focus(homeworld, available_foci[0])
    else:
        # if no focus is available on the homeworld, don't set any focus
        print "Player", player_name, ": no available foci on homeworld for starting species", starting_species

    # give homeworld starting buildings
    # use the list provided in starting_buildings.txt
    print "Player", player_name, ": add starting buildings to homeworld"
    for building in util.load_string_list(os.path.join(fo.get_resource_dir(), "starting_buildings.txt")):
        fo.create_building(building, homeworld, empire)

    # unlock starting techs, buildings, hulls, ship parts, etc.
    # use content file preunlocked_items.txt
    print "Player", player_name, ": add unlocked items"
    for item in fo.load_item_spec_list("preunlocked_items.txt"):
        fo.empire_unlock_item(empire, item.type, item.name)

    # add premade ship designs to empire
    print "Player", player_name, ": add premade ship designs"
    for ship_design in fo.design_get_premade_list():
        fo.empire_add_ship_design(empire, ship_design)

    # add starting fleets to empire
    # use content file starting_fleets.txt
    print "Player", player_name, ": add starting fleets"
    fleet_plans = fo.load_fleet_plan_list("starting_fleets.txt")
    for fleet_plan in fleet_plans:
        # first, create the fleet
        fleet = fo.create_fleet(fleet_plan.name(), home_system, empire)
        # if the fleet couldn't be created, report an error and try to continue with the next fleet plan
        if fleet == fo.invalid_object():
            util.report_error("Python setup empire: couldn't create fleet %s" % fleet_plan.name())
            continue
        # second, iterate over the list of ship design names in the fleet plan
        for ship_design in fleet_plan.ship_designs():
            # create a ship in the fleet
            # if the ship couldn't be created, report an error and try to continue with the next ship design
            if fo.create_ship("", ship_design, starting_species, fleet) == fo.invalid_object():
                util.report_error("Python setup empire: couldn't create ship %s for fleet %s"
                                  % (ship_design, fleet_plan.name()))
    return True
