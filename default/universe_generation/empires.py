import random

import foUniverseGenerator as fo

import names
import starsystems
import planets
import util


def get_empire_name_generator():
    """
    String generator, return random empire name from string list,
    if string list is empty generate random name.
    """
    empire_names = names.get_name_list("EMPIRE_NAMES")
    random.shuffle(empire_names)
    while True:
        if names:
            yield empire_names.pop()
        else:
            yield names.random_name(5)


# generate names for empires, use next(empire_name_generator) to get next name.
empire_name_generator = get_empire_name_generator()


def is_too_close_to_other_home_systems(system, home_systems):
    """
    Checks if a system is too close to the other home systems
    Home systems should be at least 200 units (linear distance)
    and 2 jumps apart
    """
    for home_system in home_systems:
        if fo.linear_distance(system, home_system) < 200:
            return True
        elif fo.jump_distance(system, home_system) < 2:
            return True
    return False


def generate_home_system_list(num_home_systems, systems):
    """
    Generates a list with a requested number of home systems
    choose the home systems from the specified list
    """

    # if the list of systems to choose home systems from is empty, raise an exception
    if not systems:
        err_msg = "Python generate_home_system_list: no systems to choose from"
        print err_msg
        raise Exception(err_msg)

    # initialize list of home systems
    home_systems = []

    # loop and get a new home systems until we have the requested number
    while len(home_systems) < num_home_systems:

        # try to choose a system until too many attempts failed or a system has been found
        attempts = 0
        found = False
        while (attempts < 100) and not found:
            attempts += 1
            # randomly choose one system from the list we got
            candidate = random.choice(systems)
            # for the first 50 attempts, only consider systems with "real" stars and at least one planet
            # if we haven't found a system after 50 attempts, consider all systems
            if attempts < 50:
                if fo.sys_get_star_type(candidate) not in starsystems.star_types_real:
                    continue
                if not fo.sys_get_planets(candidate):
                    continue
            # if our candidate is too close to the already choosen home systems, don't use it
            if is_too_close_to_other_home_systems(candidate, home_systems):
                continue
            # if our candidate passed the above tests, add it to our list
            home_systems.append(candidate)
            found = True

        # if no system could be found, just attempt to find one that's not
        # already a home system and disregard any other conditions
        if not found:
            print "Couldn't find homeworld #", len(home_systems) + 1, "after 100 attempts, just trying to find one now that's not already a home system and disregard any other condition"
            attempts = 0
            while (attempts < 50) and not found:
                attempts += 1
                # again, choose one system from the list we got
                candidate = random.choice(systems)
                # but now just check if it has already been choosen as home system
                if candidate in home_systems:
                    # if yes, try again
                    continue
                # if our candidate passed the test, add it to our list
                home_systems.append(candidate)
                found = True

        # if we still haven't found a suitable system, our galaxy obviously is too crowded
        # in that case, throw a fit, em, exception ;)
        if not found:
            raise Exception("Python generate_home_system_list: requested %d homeworlds in a galaxy with %d systems, aborting" % (num_home_systems, len(systems)))

        # if choosen system has no "real" star, change star type to a randomly selected "real" star
        if fo.sys_get_star_type(candidate) not in starsystems.star_types_real:
            star_type = random.choice(starsystems.star_types_real)
            print "Home system #", len(home_systems), "has star type", fo.sys_get_star_type(candidate), ", changing that to", star_type
            fo.sys_set_star_type(candidate, star_type)

        # if choosen system has no planets, create one in a random orbit
        # we take random values for type and size, as these will be
        # set to suitable values later
        if not fo.sys_get_planets(candidate):
            print "Home system #", len(home_systems), "has no planets, adding one"
            planet = planets.generate_planet(random.choice(planets.planet_sizes_real), random.choice(planets.planet_types_real),
                                             candidate, random.randint(0, fo.sys_get_num_orbits(candidate) - 1))
            if  planet == fo.invalid_object():
                # generate planet failed, throw an exception
                raise Exception("Python generate_home_system_list: couldn't create planet in home system")
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
        starting_species = random.choice(fo.get_playable_species())
    print "Starting species for player", player_name, "is", starting_species

    # pick a planet from the specified home system as homeworld
    planets = fo.sys_get_planets(home_system)
    # if the system is empty, throw an exception
    if not planets:
        raise Exception("Python setup_empire: got home system with no planets")
    homeworld = random.choice(planets)

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
            print "Player", player_name, ": starting species", starting_species, "has no preferred focus, using", available_foci[0], "instead"
        else:
            print "Player", player_name, ": preferred focus", preferred_focus, "for starting species", starting_species, "not available on homeworld, using", available_foci[0], "instead"
        fo.planet_set_focus(homeworld, available_foci[0])
    else:
        # if no focus is available on the homeworld, don't set any focus
        print "Player", player_name, ": no available foci on homeworld for starting species", starting_species

    # give homeworld starting buildings
    # use the list provided in starting_buildings.txt
    print "Player", player_name, ": add starting buildings to homeworld"
    for building in util.load_string_list("../starting_buildings.txt"):
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
        # if the fleet couldn't be created, throw an exception
        if fleet == fo.invalid_object():
            raise Exception("Python setup empire: couldn't create fleet " + fleet_plan.name())
        # second, iterate over the list of ship design names in the fleet plan
        for ship_design in fleet_plan.ship_designs():
            # create a ship in the fleet
            # if the ship couldn't be created, throw an exception
            if fo.create_ship("", ship_design, starting_species, fleet) == fo.invalid_object():
                raise Exception("Python setup empire: couldn't create ship " + ship_design + " for fleet " + fleet_plan.name())